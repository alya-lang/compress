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
size_t alya_zstd_bound(size_t src_len);
size_t alya_bzip2_bound(size_t src_len);
size_t alya_brotli_bound(size_t src_len);

/* --- Size Queries --- */
int alya_snappy_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len);
int alya_gzip_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len);
int alya_zstd_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len);

/* --- Compression --- */
int alya_deflate_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_zlib_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_gzip_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_lz4_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_snappy_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_zstd_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_bzip2_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level);
int alya_brotli_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int quality);

/* --- Fixed-Buffer Decompression --- */
int alya_deflate_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_zlib_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_gzip_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_lz4_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_snappy_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_zstd_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_bzip2_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);
int alya_brotli_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len);

/* --- Auto-Allocating Dynamic Decompression --- */
int alya_deflate_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_zlib_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_gzip_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_lz4_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_snappy_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_zstd_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_bzip2_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);
int alya_brotli_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len);

/* --- SZIP Archive Management --- */
int alya_szip_create_archive(const char *zip_filename);
int alya_szip_add_mem(const char *zip_filename, const char *archive_name, const uint8_t *data, size_t data_len, int level);
int alya_szip_get_num_files(const char *zip_filename);
int alya_szip_get_filename(const char *zip_filename, int file_index, char *out_name, size_t max_name_len);
int alya_szip_extract_to_mem(const char *zip_filename, const char *archive_name, uint8_t **out_buf, size_t *out_len);
int alya_szip_extract_to_file(const char *zip_filename, const char *archive_name, const char *dst_path);

/* --- Unified LZ Codecs (LZ77, LZ78, LZW, LZ4, LZSS, LZMA, LZMA2, LZJB) --- */
int alya_lz_compress(int format, const uint8_t *in, size_t in_len, uint8_t **out_buf, size_t *out_len);
int alya_lz_decompress(int format, const uint8_t *in, size_t in_len, uint8_t **out_buf, size_t *out_len);

/* --- Memory Buffer Deallocator --- */
void alya_free_buffer(void *ptr);

/* --- Checksums --- */
uint32_t alya_crc32(uint32_t crc, const uint8_t *buf, size_t len);
uint32_t alya_adler32(uint32_t adler, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* ALYA_COMPRESS_H */