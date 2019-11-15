#include "watcher.hpp"

#include <stdio.h>
#include <unistd.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

Watcher::Watcher(TaskQueue t_queue, int max, std::vector<std::shared_ptr<Statistics> >& stats)
    : _t_queue(t_queue), _max(max), _stats(stats) {}

void Watcher::watch() {
    TestInstruction test;

    std::chrono::seconds sec(1);
    double secs = 0;
    if (isatty(fileno(stdout))) {
        while (_t_queue->try_dequeue(test)) {
            secs++;
            auto finished_elements = _max - static_cast<double>(_t_queue->size_approx());
            auto progress = 100.0 * (finished_elements / static_cast<double>(_max));
            auto speed = finished_elements / secs;
            std::cout << std::fixed << std::setprecision(2) << progress << " % at " << speed << " pairs/s \r";
            std::cout.flush();
            _t_queue->enqueue(test);

            std::this_thread::sleep_for(sec);
        }
        std::cout << std::endl;
    }
}

void Watcher::set_max(int new_max) { _max = new_max; }
