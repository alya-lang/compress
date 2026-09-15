#ifndef ALYA_COMPRESS_LZ_H
#define ALYA_COMPRESS_LZ_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ALYA_LZ_FORMAT_LZ77  = 0,
    ALYA_LZ_FORMAT_LZ78  = 1,
    ALYA_LZ_FORMAT_LZW   = 2,
    ALYA_LZ_FORMAT_LZ4   = 3,
    ALYA_LZ_FORMAT_LZSS  = 4,
    ALYA_LZ_FORMAT_LZMA  = 5,
    ALYA_LZ_FORMAT_LZMA2 = 6,
    ALYA_LZ_FORMAT_LZJB  = 7
};

int alya_lz_compress(int format, const uint8_t *in, size_t in_len, uint8_t **out_buf, size_t *out_len);
int alya_lz_decompress(int format, const uint8_t *in, size_t in_len, uint8_t **out_buf, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* ALYA_COMPRESS_LZ_H */
