#pragma once 

#include <vector>
#include <memory>
#include "concurrentqueue/concurrentqueue.h"

#if WITH_STATS
#define increment_stat(x) x += 1;
#else
#define increment_stat(x)
#endif

struct TestInstruction {
    int X;
    int Y;
};

struct TestResult{
    int X;
    int Y;
    std::vector<int> S;
};

struct Statistics{
    int test_count = 0;
    int dequed_elements = 0;
    int deleted_edges = 0;
};

using TaskQueue = std::shared_ptr<moodycamel::ConcurrentQueue<TestInstruction> >;
using ResultQueue = std::shared_ptr<moodycamel::ConcurrentQueue<TestResult> >;