#include "lzw.h"
#include "standard.h"

i32 main(i32 argc, string argv[])
{
  if (argc == 2) // exe name + parameter
  {
    u64 time_start = time_now();

    // ...
    const string encodedFilePath = str_concat(argv[1], ".encoded");
    const string decodedFilePath = str_concat(argv[1], ".decoded");

    fprintf(stdout, "Input:   %s\n", argv[1]);
    fprintf(stdout, "Encoded: %s\n", encodedFilePath);
    fprintf(stdout, "Decoded: %s\n", decodedFilePath);

    fprintf(stdout, "----\n");

    fprintf(stdout, "READING...");

    // ...
    File* file = io_fileOpen(argv[1], "rb");
    if (file == NULL)
    {
      io_printError("Unable to open file for read.");
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

    // ...
    fprintf(stdout, "  %" PRIuPTR " bytes\n", bytesRead);

    // Close the file
    io_fileClose(file);

    fprintf(stdout, "ENCODING...");

    // Compress the data
    usize encodedSize;
    u8*   encodedData = lzw_encode(data, bytesRead, &encodedSize);
    fprintf(stdout, " %" PRIuPTR " bytes\n", encodedSize);

    // Write compressed data to disk
    File* encodedFile = io_fileOpen(encodedFilePath, "wb");
    if (encodedFile == NULL)
    {
      io_printError("Unable to open file for write.");
      return -4;
    }

    // ...
    io_fileWrite(encodedFile, encodedData, encodedSize);
    io_fileClose(encodedFile);

    fprintf(stdout, "DECODING...");

    // Decompress the data
    usize decodedSize;
    u8*   decodedData = lzw_decode(encodedData, encodedSize, &decodedSize);
    fprintf(stdout, " %" PRIuPTR " bytes\n", decodedSize);

    // Write decompressed data to disk
    File* decodedFile = io_fileOpen(str_concat(argv[1], ".decoded"), "wb");
    io_fileWrite(decodedFile, decodedData, decodedSize);
    io_fileClose(decodedFile);

    fprintf(stdout, "----\n");

    // ...
    if (decodedSize != bytesRead)
      fprintf(stderr, "Error: Decoded Size Incorrect\n");

    // ...
    bool  isNewError = true;
    usize errors     = 0;

    // ...
    usize errorCheckSize = (decodedSize < bytesRead) ? decodedSize : bytesRead;
    for (usize i = 0; i < errorCheckSize; i++)
    {
      if (data[i] != decodedData[i]) // mismatch
      {
        if (isNewError && errors < 10)
        {
          fprintf(stderr, "Error: Mismatched at offset %" PRIuPTR "\n", i);
          isNewError = false;
        }
        errors++;
      }
      else
      {
        isNewError = true;
      }
    }

    if (errors > 0)
      fprintf(stderr, "Error: Encountered %" PRIuPTR " errors.\n", errors);

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

  // Flush output streams
  fflush(stdout);
  fflush(stderr);

  return 0;
}
