#pragma once

#include <memory>
#include <vector>

#include "concurrentqueue/concurrentqueue.h"

#include "graph.hpp"

#if WITH_STATS
#define increment_stat(x) x += 1;
#define set_time(x) x = std::chrono::high_resolution_clock::now();
#define add_time_to(x, start, stop) \
    x += std::chrono::duration_cast<std::chrono::duration<double>>(stop - start).count();
#else
#define increment_stat(x)
#define set_time(x)
#define add_time_to(x, start, stop)
#endif

struct TestInstruction {
    int X;
    int Y;
};

struct EdgeOrientationTaskInstruction {
    int sep_idx;
    VStructure v_structure;
};

struct VStructureResult {
    double p;
    std::vector<int> sepset;
};

struct Statistics {
    int test_count = 0;
    int dequed_elements = 0;
    int deleted_edges = 0;
    double sum_time_test = 0.0;
    double sum_time_queue_element = 0.0;
};

using TaskQueue = std::shared_ptr<moodycamel::ConcurrentQueue<TestInstruction>>;
using VStructureQueue = std::shared_ptr<moodycamel::ConcurrentQueue<EdgeOrientationTaskInstruction>>;
