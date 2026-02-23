#pragma once

#include "def.hpp"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <type_traits>

constexpr size_t initial_arr_cap = 128;
constexpr f32 resize_factor = 1.7;

// interface for custom allocators based on implementation illustrated in
// Stewart Lynch's Sane C++ series
template<typename T, typename Enable = void> struct Conditional_Allocator;

template<typename T>
struct Conditional_Allocator<T, std::enable_if_t<std::is_same_v<T, void>>>
{
  void *alloc(size_t size)
  {
    return new u8[size];
  }

  void dealloc(void *pointer)
  {
    delete[] (u8 *) pointer;
  }

  T *get_allocator()
  {
    return nullptr;
  }
};

template<typename T>
struct Conditional_Allocator<T, std::enable_if_t<!std::is_same_v<T, void>>>
{
  T *allocator;
  bool owns_allocator;

  Conditional_Allocator()
  {
    allocator = new T();
    owns_allocator = true;
  }

  Conditional_Allocator(T *allocator)
  {
    this->allocator = allocator;
    owns_allocator = false;
  }

  ~Conditional_Allocator()
  {
    if (owns_allocator) delete allocator;
  }

  T *get_allocator()
  {
    return allocator;
  }

  void *alloc(size_t size)
  {
    return allocator->alloc(size);
  }

  void dealloc(void *pointer)
  {
    return allocator->dealloc(pointer);
  }
};

template<typename T, class Allocator = void>
struct Array_List
{
  struct Members : public Conditional_Allocator<Allocator>
  {
    size_t len;
    size_t cap;
    T *data;

    Members()
    {
      len = 0;
      cap = 0;
      data = nullptr;
    }

    Members(Allocator *a)
      : Conditional_Allocator<Allocator>(a)
    {
      len = 0;
      cap = 0;
      data = nullptr;
    }
  };
  Members m = Members();

  size_t len()
  {
    return m.len;
  }

  Array_List(Allocator *a, size_t init_cap)
  {
    m = Members(a);

    m.cap = init_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }

  Array_List(Allocator *a, size_t init_len, T init_val)
  {
    m = Members(a);

    m.cap = initial_arr_cap;
    while (m.cap < init_len) m.cap *= resize_factor;

    m.len = init_len;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);

    for (size_t i = 0; i < init_len; i++)
    {
      m.data[i] = init_val;
    }
  }

  Array_List(Allocator *a, size_t init_len, T init_vals[])
  {
    m = Members(a);

    m.cap = initial_arr_cap;
    while (m.cap < init_len) m.cap *= resize_factor;

    m.len = init_len;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);

    for (size_t i = 0; i < init_len; i++)
    {
      m.data[i] = init_vals[i];
    }
  }

  Array_List(Allocator *a)
  {
    m = Members(a);

    m.cap = initial_arr_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }

  Array_List(size_t init_cap)
  {
    m = Members();

    m.cap = init_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }

  Array_List(size_t init_len, T init_val)
  {
    m = Members();

    m.cap = initial_arr_cap;
    while (m.cap < init_len) m.cap *= resize_factor;

    m.len = init_len;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);

    for (size_t i = 0; i < init_len; i++)
    {
      m.data[i] = init_val;
    }
  }

  Array_List(size_t init_len, T init_vals[])
  {
    m = Members();

    m.cap = initial_arr_cap;
    while (m.cap < init_len) m.cap *= resize_factor;

    m.len = init_len;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);

    for (size_t i = 0; i < init_len; i++)
    {
      m.data[i] = init_vals[i];
    }
  }

  Array_List()
  {
    m = Members();

    m.cap = initial_arr_cap;
    m.len = 0;

    m.data = (T *) m.alloc(sizeof(T) * m.cap);
  }

  void append(T elem)
  {
    m.len += 1;
    if (m.len > m.cap)
    {
      while (m.len > m.cap) m.cap *= resize_factor;

      T *new_data = (T *) m.alloc(sizeof(T) * m.cap);
      memcpy(new_data, m.data, m.len - 1);
      m.dealloc(m.data);

      m.data = new_data;
    }
    m.data[m.len - 1] = elem;
  }

  void insert(size_t index, T elem)
  {
    assert((index >= 0) && (index < m.len));

    m.len += 1;
    if (m.len > m.cap)
    {
      while (m.len > m.cap) m.cap *= resize_factor;

      T *new_data = (T *) m.alloc(sizeof(T) * m.cap);
      memcpy(new_data, m.data, m.len - 1);
      m.dealloc(m.data);
      m.data = new_data;
    }

    for (size_t i = m.len - 1; i > index; i--)
    {
      m.data[i] = m.data[i - 1];
    }
    m.data[index] = elem;
  }

  T remove(size_t index)
  {
    assert((index >= 0) && (index < m.len));

    T removed = m.data[index];
    for (size_t i = index; i < m.len; i++)
    {
      m.data[i] = m.data[i + 1];
    }
    m.len -= 1;

    return removed;
  }

  T remove_last()
  {
    assert(m.len > 0);

    m.len -= 1;
    return m.data[m.len - 1];
  }

  void reset()
  {
    m.len = 0;
  }

  T& operator[](int index)
  {
    assert((index >= 0) && (index < m.len));
    return m.data[index];
  }
};

