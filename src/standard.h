#ifndef STANDARD_H
#define STANDARD_H

// ******************
// * Standard Types *
// ******************

#include <inttypes.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Unsigned Integer Primitives

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Signed Integer Primitives

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Floating-Point Number Primitives

typedef float  f32;
typedef double f64;

// Pointer-Sized Signed and Unsigned Integers

typedef uintptr_t uptr;
typedef intptr_t  iptr;

typedef size_t usize;

// String Related Types

typedef char*    string;
typedef uint32_t rune;

// Other Built-In Types

typedef struct
{
  FILE* handle;
  usize size;
} File;

// *******************
// * Memory Handling *
// *******************

// Allocates a new block of memory.
// std::memory::allocate[T](count: usize) -> T*
#define mem_alloc(type, count) ((type*) malloc(sizeof(type) * (count)))

// Allocates a new block of memory.
// std::memory::allocate[T](count: usize) -> T*
#define mem_stack_alloc(type, count) ((type*) alloca(sizeof(type) * (count)))

// Free a previously allocated block of memory.
// std::memory::free[T](ptr: T*)
#define mem_delete(ptr) free(ptr)

// Reallocates a previously allocated block of memory.
// std::memory::reallocate[T](ptr: T*, count: usize) -> T*
#define mem_realloc(ptr, count) realloc(ptr, sizeof(typeof(*(ptr))) * (count))

// Sets a block of memory to zero.
// std::memory::clear[T](ptr: T**, count: usize)
#define mem_clear(ptr, count) (memset((ptr), 0x0, sizeof(typeof(*(ptr))) * count))

// ***************
// * Standard IO *
// ***************

File* io_fileOpen(const string path, const string mode);
bool  io_fileClose(File* file);
usize io_fileRead(File* file, u8* buffer, usize count);
usize io_fileWrite(File* file, u8* buffer, usize count);
bool  io_fileSeek(File* file, usize offset);

void io_print(const string message);
void io_printError(const string message);

// ******************************
// * Standard String Operations *
// ******************************

// ...
string str_concat(const string a, const string b);
usize  str_length(const string a);

// ************************
// * Time and Performance *
// ************************

// Retrieves a timepoint in nanoseconds since an arbitrary point in time.
u64 time_now();

#endif
