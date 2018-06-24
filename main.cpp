
#include "graph.hpp"
#include "threadsafe_queue.cpp"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <thread>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>

#include <armadillo>


#include "condition.h"
#include "concurrentqueue/blockingconcurrentqueue.h"

#include <boost/math/special_functions/log1p.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/dynamic_bitset.hpp>


#define rep(a, b)   for(int a = 0; a < (b); ++a)
#define debug(x)    clog << #x << " = " << x << endl;
#define all(a)      (a).begin(),(a).end()
// #define endl '\n'

using namespace std;

struct TestInstruction {
    int level;
    int X;
    int Y;
    vector<int> adj;

    TestInstruction() : level(0), X(0), Y(0), adj({})
    {}

    TestInstruction(int l, int x, int y, vector<int> a) : level(l), X(x), Y(y), adj(a)
    {}
};

struct TestResult{
    int X;
    int Y;
    vector<int> S;

    TestResult() : X(0), Y(0), S({})
    {}

    TestResult(int x, int y, vector<int> s) : X(x), Y(y), S(s)
    {}
};

class PCAlgorithm {

public:
    moodycamel::BlockingConcurrentQueue<TestInstruction> _work_queue;
    moodycamel::BlockingConcurrentQueue<TestResult> _result_queue;
    IndepTestGauss *_gauss_test;
    double _alpha;

    void build_graph() {
        // _correlation.print(cout);
        _gauss_test = new IndepTestGauss(_nr_samples,_correlation);

        int level = 1;
        std::unordered_set<int> nodes_to_be_tested;
        for (int i = 0; i < _nr_variables; ++i) nodes_to_be_tested.insert(nodes_to_be_tested.end(), i);

        // we want to run as long as their are edges remaining to test on a higher level
        while(!nodes_to_be_tested.empty()) {
            std::vector<int> nodes_to_delete(0);
            // iterate over all edges to determine if they still can be tested on this level
            for (int current_node : nodes_to_be_tested) {
                auto adj = _graph.getNeighbours(current_node);
                // only do the independence testing if the current_node has enough neighbours do create a separation set
                if((int)adj.size()-1 >= level) {
                    // j is the index in the adj-Matrix for the currently tested neighbour -> adj[j] = Y
                    for(int j = 0; j < adj.size(); j++) {
                        vector<int> s(adj);
                        s.erase(s.begin() + j); // Y should not be in S for X ⊥ Y | S
                        _work_queue.enqueue(TestInstruction(level, current_node, adj[j], s));
                    }
                } else {
                    // if they have not enough neighbors for this level, they won't on the following
                    nodes_to_delete.push_back(current_node);
                }
                cout << endl;
            }
            cout << "Queued all tests, waiting for results.." << endl;

            TestResult result;
            _result_queue.wait_dequeue(result);
            cout << "Received first result.." << endl;

            while(true) {
                _graph.deleteEdge(result.X, result.Y);
                _seperation_sets.push_back(result);
                bool found = _result_queue.try_dequeue(result);
                if(!found) {
                    if (!_work_queue.size_approx()) {
                        cout << "No more tests in _work_queue.." << endl;
                        break;
                    } else {
                        _result_queue.wait_dequeue(result);
                    }
                }
            }

            for(const auto node: nodes_to_delete) {
                nodes_to_be_tested.erase(node);
            }
            print_graph();
            level++;
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

    PCAlgorithm(int vars, double alpha, int samples): _graph(vars), _alpha(alpha), _nr_variables(vars), _nr_samples(samples) {
        _correlation = arma::Mat<double>(vars, vars, arma::fill::eye);
    }

protected:
    const int STRIDE = 1;
    int _nr_variables;
    int _nr_samples;
    arma::Mat<double>_correlation;
    Graph _graph;
    vector<TestResult> _seperation_sets;

    void print_vector(const vector<int> &S) const {
        for(auto s : S)
            cout << s << " ";
        cout << endl;
    }
};

void worker(PCAlgorithm &alg) {
    while(true) {
        TestInstruction test;
        alg._work_queue.wait_dequeue(test);

        size_t num_elements = test.adj.size();
        std::vector<int> mask (num_elements, 0);

        for (int i = 0; i < test.level; i++) {
            mask[i] = 1;
        }
        std::next_permutation(mask.begin(), mask.end());

        do {
            std::vector<int> subset(test.level);
            int i = 0, j = 0;
            while (i < num_elements && j < test.level) {
                if (mask[i] == 1) {
                    subset[j] = test.adj[i];
                    j++;
                }
                i++;
            }
            auto p = alg._gauss_test->test(test.X, test.Y, subset);
            if(p >= alg._alpha) {
                alg._result_queue.enqueue(TestResult(test.X, test.Y, subset));
                break;
            }
        } while (std::next_permutation(mask.begin(), mask.end()));

    }
}

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

    PCAlgorithm alg(data.size(), 0.1, data[0].size());

    alg.build_correlation_matrix(data);

    std::thread t1(worker, std::ref(alg));
    std::thread t2(worker, std::ref(alg));

    alg.build_graph();

    alg.print_graph();


    // std::vector<uint> sep1 = {5};
    // std::vector<uint> sep2 = {3};
    // std::vector<uint> sep3 = {5};

    // IndepTestGauss indepTest(6,alg._correlation);
    // cout << indepTest.test(3,4, {5}) << endl;
    // cout << indepTest.test(4,5, sep2) << endl;
    // cout << indepTest.test(0,1, sep3) << endl;

    // alg._work_queue.enqueue(TestInstruction(1,56,57, vector<int>{1,2,3,4}));
    // TestInstruction test;
    // cout << "Aprrox. Size: " << alg._work_queue.size_approx() << endl;
    // bool found = alg._work_queue.try_dequeue(test);
    // cout << "Found: " << found << endl;
    // cout << test.X << '|' << test.Y << endl;
    // cout << "Aprrox. Size: " << alg._work_queue.size_approx() << endl;

    cout.flush();
    return 0;
}
