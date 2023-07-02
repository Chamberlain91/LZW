#include "lzw.h"
#include "standard.h"

#include <string.h>

i32 main(i32 argc, string argv[])
{
  // // ...
  // srand((u32) time_now());

  // // Write decompressed data to disk
  // File* randomFile = io_fileOpen("random.bin", "wb");
  // usize randomSize = 1024 * 1024;
  // u8*   randomData = mem_alloc(u8, randomSize);
  // for (usize i = 0; i < randomSize; i++)
  //   randomData[i] = i % 5 + rand() % 5;
  // io_fileWrite(randomFile, randomData, randomSize);
  // io_fileClose(randomFile);

  if (argc == 2) // exe name + parameter
  {
    const string encodedFilePath = str_concat(argv[1], ".lzw");
    const string decodedFilePath = str_concat(argv[1], ".lzw_decoded");

    // ---------------
    //  FILE READ STEP
    // ---------------

    // Opens the specified file.
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
      fprintf(stderr, "Read %8" PRIuPTR " bytes, expected %" PRIuPTR "\n", bytesRead, file->size);
      io_fileClose(file);
      return -3;
    }

    // Close the file.
    io_fileClose(file);

    // ---------------
    //  ENCODING STEP
    // ---------------

    fprintf(stdout, "Encoding...");

    u64 encodeTimeStart = time_now();

    // Compress the data
    usize encodedSize;
    u8*   encodedData = lzw_encode(data, bytesRead, &encodedSize);

    u64 encodeTimeEnd = time_now();
    fprintf(stdout, " %8" PRIuPTR " bytes ", encodedSize);

    // Report encoding execution time
    printf("(%.2f ms) ", (f64) (encodeTimeEnd - encodeTimeStart) / 1000000.0);
    printf("%.1f%%\n", ((f32) encodedSize / (f32) bytesRead) * 100);

    // Write compressed data to disk
    File* encodedFile = io_fileOpen(encodedFilePath, "wb");
    if (encodedFile == NULL)
    {
      io_printError("Unable to open file for write.");
      return -4;
    }

    // ---------------
    //  DECODING STEP
    // ---------------

    // ...
    io_fileWrite(encodedFile, encodedData, encodedSize);
    io_fileClose(encodedFile);

    fprintf(stdout, "Decoding...");

    u64 decodeTimeStart = time_now();

    // Decompress the data
    usize decodedSize;
    u8*   decodedData = lzw_decode(encodedData, encodedSize, &decodedSize);

    u64 decodeTimeEnd = time_now();
    fprintf(stdout, " %8" PRIuPTR " bytes ", decodedSize);

    // Report decoding execution time
    printf("(%.2f ms)\n", (f64) (decodeTimeEnd - decodeTimeStart) / 1000000.0);

    // Write decompressed data to disk
    File* decodedFile = io_fileOpen(decodedFilePath, "wb");
    io_fileWrite(decodedFile, decodedData, decodedSize);
    io_fileClose(decodedFile);

    // ---------------------
    //  ERROR CHECKING STEP
    // ---------------------

    // Check content size mismatch.
    if (decodedSize != bytesRead)
    {
      fprintf(stderr, "Error: Detected Size Mismatch.\n");
      fprintf(stderr, "       Have %" PRIuPTR " bytes ", decodedSize);
      fprintf(stderr, "(Expected %" PRIuPTR " bytes)\n", bytesRead);
    }

    // Check content mismatch.
    usize contentSize = (decodedSize < bytesRead) ? decodedSize : bytesRead;
    if (memcmp(data, decodedData, contentSize) != 0)
    {
      fprintf(stderr, "Error: Detected Content Mismatch.\n");

      usize errorCount = 0;
      for (usize i = 0; i < contentSize; i++)
      {
        if (data[i] != decodedData[i])
        {
          // fprintf(stderr, "mismatch at %llu\n", i);
          errorCount++;
        }
      }

      fprintf(stderr, "       Found %" PRIuPTR " mismatched bytes.", errorCount);
    }

    // --------------
    //  CLEANUP STEP
    // --------------

    // Free allocations
    mem_delete(encodedFilePath);
    mem_delete(encodedData);
    mem_delete(decodedFilePath);
    mem_delete(decodedData);
    mem_delete(data);
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
