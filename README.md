# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly over-engineered solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

## Build

```
make -j
make -j test
```

## PNG tools

Source: [`./src/tool/png.c`](./src/tool/png.c)

Simple PNG decoder implemented without dependencies.

### Usage
```
./out/tool/png info png_path
./out/tool/png dump_raw png_path block_type [block_types...]
./out/tool/png segment png_src_path png_dst_path [--threshold-percent=N] [-v]
```

### PNG info

Decode and inspect a PNG image.

#### Input

![](/docs/img/tokyo.png)

```
./out/tool/png info ./docs/img/tokyo.png
```
#### `stdout`:
```
FILE: ./docs/img/tokyo.png
CHUNKS:
  IHDR: 1
  IDAT: 13
  IEND: 1
  bKGD: 1
  cHRM: 1
  gAMA: 1
  pHYs: 1
  tEXt: 11
  tIME: 1
HEADER:
  width: 500
  height: 500
  bit depth: 8
  color type: rgb
  compression: 0
  filter: 0
  interlace: 0
DATA:
  data length: 756012
  data begin: 0x7fab58847800
  filters: 500
FILTERS:
  Sub: 31
  Average: 228
  Paeth: 241
```

### Image segmentation

Apply mean segmentation on PNG images.

Merges adjacent image segments by comparing the Euclidian distance between the average RGB-pixel of each segment, where each RGB-pixel (3 bytes) is interpreted as a vector of length 3: `[R, G, B]`.

#### Threshold 10%
```
./out/tool/png segment \
  --threshold-percent=10 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_10p.png
```
![](/docs/img/tokyo_segmented_10p.png)

#### Threshold 20%
```
./out/tool/png segment \
  --threshold-percent=20 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_20p.png
```
![](/docs/img/tokyo_segmented_20p.png)

#### Threshold 30%
```
./out/tool/png segment \
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
./out/tool/png dump_raw ./test-data/ff0000-1x1-rgb-fixed.png IHDR IDAT | xxd -b
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

Source: [`./src/tool/sort.c`](./src/tool/sort.c)

Simple and slow line sorting.

### Usage
```
./out/tool/sort { numeric | strings } path
```

### Example

Create data (on macOS, use `gfind`) by calculating the size of each input file used during testing:
```
find ./test-data -printf '%s\n' > test-data-sizes.txt
```

#### Sort lines as numbers
```
./out/tool/sort numeric ./test-data-sizes.txt
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
./out/tool/sort strings ./test-data-sizes.txt
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
./out/tool/sort numeric --reverse ./test-data-sizes.txt
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

Source: [`./src/tool/txt.c`](./src/tool/txt.c)

### Usage
```
./out/tool/txt concat path [paths...]
./out/tool/txt count pattern path
./out/tool/txt replace old_str new_str path
./out/tool/txt slicelines begin end path
./out/tool/txt count_words path
```

### Example

Create 2 files:
```
echo hello > hello.txt
echo there > there.txt
```

#### Concatenate
```
./out/tool/txt concat ./hello.txt ./there.txt
```
#### `stdout`:
```
hello
there
```

#### Replace
```
./out/tool/txt replace hello you ./hello.txt
```
#### `stdout`:
```
you
```

#### Count pattern occurrence
```
./out/tool/txt count '##' ./README.md
```
#### `stdout`:
```
59
```

#### Slice lines
```
./out/tool/txt slicelines 224 250 ./src/tool/txt.c
```
#### `stdout`:
```
int main(int argc, char* const argv[argc + 1]) {
  stufflib_args* args = stufflib_args_from_argv(argc, argv);
  int ok = 0;
  const char* command = stufflib_args_get_positional(args, 0);
  if (command) {
    if (strcmp(command, "concat") == 0) {
      ok = concat(args);
    } else if (strcmp(command, "count") == 0) {
      ok = count(args);
    } else if (strcmp(command, "replace") == 0) {
      ok = replace(args);
    } else if (strcmp(command, "slicelines") == 0) {
      ok = slicelines(args);
    } else if (strcmp(command, "count_words") == 0) {
      ok = count_words(args);
    } else {
      STUFFLIB_PRINT_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(args);
  }
  stufflib_args_destroy(args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

#### Get top 20 words by frequency from `./include/stufflib_png.h`

Tokenisation is simply "split at whitespace".
```
./out/tool/txt count_words ./include/stufflib_png.h \
  | ./out/tool/sort numeric --reverse /dev/stdin \
  | ./out/tool/txt slicelines 0 20 /dev/stdin
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
28 0;
23 STUFFLIB_PRINT_ERROR("failed
23 unsigned
20 static
20 *
19 "
19 for
19 ==
19 char*
18 int
17 !=
```
