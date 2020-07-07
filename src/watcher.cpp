#include "watcher.hpp"

#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

template <typename T>
Watcher<T>::Watcher(
    T t_queue, int max, std::vector<std::shared_ptr<Statistics> >& stats, std::shared_ptr<bool> stop_flag)
    : _t_queue(t_queue), _max(max), _stats(stats), _stop_flag(stop_flag) {}

template <typename T>
void Watcher<T>::watch() {
    std::chrono::seconds sec(1);
    double secs = 0;
    if (isatty(fileno(stdout))) {
        while (!(*_stop_flag)) {
            secs++;
            auto finished_elements = _max - static_cast<double>(_t_queue->size_approx());
            auto progress = 100.0 * (finished_elements / static_cast<double>(_max));
            auto speed = finished_elements / secs;
            uint64_t finished_tests = 0;
            for (auto stat : _stats) {
                finished_tests += stat->test_count;
            }
            std::cout << std::fixed << std::setprecision(2) << progress << " % at " << speed
                      << " pairs/s. Finished tests: " << finished_tests << "\r";
            std::cout.flush();

            std::this_thread::sleep_for(sec);
        }
        std::cout << std::endl;
    }
}

template <typename T>
void Watcher<T>::set_max(int new_max) {
    _max = new_max;
}

template class Watcher<TaskQueue>;
template class Watcher<VStructureQueue>;
