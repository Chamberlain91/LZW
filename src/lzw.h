#ifndef LZW_H
#define LZW_H

#include "standard.h"

// Encodes a binary blob with LZW compression.
u8* lzw_encode(u8* data, usize dataSize, usize* out_encodedSize);

// Decodes a LZW encoded binary blob.
u8* lzw_decode(u8* data, usize dataSize, usize* out_decodedSize);

#endif
