#include "lzw.h"

#include <string.h>

// -----------------
//  WRITABLE BUFFER
// -----------------

// ...
struct buffer_t
{
  u8*   storage;
  usize capacity;
  usize offset;
};

// ...
void buffer_init(struct buffer_t* buffer);

// ...
void buffer_free(struct buffer_t* buffer);

// ...
void buffer_write(struct buffer_t* buffer, const u8* data, usize dataLength);

// ...
void buffer_write_u16(struct buffer_t* buffer, u16 value);
void buffer_write_u8(struct buffer_t* buffer, u8 value);

// --------------------
//  LZW IMPLEMENTATION
// --------------------

static const u32 MAX_CODE_COUNT = ((u16) -1) + 1;

u8* lzw_encode(const u8* input, const usize inputSize, usize* out_outputSize)
{
  // LZW ENCODING ALGORITHM
  // 1. Initialize dictionary to have all strings length 1 (bytes 0-255)
  // 2. Find longest string W in the dictionary that matches the current input
  // 3. Emit dictionary index for W to output and remove W from input
  // 4. Add W followed by the next symbol in the input to the dictionary
  // 5. Go to Step 2

  // ...
  struct node_t
  {
    struct node_t* next[256];
  };

  // TODO: VALIDATE / ERROR CONDITIONS
  // error: inputSize == 0
  // error: input == NULL
  // error: outputSize == NULL ?

  // Computes the end of the input
  const u8* const inputEnd = input + inputSize;

  // A resizable output buffer/stream.
  struct buffer_t output;
  buffer_init(&output);

  // Allocate Dictionary
  struct node_t* dict = mem_alloc(struct node_t, MAX_CODE_COUNT);

  // Initialize Dictionary
  mem_clear(dict, 0, MAX_CODE_COUNT);
  usize nextCode = 256;

  // The first letter is always an "alphabet" string.
  struct node_t* node = &dict[*input++];

  // ...
  while (input < inputEnd)
  {
    const u8 symbol = *input++;

    // ...
    if (node->next[symbol] == NULL)
    {
      // Emit code for this longest match.
      buffer_write_u16(&output, (u16) (node - dict));

      // If the dictionary is full, reset begin encoding of a new chunk.
      if (nextCode == MAX_CODE_COUNT)
      {
        printf("[ENCODE] RESET DICTIONARY\n");

        // (Re)Initialize Dictionary
        mem_clear(dict, 0, MAX_CODE_COUNT);
        nextCode = 256;
      }

      // Update dictionary and begin new match.
      node->next[symbol] = &dict[nextCode++];
      node               = &dict[symbol];
    }
    else
    {
      // Extend longest match.
      node = node->next[symbol];
    }
  }

  // ...
  mem_delete(dict);

  // The user is responsible to free the output.
  *out_outputSize = output.offset;
  return output.storage;
}

u8* lzw_decode(const u8* input, usize inputSize, usize* out_outputSize)
{
  // LZW DECODING ALGORITHM
  // 1. Initialize dictionary to have all strings length 1 (bytes 0-255)
  // 2. Read the next encoded symbol. Is it in the dictionary?
  //    - Yes
  //      1. Emit corresponding W to output.
  //      2. Concatenate the previous string emitted to output with the first symbol of W. Add this to dictionary.
  //    - No
  //      1. Concatenate the previous string emitted to output with its first symbol. Call this string V.
  //      2. Add V to the dictionary and emit V to output.
  // 3. Repeat 2

  // ...
  struct span_t
  {
    usize offset;
    usize length;
  };

  // TODO: VALIDATE / ERROR CONDITIONS
  // error: inputSize == 0
  // error: input == NULL
  // error: outputSize == NULL ?

  // View the input as 16-bit codes.
  const u16* symbolsEnd = (const u16*) (input + inputSize);
  const u16* symbols    = (const u16*) (input);

  // A resizable output buffer/stream.
  struct buffer_t output;
  buffer_init(&output);

  // Allocate Dictionary
  struct span_t* dict = mem_alloc(struct span_t, MAX_CODE_COUNT);

  // Initialize Dictionary
  memset(dict, 0, sizeof(struct span_t) * MAX_CODE_COUNT);
  usize nextCode = 256;

  // ...
  usize previous = 0;

  // ...
  while (symbols != symbolsEnd) // (STEP 2 <- 3)
  {
    // ...
    const u16 symbol = *symbols++;

    // ...
    struct span_t* span = &dict[symbol];

    // ...
    if (symbol >= 256 && span->length == 0) // Unknown Code
    {
      usize start = output.offset;

      // Concatenate previous string with its first symbol, call this V.
      buffer_write(&output, &output.storage[previous], start - previous); // prev
      buffer_write(&output, &output.storage[previous], 1);                // prev[0]

      // Place V into dictionary.
      span->offset = start;
      span->length = output.offset - start;

      // Mark the start of previous output string
      previous = start;
    }
    else // Known Code
    {
      if (symbol < 256)
      {
        // Special Case: A string of length one (V = ASCII)
        buffer_write_u8(&output, (u8) symbol);
      }
      else
      {
        // 1. Write W to output.
        buffer_write(&output, &output.storage[span->offset], span->length);
      }

      usize start = output.offset;

      if (nextCode == MAX_CODE_COUNT)
      {
        printf("[DECODE] RESET DICTIONARY\n");
        // TODO: ACTUALLY MAKE WORK

        // (Re)Initialize Dictionary
        memset(dict, 0, sizeof(struct span_t) * MAX_CODE_COUNT);
        nextCode = 256;
      }

      // ...
      span = &dict[nextCode++];

      // 2. Concat previous string with first of W (V = prev + W[0])
      span->offset = previous;
      span->length = start - previous + 1;

      // Mark the start of previous output string
      previous = start;
    }
  }

  // ...
  mem_delete(dict);

  // The user is responsible to free the output.
  *out_outputSize = output.offset;
  return output.storage;
}

// --------------------------------
//  WRITABLE BUFFER IMPLEMENTATION
// --------------------------------

void buffer_init(struct buffer_t* buffer)
{
  buffer->storage  = mem_alloc(u8, 8192);
  buffer->capacity = 1024;
  buffer->offset   = 0;
}

void buffer_free(struct buffer_t* buffer)
{
  if (buffer->storage)
  {
    mem_delete(buffer->storage);
    buffer->storage  = NULL;
    buffer->capacity = 0;
    buffer->offset   = 0;
  }
}

void buffer_write(struct buffer_t* buffer, const u8* data, usize dataLength)
{
  // No data, no work.
  if (dataLength == 0)
    return;

  // If we would write beyond the buffer, resize to ~2x the capacity.
  const usize end = buffer->offset + dataLength;
  if (end >= buffer->capacity)
    mem_realloc(buffer->storage, buffer->capacity = (end + buffer->capacity));

  // // Copy into the buffer.
  // while (dataLength--)
  //   buffer->storage[buffer->offset++] = *data++;

  // Copy into the buffer.
  memcpy(&buffer->storage[buffer->offset], data, dataLength);
  buffer->offset += dataLength;
}

void buffer_write_u16(struct buffer_t* buffer, u16 value)
{
  buffer_write(buffer, (u8*) &value, sizeof(u16));
}

void buffer_write_u8(struct buffer_t* buffer, u8 value)
{
  buffer_write(buffer, &value, sizeof(u8));
}
