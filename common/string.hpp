#pragma once

#include "def.hpp"

#include <stddef.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

constexpr u64 stack_size  = 64;
constexpr u64 initial_cap = 128;

struct String8
{
  union
  {
    u8 stack[stack_size + 1];
    u8 *data;
  } buf;

  u64 len;
  u64 cap;

  bool s_alloc;

  char *str()
  {
    if (s_alloc)
    {
      return (char *) buf.stack;
    }
    else {
      return (char *) buf.data;
    }
  }

  String8(const char *data) : String8(data, strlen(data)) {}

  String8(const char *data, u64 size)
  {
    if ((size + 1) <= stack_size)
    {
      memcpy(buf.stack, data, size);
      buf.stack[size] = '\0';
      len = size;
      s_alloc = true;
    } else
    {
      cap = initial_cap;
      while ((size + 1) > cap) cap *= 2;
      buf.data = (u8 *) calloc(cap, sizeof(u8));
      memcpy(buf.data, data, size);
      buf.data[size] = '\0';
      len = size;
      s_alloc = false;
    }
  }

  void cat(const char *data)
  {
    assert(data != nullptr);

    cat(data, strlen(data));
  }

  void cat(char c)
  {
    if (s_alloc)
    {
      if ((2 + len) <= stack_size)
      {
        buf.stack[len++] = c;
        buf.stack[len] = '\0';
      }
      else
      {
        cap = initial_cap;
        while ((2 + len) > cap) cap *= 2;

        u8 *str_data = (u8 *) calloc(cap, sizeof(u8));
        memcpy(str_data, buf.stack, len);
        buf.data = str_data;

        buf.data[len++] = c;
        buf.data[len] = '\0';

        s_alloc = false;
      }
    }
    else {
      buf.data[len++] = c;
      buf.data[len] = '\0';
    }
  }

  void cat(const char *data, u64 size)
  {
    if (s_alloc)
    {
      if ((size + 1 + len) <= stack_size)
      {
        u8 *cpy_buf = &(buf.stack[len]);
        memcpy(cpy_buf, data, size);
        len += size;
        buf.stack[len] = '\0';
      }
      else
      {
        cap = initial_cap;
        while ((size + 1 + len) > cap) cap *= 2;

        u8 *str_data = (u8 *) calloc(cap, sizeof(u8));
        memcpy(str_data, buf.stack, len);
        buf.data = str_data;

        u8 *cpy_buf = &(buf.data[len]);
        memcpy(cpy_buf, data, size);

        len += size;
        buf.data[len] = '\0';

        s_alloc = false;
      }
    }
    else {
      while ((size + 1 + len) > cap) cap *= 2;
      buf.data = (u8 *) realloc(buf.data, cap);

      u8 *cpy_buf = &(buf.data[len]);
      memcpy(cpy_buf, data, size);

      len += size;
      buf.data[len] = '\0';
    }
  }

  void cut(size_t size)
  {
    if (size > len) return;

    if (s_alloc)
    {
      buf.stack[len - size] = '\0';
    }
    else {
      buf.data[len - size] = '\0';
    }
    len -= size;
  }

  void erase(size_t pos, size_t size)
  {
    if (size > len) return;
    if ((pos < 0) || (pos >= len)) return;
    if (pos + size > len) return;

    if (s_alloc)
    {
      u8 *remainder = &(buf.stack[pos + size]);
      size_t remainder_size = len - (pos + size);
      u8 *cpy_buf = &(buf.stack[pos]);
      memcpy(cpy_buf, remainder, remainder_size);
      buf.stack[len - size] = '\0';
    }
    else {
      u8 *remainder = &(buf.data[pos + size]);
      size_t remainder_size = len - (pos + size);
      u8 *cpy_buf = &(buf.data[pos]);
      memcpy(cpy_buf, remainder, remainder_size);
      buf.data[len - size] = '\0';
    }

    len -= size;
  }

  void clear()
  {
    len = 0;
    if (s_alloc)
    {
      buf.stack[0] = '\0';
    }
    else {
      buf.data[0] = '\0';
    }
  }
};

struct String8_View
{
  u8 *buffer;
  size_t len;

  String8_View(String8 str)
  {
    len = str.len;
    if (str.s_alloc)
    {
      buffer = str.buf.stack;
    }
    else {
      buffer = str.buf.data;
    }
  }

  char *str()
  {
    return (char *) buffer;
  }
};


