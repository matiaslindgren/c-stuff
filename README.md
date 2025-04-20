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

See [`./tools`](./tools)

## Requirements

It is assumed the target platform is a reasonably common, new OS, with large amounts of RAM (hundreds of MiBs or more).
The current Makefile assumes either Linux or macOS.

* `make`
* `clang-20`
* probably many more depending on your OS/distro

See the [LLVM docs](https://apt.llvm.org/) or stufflib's [CI config](./.github/workflows/c.yml) on how to new Clang versions (or use Docker).

### (Optional) Run in Docker

If you can't install Clang 20 using a package manager, you can use Docker:

```sh
./scripts/build_image.sh
./scripts/run_image.sh
```

## Makefile targets

### Build and test

Unoptimized, with sanitizers (address and UB):
```sh
make -j4 all test integration_test
```

Optimized:
```sh
make RELEASE=1 -j4 all test integration_test
```

### Keep the code tidy

```sh
make fmt
```

### Generate compilation database

```sh
make compile_commands.json
```

## References

or at least some of them

- [Modern C (November 2019), Jens Gustedt](https://www.manning.com/books/modern-c)
- [Effective C (August 2020), Robert C. Seacord](https://nostarch.com/Effective_C)
- [Fluent C (October 2022), Christopher Preschern](https://learning.oreilly.com/library/view/fluent-c/9781492097273)
- [The Unicode® Standard Version 15.0 – Core Specification](https://www.unicode.org/versions/Unicode15.0.0)
- [ZLIB Compressed Data Format Specification version 3.3](https://datatracker.ietf.org/doc/html/rfc1950)
- [DEFLATE Compressed Data Format Specification version 1.3](https://datatracker.ietf.org/doc/html/rfc1951)
- [PNG (Portable Network Graphics) Specification, Version 1.2](http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html)
- [Beej's Guide to Network Programming, v3.1.11](https://beej.us/guide/bgnet/html/split/index.html)
- [GNU Make papers (November 2014), Paul D. Smith](https://make.mad-scientist.net/papers/)
- [The GNU Make Book (April 2015), John Graham-Cumming](https://nostarch.com/gnumake)
