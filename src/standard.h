#ifndef STANDARD_H
#define STANDARD_H

// ******************
// * Standard Types *
// ******************

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef uintptr_t uptr;
typedef intptr_t  iptr;

typedef size_t usize;

typedef char* string;

typedef struct
{
  FILE* handle;
  usize size;
} File;

// *******************
// * Memory Handling *
// *******************

#define mem_alloc(type, count)  ((type*) malloc(sizeof(type) * (count)))
#define mem_realloc(ptr, count) (ptr = realloc((ptr), sizeof(typeof(*(ptr))) * (count)))
#define mem_delete(ptr)         free(ptr)

#define mem_clear(ptr, offset, length) \
  (memset((ptr + offset), 0x0, sizeof(typeof(&(ptr))) * length))

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
