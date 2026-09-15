/* Unified LZ family codecs: LZ77, LZ78, LZW, LZ4, LZSS, LZMA, LZMA2, LZJB
 * Compliant with the VLZ1 stream format.
 */

#include "lz.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define STREAM_MAGIC_0 0x56
#define STREAM_MAGIC_1 0x4C
#define STREAM_MAGIC_2 0x5A
#define STREAM_MAGIC_3 0x31

typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} Buffer;

static void buf_init(Buffer *b, size_t cap) {
    b->cap = (cap > 16) ? cap : 16;
    b->data = (uint8_t *)malloc(b->cap);
    b->len = 0;
}

static void buf_free(Buffer *b) {
    if (b->data) {
        free(b->data);
        b->data = NULL;
    }
    b->len = 0;
    b->cap = 0;
}

static inline void buf_push(Buffer *b, uint8_t v) {
    if (b->len >= b->cap) {
        size_t new_cap = b->cap * 2;
        uint8_t *n = (uint8_t *)realloc(b->data, new_cap);
        if (!n) return;
        b->data = n;
        b->cap = new_cap;
    }
    b->data[b->len++] = v;
}

static inline void buf_append(Buffer *b, const uint8_t *src, size_t len) {
    if (len == 0) return;
    if (b->len + len > b->cap) {
        size_t new_cap = b->cap;
        while (new_cap < b->len + len) new_cap *= 2;
        uint8_t *n = (uint8_t *)realloc(b->data, new_cap);
        if (!n) return;
        b->data = n;
        b->cap = new_cap;
    }
    memcpy(b->data + b->len, src, len);
    b->len += len;
}

static void write_uvarint(Buffer *out, uint64_t value) {
    uint64_t v = value;
    while (v >= 0x80) {
        buf_push(out, (uint8_t)(v & 0x7f) | 0x80);
        v >>= 7;
    }
    buf_push(out, (uint8_t)v);
}

static bool read_uvarint(const uint8_t *data, size_t len, size_t *pos, uint64_t *value) {
    uint64_t out = 0;
    uint32_t shift = 0;
    while (*pos < len && shift <= 63) {
        uint8_t b = data[*pos];
        (*pos)++;
        out |= ((uint64_t)(b & 0x7f)) << shift;
        if ((b & 0x80) == 0) {
            *value = out;
            return true;
        }
        shift += 7;
    }
    return false;
}

typedef struct {
    int window;
    int min_match;
    int max_match;
    int max_literal;
} MatchProfile;

static const MatchProfile PROFILES[] = {
    {4096, 3, 130, 128},  /* 0: LZ77 */
    {0, 0, 0, 0},         /* 1: LZ78 */
    {0, 0, 0, 0},         /* 2: LZW */
    {65535, 4, 130, 128}, /* 3: LZ4 */
    {4096, 3, 130, 128},  /* 4: LZSS */
    {32768, 3, 130, 128}, /* 5: LZMA */
    {65535, 3, 130, 128}, /* 6: LZMA2 */
    {1024, 3, 66, 128}    /* 7: LZJB */
};

#define MATCH_HASH_BITS 16
#define MATCH_HASH_SIZE (1 << MATCH_HASH_BITS)
#define MAX_MATCH_CANDIDATES 64

static inline uint32_t match_hash(const uint8_t *data, size_t pos) {
    uint32_t v = ((uint32_t)data[pos] << 16) | ((uint32_t)data[pos + 1] << 8) | (uint32_t)data[pos + 2];
    return (uint32_t)((v * 2654435761u) >> (32 - MATCH_HASH_BITS));
}

static inline void index_match_position(const uint8_t *data, size_t pos, size_t in_len, int *last_match, int *prev_match) {
    if (pos + 2 >= in_len) return;
    uint32_t h = match_hash(data, pos);
    prev_match[pos] = last_match[h];
    last_match[h] = (int)pos;
}

static void find_best_match(const uint8_t *data, size_t pos, size_t in_len, MatchProfile prof,
                            const int *last_match, const int *prev_match, size_t *out_offset, size_t *out_len) {
    *out_offset = 0;
    *out_len = 0;
    if (pos + prof.min_match > in_len) return;
    size_t max_len = (pos + prof.max_match < in_len) ? (size_t)prof.max_match : (in_len - pos);
    size_t best_len = 0;
    size_t best_off = 0;
    uint32_t h = match_hash(data, pos);
    int candidates = 0;
    int i = last_match[h];
    while (i >= 0 && candidates < MAX_MATCH_CANDIDATES) {
        size_t off = pos - (size_t)i;
        if (off > (size_t)prof.window) break;
        size_t cur = 0;
        while (cur < max_len && data[i + cur] == data[pos + cur]) cur++;
        if (cur > best_len) {
            best_len = cur;
            best_off = off;
            if (best_len == max_len) break;
        }
        i = prev_match[i];
        candidates++;
    }
    *out_offset = best_off;
    *out_len = best_len;
}

static void wrap_payload(Buffer *out, int format, size_t src_len, const uint8_t *payload, size_t payload_len) {
    buf_push(out, STREAM_MAGIC_0);
    buf_push(out, STREAM_MAGIC_1);
    buf_push(out, STREAM_MAGIC_2);
    buf_push(out, STREAM_MAGIC_3);
    buf_push(out, (uint8_t)format);
    write_uvarint(out, (uint64_t)src_len);
    if (payload_len > 0 && payload != NULL) {
        buf_append(out, payload, payload_len);
    }
}

static bool unwrap_payload(const uint8_t *in, size_t in_len, int format,
                           const uint8_t **payload, size_t *payload_len, size_t *expected_len) {
    if (in_len < 6) return false;
    if (in[0] != STREAM_MAGIC_0 || in[1] != STREAM_MAGIC_1 || in[2] != STREAM_MAGIC_2 || in[3] != STREAM_MAGIC_3) {
        return false;
    }
    if (in[4] != (uint8_t)format) return false;
    size_t pos = 5;
    uint64_t exp_len_u64 = 0;
    if (!read_uvarint(in, in_len, &pos, &exp_len_u64)) return false;
    *expected_len = (size_t)exp_len_u64;
    *payload = in + pos;
    *payload_len = in_len - pos;
    return true;
}

static int compress_profile(int format, const uint8_t *in, size_t in_len, Buffer *out) {
    MatchProfile prof = PROFILES[format];
    buf_init(out, in_len + 64);
    if (in_len == 0) {
        wrap_payload(out, format, 0, NULL, 0);
        return 0;
    }

    Buffer payload;
    buf_init(&payload, in_len);

    uint8_t literals[128];
    size_t num_lit = 0;

    int *last_match = (int *)malloc(sizeof(int) * MATCH_HASH_SIZE);
    int *prev_match = (int *)malloc(sizeof(int) * (in_len > 0 ? in_len : 1));
    for (int i = 0; i < MATCH_HASH_SIZE; i++) last_match[i] = -1;
    for (size_t i = 0; i < in_len; i++) prev_match[i] = -1;

    size_t pos = 0;
    while (pos < in_len) {
        size_t off = 0, match_len = 0;
        find_best_match(in, pos, in_len, prof, last_match, prev_match, &off, &match_len);
        if (match_len >= (size_t)prof.min_match) {
            if (num_lit > 0) {
                buf_push(&payload, (uint8_t)(num_lit - 1));
                buf_append(&payload, literals, num_lit);
                num_lit = 0;
            }
            buf_push(&payload, (uint8_t)(0x80 | (uint8_t)(match_len - prof.min_match)));
            write_uvarint(&payload, (uint64_t)off);
            for (size_t k = 0; k < match_len; k++) {
                index_match_position(in, pos + k, in_len, last_match, prev_match);
            }
            pos += match_len;
        } else {
            literals[num_lit++] = in[pos];
            if (num_lit == (size_t)prof.max_literal) {
                buf_push(&payload, (uint8_t)(num_lit - 1));
                buf_append(&payload, literals, num_lit);
                num_lit = 0;
            }
            index_match_position(in, pos, in_len, last_match, prev_match);
            pos++;
        }
    }
    if (num_lit > 0) {
        buf_push(&payload, (uint8_t)(num_lit - 1));
        buf_append(&payload, literals, num_lit);
        num_lit = 0;
    }

    free(last_match);
    free(prev_match);

    wrap_payload(out, format, in_len, payload.data, payload.len);
    buf_free(&payload);
    return 0;
}

static int decompress_profile(int format, const uint8_t *in, size_t in_len, Buffer *out) {
    MatchProfile prof = PROFILES[format];
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    size_t expected_len = 0;
    if (!unwrap_payload(in, in_len, format, &payload, &payload_len, &expected_len)) {
        return -1;
    }

    buf_init(out, expected_len + 16);
    if (expected_len == 0) return 0;

    size_t pos = 0;
    while (pos < payload_len) {
        uint8_t ctrl = payload[pos++];
        if ((ctrl & 0x80) == 0) {
            size_t lit_len = (size_t)(ctrl & 0x7f) + 1;
            if (pos + lit_len > payload_len) {
                buf_free(out);
                return -1;
            }
            buf_append(out, payload + pos, lit_len);
            pos += lit_len;
        } else {
            size_t match_len = (size_t)(ctrl & 0x7f) + prof.min_match;
            uint64_t off_u64 = 0;
            if (!read_uvarint(payload, payload_len, &pos, &off_u64)) {
                buf_free(out);
                return -1;
            }
            size_t off = (size_t)off_u64;
            if (off == 0 || off > out->len) {
                buf_free(out);
                return -1;
            }
            size_t base = out->len - off;
            for (size_t k = 0; k < match_len; k++) {
                buf_push(out, out->data[base + k]);
            }
        }
    }
    if (out->len != expected_len) {
        buf_free(out);
        return -1;
    }
    return 0;
}

/* --- LZ78 Implementation --- */

#define LZ78_HASH_SIZE 65536

typedef struct {
    uint32_t prefix;
    uint8_t suffix;
    uint32_t code;
    bool used;
} LZ78Entry;

static int compress_lz78_internal(const uint8_t *in, size_t in_len, Buffer *out) {
    buf_init(out, in_len + 64);
    if (in_len == 0) {
        wrap_payload(out, ALYA_LZ_FORMAT_LZ78, 0, NULL, 0);
        return 0;
    }

    Buffer payload;
    buf_init(&payload, in_len);

    LZ78Entry *table = (LZ78Entry *)calloc(LZ78_HASH_SIZE, sizeof(LZ78Entry));
    uint32_t next_index = 1;
    uint32_t current_prefix = 0;

    for (size_t i = 0; i < in_len; i++) {
        uint8_t b = in[i];
        uint32_t h = (uint32_t)((((uint64_t)current_prefix << 8) | b) * 2654435761u) & (LZ78_HASH_SIZE - 1);
        bool found = false;
        uint32_t code = 0;
        for (int step = 0; step < LZ78_HASH_SIZE; step++) {
            uint32_t idx = (h + step) & (LZ78_HASH_SIZE - 1);
            if (!table[idx].used) break;
            if (table[idx].prefix == current_prefix && table[idx].suffix == b) {
                found = true;
                code = table[idx].code;
                break;
            }
        }

        if (found) {
            current_prefix = code;
        } else {
            write_uvarint(&payload, (uint64_t)current_prefix);
            buf_push(&payload, 1); /* has_suffix = 1 */
            buf_push(&payload, b);

            for (int step = 0; step < LZ78_HASH_SIZE; step++) {
                uint32_t idx = (h + step) & (LZ78_HASH_SIZE - 1);
                if (!table[idx].used) {
                    table[idx].used = true;
                    table[idx].prefix = current_prefix;
                    table[idx].suffix = b;
                    table[idx].code = next_index++;
                    break;
                }
            }
            current_prefix = 0;
        }
    }

    if (current_prefix > 0) {
        write_uvarint(&payload, (uint64_t)current_prefix);
        buf_push(&payload, 0); /* has_suffix = 0 */
    }

    free(table);
    wrap_payload(out, ALYA_LZ_FORMAT_LZ78, in_len, payload.data, payload.len);
    buf_free(&payload);
    return 0;
}

typedef struct {
    size_t offset;
    size_t len;
} LZ78DecEntry;

static int decompress_lz78_internal(const uint8_t *in, size_t in_len, Buffer *out) {
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    size_t expected_len = 0;
    if (!unwrap_payload(in, in_len, ALYA_LZ_FORMAT_LZ78, &payload, &payload_len, &expected_len)) {
        return -1;
    }

    buf_init(out, expected_len + 16);
    if (expected_len == 0) return 0;

    size_t dict_cap = 1024;
    LZ78DecEntry *dict = (LZ78DecEntry *)malloc(dict_cap * sizeof(LZ78DecEntry));
    dict[0].offset = 0;
    dict[0].len = 0;
    size_t next_index = 1;

    size_t pos = 0;
    while (pos < payload_len) {
        uint64_t prefix_u64 = 0;
        if (!read_uvarint(payload, payload_len, &pos, &prefix_u64)) {
            free(dict);
            buf_free(out);
            return -1;
        }
        if (pos >= payload_len) {
            free(dict);
            buf_free(out);
            return -1;
        }
        uint8_t has_suffix = payload[pos++];
        size_t prefix = (size_t)prefix_u64;
        if (prefix >= next_index) {
            free(dict);
            buf_free(out);
            return -1;
        }

        size_t entry_offset = out->len;
        if (prefix > 0) {
            buf_append(out, out->data + dict[prefix].offset, dict[prefix].len);
        }

        if (has_suffix == 1) {
            if (pos >= payload_len) {
                free(dict);
                buf_free(out);
                return -1;
            }
            buf_push(out, payload[pos++]);
        } else if (has_suffix != 0) {
            free(dict);
            buf_free(out);
            return -1;
        }

        size_t entry_len = out->len - entry_offset;
        if (next_index >= dict_cap) {
            dict_cap *= 2;
            dict = (LZ78DecEntry *)realloc(dict, dict_cap * sizeof(LZ78DecEntry));
        }
        dict[next_index].offset = entry_offset;
        dict[next_index].len = entry_len;
        next_index++;
    }

    free(dict);
    if (out->len != expected_len) {
        buf_free(out);
        return -1;
    }
    return 0;
}

/* --- LZW Implementation --- */

#define LZW_HASH_SIZE 65536
#define LZW_MAX_CODES 65536

typedef struct {
    uint32_t prefix;
    uint8_t suffix;
    uint32_t code;
    bool used;
} LZWEntry;

static int compress_lzw_internal(const uint8_t *in, size_t in_len, Buffer *out) {
    buf_init(out, in_len + 64);
    if (in_len == 0) {
        wrap_payload(out, ALYA_LZ_FORMAT_LZW, 0, NULL, 0);
        return 0;
    }

    Buffer payload;
    buf_init(&payload, in_len);

    LZWEntry *table = (LZWEntry *)calloc(LZW_HASH_SIZE, sizeof(LZWEntry));
    uint32_t next_code = 256;
    int current_prefix = -1;

    for (size_t i = 0; i < in_len; i++) {
        uint8_t b = in[i];
        if (current_prefix == -1) {
            current_prefix = b;
            continue;
        }

        uint32_t h = (uint32_t)((((uint64_t)current_prefix << 8) | b) * 2654435761u) & (LZW_HASH_SIZE - 1);
        bool found = false;
        uint32_t code = 0;
        for (int step = 0; step < LZW_HASH_SIZE; step++) {
            uint32_t idx = (h + step) & (LZW_HASH_SIZE - 1);
            if (!table[idx].used) break;
            if (table[idx].prefix == (uint32_t)current_prefix && table[idx].suffix == b) {
                found = true;
                code = table[idx].code;
                break;
            }
        }

        if (found) {
            current_prefix = (int)code;
        } else {
            write_uvarint(&payload, (uint64_t)current_prefix);
            if (next_code < LZW_MAX_CODES) {
                for (int step = 0; step < LZW_HASH_SIZE; step++) {
                    uint32_t idx = (h + step) & (LZW_HASH_SIZE - 1);
                    if (!table[idx].used) {
                        table[idx].used = true;
                        table[idx].prefix = (uint32_t)current_prefix;
                        table[idx].suffix = b;
                        table[idx].code = next_code++;
                        break;
                    }
                }
            }
            current_prefix = b;
        }
    }

    if (current_prefix != -1) {
        write_uvarint(&payload, (uint64_t)current_prefix);
    }

    free(table);
    wrap_payload(out, ALYA_LZ_FORMAT_LZW, in_len, payload.data, payload.len);
    buf_free(&payload);
    return 0;
}

typedef struct {
    size_t offset;
    size_t len;
} LZWDecEntry;

static int decompress_lzw_internal(const uint8_t *in, size_t in_len, Buffer *out) {
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    size_t expected_len = 0;
    if (!unwrap_payload(in, in_len, ALYA_LZ_FORMAT_LZW, &payload, &payload_len, &expected_len)) {
        return -1;
    }

    buf_init(out, expected_len + 16);
    if (expected_len == 0) return 0;

    size_t dict_cap = 4096;
    LZWDecEntry *dict = (LZWDecEntry *)malloc(dict_cap * sizeof(LZWDecEntry));
    size_t next_code = 256;

    size_t pos = 0;
    uint64_t first_code_u64 = 0;
    if (!read_uvarint(payload, payload_len, &pos, &first_code_u64)) {
        free(dict);
        buf_free(out);
        return -1;
    }
    if (first_code_u64 >= 256) {
        free(dict);
        buf_free(out);
        return -1;
    }
    buf_push(out, (uint8_t)first_code_u64);

    size_t prev_offset = 0;
    size_t prev_len = 1;

    while (pos < payload_len) {
        uint64_t code_u64 = 0;
        if (!read_uvarint(payload, payload_len, &pos, &code_u64)) {
            free(dict);
            buf_free(out);
            return -1;
        }
        size_t code = (size_t)code_u64;
        size_t entry_offset = 0;
        size_t entry_len = 0;

        if (code < 256) {
            entry_offset = out->len;
            entry_len = 1;
            buf_push(out, (uint8_t)code);
        } else if (code < next_code) {
            entry_offset = out->len;
            entry_len = dict[code].len;
            buf_append(out, out->data + dict[code].offset, entry_len);
        } else if (code == next_code) {
            entry_offset = out->len;
            entry_len = prev_len + 1;
            uint8_t first_byte = out->data[prev_offset];
            buf_append(out, out->data + prev_offset, prev_len);
            buf_push(out, first_byte);
        } else {
            free(dict);
            buf_free(out);
            return -1;
        }

        if (next_code >= dict_cap) {
            dict_cap *= 2;
            dict = (LZWDecEntry *)realloc(dict, dict_cap * sizeof(LZWDecEntry));
        }
        dict[next_code].offset = prev_offset;
        dict[next_code].len = prev_len + 1;
        next_code++;

        prev_offset = entry_offset;
        prev_len = entry_len;
    }

    free(dict);
    if (out->len != expected_len) {
        buf_free(out);
        return -1;
    }
    return 0;
}

int alya_lz_compress(int format, const uint8_t *in, size_t in_len, uint8_t **out_buf, size_t *out_len) {
    Buffer out;
    int res = 0;
    switch (format) {
        case ALYA_LZ_FORMAT_LZ77:
        case ALYA_LZ_FORMAT_LZ4:
        case ALYA_LZ_FORMAT_LZSS:
        case ALYA_LZ_FORMAT_LZMA:
        case ALYA_LZ_FORMAT_LZMA2:
        case ALYA_LZ_FORMAT_LZJB:
            res = compress_profile(format, in, in_len, &out);
            break;
        case ALYA_LZ_FORMAT_LZ78:
            res = compress_lz78_internal(in, in_len, &out);
            break;
        case ALYA_LZ_FORMAT_LZW:
            res = compress_lzw_internal(in, in_len, &out);
            break;
        default:
            return -1;
    }
    if (res == 0) {
        *out_buf = out.data;
        *out_len = out.len;
    }
    return res;
}

int alya_lz_decompress(int format, const uint8_t *in, size_t in_len, uint8_t **out_buf, size_t *out_len) {
    Buffer out;
    int res = 0;
    switch (format) {
        case ALYA_LZ_FORMAT_LZ77:
        case ALYA_LZ_FORMAT_LZ4:
        case ALYA_LZ_FORMAT_LZSS:
        case ALYA_LZ_FORMAT_LZMA:
        case ALYA_LZ_FORMAT_LZMA2:
        case ALYA_LZ_FORMAT_LZJB:
            res = decompress_profile(format, in, in_len, &out);
            break;
        case ALYA_LZ_FORMAT_LZ78:
            res = decompress_lz78_internal(in, in_len, &out);
            break;
        case ALYA_LZ_FORMAT_LZW:
            res = decompress_lzw_internal(in, in_len, &out);
            break;
        default:
            return -1;
    }
    if (res == 0) {
        *out_buf = out.data;
        *out_len = out.len;
    }
    return res;
}
