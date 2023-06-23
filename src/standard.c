#include "standard.h"

#include <string.h>
#include <time.h>

u64 time_now()
{
  struct timespec now;
  timespec_get(&now, TIME_UTC);
  return ((u64) now.tv_sec * 1000000000) + (u64) now.tv_nsec;
}

string str_concat(const string a, const string b)
{
  usize aLen = str_length(a);
  usize bLen = str_length(b);

  string output = mem_alloc(char, aLen + bLen);

  strcpy(output, a);
  strcat(output, b);

  return output;
}

usize str_length(const string a)
{
  return strlen(a);
}

File* io_fileOpen(const string path, const string mode)
{
  FILE* handle = fopen(path, mode);
  if (handle == NULL)
    return NULL;

  // Determine file size
  fseek(handle, 0L, SEEK_END);
  usize size = (usize) ftell(handle);
  rewind(handle);

  // Allocate file info
  File* file   = mem_alloc(File, 1);
  file->handle = handle;
  file->size   = size;

  return file;
}

bool io_fileClose(File* file)
{
  if (file->handle)
  {
    bool success = fclose(file->handle) == 0;

    if (success)
    {
      file->handle = NULL;
      mem_delete(file);
    }

    return success;
  }
  else
  {
    // File was closed / never opened.
    // Return true to signal the resource is no longer in use.
    return true;
  }
}

usize io_fileRead(File* file, u8* buffer, usize count)
{
  return fread(buffer, sizeof(u8), count, file->handle);
}

usize io_fileWrite(File* file, u8* buffer, usize count)
{
  return fwrite(buffer, sizeof(u8), count, file->handle);
}

bool io_fileSeek(File* file, usize offset)
{
  return fseek(file->handle, (long) offset, SEEK_SET);
}

void io_print(const string message)
{
  fprintf(stdout, "%s\n", message);
}

void io_printError(const string message)
{
  fprintf(stderr, "%s\n", message);
}
