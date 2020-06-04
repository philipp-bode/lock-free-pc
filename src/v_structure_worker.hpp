#pragma once

#include <memory>
#include <vector>

#include "concurrency.hpp"
#include "skeleton.hpp"

class VStructureWorker {
 public:
    VStructureWorker(
        VStructureQueue t_queue,
        std::shared_ptr<PCAlgorithm> alg,
        std::shared_ptr<Graph> graph,
        std::shared_ptr<std::vector<std::shared_ptr<VStructureResult>>> max_separations,
        std::shared_ptr<Statistics> statistics,
        std::shared_ptr<arma::mat> data);

    // Task to fetch test from _work_queue
    // and put the results the working graph and the separation matrix.
    void execute_test();
    void _execute_search(int sep_idx, int node1, int node2, int Y, double& max_p);

    // Write independence test results to the separation set store
    inline void update_result(int sep_idx, const std::vector<int>& subset, double p) {
        (*_max_separations)[sep_idx] = std::make_shared<VStructureResult>(VStructureResult{p, subset});
    }

 protected:
    VStructureQueue _work_queue;
    std::shared_ptr<PCAlgorithm> _alg;
    std::shared_ptr<Graph> _graph;
    std::shared_ptr<std::vector<std::shared_ptr<VStructureResult>>> _max_separations;
    std::shared_ptr<Statistics> _statistics;
    std::shared_ptr<arma::mat> _data;
};
