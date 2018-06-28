#pragma once 

#include <vector>
#include <memory>
#include "concurrentqueue/concurrentqueue.h"

struct TestInstruction {
    int X;
    int Y;
};

struct TestResult{
    int X;
    int Y;
    std::vector<int> S;
};

using TaskQueue = std::shared_ptr<moodycamel::ConcurrentQueue<TestInstruction> >;
using ResultQueue = std::shared_ptr<moodycamel::ConcurrentQueue<TestResult> >;