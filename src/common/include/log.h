/* See COPYRIGHT for copyright information. */

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include "usg_common.h"

void _warn(const char*, int, const char*, ...);
void _panic(const char*, int, const char*, ...) __attribute__((noreturn));

#define warn(...) _warn(__FILE__, __LINE__, __VA_ARGS__)
#define panic(...) _panic(__FILE__, __LINE__, __VA_ARGS__)


// 	do { if (!(x)) panic("assertion failed: %s", #x); } while (0)

#define MY_ASSERT(expr, ...)        \
    if (!(expr)) { \
      warn("assertion failed: %s", #expr); \
      fprintf(stdout, __VA_ARGS__);  \
      fflush(stdout); \
      while(true); \
   }

  

// static_assert(x) will generate a compile-time error if 'x' is false.
#define MY_STATIC_ASSERT(x)	switch (x) case 0: case (x):

#endif 
