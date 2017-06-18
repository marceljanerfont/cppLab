# TaskGroup
Class to handle a group of asynchronous tasks, which can be canceled and wait completion with timeout. It uses the Parallel Pattern Library that comes with **cpprestsdk**.

### Requirements

* [cpprestsdk](https://github.com/Microsoft/cpprestsdk) 

### Example

```c++
#include <iostream>
#include <chrono>
#include <thread>
#include "../include/TaskGroup.h"

pplx::task<void> my_task(int millis, const pplx::cancellation_token_source& cts) {
    return pplx::create_task([millis, cts]() {
        int j = 0;
        int sleep_slice = 500;
        int iters = millis/sleep_slice;
        while (++j < iters) {
            LOGGER << " * ";
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_slice));
            // cheack if it is canceled
            if (cts.get_token().is_canceled()) {
                pplx::cancel_current_task();
            }
        }
    });
}

int main(int argc, char **argv) {
    TaskGroup group;
    int NUM_TASKS = 5;
    for (int i = 0; i < NUM_TASKS; ++i) {
        group.pushTask(my_task(500, group.getCts()));
    }
    group.wait(1000);
}
```



