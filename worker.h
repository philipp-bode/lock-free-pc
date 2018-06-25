#ifndef PARALLELPC_WORKER_H
#define PARALLELPC_WORKER_H


#include <memory>
#include <vector>

#include "concurrency.h"
#include "skeleton.h"



class Worker {
public:
    Worker(TaskQueue t_queue, ResultQueue r_queue, std::shared_ptr<PCAlgorithm> alg);

    // Task to fetch test from _work_queue and put the results to _result_queue
    void execute_test();



    // could be possible we need this later for better synchronization 
    bool done() const;

protected:
    TaskQueue _work_queue;
    ResultQueue _result_queue;
    std::shared_ptr<PCAlgorithm> _alg;
    bool _done = false; // needs do 
};

#endif //PARALLELPC_WORKER_H