#ifndef LZW_H
#define LZW_H

#include "standard.h"

// Encodes a binary blob with LZW compression.
u8* lzw_encode(const u8* data, const usize dataLength, usize* out_encodedSize);

// Decodes a LZW encoded binary blob.
u8* lzw_decode(const u8* data, const usize dataLength, usize* out_decodedSize);

#endif
