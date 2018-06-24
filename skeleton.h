#ifndef PARALLELPC_SKELETON_H
#define PARALLELPC_SKELETON_H


#define rep(a, b)   for(int a = 0; a < (b); ++a)

#include "graph.hpp"
#include "threadsafe_queue.cpp"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>

#include <armadillo>

#include "concurrency.h"
#include "condition.h"


using namespace std;

class PCAlgorithm {

public:
    PCAlgorithm(int vars, double alpha, int samples, int numberThreads);
    double _alpha;
    void build_graph();

    void print_graph() const;

    void build_correlation_matrix(std::vector<std::vector<double>> &data);

    inline double test(int u, int v, std::vector<int> &S) const {
        return _gauss_test.test(u,v,S);
    }

protected:

    TaskQueue _work_queue;
    ResultQueue _result_queue;
    IndepTestGauss _gauss_test;
    const int STRIDE = 1;
    int _nr_variables;
    int _nr_samples;
    int _nr_threads;
    arma::Mat<double>_correlation;
    Graph _graph;
    vector<TestResult> _seperation_sets;

    void print_vector(const vector<int> &S) const {
        for(auto s : S)
            cout << s << " ";
        cout << endl;
    }
};

#endif //PARALLELPC_SKELETON_H
