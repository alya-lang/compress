#include "compress.h"
#include "miniz.h"
#include "lz4.h"
#include "snappy.h"
#include "zstd.h"
#include "bzip2/bzlib.h"
#include "brotli/encode.h"
#include "brotli/decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Bounds Calculation --- */

size_t alya_deflate_bound(size_t src_len) {
    return (size_t)mz_compressBound((mz_ulong)src_len) + 64;
}

size_t alya_zlib_bound(size_t src_len) {
    return (size_t)mz_compressBound((mz_ulong)src_len);
}

size_t alya_gzip_bound(size_t src_len) {
    return (size_t)mz_compressBound((mz_ulong)src_len) + 64 + 18;
}

size_t alya_lz4_bound(size_t src_len) {
    return (size_t)LZ4_compressBound((int)src_len) + 4;
}

size_t alya_snappy_bound(size_t src_len) {
    return snappy_max_compressed_length(src_len);
}

size_t alya_zstd_bound(size_t src_len) {
    return ZSTD_compressBound(src_len);
}

size_t alya_bzip2_bound(size_t src_len) {
    return src_len + (src_len / 100) + 600;
}

size_t alya_brotli_bound(size_t src_len) {
    return BrotliEncoderMaxCompressedSize(src_len);
}

/* --- Size Queries --- */

int alya_snappy_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len) {
    if (!src || !out_len || src_len == 0) {
        return -1;
    }
    bool ok = snappy_uncompressed_length((const char *)src, src_len, out_len);
    return ok ? 0 : -1;
}

int alya_gzip_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len) {
    if (!src || !out_len || src_len < 18) {
        return -1;
    }
    if (src[0] != 0x1f || src[1] != 0x8b || src[2] != 0x08) {
        return -2;
    }
    uint32_t isize = (uint32_t)src[src_len - 4] |
                     ((uint32_t)src[src_len - 3] << 8) |
                     ((uint32_t)src[src_len - 2] << 16) |
                     ((uint32_t)src[src_len - 1] << 24);
    *out_len = (size_t)isize;
    return 0;
}

int alya_zstd_uncompressed_len(const uint8_t *src, size_t src_len, size_t *out_len) {
    if (!src || !out_len || src_len == 0) {
        return -1;
    }
    unsigned long long cs = ZSTD_getFrameContentSize(src, src_len);
    if (cs != ZSTD_CONTENTSIZE_UNKNOWN && cs != ZSTD_CONTENTSIZE_ERROR) {
        *out_len = (size_t)cs;
        return 0;
    }
    return -1;
}

/* --- Compression --- */

int alya_deflate_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (level < 0 || level > 9) {
        level = MZ_DEFAULT_COMPRESSION;
    }
    mz_stream stream;
    memset(&stream, 0, sizeof(stream));
    int status = mz_deflateInit2(&stream, level, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY);
    if (status != MZ_OK) {
        return status;
    }

    stream.next_in = src;
    stream.avail_in = (unsigned int)src_len;
    stream.next_out = dst;
    stream.avail_out = (unsigned int)*dst_len;

    status = mz_deflate(&stream, MZ_FINISH);
    if (status == MZ_STREAM_END) {
        *dst_len = (size_t)stream.total_out;
        mz_deflateEnd(&stream);
        return 0;
    }
    mz_deflateEnd(&stream);
    return (status < 0) ? status : -1;
}

int alya_zlib_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (level < 0 || level > 9) {
        level = MZ_DEFAULT_COMPRESSION;
    }
    mz_ulong dlen = (mz_ulong)*dst_len;
    int rc = mz_compress2(dst, &dlen, src, (mz_ulong)src_len, level);
    if (rc == MZ_OK) {
        *dst_len = (size_t)dlen;
        return 0;
    }
    return rc;
}

int alya_gzip_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (*dst_len < 18) {
        return -2;
    }

    /* 1. Standard GZIP 10-byte header */
    dst[0] = 0x1f;
    dst[1] = 0x8b;
    dst[2] = 0x08; /* deflate */
    dst[3] = 0x00; /* flags */
    dst[4] = 0x00; /* mtime */
    dst[5] = 0x00;
    dst[6] = 0x00;
    dst[7] = 0x00;
    dst[8] = 0x00; /* extra flags */
    dst[9] = 0x03; /* OS: Unix */

    /* 2. Deflate payload */
    size_t def_len = *dst_len - 18;
    int rc = alya_deflate_compress(src, src_len, dst + 10, &def_len, level);
    if (rc != 0) {
        return rc;
    }

    /* 3. Footer: CRC-32 (4 bytes LE) + ISIZE (4 bytes LE) */
    uint32_t crc = (uint32_t)mz_crc32(MZ_CRC32_INIT, src, src_len);
    uint8_t *footer = dst + 10 + def_len;
    footer[0] = (uint8_t)(crc & 0xff);
    footer[1] = (uint8_t)((crc >> 8) & 0xff);
    footer[2] = (uint8_t)((crc >> 16) & 0xff);
    footer[3] = (uint8_t)((crc >> 24) & 0xff);

    uint32_t isize = (uint32_t)(src_len & 0xffffffff);
    footer[4] = (uint8_t)(isize & 0xff);
    footer[5] = (uint8_t)((isize >> 8) & 0xff);
    footer[6] = (uint8_t)((isize >> 16) & 0xff);
    footer[7] = (uint8_t)((isize >> 24) & 0xff);

    *dst_len = 10 + def_len + 8;
    return 0;
}

int alya_lz4_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (*dst_len < 4) {
        return -2;
    }
    /* Write 4-byte little-endian uncompressed length prefix */
    dst[0] = (uint8_t)(src_len & 0xff);
    dst[1] = (uint8_t)((src_len >> 8) & 0xff);
    dst[2] = (uint8_t)((src_len >> 16) & 0xff);
    dst[3] = (uint8_t)((src_len >> 24) & 0xff);

    int c_len = LZ4_compress_default((const char *)src, (char *)(dst + 4), (int)src_len, (int)(*dst_len - 4));
    if (c_len <= 0) {
        return -1;
    }
    *dst_len = (size_t)c_len + 4;
    return 0;
}

int alya_snappy_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    struct snappy_env env;
    if (snappy_init_env(&env) != 0) {
        return -1;
    }
    size_t out_len = *dst_len;
    int rc = snappy_compress(&env, (const char *)src, src_len, (char *)dst, &out_len);
    snappy_free_env(&env);
    if (rc == 0) {
        *dst_len = out_len;
        return 0;
    }
    return -2;
}

int alya_zstd_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (level <= 0) {
        level = 3;
    }
    size_t c_size = ZSTD_compress(dst, *dst_len, src, src_len, level);
    if (ZSTD_isError(c_size)) {
        return -2;
    }
    *dst_len = c_size;
    return 0;
}

int alya_bzip2_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int level) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (level < 1 || level > 9) {
        level = 9;
    }
    unsigned int d_len = (unsigned int)*dst_len;
    int rc = BZ2_bzBuffToBuffCompress((char *)dst, &d_len, (char *)src, (unsigned int)src_len, level, 0, 30);
    if (rc == BZ_OK) {
        *dst_len = (size_t)d_len;
        return 0;
    }
    return rc;
}

int alya_brotli_compress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len, int quality) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (quality < 0 || quality > 11) {
        quality = BROTLI_DEFAULT_QUALITY;
    }
    BROTLI_BOOL ok = BrotliEncoderCompress(quality, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE,
                                           src_len, src, dst_len, dst);
    return ok ? 0 : -1;
}

/* --- Fixed-Buffer Decompression --- */

int alya_deflate_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    mz_stream stream;
    memset(&stream, 0, sizeof(stream));
    int status = mz_inflateInit2(&stream, -MZ_DEFAULT_WINDOW_BITS);
    if (status != MZ_OK) {
        return status;
    }

    stream.next_in = src;
    stream.avail_in = (unsigned int)src_len;
    stream.next_out = dst;
    stream.avail_out = (unsigned int)*dst_len;

    status = mz_inflate(&stream, MZ_FINISH);
    if (status == MZ_STREAM_END) {
        *dst_len = (size_t)stream.total_out;
        mz_inflateEnd(&stream);
        return 0;
    }
    mz_inflateEnd(&stream);
    return (status < 0) ? status : -1;
}

int alya_zlib_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    mz_ulong dlen = (mz_ulong)*dst_len;
    int rc = mz_uncompress(dst, &dlen, src, (mz_ulong)src_len);
    if (rc == MZ_OK) {
        *dst_len = (size_t)dlen;
        return 0;
    }
    return rc;
}

int alya_gzip_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (src_len < 18) {
        return -1;
    }
    if (src[0] != 0x1f || src[1] != 0x8b || src[2] != 0x08) {
        return -2;
    }

    uint8_t flg = src[3];
    size_t pos = 10;

    if (flg & 0x04) {
        if (pos + 2 > src_len) return -3;
        uint16_t xlen = (uint16_t)(src[pos] | ((uint16_t)src[pos + 1] << 8));
        pos += 2 + xlen;
    }
    if (flg & 0x08) {
        while (pos < src_len && src[pos] != 0) pos++;
        pos++;
    }
    if (flg & 0x10) {
        while (pos < src_len && src[pos] != 0) pos++;
        pos++;
    }
    if (flg & 0x02) {
        pos += 2;
    }

    if (pos + 8 > src_len) {
        return -3;
    }

    size_t comp_len = (src_len - 8) - pos;
    int rc = alya_deflate_decompress(src + pos, comp_len, dst, dst_len);
    if (rc != 0) {
        return rc;
    }

    uint32_t expected_crc = (uint32_t)src[src_len - 8] |
                            ((uint32_t)src[src_len - 7] << 8) |
                            ((uint32_t)src[src_len - 6] << 16) |
                            ((uint32_t)src[src_len - 5] << 24);
    uint32_t actual_crc = (uint32_t)mz_crc32(MZ_CRC32_INIT, dst, *dst_len);
    if (expected_crc != actual_crc) {
        return -4;
    }

    return 0;
}

int alya_lz4_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    if (src_len < 4) {
        return -2;
    }
    int d_len = LZ4_decompress_safe((const char *)(src + 4), (char *)dst, (int)(src_len - 4), (int)*dst_len);
    if (d_len < 0) {
        return -1;
    }
    *dst_len = (size_t)d_len;
    return 0;
}

int alya_snappy_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    size_t expected_len = 0;
    if (snappy_uncompressed_length((const char *)src, src_len, &expected_len) == false) {
        return -1;
    }
    if (*dst_len < expected_len) {
        return -2;
    }
    int rc = snappy_uncompress((const char *)src, src_len, (char *)dst);
    if (rc == 0) {
        *dst_len = expected_len;
        return 0;
    }
    return -3;
}

int alya_zstd_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    size_t d_size = ZSTD_decompress(dst, *dst_len, src, src_len);
    if (ZSTD_isError(d_size)) {
        return -2;
    }
    *dst_len = d_size;
    return 0;
}

int alya_bzip2_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    unsigned int d_len = (unsigned int)*dst_len;
    int rc = BZ2_bzBuffToBuffDecompress((char *)dst, &d_len, (char *)src, (unsigned int)src_len, 0, 0);
    if (rc == BZ_OK) {
        *dst_len = (size_t)d_len;
        return 0;
    }
    return rc;
}

int alya_brotli_decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t *dst_len) {
    if (!src || !dst || !dst_len) {
        return -1;
    }
    BrotliDecoderResult res = BrotliDecoderDecompress(src_len, src, dst_len, dst);
    return (res == BROTLI_DECODER_RESULT_SUCCESS) ? 0 : -1;
}

/* --- Auto-Allocating Dynamic Decompression --- */

int alya_deflate_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len == 0) {
        return -1;
    }
    size_t cap = src_len > 1024 ? src_len * 3 : 4096;
    uint8_t *buf = (uint8_t *)malloc(cap + 1);
    if (!buf) {
        return -5;
    }

    mz_stream stream;
    memset(&stream, 0, sizeof(stream));
    int status = mz_inflateInit2(&stream, -MZ_DEFAULT_WINDOW_BITS);
    if (status != MZ_OK) {
        free(buf);
        return status;
    }

    stream.next_in = src;
    stream.avail_in = (unsigned int)src_len;

    while (1) {
        stream.next_out = buf + stream.total_out;
        stream.avail_out = (unsigned int)(cap - stream.total_out);

        status = mz_inflate(&stream, MZ_FINISH);
        if (status == MZ_STREAM_END) {
            break;
        }
        if (status == MZ_OK || status == MZ_BUF_ERROR) {
            size_t new_cap = cap * 2;
            uint8_t *new_buf = (uint8_t *)realloc(buf, new_cap + 1);
            if (!new_buf) {
                mz_inflateEnd(&stream);
                free(buf);
                return -5;
            }
            buf = new_buf;
            cap = new_cap;
            continue;
        }
        mz_inflateEnd(&stream);
        free(buf);
        return (status < 0) ? status : -1;
    }

    buf[stream.total_out] = '\0';
    *out_len = (size_t)stream.total_out;
    mz_inflateEnd(&stream);
    *out_buf = buf;
    return 0;
}

int alya_zlib_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len < 2) {
        return -1;
    }
    size_t cap = src_len > 1024 ? src_len * 3 : 4096;
    uint8_t *buf = (uint8_t *)malloc(cap + 1);
    if (!buf) {
        return -5;
    }

    mz_stream stream;
    memset(&stream, 0, sizeof(stream));
    int status = mz_inflateInit2(&stream, MZ_DEFAULT_WINDOW_BITS);
    if (status != MZ_OK) {
        free(buf);
        return status;
    }

    stream.next_in = src;
    stream.avail_in = (unsigned int)src_len;

    while (1) {
        stream.next_out = buf + stream.total_out;
        stream.avail_out = (unsigned int)(cap - stream.total_out);

        status = mz_inflate(&stream, MZ_FINISH);
        if (status == MZ_STREAM_END) {
            break;
        }
        if (status == MZ_OK || status == MZ_BUF_ERROR) {
            size_t new_cap = cap * 2;
            uint8_t *new_buf = (uint8_t *)realloc(buf, new_cap + 1);
            if (!new_buf) {
                mz_inflateEnd(&stream);
                free(buf);
                return -5;
            }
            buf = new_buf;
            cap = new_cap;
            continue;
        }
        mz_inflateEnd(&stream);
        free(buf);
        return (status < 0) ? status : -1;
    }

    buf[stream.total_out] = '\0';
    *out_len = (size_t)stream.total_out;
    mz_inflateEnd(&stream);
    *out_buf = buf;
    return 0;
}

int alya_gzip_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len < 18) {
        return -1;
    }
    if (src[0] != 0x1f || src[1] != 0x8b || src[2] != 0x08) {
        return -2;
    }

    uint8_t flg = src[3];
    size_t pos = 10;
    if (flg & 0x04) {
        if (pos + 2 > src_len) return -3;
        uint16_t xlen = (uint16_t)(src[pos] | ((uint16_t)src[pos + 1] << 8));
        pos += 2 + xlen;
    }
    if (flg & 0x08) {
        while (pos < src_len && src[pos] != 0) pos++;
        pos++;
    }
    if (flg & 0x10) {
        while (pos < src_len && src[pos] != 0) pos++;
        pos++;
    }
    if (flg & 0x02) {
        pos += 2;
    }

    if (pos + 8 > src_len) {
        return -3;
    }

    size_t comp_len = (src_len - 8) - pos;
    int rc = alya_deflate_decompress_auto(src + pos, comp_len, out_buf, out_len);
    if (rc != 0) {
        return rc;
    }

    /* Verify CRC-32 */
    uint32_t expected_crc = (uint32_t)src[src_len - 8] |
                            ((uint32_t)src[src_len - 7] << 8) |
                            ((uint32_t)src[src_len - 6] << 16) |
                            ((uint32_t)src[src_len - 5] << 24);
    uint32_t actual_crc = (uint32_t)mz_crc32(MZ_CRC32_INIT, *out_buf, *out_len);
    if (expected_crc != actual_crc) {
        free(*out_buf);
        *out_buf = NULL;
        *out_len = 0;
        return -4;
    }

    return 0;
}

int alya_lz4_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len < 4) {
        return -1;
    }
    uint32_t orig_len = (uint32_t)src[0] |
                        ((uint32_t)src[1] << 8) |
                        ((uint32_t)src[2] << 16) |
                        ((uint32_t)src[3] << 24);
    uint8_t *buf = (uint8_t *)malloc(orig_len + 1);
    if (!buf) {
        return -5;
    }
    int d_len = LZ4_decompress_safe((const char *)(src + 4), (char *)buf, (int)(src_len - 4), (int)orig_len);
    if (d_len < 0) {
        free(buf);
        return -1;
    }
    buf[d_len] = '\0';
    *out_len = (size_t)d_len;
    *out_buf = buf;
    return 0;
}

int alya_snappy_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len == 0) {
        return -1;
    }
    size_t expected_len = 0;
    if (snappy_uncompressed_length((const char *)src, src_len, &expected_len) == false) {
        return -1;
    }
    uint8_t *buf = (uint8_t *)malloc(expected_len + 1);
    if (!buf) {
        return -5;
    }
    int rc = snappy_uncompress((const char *)src, src_len, (char *)buf);
    if (rc != 0) {
        free(buf);
        return -2;
    }
    buf[expected_len] = '\0';
    *out_len = expected_len;
    *out_buf = buf;
    return 0;
}

int alya_zstd_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len == 0) {
        return -1;
    }
    unsigned long long content_size = ZSTD_getFrameContentSize(src, src_len);
    size_t cap;
    if (content_size != ZSTD_CONTENTSIZE_UNKNOWN && content_size != ZSTD_CONTENTSIZE_ERROR) {
        cap = (size_t)content_size;
    } else {
        cap = src_len * 4 + 4096;
    }
    uint8_t *buf = (uint8_t *)malloc(cap + 1);
    if (!buf) {
        return -5;
    }

    size_t d_size = ZSTD_decompress(buf, cap, src, src_len);
    if (ZSTD_isError(d_size)) {
        free(buf);
        return -2;
    }
    buf[d_size] = '\0';
    *out_buf = buf;
    *out_len = d_size;
    return 0;
}

int alya_bzip2_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len == 0) {
        return -1;
    }
    size_t cap = src_len > 1024 ? src_len * 4 : 4096;
    uint8_t *buf = (uint8_t *)malloc(cap + 1);
    if (!buf) {
        return -5;
    }

    bz_stream strm;
    memset(&strm, 0, sizeof(strm));
    int rc = BZ2_bzDecompressInit(&strm, 0, 0);
    if (rc != BZ_OK) {
        free(buf);
        return rc;
    }

    strm.next_in = (char *)src;
    strm.avail_in = (unsigned int)src_len;

    while (1) {
        strm.next_out = (char *)(buf + strm.total_out_lo32);
        strm.avail_out = (unsigned int)(cap - strm.total_out_lo32);

        rc = BZ2_bzDecompress(&strm);
        if (rc == BZ_STREAM_END) {
            break;
        }
        if (rc == BZ_OK) {
            size_t new_cap = cap * 2;
            uint8_t *new_buf = (uint8_t *)realloc(buf, new_cap + 1);
            if (!new_buf) {
                BZ2_bzDecompressEnd(&strm);
                free(buf);
                return -5;
            }
            buf = new_buf;
            cap = new_cap;
            continue;
        }
        BZ2_bzDecompressEnd(&strm);
        free(buf);
        return rc;
    }

    size_t total = (size_t)strm.total_out_lo32;
    buf[total] = '\0';
    *out_len = total;
    *out_buf = buf;
    BZ2_bzDecompressEnd(&strm);
    return 0;
}

int alya_brotli_decompress_auto(const uint8_t *src, size_t src_len, uint8_t **out_buf, size_t *out_len) {
    if (!src || !out_buf || !out_len || src_len == 0) {
        return -1;
    }
    size_t cap = src_len > 1024 ? src_len * 4 : 4096;
    uint8_t *buf = (uint8_t *)malloc(cap + 1);
    if (!buf) {
        return -5;
    }

    BrotliDecoderState *state = BrotliDecoderCreateInstance(NULL, NULL, NULL);
    if (!state) {
        free(buf);
        return -1;
    }

    size_t avail_in = src_len;
    const uint8_t *next_in = src;
    size_t total_out = 0;

    while (1) {
        size_t avail_out = cap - total_out;
        uint8_t *next_out = buf + total_out;

        BrotliDecoderResult res = BrotliDecoderDecompressStream(state, &avail_in, &next_in, &avail_out, &next_out, &total_out);
        if (res == BROTLI_DECODER_RESULT_SUCCESS) {
            break;
        }
        if (res == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
            size_t new_cap = cap * 2;
            uint8_t *new_buf = (uint8_t *)realloc(buf, new_cap + 1);
            if (!new_buf) {
                BrotliDecoderDestroyInstance(state);
                free(buf);
                return -5;
            }
            buf = new_buf;
            cap = new_cap;
            continue;
        }
        BrotliDecoderDestroyInstance(state);
        free(buf);
        return -1;
    }

    buf[total_out] = '\0';
    *out_len = total_out;
    *out_buf = buf;
    BrotliDecoderDestroyInstance(state);
    return 0;
}

/* --- SZIP Archive Management --- */

int alya_szip_create_archive(const char *zip_filename) {
    if (!zip_filename) {
        return -1;
    }
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_writer_init_file(&zip, zip_filename, 0)) {
        return -1;
    }
    if (!mz_zip_writer_finalize_archive(&zip)) {
        mz_zip_writer_end(&zip);
        return -2;
    }
    if (!mz_zip_writer_end(&zip)) {
        return -3;
    }
    return 0;
}

int alya_szip_add_mem(const char *zip_filename, const char *archive_name, const uint8_t *data, size_t data_len, int level) {
    if (!zip_filename || !archive_name || !data) {
        return -1;
    }
    if (level < 0 || level > 9) {
        level = MZ_DEFAULT_COMPRESSION;
    }
    mz_bool ok = mz_zip_add_mem_to_archive_file_in_place(zip_filename, archive_name, data, data_len, NULL, 0, (mz_uint)level);
    return ok ? 0 : -2;
}

int alya_szip_get_num_files(const char *zip_filename) {
    if (!zip_filename) {
        return -1;
    }
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zip_filename, 0)) {
        return -1;
    }
    int count = (int)mz_zip_reader_get_num_files(&zip);
    mz_zip_reader_end(&zip);
    return count;
}

int alya_szip_get_filename(const char *zip_filename, int file_index, char *out_name, size_t max_name_len) {
    if (!zip_filename || !out_name || file_index < 0) {
        return -1;
    }
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zip_filename, 0)) {
        return -1;
    }
    mz_uint actual = mz_zip_reader_get_filename(&zip, (mz_uint)file_index, out_name, (mz_uint)max_name_len);
    mz_zip_reader_end(&zip);
    return actual > 0 ? 0 : -2;
}

int alya_szip_extract_to_mem(const char *zip_filename, const char *archive_name, uint8_t **out_buf, size_t *out_len) {
    if (!zip_filename || !archive_name || !out_buf || !out_len) {
        return -1;
    }
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zip_filename, 0)) {
        return -1;
    }
    size_t uncomp_size = 0;
    void *p = mz_zip_reader_extract_file_to_heap(&zip, archive_name, &uncomp_size, 0);
    mz_zip_reader_end(&zip);
    if (!p) {
        return -2;
    }
    *out_buf = (uint8_t *)p;
    *out_len = uncomp_size;
    return 0;
}

int alya_szip_extract_to_file(const char *zip_filename, const char *archive_name, const char *dst_path) {
    if (!zip_filename || !archive_name || !dst_path) {
        return -1;
    }
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_reader_init_file(&zip, zip_filename, 0)) {
        return -1;
    }
    int file_idx = mz_zip_reader_locate_file(&zip, archive_name, NULL, 0);
    if (file_idx < 0) {
        mz_zip_reader_end(&zip);
        return -2;
    }
    mz_bool ok = mz_zip_reader_extract_to_file(&zip, (mz_uint)file_idx, dst_path, 0);
    mz_zip_reader_end(&zip);
    return ok ? 0 : -3;
}

/* --- Memory Buffer Deallocator --- */

void alya_free_buffer(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

/* --- Checksums --- */

uint32_t alya_crc32(uint32_t crc, const uint8_t *buf, size_t len) {
    return (uint32_t)mz_crc32((mz_ulong)crc, buf, (mz_ulong)len);
}

uint32_t alya_adler32(uint32_t adler, const uint8_t *buf, size_t len) {
    return (uint32_t)mz_adler32((mz_ulong)adler, buf, (mz_ulong)len);
}