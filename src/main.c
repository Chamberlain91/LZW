#include "lzw.h"
#include "standard.h"

i32 main(i32 argc, string argv[])
{
  if (argc == 2) // exe name + parameter
  {
    u64 time_start = time_now();

    io_print("READING DATA");

    // ...
    File* file = io_fileOpen(argv[1], "rb");
    if (file == NULL)
    {
      io_printError("Unable to open file");
      return -1;
    }

    // Allocate memory for file data.
    u8* data = mem_alloc(u8, file->size);
    if (data == NULL)
    {
      io_printError("Could not allocate memory.");
      io_fileClose(file);
      return -2;
    }

    // Read the file contents into the buffer and close.
    usize bytesRead = io_fileRead(file, data, file->size);
    if (bytesRead != file->size)
    {
      fprintf(stderr, "Read %" PRIuPTR " bytes, expected %" PRIuPTR "\n", bytesRead, file->size);
      io_fileClose(file);
      return -3;
    }

    // Close the file
    io_fileClose(file);

    io_print("ENCODING...");

    // Compress the data
    usize encodedSize;
    u8*   encodedData = lzw_encode(data, file->size, &encodedSize);
    printf("ENCODED: %" PRIuPTR " bytes\n", encodedSize);

    // Write compressed data to disk
    string encodedFilePath = str_concat(argv[1], ".encoded");
    File*  encodedFile     = io_fileOpen(encodedFilePath, "wb");
    io_fileWrite(encodedFile, encodedData, encodedSize);
    io_fileClose(encodedFile);

    io_print("DECODING...");

    // Decompress the data
    usize decodedSize;
    u8*   decodedData = lzw_decode(encodedData, encodedSize, &decodedSize);
    printf("DECODED: %" PRIuPTR " bytes (expected %" PRIuPTR ")\n", decodedSize, encodedSize);

    // Write decompressed data to disk
    string decodedFilePath = str_concat(argv[1], ".decoded");
    File*  decodedFile     = io_fileOpen(str_concat(argv[1], ".decoded"), "wb");
    io_fileWrite(decodedFile, decodedData, decodedSize);
    io_fileClose(decodedFile);
    
    io_print("WRITES COMPLETE");

    // Free allocations
    mem_delete(encodedFilePath);
    mem_delete(encodedData);
    mem_delete(decodedFilePath);
    mem_delete(decodedData);
    mem_delete(data);

    // ...
    u64 elapsed_ns = time_now() - time_start;
    f64 elapsed    = (f64) elapsed_ns / 1000000.0;
    printf("Execution took %f ms\n", elapsed);
  }
  else
  {
    io_print("USAGE: lzw [file]");
    return 0;
  }

  return 0;
}
