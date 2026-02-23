#pragma once

#include <stdio.h>

#include "def.hpp"

constexpr size_t arena_region_default_capacity = (8 * 1024);

struct Region
{
  size_t capacity;
  size_t count;
  Region *next;
  uintptr_t *buffer;

  Region(size_t capacity)
  {
    buffer = (uintptr_t *) new u8[
      sizeof(Region) + sizeof(uintptr_t) * capacity
    ];
    next = nullptr;
    count = 0;
    this->capacity = capacity;
  }

  ~Region()
  {
    delete[] buffer;
  }
};

struct Arena_Allocator
{
  Region *begin;
  Region *end;

  size_t region_default_capacity = arena_region_default_capacity;

  Arena_Allocator()
  {
    begin = nullptr;
    end   = nullptr;
  }

  Arena_Allocator(size_t default_cap)
  {
    begin = nullptr;
    end   = nullptr;

    region_default_capacity = default_cap;
  }

  void *alloc(size_t size)
  {
    // alignment by size of uintptr_t
    size_t aligned_count = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);

    if (begin == nullptr)
    {
      size_t capacity = arena_region_default_capacity;
      if (capacity < aligned_count) capacity = aligned_count;
      begin = new Region(capacity);
      end = begin;
    }

    while ((end->count + aligned_count > end->capacity)
           && (end->next != nullptr))
    {
      end = end->next;
    }

    if (end->count + aligned_count > end->capacity)
    {
      size_t capacity = arena_region_default_capacity;
      if (capacity < aligned_count) capacity = aligned_count;
      end->next = new Region(capacity);
      end = end->next;
    }

    void *result = &end->buffer[end->count];
    end->count += size;
    return result;
  }

  void dealloc(void *pointer)
  {
    (void) pointer;
  }

  void *realloc(void *old_ptr, size_t old_size, size_t new_size)
  {
    if (new_size <= old_size) return old_ptr;

    void *new_ptr = alloc(new_size);

    u8 *new_ptr_bytes = (u8 *) new_ptr;
    u8 *old_ptr_bytes = (u8 *) old_ptr;

    for (size_t i = 0; i < old_size; i++)
    {
      new_ptr_bytes[i] = old_ptr_bytes[i];
    }

    return new_ptr;
  }

  void *memcpy(void *dest, const void *src, size_t n)
  {
    u8 *dest_ptr_bytes = (u8 *) dest;
    u8 *src_ptr_bytes  = (u8 *) src;

    for (size_t i = 0; i < n; i++)
    {
      dest_ptr_bytes[i] = src_ptr_bytes[i];
    }

    return dest;
  }

  void reset()
  {
    Region *region = begin;

    while (region != nullptr)
    {
      Region *curr = region;
      region = region->next;
      curr->count = 0;
    }

    end = begin;
  }

  void trim()
  {
    Region *region = end->next;

    while (region != nullptr)
    {
      Region *curr = region;
      region = region->next;
      delete curr;
    }
    end->next = nullptr;
  }

  ~Arena_Allocator()
  {
    Region *region = begin;

    while (region != nullptr)
    {
      Region *curr = region;
      region = region->next;
      delete curr;
    }
    end->next = nullptr;
  }
};

