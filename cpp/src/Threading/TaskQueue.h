#pragma once

#include "Task.h"
#include <queue>
#include <mutex>
#include <condition_variable>

class TaskQueue
{
public:
    void push(Task task);
    Task pop();

private:
    std::queue<Task> queue;
    std::mutex mutex;
    std::condition_variable condition;
};   