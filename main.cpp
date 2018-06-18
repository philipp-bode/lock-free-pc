
#include "graph.hpp"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>

#include <armadillo>


#include "condition.h"

#include <boost/math/special_functions/log1p.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/dynamic_bitset.hpp>


#define rep(a, b)   for(int a = 0; a < (b); ++a)
#define debug(x)    clog << #x << " = " << x << endl;
#define all(a)      (a).begin(),(a).end()
#define endl '\n'

using namespace std;

class PCAlgorithm {

public:

    void build_graph() {
        _correlation.print(cout);
        IndepTestGauss indepTest(_nr_variables,_correlation);

        int level = 1;
        std::unordered_set<int> nodes_to_be_tested;
        for (int i = 0; i < _nr_variables; ++i) nodes_to_be_tested.insert(nodes_to_be_tested.end(), i);

        // we want to run as long as their are edges remaining to test on a higher level
        while(!nodes_to_be_tested.empty()) {
            cout << "Level: " << level << endl;
            std::vector<pair<int,int> > edges_to_delete(0);
            std::vector<int> nodes_to_delete(0);
            // iterate over all edges to determine if they still can be tested on this level
            for (int current_node : nodes_to_be_tested) {
                cout << "Current node (X): " << current_node << endl;
                auto adj = _graph.getNeighbours(current_node);
                // only do the independence testing if the current_node has enough neighbours do create a separation set
                if(adj.size()-1 >= level) {
                    // j is the index in the adj-Matrix for the currently tested neighbour -> adj[j] = Y
                    for(int j = 0; j < adj.size() && adj[j] < current_node; j++) {
                        cout << "j: " << j << ", Y: " << adj[j] << endl;
                        std::vector<int> S(adj);
                        S.erase(S.begin() + j); // Y should not be in S for X ⊥ Y | S

                        cout << "  " << "S(" << S.size() << "): ";
                        for(auto s : S)
                            cout << s << ", ";
                        cout << endl;

                        // generating subsets by iteratively combining one element with l-1 following elements until
                        // this element was combined with all following elements
                        for(int u = 0; u + level <= S.size(); u++) {
                            for(int v = level; v < S.size(); v++) {
                                std::vector<int> subset(S.begin() + v - level + 1, S.begin() + v); //evaluates to an empty subset on first level because the iterators are equivalent
                                subset.push_back(S[u]);// TODO: may lead to duplicates in set on higher levels

                                cout << "  " <<  "Subset(" << subset.size() << "): ";
                                for(auto sub : subset)
                                    cout << sub << ", ";
                                cout << endl;

                                auto p = indepTest.test(current_node,adj[j],subset);
                                if((1-p) < _alpha) {
                                    edges_to_delete.emplace_back(current_node,adj[j]);
                                    _seperation_sets.push_back({{current_node,adj[j]}, subset});
                                    cout << "Node deleted" << endl;
                                    goto endloop;
                                }

                                //TODO: only workaround for now
                                if(level == 1) {
                                    break;
                                }
                            }
                        }
                        endloop:
                        ;
                    }
                } else {
                    // if they have not enough neighbors for this level, they won't on the following
                    nodes_to_delete.push_back(current_node);
                    cout << "-"<< endl;
                }
            }

            for(const auto edge : edges_to_delete) {
                _graph.deleteEdge(edge.first, edge.second);
            }
            for(const auto node: nodes_to_delete) {
                nodes_to_be_tested.erase(node);
            }
            level++;
            cout << endl;
        }
    }

    void print_graph() {
        _graph.print();
    }

    void build_correlation_matrix(std::vector<std::vector<double>> &data) {
        int n = data[0].size();
        rep(i, _nr_variables) {
            rep(j, i) {
                gsl_vector_const_view gsl_x = gsl_vector_const_view_array( &data[i][0], n);
                gsl_vector_const_view gsl_y = gsl_vector_const_view_array( &data[j][0], n);
                double pearson = gsl_stats_correlation(
                    gsl_x.vector.data, STRIDE,
                    gsl_y.vector.data, STRIDE,
                    n
                );
                _correlation(i,j) = _correlation(j,i) = pearson;
                if(pearson < _alpha) {
                    _graph.deleteEdge(i,j);
                }
            }
        }
    }

    PCAlgorithm(int vars, double alpha): _graph(vars), _alpha(alpha), _nr_variables(vars) {
        _correlation = arma::Mat<double>(vars, vars, arma::fill::eye);
    }

protected:
    const int STRIDE = 1;
    int _nr_variables;
    arma::Mat<double>_correlation;
    Graph _graph;
    double _alpha;
    vector<pair<pair<int, int>, vector<int> > > _seperation_sets;

};

std::vector<std::vector<double>> read_data() {
    // std::freopen("Scerevisiae.csv", "r", stdin);
    std::freopen("../cooling_house.data", "r", stdin);
    int variables, observations;
    double next_val;


    cin >> variables >> observations;
    std::vector<std::vector<double>> data(variables, std::vector<double>(observations));

    rep(o, observations) {
        rep(v, variables) {
            cin >> next_val;
            data[v][o] = next_val;
        }
    }

    return data;

}


int main(int argc, char* argv[])
{
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.precision(10);

    // kann man überlegen, ob man das nicht auch in die Klasse mit rein zieht
    auto data = read_data();

    PCAlgorithm alg(data.size(), 0.1);

    alg.build_correlation_matrix(data);

    alg.build_graph();

    alg.print_graph();

    // std::vector<uint> sep1 = {5};
    // std::vector<uint> sep2 = {3};
    // std::vector<uint> sep3 = {5};

    // IndepTestGauss indepTest(6,alg._correlation);
    // cout << indepTest.test(3,4, {5}) << endl;
    // cout << indepTest.test(4,5, sep2) << endl;
    // cout << indepTest.test(0,1, sep3) << endl;



    cout.flush();
    return 0;
}
