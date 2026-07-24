#include "TaskQueue.h"

void TaskQueue::push(Task task)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(task);
    }
    condition.notify_one();
}

Task TaskQueue::pop()
{
    std::unique_lock<std::mutex> lock(mutex);

    condition.wait(lock, [this]()
    {
        return !queue.empty();
    });

    Task task = queue.front();
    queue.pop();

    return task;
}   