#include "v_structure_worker.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "skeleton.hpp"

VStructureWorker::VStructureWorker(
    VStructureQueue t_queue,
    std::shared_ptr<PCAlgorithm> alg,
    std::shared_ptr<Graph> graph,
    std::shared_ptr<std::vector<std::shared_ptr<VStructureResult>>> max_separations,
    std::shared_ptr<Statistics> statistics,
    std::shared_ptr<arma::mat> data)
    : _work_queue(t_queue),
      _alg(alg),
      _graph(graph),
      _max_separations(max_separations),
      _statistics(statistics),
      _data(data) {}

void VStructureWorker::_execute_search(int sep_idx, int node1, int node2, int Y, double& max_p) {
    std::vector<int> adj = _graph->getNeighboursWithout(node1, node2);

    int n = adj.size();
    auto powerset_size = static_cast<int>(pow(2, n));
    std::vector<int> subset;

    for (int i = 1; i < powerset_size; i++) {
        bool contains_Y = false;
        for (int j = 0; j < n; j++) {
            if ((i & (1 << j)) != 0) {
                if (adj[j] == Y) contains_Y = true;
                subset.push_back(adj[j]);
            }
        }
        auto p = _alg->test(node1, node2, subset);
        if (p > max_p) {
            max_p = p;
            if (!contains_Y) update_result(sep_idx, subset, p);
        }
        subset.clear();
    }
}

void VStructureWorker::execute_test() {
    EdgeOrientationTaskInstruction test;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_loop, end_loop, start_time_test, end_time_test,
        start_perm, end_perm;

    set_time(start_loop);
    while (_work_queue->try_dequeue(test)) {
        increment_stat(_statistics->dequed_elements);
        double max_p = 0;
        _execute_search(test.sep_idx, test.v_structure.X, test.v_structure.Z, test.v_structure.Y, max_p);
        _execute_search(test.sep_idx, test.v_structure.Z, test.v_structure.X, test.v_structure.Y, max_p);
    }
}
