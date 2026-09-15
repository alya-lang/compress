#ifndef ALYA_COMPRESS_H
#define ALYA_COMPRESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- Bounds Calculation --- */
size_t alya_deflate_bound(size_t src_len);
size_t alya_zlib_bound(size_t src_len);
size_t alya_gzip_bound(size_t src_len);
size_t alya_lz4_bound(size_t src_len);
size_t alya_snappy_bound(size_t src_len);

/* --- Size Queries --- */
int alya_snappy_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len);
int alya_gzip_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len);

/* --- Compression --- */
int alya_deflate_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_zlib_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_gzip_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_lz4_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_snappy_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);

/* --- Fixed-Buffer Decompression --- */
int alya_deflate_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_zlib_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_gzip_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_lz4_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_snappy_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);

/* --- Auto-Allocating Dynamic Decompression --- */
int alya_deflate_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_zlib_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_gzip_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_lz4_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_snappy_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);

/* --- Memory Buffer Deallocator --- */
void alya_free_buffer(void *ptr);

/* --- Checksums --- */
uint32_t alya_crc32(uint32_t crc, const uint8_t *buf, size_t len);
uint32_t alya_adler32(uint32_t adler, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* ALYA_COMPRESS_H */