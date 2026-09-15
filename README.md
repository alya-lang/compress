# compress

[![CI](https://github.com/alya-lang/compress/actions/workflows/ci.yml/badge.svg)](https://github.com/alya-lang/compress/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/alya-lang/compress?color=blue&label=License)](LICENSE)
[![Alya](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.alya-version&label=Alya&color=orange&prefix=%3E%3D)](https://github.com/alya-lang/alya)
[![Package Version](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.version&label=Version&color=brightgreen)](alya.toml)

Comprehensive, high-performance compression and decompression toolkit for the **Alya Programming Language**, providing zero-dependency native implementations of **Brotli**, **Bzip2**, **DEFLATE**, **GZIP**, **LZ4 / LZ**, **Google Snappy**, **SZIP / ZIP Archives**, **ZLIB**, **Meta Zstandard (zstd)**, and integrity checksums (**CRC-32**, **Adler-32**).

---

## 🌟 Features

- 🚀 **9 Industry-Standard Compression Modules**:
  - **Brotli (RFC 7932)**: Google Brotli general-purpose lossless compression with high compression ratios, widely used in modern HTTP/HTTPS web assets.
  - **Bzip2**: Classic Burrows-Wheeler block-sorting data compressor (libbzip2 1.0.8) with high density.
  - **Raw DEFLATE (RFC 1951)**: Headerless streaming compression engine.
  - **GZIP (RFC 1952)**: Full specification support with standard 10-byte header, CRC-32 verification, and uncompressed size trailer.
  - **LZ4 / LZ**: Ultra-fast block compression reaching over 1,000,000 ops/sec.
  - **Google Snappy**: High-throughput framing and block compression optimized for distributed storage and networking.
  - **SZIP & ZIP Archives**: Buffer compression and complete in-place `.zip` archive creation, listing, text/binary extraction, and extraction to disk.
  - **ZLIB (RFC 1950)**: Standard RFC 1950 format with Adler-32 checksum validation.
  - **Zstandard / Zstd (RFC 8878)**: Meta / Facebook real-time compression algorithm providing high compression ratios with extreme decompression speeds.
- 🗄️ **Full ZIP Archive Support**: Create, inspect, append, and extract `.zip` files directly from Alya without external utilities.
- 🔒 **Data Integrity**: Built-in **CRC-32** and **Adler-32** hardware-accelerated checksum calculation functions.
- ⚡ **Zero External Dependencies**: Bundles lightweight, battle-tested C engines (`miniz`, `lz4`, `snappy`, `zstd`, `bzip2`, `brotli`) compiled automatically via `alyac`.
- 🛡️ **Dual API Support**: Clean type-separated functions for both UTF-8 strings (`*_str`) and raw binary byte lists.
- 🔧 **Hexadecimal Utilities**: Convenient helpers for encoding compressed byte buffers to and from hex representation.

---

## 📊 Algorithm Comparison Matrix

| Algorithm | Standard / Origin | Best Used For | Ratio | Throughput (Decompress) |
|---|---|---|---|---|
| **Brotli** | Google / RFC 7932 | Web assets (JS, CSS, HTML), highest density | Ultra High | ~416,000 ops/sec |
| **Bzip2** | Julian Seward / BWT | Archive files, log compression | Very High | ~57,000 ops/sec |
| **Zstandard (zstd)** | Meta / RFC 8878 | Modern databases, file systems, general storage | High | ~304,000 ops/sec |
| **GZIP** | Jean-loup Gailly / RFC 1952 | HTTP transfers, `.tar.gz` distribution | High | ~243,000 ops/sec |
| **ZLIB** | Mark Adler / RFC 1950 | Network protocols, PNG image streams | High | ~263,000 ops/sec |
| **Raw DEFLATE** | RFC 1951 | Low-level streaming pipelines | High | ~238,000 ops/sec |
| **SZIP** | miniz / PKWARE ZIP | `.zip` archives, portable file bundles | High | ~128,000 ops/sec |
| **Snappy** | Google | Big data, MapReduce, Cassandra/RocksDB | Balanced | **~1,190,000 ops/sec** |
| **LZ4** | Yann Collet | Real-time RPC, database caching, logs | Balanced | **~1,086,000 ops/sec** |

---

## 📁 Project Architecture

```
compress/
├── alya.toml               # Package manifest with [build] c-sources
├── c/                      # Bundled zero-dependency C implementations
│   ├── compress.h          # Unified C compression headers
│   ├── compress.c          # Engine dispatch & dynamic memory auto-allocators
│   ├── miniz.h / miniz.c   # Deflate, Zlib, Gzip, Szip & ZIP archive engine
│   ├── lz4.h / lz4.c       # LZ4 v1.10.0 high-speed block engine
│   ├── snappy.h / snappy.c # Google Snappy C block engine
│   ├── zstd.h / zstd.c     # Meta Zstandard amalgamated engine
│   ├── bzip2/ / bzip2.c    # Libbzip2 1.0.8 BWT engine
│   └── brotli/ / brotli.c  # Google Brotli 1.0.9 engine
├── src/
│   ├── lib.alya            # Public API facade
│   ├── types.alya          # Memory preparation, buffers & hex helpers
│   ├── ffi.alya            # Native extern "C" bindings
│   ├── brotli.alya         # Brotli compressor & decompressor
│   ├── bzip2.alya          # Bzip2 compressor & decompressor
│   ├── deflate.alya        # Raw DEFLATE compressor & decompressor
│   ├── gzip.alya           # GZIP compressor & decompressor
│   ├── lz4.alya            # LZ4 / LZ compressor & decompressor
│   ├── snappy.alya         # Snappy compressor & decompressor
│   ├── szip.alya           # Szip compressor & ZIP archive manager
│   ├── zlib.alya           # ZLIB compressor & decompressor
│   ├── zstd.alya           # Zstandard compressor & decompressor
│   └── checksum.alya       # CRC-32 and Adler-32 utilities
├── examples/
│   └── demo.alya           # Real-world runnable demonstration (all 9 engines + ZIP)
├── tests/
│   └── test_basic.alya     # Automated test suite (67/67 tests passing)
└── benches/
    └── bench_basic.alya    # Comprehensive 19-method benchmark suite
```

---

## 📦 Installation

Add `compress` to the `[dependencies]` section in your `alya.toml`:

```toml
[dependencies]
compress = { git = "https://github.com/alya-lang/compress", tag = "v0.2.0" }
```

Or install it directly using the Alya package CLI:

```bash
alyac add compress --git https://github.com/alya-lang/compress --tag v0.2.0
alyac install
```

---

## 🚀 Quick Start

### 1. String Compression & Decompression

```alya
import "compress" as compress

function main()
    let text = "Alya is a modern, high-performance programming language designed for systems tooling."

    # Brotli (highest compression ratio)
    let br = compress::brotli_str(text)
    let restored_br = compress::unbrotli_str(br)
    say "Brotli match: " + str(restored_br == text)

    # Zstandard (fast + high ratio)
    let zst = compress::zstd_str(text)
    let restored_zst = compress::unzstd_str(zst)
    say "Zstd match: " + str(restored_zst == text)

    # GZIP roundtrip
    let gz = compress::gzip_str(text)
    let restored_gz = compress::gunzip_str(gz)
    say "GZIP match: " + str(restored_gz == text)

    # LZ4 ultra-fast roundtrip (>1M ops/sec)
    let lz = compress::lz4_str(text)
    let restored_lz = compress::unlz4_str(lz)
    say "LZ4 match: " + str(restored_lz == text)

    # Snappy roundtrip (>1M ops/sec)
    let snp = compress::snappy_str(text)
    let restored_snp = compress::unsnappy_str(snp)
    say "Snappy match: " + str(restored_snp == text)

    # Bzip2 roundtrip
    let bz = compress::bzip2_str(text)
    let restored_bz = compress::unbzip2_str(bz)
    say "Bzip2 match: " + str(restored_bz == text)
end

main()
```

### 2. ZIP Archive Creation & Extraction

```alya
import "compress" as compress

function main()
    let zip_file = "my_archive.zip"

    # Create empty archive or append files
    compress::szip_create(zip_file)
    compress::szip_add_text(zip_file, "hello.txt", "Hello from Alya compress!")
    compress::szip_add_text(zip_file, "notes/config.json", "{\"engine\": \"szip\", \"status\": \"ok\"}")

    # Inspect contents
    let count = compress::szip_file_count(zip_file)
    say "Total files in archive: " + str(count)

    let files = compress::szip_list(zip_file)
    for f in files
        say " - " + f
    end

    # Extract directly to memory
    let text = compress::szip_extract_text(zip_file, "hello.txt")
    say "Extracted: " + text

    # Extract to disk
    compress::szip_extract_to_file(zip_file, "hello.txt", "extracted_hello.txt")
end

main()
```

### 3. Binary Byte Array Handling

```alya
import "compress" as compress

function main()
    let bytes = [0, 1, 2, 3, 250, 251, 252, 253, 254, 255]

    # Compress binary data with Zstd
    let compressed = compress::zstd(bytes)
    say "Compressed size: " + str(len(compressed))

    # Decompress binary data
    let restored = compress::unzstd(compressed)
    say "Restored size: " + str(len(restored))
end

main()
```

### 4. Integrity Checksums & Hex Utilities

```alya
import "compress" as compress

function main()
    let phrase = "The quick brown fox jumps over the lazy dog"

    # Checksums
    let crc = compress::crc32_str(phrase)     # 1095738169 (0x414fa339)
    let adler = compress::adler32_str(phrase) # 1541148634 (0x5bd40fba)
    say "CRC-32: " + str(crc)
    say "Adler-32: " + str(adler)

    # Hex utilities
    let gz = compress::gzip_str(phrase)
    let hex_encoded = compress::bytes_to_hex(gz)
    say "Hex preview: " + hex_encoded
end

main()
```

---

## 📖 API Reference

### Brotli (RFC 7932)
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `brotli_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using Brotli (default quality 6). |
| `brotli_str_quality(text, quality)` | `text: string, quality: int` | `list<int>` | Compresses string with quality level (0..11). |
| `unbrotli_str(bytes)` | `bytes: list<int>` | `string` | Decompresses Brotli byte list directly into UTF-8 string. |
| `brotli(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using Brotli. |
| `brotli_quality(bytes, quality)` | `bytes: list<int>, quality: int` | `list<int>` | Compresses binary bytes with quality level (0..11). |
| `unbrotli(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses Brotli byte list into binary byte list. |

### Bzip2
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `bzip2_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using Bzip2 (block size 9). |
| `bzip2_str_block(text, block_100k)` | `text: string, block_100k: int` | `list<int>` | Compresses string with 100k block size (1..9). |
| `unbzip2_str(bytes)` | `bytes: list<int>` | `string` | Decompresses Bzip2 byte list directly into UTF-8 string. |
| `bzip2(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using Bzip2. |
| `bzip2_block(bytes, block_100k)` | `bytes: list<int>, block_100k: int` | `list<int>` | Compresses binary bytes with 100k block size (1..9). |
| `unbzip2(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses Bzip2 byte list into binary byte list. |

### Zstandard / Zstd (RFC 8878)
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `zstd_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using Zstandard (default level 3). |
| `zstd_str_level(text, level)` | `text: string, level: int` | `list<int>` | Compresses string with compression level (1..22). |
| `unzstd_str(bytes)` | `bytes: list<int>` | `string` | Decompresses Zstandard byte list directly into UTF-8 string. |
| `zstd(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using Zstandard. |
| `zstd_level(bytes, level)` | `bytes: list<int>, level: int` | `list<int>` | Compresses binary bytes with compression level (1..22). |
| `unzstd(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses Zstandard byte list into binary byte list. |

### GZIP (RFC 1952)
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `gzip_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string to GZIP bytes (default level 6). |
| `gzip_str_level(text, level)` | `text: string, level: int` | `list<int>` | Compresses UTF-8 string with compression level (1..9). |
| `gunzip_str(bytes)` | `bytes: list<int>` | `string` | Decompresses GZIP byte list directly into UTF-8 string. |
| `gzip(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using GZIP. |
| `gzip_level(bytes, level)` | `bytes: list<int>, level: int` | `list<int>` | Compresses binary byte array with compression level (1..9). |
| `gunzip(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses GZIP byte list into binary byte list. |

### ZLIB (RFC 1950)
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `zlib_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string to ZLIB bytes (default level 6). |
| `zlib_str_level(text, level)` | `text: string, level: int` | `list<int>` | Compresses UTF-8 string with compression level (1..9). |
| `unzlib_str(bytes)` | `bytes: list<int>` | `string` | Decompresses ZLIB byte list directly into UTF-8 string. |
| `zlib(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using ZLIB. |
| `unzlib(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses ZLIB byte list into binary byte list. |

### Raw DEFLATE (RFC 1951)
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `deflate_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string to raw DEFLATE bytes. |
| `inflate_str(bytes)` | `bytes: list<int>` | `string` | Decompresses raw DEFLATE bytes directly into UTF-8 string. |
| `deflate(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using raw DEFLATE. |
| `inflate(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses raw DEFLATE bytes into binary byte list. |

### LZ4 / LZ Fast Real-Time Compression
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `lz4_str(text)` / `lz_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using LZ4 block format. |
| `unlz4_str(bytes)` / `unlz_str(bytes)` | `bytes: list<int>` | `string` | Decompresses LZ4 bytes directly into UTF-8 string. |
| `lz4(bytes)` / `lz(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using LZ4. |
| `unlz4(bytes)` / `unlz(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses LZ4 byte list into binary byte list. |

### Google Snappy
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `snappy_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using Snappy block compression. |
| `unsnappy_str(bytes)` | `bytes: list<int>` | `string` | Decompresses Snappy bytes directly into UTF-8 string. |
| `snappy(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using Snappy. |
| `unsnappy(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses Snappy byte list into binary byte list. |

### SZIP & ZIP Archives
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `szip_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using SZIP buffer deflate. |
| `unszip_str(bytes)` | `bytes: list<int>` | `string` | Decompresses SZIP buffer directly into UTF-8 string. |
| `szip(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using SZIP buffer deflate. |
| `unszip(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses SZIP bytes into binary byte list. |
| `szip_create(archive_path)` | `archive_path: string` | `bool` | Creates or truncates a valid empty `.zip` archive file. |
| `szip_add_text(archive, name, text)` | `string, string, string` | `bool` | Appends UTF-8 string content as an entry into `.zip` archive. |
| `szip_add_data(archive, name, bytes)` | `string, string, list<int>` | `bool` | Appends binary bytes as an entry into `.zip` archive. |
| `szip_file_count(archive_path)` | `archive_path: string` | `int` | Returns number of files contained in `.zip` archive. |
| `szip_list(archive_path)` | `archive_path: string` | `list<string>` | Returns list of all entry paths inside `.zip` archive. |
| `szip_has_file(archive_path, filename)`| `string, string` | `bool` | Checks if specific entry exists in `.zip` archive. |
| `szip_extract_text(archive, filename)` | `string, string` | `string` | Extracts file entry directly into UTF-8 string. |
| `szip_extract_data(archive, filename)` | `string, string` | `list<int>` | Extracts file entry directly into binary byte array. |
| `szip_extract_to_file(archive, name, out)`| `string, string, string` | `bool` | Extracts entry directly to disk file destination. |

### Checksums & Utilities
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `crc32_str(text)` | `text: string` | `int` | Computes 32-bit Cyclic Redundancy Checksum of string. |
| `crc32(bytes)` | `bytes: list<int>` | `int` | Computes CRC-32 checksum of byte array. |
| `adler32_str(text)` | `text: string` | `int` | Computes Adler-32 checksum of string (initial seed 1). |
| `adler32(bytes)` | `bytes: list<int>` | `int` | Computes Adler-32 checksum of byte array. |
| `bytes_to_hex(bytes)` | `bytes: list<int>` | `string` | Converts byte list to lowercase hexadecimal string. |
| `bytes_from_hex(hex_str)` | `hex_str: string` | `list<int>` | Parses hexadecimal string into list of byte integers. |
| `compress_version()` | none | `string` | Returns current package version (`"0.2.0"`). |

---

## ⚡ Performance Benchmarks

Measured on Windows 11 with AMD Ryzen / MinGW GCC via `benches/bench_basic.alya`:

| Method | Mean (ns/op) | Throughput | Description |
|:---|---:|---:|:---|
| `adler32_str()` | **280 ns** | **3,571,000 ops/s** | Adler-32 hardware-accelerated checksum |
| `crc32_str()` | **500 ns** | **2,000,000 ops/s** | CRC-32 hardware-accelerated checksum |
| `unsnappy_str()` | **840 ns** | **1,190,000 ops/s** | Snappy block decompression |
| `unlz4_str()` | **920 ns** | **1,086,000 ops/s** | LZ4 real-time block decompression |
| `lz4_str()` | ~2.3 µs | **434,000 ops/s** | LZ4 real-time block compression |
| `unbrotli_str()` | ~2.4 µs | **416,000 ops/s** | Brotli decompression |
| `snappy_str()` | ~2.7 µs | **370,000 ops/s** | Snappy block compression |
| `unzstd_str()` | ~3.2 µs | **304,000 ops/s** | Zstandard decompression |
| `unzlib_str()` | ~3.8 µs | **263,000 ops/s** | ZLIB RFC 1950 decompression |
| `gunzip_str()` | ~4.1 µs | **243,000 ops/s** | GZIP RFC 1952 decompression |
| `inflate_str()` | ~4.2 µs | **238,000 ops/s** | Raw DEFLATE RFC 1951 decompression |
| `zstd_str()` | ~5.8 µs | **172,000 ops/s** | Zstandard compression |
| `szip_str()` | ~7.8 µs | **128,000 ops/s** | SZIP buffer compression |
| `unszip_str()` | ~7.8 µs | **128,000 ops/s** | SZIP buffer decompression |
| `deflate_str()` | ~8.0 µs | **125,000 ops/s** | Raw DEFLATE compression |
| `zlib_str()` | ~8.4 µs | **119,000 ops/s** | ZLIB RFC 1950 compression |
| `gzip_str()` | ~11.1 µs | **90,000 ops/s** | GZIP RFC 1952 compression |
| `unbzip2_str()` | ~17.5 µs | **57,000 ops/s** | Bzip2 BWT decompression |
| `brotli_str()` | ~21.7 µs | **46,000 ops/s** | Brotli high-density compression |

---

## 🧪 Running Tests & Benchmarks

Run the complete test suite (67 assertions covering all 9 algorithms & ZIP archives):

```bash
alyac run tests/test_basic.alya
```

Run the 19-method benchmark suite:

```bash
alyac run benches/bench_basic.alya
```

Run the comprehensive runnable demonstration:

```bash
alyac run examples/demo.alya
```

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository and clone it locally
2. Install dependencies:
   ```bash
   alyac install
   ```
3. Run test suite to verify baseline:
   ```bash
   alyac run tests/test_basic.alya
   ```
4. Create your feature branch (`git checkout -b feature/my-feature`)
5. Commit your changes (`git commit -m "feat: add feature"`) and open a Pull Request

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
Bundled third-party libraries:
- `miniz`: MIT License (Rich Geldreich)
- `lz4`: BSD 2-Clause License (Yann Collet)
- `snappy`: New BSD License (Google Inc.)
- `zstd`: BSD 3-Clause License (Meta Platforms, Inc.)
- `bzip2`: Julian Seward Bzip2 License (Julian Seward)
- `brotli`: MIT License (Google Inc.)