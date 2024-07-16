# TODO

- [ ] unify initialization and destruction API
- [ ] replace dynamic allocations with paged memory arena
- [ ] fix argv parsing of required vs optional args, e.g. negative numbers as positional args are currently not possible
- [ ] cleanup and comment DEFLATE decoder, implement simple compression
- [ ] implement better hash func?
- [ ] replace hard-coded `size_t` hashmap value with `void*`
- [ ] optimize huffman code tree
- [ ] handle different PNG color types
- [ ] verify the IO header handles all errors gracefully
- [ ] implement more and better math stuff
- [ ] use iterator API in PNG chunks parsing
- [ ] output all text as JSON
- [ ] fix rand lib seeds
- [ ] replace all c-strings with unicode
- [ ] ensure unicode decode/encode conforms to standard
- [ ] separate tests from other headers
- [ ] refactor entire stufflib into components, compile as static library, link manually
- [ ] `printf` for unicode (`wchar_t` and `wprintf`?)
- [ ] try to replace `void*` casts with `_Generic`
- [ ] return `struct sl_data` from math functions
- [ ] store hashes in hash table nodes to avoid recomputing on resize
- [ ] use power of 2 for underlying array and map hash to index with bitand size, instead of modulo (rely on compiler for this but use pow 2 sizes)
- [ ] rethink unnecessary buffering when reading files with iterators
- [ ] simplify line reader iterators
- [ ] span with item size, length, capacity
- [ ] benchmark
    - CRC32 as hash function, collisions etc
    - sorting, especially edge cases
    - utf-8 decoder
    - linalg
    - everything

- [x] unify iterator API for getting values, don't transform, only return a single item/range/chunk
- [x] replace str lib with unicode strings
- [x] cleanup hashmap of unnecessary code, doesn't need e.g. insert
- [x] implement non-allocating args parser
- [x] implement log levels and replace `STUFFLIB_LOG` macros as no-ops if level too low

## read
- https://www.cl.cam.ac.uk/~mgk25/unicode.html
- https://stackoverflow.com/questions/526430/c-programming-how-to-program-for-unicode
- https://www.unicode.org/versions/Unicode15.0.0/
- https://developer.apple.com/documentation/accelerate/blas?language=objc
- `#include <Accelerate/Accelerate.h>`
