#pragma once

#include <memory>
#include <vector>

#include "concurrency.hpp"

template <typename T>
class Watcher {
 public:
    Watcher(T t_queue, int max, std::vector<std::shared_ptr<Statistics> >& stats, std::shared_ptr<bool> stop_flag);

    void watch();

    void set_max(int new_max);

 protected:
    T _t_queue;
    int _max;
    std::vector<std::shared_ptr<Statistics> >& _stats;
    std::shared_ptr<bool> _stop_flag;
};
