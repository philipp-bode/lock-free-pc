#pragma once

#define rep(a, b) for (int a = 0; a < (b); ++a)

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
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
    enum class Correlation { PEARSON, SPEARMAN };

    PCAlgorithm(std::shared_ptr<arma::mat> data, double alpha, int numberThreads, std::string test_name = "pearson");
    double _alpha;
    void build_graph();
    void test_v_structures();
    void print_graph() const;

    std::vector<int> get_edges() const;
    std::vector<double> get_edges_with_weight() const;

    int getNumberOfVariables();

    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> get_separation_matrix();

    inline double test(int u, int v, std::vector<int>& S) const { return _gauss_test.test(u, v, S); }

    void persist_result(const std::string data_name, const std::vector<std::string>& column_names);
    std::shared_ptr<arma::mat> _correlation;

 protected:
    std::unordered_map<std::string, PCAlgorithm::Correlation> const _test_names = {
        {"pearson", PCAlgorithm::Correlation::PEARSON}, {"spearman", PCAlgorithm::Correlation::SPEARMAN}};

    TaskQueue _work_queue;
    IndepTestGauss _gauss_test;
    const int STRIDE = 1;
    int _nr_variables;
    int _nr_samples;
    int _nr_threads;
    PCAlgorithm::Correlation _correlation_type;
    std::shared_ptr<arma::mat> _data;
    std::shared_ptr<Graph> _graph;
    std::shared_ptr<Graph> _working_graph;
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> _separation_matrix;
    std::string _test_name;
    void print_vector(const std::vector<int>& S) const {
        for (auto s : S) std::cout << s << " ";
        std::cout << std::endl;
    }
};
