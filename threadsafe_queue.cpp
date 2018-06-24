// Based on https://stackoverflow.com/a/16075550
// with added emplace-wrapper.

#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
template <class T>
class SafeQueue {
public:
    SafeQueue(void)
        : q()
        , m()
        , c()
    {}

    ~SafeQueue(void){}

    // Add an element to the queue.
    void enqueue(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.push_front(t);
        c.notify_one();
    }

    // Wrap std::queue::emplace.
    template <class... Args>
    void emplace(Args&&... args) {
        std::lock_guard<std::mutex> lock(m);
        q.emplace_front(std::forward<Args>(args)...);
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void) {
        std::unique_lock<std::mutex> lock(m);
        while(q.empty()) {
            // release lock as long as the wait and reaquire it afterwards.
            c.wait(lock);
        }

        T val = q.back();
        q.pop_back();
        return val;
    }

    private:
        std::deque<T> q;
        mutable std::mutex m;
        std::condition_variable c;
};
#endif
