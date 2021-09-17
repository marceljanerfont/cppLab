#ifndef MEMORY_UTILS_
#define MEMORY_UTILS_

#include <algorithm>

// memory utilities
namespace mu {
/** @brief  Automatically Allocated Buffer Class

The class is used for temporary buffers in functions and methods.
If a temporary buffer is usually small (a few K's of memory),
but its size depends on the parameters, it makes sense to create a small
fixed-size array on stack and use it if it's large enough. If the required buffer size
is larger than the fixed size, another buffer of sufficient size is allocated dynamically
and released after the processing. Therefore, in typical cases, when the buffer size is small,
there is no overhead associated with malloc()/free().
At the same time, there is no limit on the size of processed data.

This is what AutoBuffer does. The template takes 2 parameters - type of the buffer elements and
the number of stack-allocated elements.
*/
template < typename _Tp, size_t fixed_size = 1024 / sizeof(_Tp) + 8 > class AutoBuffer {
 public:
  typedef _Tp value_type;
  //! the default constructor
  AutoBuffer();
  //! constructor taking the real buffer size
  explicit AutoBuffer(size_t _size);

  //! the copy constructor
  AutoBuffer(const AutoBuffer<_Tp, fixed_size> &buf);
  //! the assignment operator
  AutoBuffer<_Tp, fixed_size> &operator = (const AutoBuffer<_Tp, fixed_size> &buf);

  //! destructor. calls deallocate()
  ~AutoBuffer();

  //! allocates the new buffer of size _size. if the _size is small enough, stack-allocated buffer is used
  void allocate(size_t _size);
  //! deallocates the buffer if it was dynamically allocated
  void deallocate();
  //! resizes the buffer and preserves the content
  void resize(size_t _size);
  //! returns the current buffer size
  size_t size() const;
  //! if it is detached, no deallocation will be called from the destructor
  inline void detach() {
    is_detached = true;
  }
  //! returns pointer to the real buffer, stack-allocated or heap-allocated
  inline _Tp *data() {
    return ptr;
  }
  //! returns read-only pointer to the real buffer, stack-allocated or heap-allocated
  inline const _Tp *data() const {
    return ptr;
  }

  //! returns pointer to the real buffer, stack-allocated or heap-allocated
  operator _Tp *() {
    return ptr;
  }
  //! returns read-only pointer to the real buffer, stack-allocated or heap-allocated
  operator const _Tp *() const {
    return ptr;
  }

 protected:
  //! pointer to the real buffer, can point to buf if the buffer is small enough
  _Tp *ptr;
  //! size of the real buffer
  size_t sz;
  //! pre-allocated buffer. At least 1 element to confirm C++ standard requirements
  _Tp buf[(fixed_size > 0) ? fixed_size : 1];
  //! if it is detached, no deallocation will be called from the destructor
  bool is_detached;
};

/////////////////////////////// AutoBuffer implementation ////////////////////////////////////////

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer(): is_detached(false) {
  ptr = buf;
  sz = fixed_size;
}

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer(size_t _size): is_detached(false) {
  ptr = buf;
  sz = fixed_size;
  allocate(_size);
}

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::AutoBuffer(const AutoBuffer<_Tp, fixed_size> &abuf): is_detached(false) {
  ptr = buf;
  sz = fixed_size;
  allocate(abuf.size());
  for (size_t i = 0; i < sz; i++)
    ptr[i] = abuf.ptr[i];
}

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size> &AutoBuffer<_Tp, fixed_size>::operator = (const AutoBuffer<_Tp, fixed_size> &abuf) {
  if (this != &abuf) {
    deallocate();
    allocate(abuf.size());
    for (size_t i = 0; i < sz; i++)
      ptr[i] = abuf.ptr[i];
  }
  return *this;
}

template<typename _Tp, size_t fixed_size> inline AutoBuffer<_Tp, fixed_size>::~AutoBuffer() {
  if (!is_detached) {
    deallocate();
  }
}

template<typename _Tp, size_t fixed_size> inline void AutoBuffer<_Tp, fixed_size>::allocate(size_t _size) {
  if (_size <= sz) {
    sz = _size;
    return;
  }
  deallocate();
  sz = _size;
  if (_size > fixed_size) {
    ptr = new _Tp[_size];
  }
}

template<typename _Tp, size_t fixed_size> inline void AutoBuffer<_Tp, fixed_size>::deallocate() {
  if (ptr != buf) {
    delete[] ptr;
    ptr = buf;
    sz = fixed_size;
  }
}

template<typename _Tp, size_t fixed_size> inline void AutoBuffer<_Tp, fixed_size>::resize(size_t _size) {
  if (_size <= sz) {
    sz = _size;
    return;
  }
  size_t i, prevsize = sz, minsize = std::min(prevsize, _size);
  _Tp *prevptr = ptr;

  ptr = _size > fixed_size ? new _Tp[_size] : buf;
  sz = _size;

  if (ptr != prevptr)
    for (i = 0; i < minsize; i++)
      ptr[i] = prevptr[i];
  for (i = prevsize; i < _size; i++)
    ptr[i] = _Tp();

  if (prevptr != buf)
    delete[] prevptr;
}

template<typename _Tp, size_t fixed_size> inline size_t AutoBuffer<_Tp, fixed_size>::size() const {
  return sz;
}
}

#endif //MEMORY_UTILS_
