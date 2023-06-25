#include "lzw.h"

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

void buffer_init(struct buffer_t* buffer);
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
  buffer_init(&output);

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

    // If there are no more available codes, reset the dictionary.
    if (nextCode == MAX_CODE_COUNT)
    {
      mem_clear(dict, MAX_CODE_COUNT);
      nextCode = 256;
    }

    // Update dictionary, concatenate previous string with new symbol.
    node->next[*input] = &dict[nextCode++];
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
  mem_clear(dict, MAX_CODE_COUNT);

  // ...
  usize nextCode = 256;
  usize prevCode = 0;

  // While we have symbols to read.
  while (symbols < symbolsEnd)
  {
    // Read the next encoded symbol.
    const u16 symbol = *symbols++;

    // ...
    const usize start = output.offset;

    // ...
    struct span_t* entry = &dict[symbol];

    if (entry->length > 0) // known code
    {
      if (prevCode == symbol)
      {
        // Write the partially known substring to the output + first character of this string.
        buffer_write(&output, &output.storage[entry->offset], entry->length - 1);
        buffer_write(&output, &output.storage[entry->offset], 1);
      }
      else
      {
        // Write the known substring to output.
        buffer_write(&output, &output.storage[entry->offset], entry->length);
      }
    }
    else if (symbol < 256) // unknown alphabet code
    {
      // Special Case: An "alphabet" string of length one.
      buffer_write_u8(&output, (u8) symbol);

      // Mark the most recent version of the "alphabet" string.
      entry->offset = start;
      entry->length = output.offset - start;
    }
    else // unknown code
    {
      io_printError("UNKNOWN CODE?");
      exit(64);
    }

    // If there are no more available codes...
    if (nextCode == MAX_CODE_COUNT)
    {
      // (Re)Initialize Dictionary
      mem_clear(dict, MAX_CODE_COUNT);
      nextCode = 256;
    }

    // Stores a new entry for the current string + first symbol of the next.
    struct span_t* new_entry = &dict[prevCode = nextCode++];
    new_entry->offset        = start;
    new_entry->length        = (output.offset - start) + 1;
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
  buffer->capacity = 4096;
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
