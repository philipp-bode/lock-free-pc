#pragma once 

#include <vector>
#include <memory>
#include "concurrentqueue/blockingconcurrentqueue.h"

struct TestInstruction {
    int level;
    int X;
    int Y;
    std::shared_ptr<std::vector<int> > adj;
};

struct TestResult{
    int X;
    int Y;
    std::vector<int> S;
};

using TaskQueue = std::shared_ptr<moodycamel::BlockingConcurrentQueue<TestInstruction> >;
using ResultQueue = std::shared_ptr<moodycamel::BlockingConcurrentQueue<TestResult> >;