#ifndef PARALLELPC_WORKER_H
#define PARALLELPC_WORKER_H


#include <memory>
#include <vector>

#include "concurrency.h"
#include "skeleton.h"



class Worker {
public:
    Worker(TaskQueue t_queue, ResultQueue r_queue, std::shared_ptr<PCAlgorithm> alg);

    void execute();

    bool done();

protected:
    TaskQueue _work_queue;
    ResultQueue _result_queue;
    std::shared_ptr<PCAlgorithm> _alg;
};

#endif //PARALLELPC_WORKER_H