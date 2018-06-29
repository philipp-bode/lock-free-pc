#ifndef PARALLELPC_WATCHER_H
#define PARALLELPC_WATCHER_H


#include <memory>
#include <vector>

#include "concurrency.h"
#include "skeleton.h"



class Watcher {
public:
    Watcher(
        std::vector<int> *stats,
        int max
    );

    void watch();

    void set_max(int new_max);

    // could be possible we need this later for better synchronization 
    bool done() const;

protected:
    std::vector<int> *_stats;
    int _max;
    bool _done = false; // needs do 
};

#endif //PARALLELPC_WATCHER_H