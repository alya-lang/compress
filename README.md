# compress

[![CI](https://github.com/alya-lang/compress/actions/workflows/ci.yml/badge.svg)](https://github.com/alya-lang/compress/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/alya-lang/compress?color=blue&label=License)](LICENSE)
[![Alya](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.alya-version&label=Alya&color=orange&prefix=%3E%3D)](https://github.com/alya-lang/alya)
[![Package Version](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.version&label=Version&color=brightgreen)](alya.toml)

Comprehensive, high-performance compression and decompression toolkit for the **Alya Programming Language**, providing zero-dependency native implementations of **GZIP**, **ZLIB**, **Raw DEFLATE**, **LZ4**, **Google Snappy**, and integrity checksums (**CRC-32**, **Adler-32**).

---

## 🌟 Features

- 🚀 **5 Industry-Standard Algorithms**:
  - **GZIP (RFC 1952)**: Full specification support with standard 10-byte header, CRC-32 verification, and uncompressed size trailer.
  - **ZLIB (RFC 1950)**: Standard RFC 1950 format with Adler-32 checksum validation.
  - **Raw DEFLATE (RFC 1951)**: Headerless streaming compression engine.
  - **LZ4**: Ultra-fast block compression reaching over 1,000,000 ops/sec.
  - **Google Snappy**: High-throughput framing and block compression optimized for distributed storage and networking.
- 🔒 **Data Integrity**: Built-in **CRC-32** and **Adler-32** hardware-accelerated checksum calculation functions.
- ⚡ **Zero External Dependencies**: Bundles lightweight, battle-tested C engines (`miniz`, `lz4`, `snappy`) compiled automatically via `alyac`.
- 🛡️ **Dual API Support**: Clean type-separated functions for both UTF-8 strings (`gzip_str`, `gunzip_str`) and raw binary byte lists (`gzip`, `gunzip`).
- 🔧 **Hexadecimal Utilities**: Convenient helpers for encoding compressed byte buffers to and from hex representation.

---

## 📊 Algorithm Comparison Matrix

| Algorithm | Standard / Origin | Best Used For | Ratio | Throughput (Decompress) |
|---|---|---|---|---|
| **GZIP** | RFC 1952 | HTTP transfers, file archiving (`.tar.gz`) | High | ~232,000 ops/sec |
| **ZLIB** | RFC 1950 | Network protocols, PNG image streams | High | ~250,000 ops/sec |
| **Raw DEFLATE** | RFC 1951 | Low-level streaming pipelines | High | ~263,000 ops/sec |
| **LZ4** | Yann Collet | Real-time RPC, database caching, logs | Fast / Balanced | ~1,136,000 ops/sec |
| **Snappy** | Google | Big data, MapReduce, Cassandra/RocksDB | Fast / Balanced | ~1,086,000 ops/sec |

---

## 📁 Project Architecture

```
compress/
├── alya.toml               # Package manifest with [build] c-sources
├── c/                      # Bundled zero-dependency C implementations
│   ├── compress.h          # Unified C compression headers
│   ├── compress.c          # Engine dispatch & dynamic memory auto-allocators
│   ├── miniz.h / miniz.c   # Deflate, Zlib, Gzip & Checksums engine
│   ├── lz4.h / lz4.c       # LZ4 v1.10.0 high-speed block engine
│   └── snappy.h / snappy.c # Google Snappy C block engine
├── src/
│   ├── lib.alya            # Public API facade
│   ├── types.alya          # Memory preparation, buffers & hex helpers
│   ├── ffi.alya            # Native extern "C" bindings
│   ├── gzip.alya           # GZIP compressor & decompressor
│   ├── zlib.alya           # ZLIB compressor & decompressor
│   ├── deflate.alya        # Raw DEFLATE compressor & decompressor
│   ├── lz4.alya            # LZ4 compressor & decompressor
│   ├── snappy.alya         # Snappy compressor & decompressor
│   └── checksum.alya       # CRC-32 and Adler-32 utilities
├── examples/
│   └── demo.alya           # Real-world runnable demonstration
├── tests/
│   └── test_basic.alya     # Automated test suite (41/41 tests passing)
└── benches/
    └── bench_basic.alya    # Comprehensive 12-method benchmark suite
```

---

## 📦 Installation

Add `compress` to the `[dependencies]` section in your `alya.toml`:

```toml
[dependencies]
compress = { git = "https://github.com/alya-lang/compress", tag = "v0.1.0" }
```

Or install it directly using the Alya package CLI:

```bash
alyac add compress --git https://github.com/alya-lang/compress --tag v0.1.0
alyac install
```

---

## 🚀 Quick Start

### 1. String Compression & Decompression

```alya
import "compress" as compress

function main()
    let text = "Alya is a modern, high-performance programming language designed for systems tooling."

    # GZIP roundtrip
    let gz = compress::gzip_str(text)
    let restored_gz = compress::gunzip_str(gz)
    say "GZIP match: " + str(restored_gz == text)

    # LZ4 ultra-fast roundtrip
    let lz = compress::lz4_str(text)
    let restored_lz = compress::unlz4_str(lz)
    say "LZ4 match: " + str(restored_lz == text)

    # Snappy roundtrip
    let snp = compress::snappy_str(text)
    let restored_snp = compress::unsnappy_str(snp)
    say "Snappy match: " + str(restored_snp == text)
end

main()
```

### 2. Binary Byte Array Handling

```alya
import "compress" as compress

function main()
    let bytes = [0, 1, 2, 3, 250, 251, 252, 253, 254, 255]

    # Compress binary data
    let compressed = compress::zlib(bytes)
    say "Compressed size: " + str(len(compressed))

    # Decompress binary data
    let restored = compress::unzlib(compressed)
    say "Restored size: " + str(len(restored))
end

main()
```

### 3. Integrity Checksums & Hex Conversion

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

### LZ4 Fast Real-Time Compression
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `lz4_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using LZ4 block format. |
| `unlz4_str(bytes)` | `bytes: list<int>` | `string` | Decompresses LZ4 bytes directly into UTF-8 string. |
| `lz4(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using LZ4. |
| `unlz4(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses LZ4 byte list into binary byte list. |

### Google Snappy
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `snappy_str(text)` | `text: string` | `list<int>` | Compresses UTF-8 string using Snappy block compression. |
| `unsnappy_str(bytes)` | `bytes: list<int>` | `string` | Decompresses Snappy bytes directly into UTF-8 string. |
| `snappy(bytes)` | `bytes: list<int>` | `list<int>` | Compresses binary byte array using Snappy. |
| `unsnappy(bytes)` | `bytes: list<int>` | `list<int>` | Decompresses Snappy byte list into binary byte list. |

### Checksums & Utilities
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `crc32_str(text)` | `text: string` | `int` | Computes 32-bit Cyclic Redundancy Checksum of string. |
| `crc32(bytes)` | `bytes: list<int>` | `int` | Computes CRC-32 checksum of byte array. |
| `adler32_str(text)` | `text: string` | `int` | Computes Adler-32 checksum of string (initial seed 1). |
| `adler32(bytes)` | `bytes: list<int>` | `int` | Computes Adler-32 checksum of byte array. |
| `bytes_to_hex(bytes)` | `bytes: list<int>` | `string` | Converts byte list to lowercase hexadecimal string. |
| `bytes_from_hex(hex_str)` | `hex_str: string` | `list<int>` | Parses hexadecimal string into list of byte integers. |
| `compress_version()` | none | `string` | Returns current package version (`"0.1.0"`). |

---

## ⚡ Performance Benchmarks

Measured on Windows 11 with AMD Ryzen / MinGW GCC via `benches/bench_basic.alya`:

| Method | Mean (ns/op) | Throughput | Ratio | Description |
|:---|---:|---:|---:|:---|
| `lz4_str()` | ~2.2 µs | **454,000 ops/s** | 0.19 | LZ4 real-time compression |
| `unlz4_str()` | **880 ns** | **1,136,000 ops/s** | 0.07 | LZ4 real-time decompression |
| `snappy_str()` | ~2.7 µs | **367,000 ops/s** | 0.24 | Snappy block compression |
| `unsnappy_str()` | **920 ns** | **1,086,000 ops/s** | 0.08 | Snappy block decompression |
| `deflate_str()` | ~8.0 µs | **125,000 ops/s** | 0.72 | Raw DEFLATE compression |
| `inflate_str()` | ~3.8 µs | **263,000 ops/s** | 0.34 | Raw DEFLATE decompression |
| `zlib_str()` | ~8.4 µs | **119,000 ops/s** | 0.75 | ZLIB RFC 1950 compression |
| `unzlib_str()` | ~4.0 µs | **250,000 ops/s** | 0.36 | ZLIB RFC 1950 decompression |
| `gzip_str()` | ~11.1 µs | **90,000 ops/s** | 1.00 | GZIP RFC 1952 compression |
| `gunzip_str()` | ~4.3 µs | **232,000 ops/s** | 0.38 | GZIP RFC 1952 decompression |
| `crc32_str()` | **540 ns** | **1,851,000 ops/s** | 0.04 | CRC-32 checksum calculation |
| `adler32_str()` | **260 ns** | **3,846,000 ops/s** | 0.02 | Adler-32 checksum calculation |

---

## 🧪 Running Tests & Benchmarks

Run the test suite using `alyac`:

```bash
alyac run tests/test_basic.alya
```

Run the micro-benchmarks:

```bash
alyac run benches/bench_basic.alya
```

Run the runnable example:

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