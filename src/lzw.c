#include "lzw.h"

#include <string.h>

// -----------------
//  WRITABLE BUFFER
// -----------------

// ...
typedef struct
{
  u8*   storage;
  usize capacity;
  usize offset;
} buffer_t;

// ...
void buffer_init(buffer_t* buffer);

// ...
void buffer_free(buffer_t* buffer);

// ...
void buffer_write(buffer_t* buffer, u8* data, usize dataLength);

// ...
void buffer_write_u16(buffer_t* buffer, u16 value);

// -------------------
//  DICTIONARY (TRIE)
// -------------------

static const usize INVALID_CHILD = (usize) -1;

// ...
typedef struct
{
  usize       children[256];
  const void* value;
} dict_node_t;

// ...
typedef struct
{
  dict_node_t* nodes;
  usize        capacity;
  usize        count;
} dict_t;

// ...
void dict_init(dict_t* dict);

// ...
void dict_free(dict_t* dict);

// ...
dict_node_t* dict_node_init(dict_t* dict);

// --------------------
//  LZW IMPLEMENTATION
// --------------------

static const u16 RESET_CODE = (u16) -1;

u8* lzw_encode(u8* input, usize inputSize, usize* out_encodedSize)
{
  // TODO: LZW ENCODING ALGORITHM
  // 1. Initialize dictionary to have all strings length 1 (bytes 0-255)
  // 2. Find longest string W in the dictionary that matches the current input
  // 3. Emit dictionary index for W to output and remove W from input
  // 4. Add W followed by the next symbol in the input to the dictionary
  // 5. Go to Step 2

  // A resizable output buffer/stream.
  buffer_t output;
  buffer_init(&output);

  // A trie based dictionary.
  dict_t dict;
  dict_init(&dict);

  // (STEP 1) Initialize the dictionary with the single byte strings.
  for (usize i = 0; i < 256; i++)
  {
    // Retrieve a new dictionary nodes.
    dict_node_t* node = dict_node_init(&dict);
    node->value       = (void*) (usize) (dict.count - 1);
  }

  // This cursor tracks a position within the dictionary.
  dict_node_t* cursor = NULL;

  // Scan over the input content, byte-by-byte.
  const u8* inputEnd = input + inputSize;
  while (input != inputEnd) // (STEP 5 -> 2)
  {
    // Read the next byte.
    const u8 symbol = *input++;

    // Get the next node index for the current input.
    usize nextIndex = (cursor == NULL) ? symbol : cursor->children[symbol];

    // If the next index is not defined, this is the longest sequence we have.
    if (nextIndex == INVALID_CHILD)
    {
      // (STEP 3) Found longest existing match W, emit code.

      const u16 code = (u16) (usize) cursor->value;
      buffer_write_u16(&output, code);

      // (STEP 4) Add the sequence W + symbol to the dictionary.

      // If we've reached the max dictionary size, we cannot add more nodes.
      if ((dict.count + 1) == RESET_CODE)
      {
        // TODO: RESET DICTIONARY UPON REACHING CODE
        dict.count = 256;
      }

      // Point the current node to the upcoming node.
      cursor->children[symbol] = dict.count + 1;

      // Get the next dictionary node, this may cause reallocation and invalid cursor.
      dict_node_t* node = dict_node_init(&dict);
      node->value       = (void*) (usize) (dict.count - 1);

      // Begin matching new sequence.
      cursor = NULL;
    }
    else
    {
      // Matching a longer input string.
      cursor = &dict.nodes[nextIndex];
    }
  }

  // Cleanup
  dict_free(&dict);

  // The user is responsible to free the output.
  *out_encodedSize = output.offset;
  return output.storage;
}

u8* lzw_decode(u8* data, usize dataSize, usize* out_decodedSize)
{
  *out_decodedSize = dataSize;
  data             = mem_alloc(u8, dataSize);

  // TODO: LZW DECODING ALGORITHM
  // 1. Initialize dictionary to ahve all strings length 1 (bytes 0-255)
  // 2. Read the next encoded symbol. Is it in the dictionary?
  //    - Yes
  //      1. Emit corresponding W to output.
  //      2. Concatenate the previous string emitted to output with the first symbol of W. Add this to dictionary.
  //    - No
  //      1. Concatenate the previous string emitted to output with its first symbol. Call this string V.
  //      2. Add V to the dictionary and emit V to output.
  // 3. Repeat 2

  return data;
}

// --------------------------------
//  WRITABLE BUFFER IMPLEMENTATION
// --------------------------------

void buffer_init(buffer_t* buffer)
{
  buffer->storage  = mem_alloc(u8, 1024);
  buffer->capacity = 1024;
  buffer->offset   = 0;
}

void buffer_free(buffer_t* buffer)
{
  if (buffer->storage)
  {
    mem_delete(buffer->storage);
    buffer->storage  = NULL;
    buffer->capacity = 0;
    buffer->offset   = 0;
  }
}

void buffer_write(buffer_t* buffer, u8* data, usize dataLength)
{
  // If we would write beyond the buffer, resize to ~2x the capacity.
  const usize end = buffer->offset + dataLength;
  if (end >= buffer->capacity)
    mem_realloc(buffer->storage, buffer->capacity = (end + buffer->capacity));

  // Copy into the buffer.
  memcpy(&buffer->storage[buffer->offset], data, dataLength);
  buffer->offset += dataLength;
}

void buffer_write_u16(buffer_t* buffer, u16 value)
{
  buffer_write(buffer, (u8*) &value, sizeof(u16));
}

// ----------------------------------
//  DICTIONARY (TRIE) IMPLEMENTATION
// ----------------------------------

void dict_init(dict_t* dict)
{
  // Allocate memory for 256 nodes by default.
  dict->nodes    = mem_alloc(dict_node_t, 1);
  dict->capacity = 1;
  dict->count    = 0;
}

void dict_free(dict_t* dict)
{
  // Free the node memory
  if (dict->nodes)
    mem_delete(dict->nodes);

  // Invalidate dictionary values
  dict->nodes    = NULL;
  dict->capacity = 0;
  dict->count    = 0;
}

dict_node_t* dict_node_init(dict_t* dict)
{
  // If we are at capacity, we will need to reallocate the memory.
  if (dict->count >= dict->capacity)
    mem_realloc(dict->nodes, dict->capacity *= 2);

  // Get the next available node.
  dict_node_t* node = &dict->nodes[dict->count++];

  // Zero out values.
  node->value = NULL;
  for (usize i = 0; i < 256; i++)
    node->children[i] = INVALID_CHILD;

  // Return the initialized node.
  return node;
}
