#pragma once

#include "MPSCQueue.h"

template<class T>
class LFQueue
{
public:
    LFQueue() { }
    ~LFQueue() { }

    LFQueue(LFQueue const&) = delete;
    LFQueue& operator=(LFQueue const&) = delete;

    void push(T* data)
    {
        _queue.Enqueue(data);
    }

    T* pop()
    {
        T* data = NULL;
        _queue.Dequeue(data);

        return data;
    }

private:
    MPSCQueue<T> _queue;
};
