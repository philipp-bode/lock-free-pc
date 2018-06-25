#include "worker.h"
#include "skeleton.h"

#include <set>
#include <algorithm>

#include "concurrentqueue/blockingconcurrentqueue.h"

Worker::Worker(TaskQueue t_queue, ResultQueue r_queue, shared_ptr<PCAlgorithm> alg) : _work_queue(t_queue), _result_queue(r_queue), _alg(alg) {}

void Worker::execute_test() {
    TestInstruction test;
    while(_work_queue->try_dequeue(test)) {

        size_t num_elementsX = test.adjX->size();
        std::vector<int> maskX (num_elementsX, 0);

        for (int i = 0; i < test.level; i++) {
            maskX[i] = 1;
        }
        std::next_permutation(maskX.begin(), maskX.end());

        do {
            std::vector<int> subset(test.level);
            int i = 0, j = 0;
            while (i < num_elementsX && j < test.level) {
                if (maskX[i] == 1) {
                    subset[j] = test.adjX->at(i);
                    j++;
                }
                i++;
            }
            auto p = _alg->test(test.X, test.Y, subset);
            if(p >= _alg->_alpha) {
                // TODO: shoudln't be a queue instead use a working copy of the adjacence matrix
                _result_queue->enqueue(TestResult{test.X, test.Y, subset});
                break;
            }
        } while (std::next_permutation(maskX.begin(), maskX.end()));

        if(size_t num_elements = test.adjY->size()) {
            std::vector<int> mask(num_elements, 0);

            int last_equal_idx = 0;
            for (auto node : *(test.adjX)) {
                auto iter = find(test.adjY->begin(), test.adjY->end(), node);
                if (iter != test.adjY->end()) {
                    iter_swap(test.adjY->begin() + last_equal_idx, iter);
                    last_equal_idx++;
                }
            }

            for (int i = 0; i < test.level; i++) {
                mask[i] = 1;
            }
            std::next_permutation(mask.begin(), mask.end());

            do {
                std::vector<int> subset(test.level);
                int i = 0, j = 0;
                int last_found;
                while (i < num_elements && j < test.level) {
                    if (mask[i] == 1) {
                        subset[j] = test.adjY->at(i);
                        last_found = j;
                        j++;
                    }
                    i++;
                }
                if (last_found > last_equal_idx) {
                    auto p = _alg->test(test.X, test.Y, subset);
                    if (p >= _alg->_alpha) {
                        // TODO: shoudln't be a queue instead use a working copy of the adjacence matrix
                        _result_queue->enqueue(TestResult{test.X, test.Y, subset});
                        break;
                    }
                }
            } while (std::next_permutation(mask.begin(), mask.end()));
        }

    }
    // needs a mutex to guarantee thread safety
    _done = true;
}

bool Worker::done() const {
    return _done;
}
