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

void VStructureWorker::_execute_search(int sep_idx, int node1, int node2, int Y, double &max_p) {
    std::vector<int> adj = _graph->getNeighboursWithout(node1, node2);

    int n = adj.size();
    auto powerset_size = static_cast<int>(pow(2, n));
    std::vector<int> subset;

    for (int i = 1; i < powerset_size; i++) {
        bool contains_Y = false;
        for (int j = 0; j < n; j++) {
            if ((i & (1 << j)) != 0) {
                if (adj[j] == Y)
                    contains_Y = true;
                subset.push_back(adj[j]);
            }
        }
        auto p = _alg->test(node1, node2, subset);
        if (p > max_p) {
            max_p = p;
            if (!contains_Y)
                update_result(sep_idx, subset, p);
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
        // std::vector<int> adjX = _graph->getNeighboursWithout(test.v_structure.X, test.v_structure.Z);
        // // std::vector<int> subset(1);
        // // subset[0] = test.v_structure.Y;

        // int n = adjX.size();
        // auto powerset_size = static_cast<int>(pow(2, n));
        // std::vector<int> subset;

        // // std::cout << "---------------" << std::endl;
        // // std::cout << test.v_structure.X << " -> " << test.v_structure.Y << " <- " << test.v_structure.Z << std::endl;
        // for (int i = 1; i < powerset_size; i++) {
        //     bool contains_Y = false;
        //     for (int j = 0; j < n; j++) {
        //         // Check if jth bit in the i is set. If the bit
        //         // is set, we consider jth element from set
        //         if ((i & (1 << j)) != 0) {
        //             if (adjX[j] == test.v_structure.Y)
        //                 contains_Y = true;
        //             subset.push_back(adjX[j]);
        //         }
        //     }
        //     auto p = _alg->test(test.v_structure.X, test.v_structure.Z, subset);
        //     if (p > max_p) {
        //         // for (auto &temp : subset) {
        //         //     std::cout << temp << ",";
        //         // }
        //         // std::cout << std::endl;
        //         // std::cout << "p: " << p << std::endl;
        //         max_p = p;
        //         if (!contains_Y)
        //             update_result(test.sep_idx, subset, p);
        //     }
        //     subset.clear();
        // }
        // std::cout << "---------------" << std::endl;
    }
}

// void VStructureWorker::test_single_conditional() {
//     TestInstruction test;
//     std::chrono::time_point<std::chrono::high_resolution_clock> start_loop, end_loop, start_time_test, end_time_test;

//     set_time(start_loop);
//     while (_work_queue->try_dequeue(test)) {
//         increment_stat(_statistics->dequed_elements);
//         std::vector<int> adjX = _graph->getNeighboursWithout(test.X, test.Y);
//         std::vector<int> sep(1);
//         bool separated = false;

//         for (auto const neighbour : adjX) {
//             sep[0] = neighbour;
//             set_time(start_time_test);
//             auto p = _alg->test(test.X, test.Y, sep);
//             set_time(end_time_test);
//             add_time_to(_statistics->sum_time_test, start_time_test, end_time_test);
//             increment_stat(_statistics->test_count);
//             if (p >= _alg->_alpha) {
//                 update_result(test.X, test.Y, sep);
//                 separated = true;
//                 break;
//             }
//         }

//         if (!separated) {
//             std::vector<int> adjY = _graph->getNeighboursWithout(test.Y, test.X);
//             for (auto const neighbour : adjY) {
//                 sep[0] = neighbour;
//                 set_time(start_time_test);
//                 auto p = _alg->test(test.X, test.Y, sep);
//                 set_time(end_time_test);
//                 add_time_to(_statistics->sum_time_test, start_time_test, end_time_test);
//                 increment_stat(_statistics->test_count);
//                 if (p >= _alg->_alpha) {
//                     update_result(test.X, test.Y, sep);
//                     break;
//                 }
//             }
//         }
//     }
//     set_time(end_loop);
//     add_time_to(_statistics->sum_time_queue_element, start_loop, end_loop);
// }

// void VStructureWorker::test_higher_order() {
//     TestInstruction test;
//     std::chrono::time_point<std::chrono::high_resolution_clock> start_loop, end_loop, start_time_test, end_time_test,
//         start_perm, end_perm;

//     set_time(start_loop);
//     while (_work_queue->try_dequeue(test)) {
//         increment_stat(_statistics->dequed_elements);
//         std::vector<int> adjX = _graph->getNeighboursWithout(test.X, test.Y);
//         bool separated = false;

//         size_t num_elementsX = adjX.size();
//         if (num_elementsX >= _level) {
//             std::vector<int> maskX(num_elementsX, 0);

//             for (int i = 0; i < _level; i++) {
//                 maskX[i] = 1;
//             }
//             std::next_permutation(maskX.begin(), maskX.end());

//             do {
//                 std::vector<int> subset(_level);
//                 int i = 0, j = 0;
//                 while (i < num_elementsX && j < _level) {
//                     if (maskX[i] == 1) {
//                         subset[j] = adjX.at(i);
//                         j++;
//                     }
//                     i++;
//                 }
//                 set_time(start_time_test);
//                 auto p = _alg->test(test.X, test.Y, subset);
//                 set_time(end_time_test);
//                 add_time_to(_statistics->sum_time_test, start_time_test, end_time_test);
//                 increment_stat(_statistics->test_count);
//                 if (p >= _alg->_alpha) {
//                     update_result(test.X, test.Y, subset);
//                     separated = true;
//                     break;
//                 }
//             } while (std::next_permutation(maskX.begin(), maskX.end()));
//         }

//         std::vector<int> adjY = _graph->getNeighboursWithout(test.Y, test.X);

//         size_t num_elements = adjY.size();
//         if (!separated && num_elements >= _level) {
//             std::vector<int> mask(num_elements, 0);

//             int last_equal_idx = 0;
//             for (auto node : adjX) {
//                 auto iter = find(adjY.begin(), adjY.end(), node);
//                 if (iter != adjY.end()) {
//                     iter_swap(adjY.begin() + last_equal_idx, iter);
//                     last_equal_idx++;
//                 }
//             }

//             for (int i = 0; i < _level; i++) {
//                 mask[i] = 1;
//             }
//             std::next_permutation(mask.begin(), mask.end());

//             do {
//                 std::vector<int> subset(_level);
//                 int i = 0, j = 0;
//                 int last_found;
//                 while (i < num_elements && j < _level) {
//                     if (mask[i] == 1) {
//                         subset[j] = adjY.at(i);
//                         last_found = i;
//                         j++;
//                     }
//                     i++;
//                 }
//                 if (last_found >= last_equal_idx) {
//                     set_time(start_time_test);
//                     auto p = _alg->test(test.X, test.Y, subset);
//                     set_time(end_time_test);
//                     add_time_to(_statistics->sum_time_test, start_time_test, end_time_test);
//                     increment_stat(_statistics->test_count);
//                     if (p >= _alg->_alpha) {
//                         update_result(test.X, test.Y, subset);
//                         break;
//                     }
//                 }
//             } while (std::next_permutation(mask.begin(), mask.end()));
//         }
//     }
//     set_time(end_loop);
//     add_time_to(_statistics->sum_time_queue_element, start_loop, end_loop);
// }

// void VStructureWorker::execute_test() {
// }
