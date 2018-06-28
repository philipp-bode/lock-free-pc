#ifndef PARALLELPC_WORKER_H
#define PARALLELPC_WORKER_H


#include <memory>
#include <vector>

#include "concurrency.h"
#include "skeleton.h"



class Worker {
public:
    Worker(
        TaskQueue t_queue,
        std::shared_ptr<PCAlgorithm> alg,
        int level,
        std::shared_ptr<Graph> graph,
        std::shared_ptr<Graph> working_graph,
        std::shared_ptr<std::vector<std::vector<int>*>> sep_matrix,
        int *test_count
    );

    // Task to fetch test from _work_queue and put the results to _result_queue
    void execute_test();

    // Write independence test results to the graph and seperation set store 
    void update_result(int x, int y, const std::vector<int> &subset);
    void test_single_conditional();
    void test_higher_order();

    // could be possible we need this later for better synchronization 
    bool done() const;

protected:
    TaskQueue _work_queue;
    std::shared_ptr<PCAlgorithm> _alg;
    int _level;
    std::shared_ptr<Graph> _graph;
    std::shared_ptr<Graph> _working_graph;
    std::shared_ptr<std::vector<std::vector<int>*>> _seperation_matrix;
    int *_test_count;
    bool _done = false; // needs do 
};

#endif //PARALLELPC_WORKER_H