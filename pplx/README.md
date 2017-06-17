# pplx
Parallel Pattern Library that comes with **cpprestsdk**.

### Requirements

* [cpprestsdk](https://github.com/Microsoft/cpprestsdk)

### pplx::task
* Asynchronous task `task.wait()` wait the result, and `task.get() return the result. The result type is templated in its class `pplx::task<_ReturnType>::task()`
* It is possible chain task by `task.then([](){..}).then(other_lambda).get()`

