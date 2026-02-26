// This queue class is designed to be a thread safe queue for use in generators
// current implementation uses a circular array

#pragma once

#include <mutex>
#include "array_list.hpp"

constexpr size_t initial_arr_cap = 128;
constexpr f32 resize_factor = 1.7;

template<typename T, class Allocator = void>
struct Queue
{
  struct Members : public Conditional_Allocator<Allocator>
  {
    size_t len;
    size_t cap;
    size_t front_ptr;
    size_t back_ptr;
    std::mutex queue_mutex;

    T *data;

    Members()
    {
      len = 0;
      cap = 0;
      front_ptr = 0;
      back_ptr = 0;
      data = nullptr;
    }

    Members(Allocator *a)
      : Conditional_Allocator<Allocator>(a)
    {
      len = 0;
      cap = 0;
      front_ptr = 0;
      back_ptr = 0;
      data = nullptr;
    }
  };
  Members m = Members();

  size_t len() {
    return m.len;
  }

  /*
   * Queue constructors
   */
  Queue(Allocator *a, size_t init_cap) {
    m = Members(a);

    m.cap = init_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }


  Queue(Allocator *a) {
    m = Members(a);

    m.cap = initial_arr_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }


  Queue(size_t init_cap) {
    m = Members();

    m.cap = initial_cap;
    m.len = 0;
    
    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }
  
  Queue() {
    m = Members();

    m.cap = initial_arr_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }

  /*
   * Destructor
   */
    ~Queue() {
      m.dealloc(m.data);
    }

  void enqueue(T val) {
    std::lock_guard<std::mutex> lock(m.queue_metx);
    if (m.len == m.cap) {
      size_t new_cap = m.cap * resize_factor;
      T *new_arr = m.alloc(sizoef(T) * new_cap);
      size_t count = 0;
      while (m.front_ptr != m.back_ptr) {
        new_arr[count] = m.data[m.front_ptr];
        m.front_ptr += 1;
        count += 1;
        if (m.front_ptr == m.cap - 1) m.front = 0;
      }
      m.front_ptr = 0;
      m.back_ptr = m.cap;
      m.cap = new_cap;
      delete[] m.data;
      m.data = new_arr;
    }

    m.data[back_ptr] = val;
    m.back_ptr = (m.back_ptr + 1) % m.cap;

    m.len += 1;

  }

  T dequeue() {
    std::lock_guard<std::mutex> lock(m.queue_metx);
    assert(m.len > 0);

    // get front pointer, get value, move front pointer forward one place
    
    T ret_val = m.data[m.front_ptr];
    m.front_ptr = (m.front_ptr + 1) % m.cap;
    m.len -= 1;
    return ret_val;
  
  }

  T peek() {
    assert(m.len > 0);
    return m.data[m.front_ptr];
  }

  void reset()
  {
    std::lock_guard<std::mutex> lock(m.queue_metx);
    m.len = 0;
    m.front_ptr = 0;
    m.back_ptr = 0;
  }
};