#include "worker.h"
#include "skeleton.h"

#include <set>
#include <algorithm>

#include "concurrentqueue/blockingconcurrentqueue.h"


Worker::Worker(
    TaskQueue t_queue,
    shared_ptr<PCAlgorithm> alg,
    int level,
    std::shared_ptr<Graph> graph,
    std::shared_ptr<Graph> working_graph,
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> sep_matrix,
    std::shared_ptr<Statistics> statistics
) :
    _work_queue(t_queue),
    _alg(alg),
    _level(level),
    _graph(graph),
    _working_graph(working_graph),
    _separation_matrix(sep_matrix),
    _statistics(statistics)
{}

void Worker::test_single_conditional() {
    TestInstruction test;

    while(_work_queue->try_dequeue(test)) {
        increment_stat(_statistics->dequed_elements)  // measure balance
        vector<int> adjX = _graph->getNeighboursWithout(test.Y, test.X);
        vector<int> sep(1);
        bool separated = false;

        for(auto const neighbour : adjX) {
            sep[0] = neighbour;
            auto p = _alg->test(test.X, test.Y, sep);
            increment_stat(_statistics->test_count)  // measure balance
            if(p >= _alg->_alpha) {
                update_result(test.X, test.Y, sep);
                separated = true;
                break;
            }
        }

        if (!separated) {
            vector<int> tmpAdjY = _graph->getNeighboursWithout(test.X, test.Y);
            vector<int> adjY;
            std::set_difference(tmpAdjY.begin(), tmpAdjY.end(),adjX.begin(), adjX.end(), std::inserter(adjY, adjY.begin()));
            for(auto const neighbour : adjY) {
                sep[0] = neighbour;
                auto p = _alg->test(test.X, test.Y, sep);
                increment_stat(_statistics->test_count)  // measure balance
                if(p >= _alg->_alpha) {
                    update_result(test.X, test.Y, sep);
                    break;
                }
            }
        }

    }
}

void Worker::test_higher_order() {
    TestInstruction test;
    while(_work_queue->try_dequeue(test)) {
        increment_stat(_statistics->dequed_elements) // measure balance
        
        if(test.X > test.Y) {
            auto tmp = test.X;
            test.X = test.Y;
            test.Y = tmp;
        }
        vector<int> adjX = _graph->getNeighboursWithout(test.X, test.Y);
        vector<int> adjY = _graph->getNeighboursWithout(test.Y, test.X);

        bool separated = false;

        size_t num_elementsX = adjX.size();
        if(num_elementsX >= _level) {
            std::vector<int> maskX (num_elementsX, 1);

            for (int i = 0; i < _level; i++) {
                maskX[i] = 0;
            }

            do {
                std::vector<int> subset(_level);
                int i = 0, j = 0;
                while (i < num_elementsX && j < _level) {
                    if (maskX[i] == 0) {
                        subset[j] = adjX.at(i);
                        j++;
                    }
                    i++;
                }
                auto p = _alg->test(test.X, test.Y, subset);
                increment_stat(_statistics->test_count) // measure balance
                if(p >= _alg->_alpha) {
                    update_result(test.X, test.Y, subset);
                    separated = true;
                    break;
                }
            } while (std::next_permutation(maskX.begin(), maskX.end()));
        }

        size_t num_elements = adjY.size();
        if(!separated && num_elements >= _level) {
            std::vector<int> mask(num_elements, 1);

            vector<int> adjCom;
            vector<int> neighbors;
            for (auto node : adjY) {
                auto iter = find(adjX.begin(), adjX.end(), node);
                if (iter != adjX.end()) {
                    adjCom.push_back(node);
                } else {
                    neighbors.push_back(node);
                }
            }
            int first_equal_idx = neighbors.size();

            for (auto node : adjCom){
                neighbors.push_back(node);
            }

            for (int i = 0; i < _level; i++) {
                mask[i] = 0;
            }

            do {
                std::vector<int> subset(_level);
                int i = 0, j = 0;
                int first_found = -1;
                while (i < num_elements && j < _level) {
                    if (mask[i] == 0) {
                        subset[j] = neighbors.at(i);
                        j++;
                        if (first_found == -1){
                            first_found = i;
                        }
                    }
                    i++;
                }
                if (first_found < first_equal_idx) {
                    auto p = _alg->test(test.X, test.Y, subset);
                    increment_stat(_statistics->test_count) // measure balance
                    if (p >= _alg->_alpha) {
                        update_result(test.X, test.Y, subset);
                        break;
                    }
                }
            } while (std::next_permutation(mask.begin(), mask.end()));
        }
    }

}

void Worker::execute_test() {
    if (_level == 1) {
        Worker::test_single_conditional();
    } else {
        Worker::test_higher_order();
    }
}

