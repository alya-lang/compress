# compress

[![CI](https://github.com/alya-lang/compress/actions/workflows/ci.yml/badge.svg)](https://github.com/alya-lang/compress/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/alya-lang/compress?color=blue&label=License)](LICENSE)
[![Alya](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.alya-version&label=Alya&color=orange&prefix=%3E%3D)](https://github.com/alya-lang/alya)
[![Package Version](https://img.shields.io/badge/dynamic/toml?url=https%3A%2F%2Fraw.githubusercontent.com%2Falya-lang%2Fcompress%2Fmain%2Falya.toml&query=%24.package.version&label=Version&color=brightgreen)](alya.toml)

Comprehensive compression library for Alya (Gzip, Zlib, Deflate, Snappy, LZ4)

---

## 🌟 Features

- ⚡ **Lightweight & Fast**: Built for speed with minimal overhead
- 🧩 **Modular Architecture**: Multi-module design supporting flat modules (`types.alya`) and subfolder hierarchies (`core/formatter.alya`)
- 🛡️ **Reliable & Typed**: Explicit struct definitions and clean namespaced APIs
- 🧪 **Well Tested**: Comprehensive test suite with standard assertions

---

## 📁 Project Architecture

```
compress/
├── alya.toml               # Package manifest
├── c/                      # (Optional) Native C sources for zero-dependency FFI packages
├── src/
│   ├── lib.alya            # Public API facade
│   ├── types.alya          # Data structures & struct definitions
│   ├── ffi.alya            # (Optional) Native extern "C" declarations
│   └── core/               # Subdirectory module hierarchy (optional for larger packages)
│       └── formatter.alya  # Domain formatting logic & internal helpers
├── examples/
│   └── demo.alya           # Runnable usage examples
├── tests/
│   └── test_basic.alya     # Automated test suite
└── benches/
    └── bench_basic.alya    # Micro-benchmarks
```

> [!NOTE]
> **Modular Source & Native C:** Modules can be structured flat inside `src/` (e.g. `src/types.alya`) or grouped into subdirectories (e.g. `src/core/formatter.alya`). Packages bundling native C sources declare them in `alya.toml` under `[build]` (`c-sources`, `c-flags`, `c-include-dirs`); `alyac` automatically compiles and caches them into `.o` object files in `~/.alya/c_obj` with zero runtime dependency overhead.

---

## 📦 Installation

Add `compress` to the `[dependencies]` section in your `alya.toml`:

```toml
[dependencies]
compress = { git = "https://github.com/alya-lang/compress", branch = "main" }
```

Or install it directly using the Alya package CLI:

```bash
alyac add compress --git https://github.com/alya-lang/compress --branch main
alyac install
```

---

## 🚀 Quick Start

```alya
import "compress" as pkg

function main()
    # Basic facade call
    let greeting = pkg::hello("Alya")
    say greeting

    # Struct construction and domain helpers
    let cfg = pkg::new_config("Community", 2)
    say "Target: " + cfg.name
    say "Formatted: " + pkg::core_format_custom(cfg)
end

main()
```

---

## 📖 API Reference

| Function | Arguments | Returns | Description |
|---|---|---|---|
| `hello(name)` | `name = "World"` | `string` | Returns a friendly greeting message. |
| `new_config(name, count)` | `name = "World", count = 1` | `CompressConfig` | Constructs a new configuration struct. |
| `core_format_greeting(name)` | `name` | `string` | Core formatter producing `Hello, {name}!`. |
| `core_format_custom(config)` | `config: CompressConfig` | `string` | Formats greeting using prefix and name from config. |

---

## 🧪 Running Tests & Benchmarks

Run the test suite using `alyac`:

```bash
alyac run tests/test_basic.alya
```

Run the benchmark suite:

```bash
alyac run benches/bench_basic.alya
```

Run the example demo:

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
3. Create your feature branch (`git checkout -b feature/my-feature`)
4. Verify tests and formatting before opening a PR:
   ```bash
   alyac test
   alyac fmt . --check
   ```
5. Commit your changes (`git commit -m "feat: add feature"`) and open a Pull Request

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.