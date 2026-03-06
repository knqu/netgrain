#pragma once

#include <mutex>

#include <stdlib.h>

#include "array_list.hpp"

template<typename T>
struct Blocking_Queue
{
  std::mutex mutex;

  size_t len;
  size_t cap;
  size_t front_ptr;
  size_t back_ptr;

  T *data;

  size_t size()
  {
    mutex.lock();
    size_t size = len;
    mutex.unlock();

    return size;
  }

  bool empty()
  {
    mutex.lock();
    bool is_empty = len == 0;
    mutex.unlock();

    return is_empty;
  }

  Blocking_Queue(size_t init_cap)
  {
    cap = init_cap;
    len = 0;
    
    data = (T *) malloc(sizeof(T) * cap);
  }
  
  Blocking_Queue()
  {
    cap = initial_arr_cap;
    len = 0;

    data = (T *) malloc(sizeof(T) * cap);
  }

  ~Blocking_Queue()
  {
    free(data);
  }

  void enqueue(T val)
  {
    mutex.lock();
    if (len == cap)
    {
      size_t new_cap = cap * resize_factor;
      T *new_arr = (T *) malloc(sizeof(T) * new_cap);
      size_t count = 0;
      while (front_ptr != back_ptr)
      {
        new_arr[count] = data[front_ptr];
        front_ptr += 1;
        count += 1;
        if (front_ptr == cap - 1) front_ptr = 0;
      }
      front_ptr = 0;
      back_ptr = cap;
      cap = new_cap;
      free(data);
      data = new_arr;
    }

    data[back_ptr] = val;
    back_ptr = (back_ptr + 1) % cap;

    len += 1;
    mutex.unlock();
  }

  T dequeue()
  {
    mutex.lock();
    assert(len > 0);

    // get front pointer, get value, move front pointer forward one place
    
    T ret_val = data[front_ptr];
    front_ptr = (front_ptr + 1) % cap;
    len -= 1;
    mutex.unlock();

    return ret_val;
  }

  T peek()
  {
    mutex.lock();
    assert(len > 0);
    T val = data[front_ptr];
    mutex.unlock();

    return val;
  }

  void reset()
  {
    mutex.lock();
    len = 0;
    front_ptr = 0;
    back_ptr = 0;
    mutex.unlock();
  }
};

