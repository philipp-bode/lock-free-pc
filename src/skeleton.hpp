#pragma once

#define rep(a, b) for (int a = 0; a < (b); ++a)

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "armadillo"
#include "boost/multi_array.hpp"
#include "gsl/gsl_statistics.h"
#include "gsl/gsl_vector.h"

#include "concurrency.hpp"
#include "constraint.hpp"
#include "graph.hpp"

class PCAlgorithm : public std::enable_shared_from_this<PCAlgorithm> {
 public:
    PCAlgorithm(int vars, double alpha, int samples, int numberThreads);
    double _alpha;
    void build_graph();

    void print_graph() const;

    std::vector<int> get_edges() const;

    int getNumberOfVariables();

    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> get_separation_matrix();

    void build_correlation_matrix(std::vector<std::vector<double>>& data);
    void build_correlation_matrix(arma::Mat<double>& data);
    inline double test(int u, int v, std::vector<int>& S) const { return _gauss_test.test(u, v, S); }

    void persist_result(const std::string data_name, const std::vector<std::string>& column_names);

 protected:
    TaskQueue _work_queue;
    IndepTestGauss _gauss_test;
    const int STRIDE = 1;
    int _nr_variables;
    int _nr_samples;
    int _nr_threads;
    arma::Mat<double> _correlation;
    std::shared_ptr<Graph> _graph;
    std::shared_ptr<Graph> _working_graph;
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> _separation_matrix;

    void print_vector(const std::vector<int>& S) const {
        for (auto s : S) std::cout << s << " ";
        std::cout << std::endl;
    }
};
