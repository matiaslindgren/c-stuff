# TODO

- [ ] unify initialization and destruction API
- [ ] replace dynamic allocations with paged memory arena
- [ ] implement non-allocating args parser
- [ ] fix argv parsing of required vs optional args, e.g. negative numbers as positional args are currently not possible
- [ ] cleanup and comment DEFLATE decoder, implement simple compression
- [ ] benchmark CRC32 as hash function, collisions etc
- [ ] optimize huffman code tree
- [ ] handle different PNG color types
- [ ] verify the IO header handles all errors gracefully
- [ ] implement more and better math stuff
- [ ] use iterator API in PNG chunks parsing
- [ ] output all text as JSON
- [ ] fix rand lib seeds
- [ ] benchmark sorting, especially edge cases
- [ ] replace all c-strings with unicode
- [ ] ensure unicode decode/encode conforms to standard
- [ ] separate tests from other headers
- [ ] refactor entire stufflib into components, compile as static library, link manually
- [ ] benchmark utf-8 decoder
- [ ] implement log levels and replace `STUFFLIB_LOG` macros as no-ops if level too low
- [ ] `printf` for unicode (`wchar_t` and `wprintf`?)
- [ ] try to replace `void*` casts with `_Generic`

- [x] unify iterator API for getting values, don't transform, only return a single item/range/chunk
- [x] replace str lib with unicode strings
- [x] cleanup hashmap of unnecessary code, doesn't need e.g. insert

## read
- https://www.cl.cam.ac.uk/~mgk25/unicode.html
- https://stackoverflow.com/questions/526430/c-programming-how-to-program-for-unicode
- https://www.unicode.org/versions/Unicode15.0.0/
