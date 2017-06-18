#include <chrono> 

#include "../include/TaskGroup.h"
#include "../include/utils.h"


TaskGroup::TaskGroup():
m_succeeded(0) {
    LOGGER << "constructor";
}

TaskGroup::~TaskGroup() {
    LOGGER << "destructor";
    terminate();
}

void TaskGroup::pushTask(pplx::task<void>&& task) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_tasks.push_back(task.then([this] () {
        LOGGER << "task terminated";
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            this->m_succeeded++;
        }
        this->m_cv.notify_one();
    }));
}

bool TaskGroup::wait(int millis) {
    LOGGER << "waiting for " << millis << " ms";
    std::unique_lock<std::mutex> lk(m_mutex);
    auto all_tasks = pplx::when_all(begin(m_tasks), end(m_tasks));
    m_cv.wait_for(lk, std::chrono::milliseconds(millis), [&all_tasks]() {
        LOGGER << "is done? " << all_tasks.is_done();
        return all_tasks.is_done();
    });
    // to remove
    LOGGER << "waiting result: " << all_tasks.is_done() <<
    ", succeeded: " << m_succeeded <<
    ", total: " << m_tasks.size();
    return all_tasks.is_done();
}

void TaskGroup::terminate() {
    LOGGER << "terminating";
    std::lock_guard<std::mutex> lk(m_mutex);
    m_cts.cancel();
    auto all_tasks = pplx::when_all(begin(m_tasks), end(m_tasks));
    all_tasks.wait();
    m_tasks.clear();
    m_succeeded = 0;
    LOGGER << "terminated";
}

void TaskGroup::getStatus(int &succeeded, int &total) {
    std::lock_guard<std::mutex> lk(m_mutex);
    succeeded = m_succeeded;
    total = (int)m_tasks.size();
}

const pplx::cancellation_token_source& TaskGroup::getCts() {
    return m_cts;
}