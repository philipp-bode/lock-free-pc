#pragma once

#include <memory>
#include <vector>

#include "concurrency.hpp"



class Watcher {
public:
    Watcher(
        TaskQueue t_queue,
        int max,
        std::vector<std::shared_ptr<Statistics> >& stats
    );

    void watch();

    void set_max(int new_max);

protected:
    TaskQueue _t_queue;
    int _max;
    std::vector<std::shared_ptr<Statistics> >& _stats;
};
