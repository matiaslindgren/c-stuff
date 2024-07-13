# c-stuff

C scribbles. "Everything library". Minimal dependencies (stdlib, openBLAS).

Unstable API, don't depend on this.

## Tools

Functionality (very limited):

* image processing
* text processing
* sorting
* dataset parsing
* support vector machines

See [`./src/tools`](./src/tools)

## Requirements

It is assumed the target platform is a reasonably common, new OS, with large amounts of RAM (hundreds of MiBs or more).
The current Makefile assumes either Linux or macOS.

* `make`
* `clang-18`
* probably many more depending on your OS/distro

See the [LLVM docs](https://apt.llvm.org/) or stufflib's [CI config](./.github/workflows/c.yml) on how to new Clang versions (or use Docker).

### (Optional) Run in Docker

If you can't install Clang 18 using a package manager, you can use Docker:

```sh
./scripts/build_image.sh
./scripts/run_image.sh
```

## Makefile targets

### Build and test

Unoptimized, with sanitizers (address and UB):
```sh
make DEBUG=1 -j4 all test integration_test
```

Optimized:
```sh
make -j4 all test integration_test
```

### Keep the code tidy

```sh
make fmt
```

### Generate compilation database

```sh
make DEBUG=1 compile_commands.json
```

## References

or at least some of them

- [Modern C, Jens Gustedt](https://gustedt.gitlabpages.inria.fr/modern-c/)
- [Effective C, Robert C. Seacord](https://nostarch.com/Effective_C)
- [The Unicode® Standard Version 15.0 – Core Specification](https://www.unicode.org/versions/Unicode15.0.0)
- [ZLIB Compressed Data Format Specification version 3.3](https://datatracker.ietf.org/doc/html/rfc1950)
- [DEFLATE Compressed Data Format Specification version 1.3](https://datatracker.ietf.org/doc/html/rfc1951)
- [PNG (Portable Network Graphics) Specification, Version 1.2](http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html)
