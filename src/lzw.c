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
static const u16 INVALID_INDEX  = MAX_CODE_COUNT - 1;
static const u16 ALPHABET_SIZE  = 256;

u8* lzw_encode(const u8* input, const usize inputSize, usize* out_outputSize)
{
  // LZW ENCODING ALGORITHM
  // TODO: WRITE BETTER ALGORITHM STEPS

  // ...
  struct entry_t
  {
    u16 character;
    u16 prefix;
    // Binary Tree
    u16 first;
    u16 left;
    u16 right;
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

  // Allocate dictionary memory.
  struct entry_t* dict = mem_alloc(struct entry_t, MAX_CODE_COUNT);

  // Initialize the first 256 entries.
  for (u16 i = 0; i < ALPHABET_SIZE; i++)
  {
    dict[i] = (struct entry_t) {
      .character = i,
      .prefix    = INVALID_INDEX,
      .first     = INVALID_INDEX,
      .left      = INVALID_INDEX,
      .right     = INVALID_INDEX,
    };
  }

  // The first available code...?
  usize nextCode = ALPHABET_SIZE;

  // The current string begins with the first character of input.
  u16 prefix = *input++;

inputLoop: // We iterate until we have processed all content.
  while (input < inputEnd)
  {
    // Reads the next byte.
    const u16 character = *input++;

    // Find the current string in dictionary...
    u16 searchIndex = dict[prefix].first;
    if (searchIndex == INVALID_INDEX)
    {
      // Point this prefix string to the first entry that consumes this prefix.
      dict[prefix].first = (u16) nextCode;
    }
    else
    {
      // Traverse the prefix binary tree, searching for desired character.
      while (true)
      {
        struct entry_t* entry = &dict[searchIndex];

        // If we find an exact character match...
        if (character == entry->character)
        {
          // We need to continue reading input until we do not find a prefix entry.
          prefix = searchIndex;
          goto inputLoop;
        }
        else
        {
          // Get the left or right child member dependant on ordinal comparison.
          u16* child = (character < entry->character) ? &entry->left : &entry->right;

          if ((*child) == INVALID_INDEX)
          {
            // Unable to find an entry for the desired character, it will be created.
            *child = (u16) nextCode;
            break;
          }

          // Has a child, so we will continue looking for the desired character.
          searchIndex = *child;
        }
      }
    }

    // We could not find an entry for the current string, create it.
    // Each entry is represented by its prefix entry and suffix character.
    dict[nextCode++] = (struct entry_t) {
      .character = character,
      .prefix    = prefix,
      .first     = INVALID_INDEX,
      .left      = INVALID_INDEX,
      .right     = INVALID_INDEX,
    };

    // Dictionary is full, reset it to begin encoding a new chunk.
    if (nextCode == MAX_CODE_COUNT)
    {
      // Reset the first 256 entries.
      for (u16 i = 0; i < ALPHABET_SIZE; i++)
      {
        dict[i] = (struct entry_t) {
          .character = i,
          .prefix    = INVALID_INDEX,
          .first     = INVALID_INDEX,
          .left      = INVALID_INDEX,
          .right     = INVALID_INDEX,
        };
      }

      // Reset next available code.
      nextCode = ALPHABET_SIZE;
    }

    // Emit code for this sequence.
    buffer_write_u16(&output, prefix);

    // Start matching a new string.
    prefix = character;
  }

  // Outputs the final string.
  buffer_write_u16(&output, prefix);

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

  // Initialize dictionary.
  struct span_t* dict = mem_alloc(struct span_t, MAX_CODE_COUNT);
  mem_clear(dict, MAX_CODE_COUNT, 0x0);
  usize nextCode = ALPHABET_SIZE;

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
    if (code < ALPHABET_SIZE)
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
      mem_clear(dict, MAX_CODE_COUNT, 0x0);
      nextCode = ALPHABET_SIZE;
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
