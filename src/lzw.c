#include "lzw.h"

#include <ctype.h>
#include <string.h>

// -------------------
//  WRITE-ONLY BUFFER
// -------------------

struct buffer_t
{
  u8*   storage;
  usize capacity;
  usize offset;
};

void buffer_init(struct buffer_t* buffer, usize capacity);
void buffer_free(struct buffer_t* buffer);
void buffer_write_u16(struct buffer_t* buffer, u16 value);
void buffer_write_u8(struct buffer_t* buffer, u8 value);
void buffer_write(struct buffer_t* buffer, const u8* data, usize dataSize);

// --------------------
//  LZW IMPLEMENTATION
// --------------------

static const u32 MAX_CODE_COUNT = 1 << 16;

u8* lzw_encode(const u8* input, const usize inputSize, usize* out_outputSize)
{
  // LZW ENCODING ALGORITHM
  // TODO: WRITE BETTER ALGORITHM STEPS

  // A trie like structure.
  struct node_t
  {
    // Points to the next node for any particular symbol.
    struct node_t* next[256];
  };

  // TODO: VALIDATE / ERROR CONDITIONS
  // error: inputSize == 0
  // error: input == NULL
  // error: outputSize == NULL ?

  // Computes the end of the input.
  const u8* const inputEnd = input + inputSize;

  // Initialize output stream.
  struct buffer_t output;
  buffer_init(&output, inputSize);

  // Initialize dictionary.
  struct node_t* dict = mem_alloc(struct node_t, MAX_CODE_COUNT);
  mem_clear(dict, MAX_CODE_COUNT);

  // Keeps track of the next code to emit.
  usize nextCode = 256;

  // We iterate until we have processed all content.
  while (input < inputEnd)
  {
    // Read the longest known sequence of symbols.
    struct node_t* node = &dict[*input++];
    while ((input < inputEnd) && node->next[*input] != NULL)
      node = node->next[*input++];

    // Emit code for this sequence.
    buffer_write_u16(&output, (u16) (node - dict));

    // If there is more data to encode.
    if (input < inputEnd)
    {
      // Update dictionary, concatenate previous string with new symbol.
      node->next[*input] = &dict[nextCode++];

      // Dictionary is full, reset it to begin encoding a new chunk.
      if (nextCode == MAX_CODE_COUNT)
      {
        // printf("RESET CODES\n");
        mem_clear(dict, MAX_CODE_COUNT);
        nextCode = 256;
      }
    }
  }

  // Free the dictionary memory.
  mem_delete(dict);

  // The user is responsible to free the output.
  *out_outputSize = output.offset;
  return output.storage;
}

u8* lzw_decode(const u8* input, usize inputSize, usize* out_outputSize)
{
  // LZW DECODING ALGORITHM
  // TODO: WRITE BETTER ALGORITHM STEPS

  // A span of bytes, to associate codes with a byte-string in the output.
  struct span_t
  {
    // Offset into the output buffer.
    usize offset;

    // Length of the span in bytes.
    usize length;
  };

  // TODO: VALIDATE / ERROR CONDITIONS
  // error: inputSize == 0
  // error: input == NULL
  // error: outputSize == NULL ?

  // View the input as 16-bit codes.
  const u16* codesEnd = (const u16*) (input + inputSize);
  const u16* codes    = (const u16*) (input);

  // A resizable output buffer/stream.
  struct buffer_t output;
  buffer_init(&output, inputSize);

  // Allocate and prepare dictionary.
  struct span_t* dict = mem_alloc(struct span_t, MAX_CODE_COUNT);
  mem_clear(dict, MAX_CODE_COUNT);
  usize nextCode = 256;

  // First code is always an alphabet code.
  dict[*codes] = (struct span_t) {.offset = 0, .length = 1};
  buffer_write_u8(&output, (u8) *codes++);

  // Start of the previous string output.
  usize previous = 0;

  // While we have symbols to read
  while (codes < codesEnd)
  {
    // Read the next encoded symbol
    const u16 code = *codes++;

    // Start of the current output string
    const usize start = output.offset;

    // If the code is an alphabet character...
    if (code < 256)
    {
      // Write the alphabet byte to the output.
      buffer_write_u8(&output, (u8) code);
    }
    // Was not an alphabet character...
    else
    {
      // Get the relevant entry for the current code
      const struct span_t* entry = &dict[code];

      // If the code is a known dictionary entry...
      if (entry->length > 0)
      {
        // Output the known dictionary entry.
        buffer_write(&output, &output.storage[entry->offset], entry->length);
      }
      // The code is an unknown code...
      else
      {
        // Writes "previous + current[0]" to the output.
        buffer_write(&output, &output.storage[previous], start - previous);
        buffer_write(&output, &output.storage[previous], 1);
      }
    }

    // Construct the next entry for "previous + current[0]".
    dict[nextCode++] = (struct span_t) {
      .offset = previous,
      .length = start - previous + 1,
    };

    // Dictionary is full, reset it to begin encoding a new chunk.
    if (nextCode == MAX_CODE_COUNT)
    {
      mem_clear(dict, MAX_CODE_COUNT);
      nextCode = 256;
    }

    // The current output string is now the previous output for the next iteration.
    previous = start;
  }

  // Free the dictionary memory.
  mem_delete(dict);

  // The user is responsible to free the output.
  *out_outputSize = output.offset;
  return output.storage;
}

// --------------------------------
//  WRITABLE BUFFER IMPLEMENTATION
// --------------------------------

void buffer_init(struct buffer_t* buffer, usize capacity)
{
  buffer->capacity = capacity;
  buffer->storage  = mem_alloc(u8, buffer->capacity);
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

void buffer_write(struct buffer_t* buffer, const u8* data, usize dataSize)
{
  // No data, no work.
  if (dataSize == 0)
    return;

  // If we would write beyond the buffer, resize to ~2x the capacity.
  const usize offsetEnd = buffer->offset + dataSize;
  if (offsetEnd >= buffer->capacity)
  {
    // Create temporary copy of the data.
    u8* temp = (u8*) mem_stack_alloc(u8, dataSize);
    memcpy(temp, data, dataSize);
    data = temp;

    // Reallocate the buffer storage.
    buffer->capacity = offsetEnd + buffer->capacity;
    buffer->storage  = mem_realloc(buffer->storage, buffer->capacity);
  }

  // Copy into the buffer.
  memcpy(&buffer->storage[buffer->offset], data, dataSize);
  buffer->offset += dataSize;
}

void buffer_write_u16(struct buffer_t* buffer, u16 value)
{
  buffer_write(buffer, (u8*) &value, sizeof(u16));
}

void buffer_write_u8(struct buffer_t* buffer, u8 value)
{
  buffer_write(buffer, &value, sizeof(u8));
}
