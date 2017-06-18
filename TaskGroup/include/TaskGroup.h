#ifndef __TASK_GROUP_H__
#define __TASK_GROUP_H__

#include <vector>
#include <thread>
#include <condition_variable>

#include "pplx/pplxtasks.h"

// An implementation of a group of pplx::task
// with no return value: pplx::task<void>
// in case we want specialized return type value, 
// then template it as template<_ReturnType=void>
class TaskGroup {
public:
    TaskGroup();
    ~TaskGroup();

    void pushTask(pplx::task<void>&& task);

    // return true if all tasks have ended
    bool wait(int millis);
    void terminate();

    // getters
    void getStatus(int &succeeded, int &total);
    const pplx::cancellation_token_source& getCts();

private:
    pplx::cancellation_token_source m_cts;
    std::vector<pplx::task<void>>   m_tasks;
    std::mutex                      m_mutex;
    std::condition_variable         m_cv;
    int                             m_succeeded;
    
};
#endif // __TASK_GROUP_H__