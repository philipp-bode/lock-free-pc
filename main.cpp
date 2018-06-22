
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
                if((int)adj.size()-1 >= level) {
                    // j is the index in the adj-Matrix for the currently tested neighbour -> adj[j] = Y
                    for(int j = 0; j < adj.size() && adj[j] < current_node; j++) {
                        vector<int> s(adj);
                        s.erase(s.begin() + j); // Y should not be in S for X ⊥ Y | S

                        process_edge(indepTest, level, edges_to_delete, current_node, adj[j], s);
                    }
                } else {
                    // if they have not enough neighbors for this level, they won't on the following
                    nodes_to_delete.push_back(current_node);
                    cout << "-";
                }
                cout << endl;
            }

            for(const auto edge : edges_to_delete) {
                _graph.deleteEdge(edge.first, edge.second);
            }
            for(const auto node: nodes_to_delete) {
                nodes_to_be_tested.erase(node);
            }
            level++;
            cout << "------------------------------------" << endl;
        }
    }

    void process_edge(IndepTestGauss &indepTest, int level, vector<pair<int, int>> &edges_to_delete, int x, int y,
                     const vector<int> &s) {
        cout << "Y: " << y << endl;
        cout << "  " << "S: ";
        print_vector(s);

        // iterates over all subsets of s with size of level
        for (vector<int> subset : getSubsets(s, level)) {
            cout << "  " <<  "Subset: ";
            print_vector(subset);

            auto p = indepTest.test(x, y, subset);
            if(p >= _alpha) {
                edges_to_delete.emplace_back(x, y);
                _seperation_sets.push_back({{x, y}, subset});
                cout << "  Node deleted" << endl;
                break;
            }
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

    std::vector<std::vector<int>> getSubsets(const std::vector<int> &S, int k) {
        size_t num_elements = S.size();
        std::vector<int> mask (num_elements, 0);
        std::vector<std::vector<int>> out;

        for (int i = 0; i < k; i++) {
            mask[i] = 1;
        }
        std::next_permutation(mask.begin(), mask.end());

        do {
            std::vector<int> tmp_vec (k);
            int i = 0, j = 0;
            while (i < num_elements && j < k) {
                if (mask[i] == 1) {
                    tmp_vec[j] = S[i];
                    j++;
                }
                i++;
            }
            out.push_back(tmp_vec);
        } while (std::next_permutation(mask.begin(), mask.end()));

        return out;
    }

    void print_vector(const vector<int> &S) const {
        for(auto s : S)
            cout << s << " ";
        cout << endl;
    }
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
