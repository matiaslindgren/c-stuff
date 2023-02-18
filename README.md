# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly over-engineered solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

## Requirements

* `make`
* `clang-16`, see the [LLVM docs](https://apt.llvm.org/) or stufflib's [CI config](./.github/workflows/c.yml) on how to install pre-release versions of Clang.

## Building

```sh
make -j
make -j test_debug
make -j test_release
make test_tools
```

## PNG tools

Source: [`./src/tools/png.c`](./src/tools/png.c)

Simple PNG decoder implemented without dependencies.

### Usage
```
./build/release/tools/png info png_path
./build/release/tools/png dump_raw png_path block_type [block_types...]
./build/release/tools/png segment png_src_path png_dst_path [--threshold-percent=N] [-v]
```

### PNG info

Decode and inspect a PNG image. Outputs JSON.

This example requires `jq` for formatting the output.
If you don't want to install `jq`, remove `| jq .` from the below example to get the unformatted JSON on a single line.

#### Input

![](/docs/img/tokyo.png)

```
./build/release/tools/png info ./docs/img/tokyo.png | jq .
```
#### `stdout`:
```
{
  "chunks": {
    "IHDR": 1,
    "IDAT": 13,
    "IEND": 1,
    "bKGD": 1,
    "cHRM": 1,
    "gAMA": 1,
    "pHYs": 1,
    "tEXt": 11,
    "tIME": 1
  },
  "header": {
    "width": 500,
    "height": 500,
    "bit depth": 8,
    "color type": "rgb",
    "compression": 0,
    "filter": 0,
    "interlace": 0
  },
  "data": {
    "length": 756012,
    "filters": {
      "Sub": 31,
      "Average": 228,
      "Paeth": 241
    }
  }
}
```

### Image segmentation

Apply mean segmentation on PNG images.

Merges adjacent image segments by comparing the Euclidian distance between the average RGB-pixel of each segment, where each RGB-pixel (3 bytes) is interpreted as a vector of length 3: `[R, G, B]`.

#### Threshold 10%
```
./build/release/tools/png segment \
  --threshold-percent=10 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_10p.png
```
![](/docs/img/tokyo_segmented_10p.png)

#### Threshold 20%
```
./build/release/tools/png segment \
  --threshold-percent=20 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_20p.png
```
![](/docs/img/tokyo_segmented_20p.png)

#### Threshold 30%
```
./build/release/tools/png segment \
  --threshold-percent=30 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_30p.png
```
![](/docs/img/tokyo_segmented_30p.png)


### Dump raw chunks

Decode a PNG image into chunks and write raw chunk data to stdout.
Use positional arguments to filter a subset of chunk types.

#### Example: dump IHDR and IDAT contents of a single red pixel

This example requires `xxd`.

```
./build/release/tools/png dump_raw ./test-data/ff0000-1x1-rgb-fixed.png IHDR IDAT | xxd -b
```
#### `stdout`:
```
00000000: 00000000 00000000 00000000 00000001 00000000 00000000  ......
00000006: 00000000 00000001 00001000 00000010 00000000 00000000  ......
0000000c: 00000000 00001000 00011101 01100011 11111000 11001111  ...c..
00000012: 11000000 00000000 00000000 00000011 00000001 00000001  ......
00000018: 00000000                                               .
```

## Sorting

Source: [`./src/tools/sort.c`](./src/tools/sort.c)

Simple line sorting.

### Usage
```
./build/release/tools/sort { numeric | strings } path
```

### Example

Create data (on macOS, use `gfind`) by calculating the size of each input file used during testing:
```
find ./test-data -printf '%s\n' > test-data-sizes.txt
```

#### Sort lines as numbers
```
./build/release/tools/sort numeric ./test-data-sizes.txt
```
#### `stdout`:
```
0
1
11
69
69
69
72
72
72
72
160
237
238
238
292
1554
2970
4096
11223
24733
```

#### Sort lines as strings
```
./build/release/tools/sort strings ./test-data-sizes.txt
```
#### `stdout`:
```
0
1
11
11223
1554
160
237
238
238
24733
292
2970
4096
69
69
69
72
72
72
72
```

#### Sort lines as numbers in descending order
```
./build/release/tools/sort numeric --reverse ./test-data-sizes.txt
```
#### `stdout`:
```
24733
11223
4096
2970
1554
292
238
238
237
160
72
72
72
72
69
69
69
11
1
0
```

## Stream text editor

Source: [`./src/tools/txt.c`](./src/tools/txt.c)

### Usage
```
./build/release/tools/txt concat path [paths...]
./build/release/tools/txt count pattern path
./build/release/tools/txt replace old_str new_str path
./build/release/tools/txt slicelines begin end path
./build/release/tools/txt count_words path
```

### Example

Create 2 files:
```
echo hello > hello.txt
echo there > there.txt
```

#### Concatenate
```
./build/release/tools/txt concat ./hello.txt ./there.txt
```
#### `stdout`:
```
hello
there
```

#### Replace
```
./build/release/tools/txt replace hello you ./hello.txt
```
#### `stdout`:
```
you
```

#### Count pattern occurrence
```
./build/release/tools/txt count '##' ./README.md
```
#### `stdout`:
```
60
```

#### Slice lines
```
./build/release/tools/txt slicelines 224 250 ./src/tools/txt.c
```
#### `stdout`:
```
int main(int argc, char* const argv[argc + 1]) {
  stufflib_args args = stufflib_args_from_argv(argc, argv);
  bool ok = false;
  const char* command = stufflib_args_get_positional(&args, 0);
  if (command) {
    if (strcmp(command, "concat") == 0) {
      ok = concat(&args);
    } else if (strcmp(command, "count") == 0) {
      ok = count(&args);
    } else if (strcmp(command, "replace") == 0) {
      ok = replace(&args);
    } else if (strcmp(command, "slicelines") == 0) {
      ok = slicelines(&args);
    } else if (strcmp(command, "count_words") == 0) {
      ok = count_words(&args);
    } else {
      STUFFLIB_PRINT_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  stufflib_args_destroy(&args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

#### Get top 20 words by frequency from `./include/stufflib_png.h`

Tokenisation is simply "split at whitespace".
```
./build/release/tools/txt count_words ./include/stufflib_png.h \
  | ./build/release/tools/sort numeric --reverse /dev/stdin \
  | ./build/release/tools/txt slicelines 0 20 /dev/stdin
```
#### `stdout`:
```
177 =
122 {
111 }
81 const
54 if
45 +
42 size_t
37 return
31 goto
23 STUFFLIB_PRINT_ERROR("failed
23 unsigned
20 static
20 *
19 ==
19 "
19 for
19 char*
17 !=
16 <
16 chunk
```
