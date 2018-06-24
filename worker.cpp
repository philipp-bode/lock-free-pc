#include "worker.h"
#include "skeleton.h"

#include "concurrentqueue/blockingconcurrentqueue.h"

Worker::Worker(TaskQueue t_queue, ResultQueue r_queue, shared_ptr<PCAlgorithm> alg) : _work_queue(t_queue), _result_queue(r_queue), _alg(alg) {}

void Worker::execute() {
    while(true) {
        TestInstruction test;
        _work_queue->wait_dequeue(test);

        size_t num_elements = test.adj->size();
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
                    subset[j] = test.adj->at(i);
                    j++;
                }
                i++;
            }
            auto p = _alg->test(test.X, test.Y, subset);
            if(p >= _alg->_alpha) {
                _result_queue->enqueue(TestResult{test.X, test.Y, subset});
                break;
            }
        } while (std::next_permutation(mask.begin(), mask.end()));

    }
}
