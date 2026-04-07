# c-stuff

C scribbles. Just for fun and learning. "Everything library". Extremely unstable API.

## Tools

[`./tools`](./tools): image processing, text processing, sorting, dataset parsing, SVM.

## Requirements

- `clang-21`
- `make`
- Linux or macOS

See [LLVM docs](https://apt.llvm.org/) or [CI config](./.github/workflows/c.yml) for installation.

Alternatively, use Docker:
```sh
./scripts/build_image.sh && ./scripts/run_image.sh
```

## Build

See [`.github/workflows/c.yml`](.github/workflows/c.yml)

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
