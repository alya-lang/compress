# compress

[![CI](https://github.com/alya-lang/compress/actions/workflows/ci.yml/badge.svg)](https://github.com/alya-lang/compress/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/alya-lang/compress?color=blue&label=License)](LICENSE)
[![Alya](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.alya-version&label=Alya&color=orange&prefix=%3E%3D)](https://github.com/alya-lang/alya)
[![Package Version](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.version&label=Version&color=brightgreen)](alya.toml)

Comprehensive, high-performance compression and decompression toolkit for the **Alya Programming Language**, providing zero-dependency native implementations of all standard compression modules: **Brotli**, **Bzip2**, **DEFLATE**, **GZIP**, the complete **LZ Family** (**LZ4**, **LZ77**, **LZ78**, **LZJB**, **LZMA**, **LZMA2**, **LZSS**, **LZW**), **Google Snappy**, **SZIP / ZIP Archives**, **ZLIB**, **Meta Zstandard (zstd)**, and integrity checksums (**CRC-32**, **Adler-32**).

---

## 🌟 Features

- **`compress.brotli`** (Google Brotli, RFC 7932): High-density lossless compression, standard for modern web assets.
- **`compress.bzip2`** (Burrows-Wheeler, libbzip2 1.0.8): Block-sorting data compressor with high ratio.
- **`compress.deflate`** (Raw DEFLATE, RFC 1951): Headerless streaming compression engine.
- **`compress.gzip`** (RFC 1952): GZIP container with 10-byte header, CRC-32 verification, and length trailer.
- **`compress.lz`**: The complete Lempel-Ziv family supporting **all 8 variations**:
  - `lz4`: Ultra-fast block compression reaching over 1,000,000 ops/sec.
  - `lz77`: Classic Abraham Lempel & Jacob Ziv 1977 sliding window compression.
  - `lz78`: Lempel-Ziv 1978 dictionary-based prefix code compression.
  - `lzjb`: Jeff Bonwick's LZ variant optimized for file system caching and storage.
  - `lzma`: Lempel-Ziv-Markov chain algorithm (7-Zip format profile).
  - `lzma2`: XZ-compatible chunked LZMA profile.
  - `lzss`: Lempel-Ziv-Storer-Szymanski 1982 sliding window algorithm.
  - `lzw`: Lempel-Ziv-Welch 1984 dictionary algorithm (GIF / UNIX compress).
- **`compress.snappy`** (Google Snappy): High-throughput framing and block compression.
- **`compress.huffman`**: Pure Alya Canonical Huffman optimal prefix-tree compression and bitstream codec.
- **`compress.szip`** (miniz ZIP Archive): In-place `.zip` archive creation, listing, file inspection, and extraction.
- **`compress.zlib`** (RFC 1950): ZLIB standard format with Adler-32 validation.
- **`compress.zstd`** (Meta / Facebook Zstandard, RFC 8878): Modern high-speed, high-density real-time engine.
- **Data Integrity**: Hardware-accelerated **CRC-32** and **Adler-32** checksums.

### Algorithm Overview

| Module / Variation | Algorithm Origin | Best Used For | Ratio Profile |
|:---|:---|:---|:---:|
| `compress.brotli` | Google / RFC 7932 | Web assets, maximum compression | Ultra High |
| `compress.bzip2` | Julian Seward / BWT | Archive density, log files | Very High |
| `compress.zstd` | Meta / RFC 8878 | Modern databases, file systems | High |
| `compress.gzip` | RFC 1952 | HTTP transfers, `.tar.gz` | High |
| `compress.zlib` | RFC 1950 | Network protocols, PNG | High |
| `compress.deflate` | RFC 1951 | Low-level streaming | High |
| `compress.szip` | miniz / PKWARE | `.zip` archive files | High |
| `compress.lz` (`lz4`) | Yann Collet | Real-time RPC, cache | Balanced |
| `compress.lz` (`lzjb`) | Jeff Bonwick | Fast storage block | Balanced |
| `compress.lz` (`lz77`) | Lempel & Ziv | Sliding window stream | Balanced |
| `compress.lz` (`lzss`) | Storer & Szymanski | Low memory footprint | Balanced |
| `compress.lz` (`lzma`) | Igor Pavlov | High compression profile | High |
| `compress.lz` (`lzma2`) | Igor Pavlov | Chunked LZMA profile | High |
| `compress.lz` (`lzw`) | Terry Welch | Dynamic dictionary | Balanced |
| `compress.lz` (`lz78`) | Lempel & Ziv | Dictionary prefix codes | Variable |
| `compress.snappy` | Google | Distributed storage, big data | Balanced |

---

## 📁 Project Architecture

```
compress/
├── alya.toml               # Package manifest with [build] c-sources
├── c/                      # Bundled zero-dependency C implementations
│   ├── compress.h / .c     # Unified engine dispatch & memory managers
│   ├── lz.h / lz.c         # Unified LZ family codecs (lz77, lz78, lzw, lz4, lzss, lzma, lzma2, lzjb)
│   ├── miniz.h / miniz.c   # Deflate, Zlib, Gzip, Szip & ZIP archive engine
│   ├── lz4.h / lz4.c       # LZ4 v1.10.0 high-speed block engine
│   ├── snappy.h / snappy.c # Google Snappy C block engine
│   ├── zstd.h / zstd.c     # Meta Zstandard amalgamated engine
│   ├── bzip2/ / bzip2.c    # Libbzip2 1.0.8 BWT engine
│   └── brotli/ / brotli.c  # Google Brotli 1.0.9 engine
├── src/
│   ├── lib.alya            # Public API facade
│   ├── lz.alya             # Unified LZ variations & generic dispatcher
│   ├── types.alya          # Memory buffers, string helpers & hex
│   ├── ffi.alya            # Native extern "C" bindings
│   ├── brotli.alya         # Brotli compressor & decompressor
│   ├── bzip2.alya          # Bzip2 compressor & decompressor
│   ├── deflate.alya        # Raw DEFLATE compressor & decompressor
│   ├── gzip.alya           # GZIP compressor & decompressor
│   ├── lz4.alya            # LZ4 compressor & decompressor
│   ├── snappy.alya         # Snappy compressor & decompressor
│   ├── szip.alya           # Szip compressor & ZIP archive manager
│   ├── zlib.alya           # ZLIB compressor & decompressor
│   ├── zstd.alya           # Zstandard compressor & decompressor
│   ├── huffman.alya        # Pure Alya Canonical Huffman optimal prefix codec
│   └── checksum.alya       # CRC-32 and Adler-32 utilities
├── examples/
│   ├── demo.alya           # Real-world runnable demonstration (all 16 codecs)
│   └── huffman_demo.alya   # Huffman compression demonstration
├── tests/
│   ├── test_basic.alya     # Automated test suite (all 10 modules)
│   └── test_huffman.alya   # Dedicated Huffman compression test suite
└── benches/
    └── bench_basic.alya    # Comprehensive 25-method benchmark suite
```

---

## 📦 Installation

Add `compress` to the `[dependencies]` section in your `alya.toml`:

```toml
[dependencies]
compress = { git = "https://github.com/alya-lang/compress", branch = "main" }
```

Or install it directly using the Alya package CLI:

```bash
alya add compress --git https://github.com/alya-lang/compress --branch main
alya install
```

---

## 🚀 Quick Start

### 1. LZ Family Variations (`compress.lz`)

You can use the direct format-specific APIs or the generic format selector:

```alya
import "compress" as compress

function main()
    let text = "Alya is a modern, high-performance programming language designed for systems tooling."

    # Direct APIs
    let c_lz77 = compress::lz77_str(text)
    say "LZ77 match: " + str(compress::unlz77_str(c_lz77) == text)

    let c_lzjb = compress::lzjb_str(text)
    say "LZJB match: " + str(compress::unlzjb_str(c_lzjb) == text)

    let c_lzw = compress::lzw_str(text)
    say "LZW match:  " + str(compress::unlzw_str(c_lzw) == text)

    let c_lzma = compress::lzma_str(text)
    say "LZMA match: " + str(compress::unlzma_str(c_lzma) == text)

    # Generic format dispatcher
    let c_gen = compress::lz_compress_str(text, "lzss")
    say "Generic LZSS match: " + str(compress::lz_decompress_str(c_gen, "lzss") == text)
end

main()
```

### 2. General Algorithms (Brotli, Zstd, GZIP, LZ4, Snappy)

```alya
import "compress" as compress

function main()
    let text = "High throughput systems and ecosystem packages."

    # Brotli (highest compression ratio)
    let br = compress::brotli_str(text)
    say "Brotli: " + str(compress::unbrotli_str(br) == text)

    # Zstandard (fast real-time)
    let zst = compress::zstd_str(text)
    say "Zstd:   " + str(compress::unzstd_str(zst) == text)

    # GZIP
    let gz = compress::gzip_str(text)
    say "GZIP:   " + str(compress::gunzip_str(gz) == text)

    # LZ4 (>1M ops/sec)
    let lz = compress::lz4_str(text)
    say "LZ4:    " + str(compress::unlz4_str(lz) == text)

    # Snappy
    let snp = compress::snappy_str(text)
    say "Snappy: " + str(compress::unsnappy_str(snp) == text)
end

main()
```

### 3. ZIP Archive Operations

```alya
import "compress" as compress

function main()
    let zip_file = "my_archive.zip"

    compress::szip_create(zip_file)
    compress::szip_add_text(zip_file, "hello.txt", "Hello from Alya!")
    compress::szip_add_text(zip_file, "config.json", "{\"status\": \"ok\"}")

    let files = compress::szip_list(zip_file)
    for f in files
        say "Entry: " + f
    end

    let content = compress::szip_extract_text(zip_file, "hello.txt")
    say "Extracted: " + content
end

main()
```

---

## 📖 API Reference

### LZ Family (`compress.lz`)
| Function | Arguments | Returns | Description |
|---|---|---|---|
| `lz_compress(bytes, format)` | `bytes: list<int>, format: str/int` | `list<int>` | Compresses bytes using selected LZ variation. |
| `lz_compress_str(text, format)` | `text: str, format: str/int` | `list<int>` | Compresses string using selected LZ variation. |
| `lz_decompress(bytes, format)` | `bytes: list<int>, format: str/int` | `list<int>` | Decompresses bytes using selected LZ variation. |
| `lz_decompress_str(bytes, format)` | `bytes: list<int>, format: str/int` | `str` | Decompresses bytes directly to UTF-8 string. |
| `lz77(bytes)` / `lz77_str(text)` | `bytes / text` | `list<int>` | LZ77 sliding window compression. |
| `unlz77(bytes)` / `unlz77_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZ77 decompression. |
| `lz78(bytes)` / `lz78_str(text)` | `bytes / text` | `list<int>` | LZ78 dictionary prefix compression. |
| `unlz78(bytes)` / `unlz78_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZ78 decompression. |
| `lzw(bytes)` / `lzw_str(text)` | `bytes / text` | `list<int>` | LZW dictionary compression. |
| `unlzw(bytes)` / `unlzw_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZW decompression. |
| `lzss(bytes)` / `lzss_str(text)` | `bytes / text` | `list<int>` | LZSS sliding window compression. |
| `unlzss(bytes)` / `unlzss_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZSS decompression. |
| `lzma(bytes)` / `lzma_str(text)` | `bytes / text` | `list<int>` | LZMA profile compression. |
| `unlzma(bytes)` / `unlzma_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZMA decompression. |
| `lzma2(bytes)` / `lzma2_str(text)` | `bytes / text` | `list<int>` | LZMA2 chunked profile compression. |
| `unlzma2(bytes)` / `unlzma2_str(bytes)`| `bytes: list<int>` | `list<int> / str` | LZMA2 decompression. |
| `lzjb(bytes)` / `lzjb_str(text)` | `bytes / text` | `list<int>` | LZJB file system profile compression. |
| `unlzjb(bytes)` / `unlzjb_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZJB decompression. |
| `lz4(bytes)` / `lz4_str(text)` | `bytes / text` | `list<int>` | LZ4 high-speed real-time block compression. |
| `unlz4(bytes)` / `unlz4_str(bytes)` | `bytes: list<int>` | `list<int> / str` | LZ4 decompression. |

### Core Algorithms
- **`brotli` / `brotli_str` / `unbrotli` / `unbrotli_str`**: Google Brotli (RFC 7932)
- **`bzip2` / `bzip2_str` / `unbzip2` / `unbzip2_str`**: Burrows-Wheeler Bzip2
- **`deflate` / `deflate_str` / `inflate` / `inflate_str`**: Raw DEFLATE (RFC 1951)
- **`gzip` / `gzip_str` / `gunzip` / `gunzip_str`**: GZIP (RFC 1952)
- **`snappy` / `snappy_str` / `unsnappy` / `unsnappy_str`**: Google Snappy
- **`szip` / `szip_str` / `unszip` / `unszip_str`**: SZIP buffer compression
- **`zlib` / `zlib_str` / `unzlib` / `unzlib_str`**: ZLIB (RFC 1950)
- **`zstd` / `zstd_str` / `unzstd` / `unzstd_str`**: Meta Zstandard (RFC 8878)
- **`crc32` / `crc32_str` / `adler32` / `adler32_str`**: Hardware-accelerated checksums

---

## 🧪 Running Tests, Benchmarks & Documentation

Run the automated test suite using `alya test`:

```bash
alya test
```

Generate static API documentation:

```bash
alya doc . -o docs --markdown
```

Run the benchmark suite:

```bash
alya run benches/bench_basic.alya
```

Run the demonstration:

```bash
alya run examples/demo.alya
```

Check code formatting:

```bash
alya fmt . --check
```

---

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository and clone it locally
2. Install dependencies:
   ```bash
   alya install
   ```
3. Create your feature branch (`git checkout -b feature/my-feature`)
4. Verify tests and formatting before opening a PR:
   ```bash
   alya test
   ```
5. Commit your changes (`git commit -m "feat: add feature"`) and open a Pull Request

---

## 📄 License

MIT License. Bundled third-party libraries:
- `miniz`: MIT License (Rich Geldreich)
- `lz4`: BSD 2-Clause License (Yann Collet)
- `snappy`: New BSD License (Google Inc.)
- `zstd`: BSD 3-Clause License (Meta Platforms, Inc.)
- `bzip2`: Julian Seward Bzip2 License (Julian Seward)
- `brotli`: MIT License (Google Inc.)