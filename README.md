# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly over-engineered solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

## Requirements

* `make`
* `clang-17`, see the [LLVM docs](https://apt.llvm.org/) or stufflib's [CI config](./.github/workflows/c.yml) on how to install pre-release versions of Clang.

### (Optional) Run in Docker

If you can't install Clang 17 using a package manager, you can use Docker:

```sh
./scripts/build_image.sh
./scripts/run_image.sh
```

## Build and test

```sh
make -j && make run_debug_tests && make run_release_tests && make -j run_integration_tests
```

## PNG tools

Source: [`./src/tools/png.c`](./src/tools/png.c)

Simple PNG decoder implemented without dependencies.

### Usage
```
./build/debug/tools/png info png_path
./build/debug/tools/png dump_raw png_path block_type [block_types...]
./build/debug/tools/png segment png_src_path png_dst_path [--threshold-percent=N] [-v]
```

### PNG info

Decode a PNG image and output information in JSON.

This example requires `jq` for formatting the output.
If you don't want to install `jq`, remove `| jq .` from the below example to get the unformatted JSON on a single line.

#### Input

![](/docs/img/tokyo.png)

```
./build/debug/tools/png info ./docs/img/tokyo.png | jq .
```
**`stdout`**:
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
./build/debug/tools/png segment \
  --threshold-percent=10 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_10p.png
```
![](/docs/img/tokyo_segmented_10p.png)

#### Threshold 20%
```
./build/debug/tools/png segment \
  --threshold-percent=20 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_20p.png
```
![](/docs/img/tokyo_segmented_20p.png)

#### Threshold 30%
```
./build/debug/tools/png segment \
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
./build/debug/tools/png dump_raw ./test-data/png/ff0000-1x1-rgb-fixed.png IHDR IDAT | xxd -b
```
**`stdout`**:
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
./build/debug/tools/sort { numeric | ascii } path
```

### Example

Create data (on macOS, use `gfind`) by calculating the size of each input file used during testing:
```
find ./test-data/png -printf '%s\n' > test-data-sizes.txt
```

#### Sort lines as numbers
```
./build/debug/tools/sort numeric ./test-data-sizes.txt
```
**`stdout`**:
```
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
1554
2970
4096
11223
24733
```

#### Sort lines as ASCII strings
```
./build/debug/tools/sort ascii ./test-data-sizes.txt
```
**`stdout`**:
```
11223
1554
160
237
238
238
24733
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
./build/debug/tools/sort numeric --reverse ./test-data-sizes.txt
```
**`stdout`**:
```
24733
11223
4096
2970
1554
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
```

## Text tool

Source: [`./src/tools/txt.c`](./src/tools/txt.c)

### Usage
```
./build/debug/tools/txt concat path [paths...]
./build/debug/tools/txt count pattern path
./build/debug/tools/txt slicelines begin end path
./build/debug/tools/txt replace pattern replacement path
./build/debug/tools/txt linefreq path
```

### Examples

#### Concatenate
```
./build/debug/tools/txt concat ./test-data/txt/wikipedia/water_{ja,is,hi,zh}.txt
```
**`stdout`**:
```
水（みず、（英: water、他言語呼称は「他言語での呼称」の項を参照）とは、化学式 H2O で表される、水素と酸素の化合物である。日本語においては特に湯と対比して用いられ、液体ではあるが温度が低く、かつ凝固して氷にはなっていない物を言う。また、液状の物全般を指す。
Vatn er ólífrænn lyktar-, bragð- og nær litlaus vökvi sem er lífsnauðsynlegur öllum þekktum lífverum, þrátt fyrir að gefa þeim hvorki fæðu, orku né næringarefni. Vatnssameindin er samsett úr tveimur vetnisfrumeindum og einni súrefnisfrumeind sem tengjast með samgildistengi og hefur efnaformúluna H2O. Vatn er uppistaðan í vatnshvolfi jarðar. Orðið „vatn“ á við um efnið eins og það kemur fyrir við staðalhita og staðalþrýsting.
जल या पानी एक आम रासायनिक पदार्थ है जिसका अणु दो हाइड्रोजन परमाणु और एक प्राणवायु परमाणु से बना है - H2O। यह सारे प्राणियों के जीवन का आधार है। आमतौर पर जल शब्द का प्रयोग द्रव अवस्था के लिए उपयोग में लाया जाता है पर यह ठोस अवस्था (बर्फ) और गैसीय अवस्था (भाप या जल वाष्प) में भी पाया जाता है। पानी जल-आत्मीय सतहों पर तरल-क्रिस्टल के रूप में भी पाया जाता है।
水是地球上最常见的物质之一，是由氢、氧两种元素經過化學反應後组成的无机化合物（分子式：H2O），在常温常压下为无色无味的透明液体。
```

#### Count pattern occurrence
```
./build/debug/tools/txt count '#' README.md
./build/debug/tools/txt count '##' README.md
./build/debug/tools/txt count 'ある' README.md
```
**`stdout`**:
```
105
42
3
```

#### Slice lines
```
./build/debug/tools/txt slicelines 312 336 ./src/tools/txt.c
```
**`stdout`**:
```
int main(int argc, char* const argv[argc + 1]) {
  struct sl_args args = sl_args_from_argv(argc, argv);
  bool ok = false;
  const char* command = sl_args_get_positional(&args, 0);
  if (command) {
    if (strcmp(command, "concat") == 0) {
      ok = concat(&args);
    } else if (strcmp(command, "count") == 0) {
      ok = count(&args);
    } else if (strcmp(command, "slicelines") == 0) {
      ok = slicelines(&args);
    } else if (strcmp(command, "replace") == 0) {
      ok = replace(&args);
    } else if (strcmp(command, "linefreq") == 0) {
      ok = linefreq(&args);
    } else {
      SL_LOG_ERROR("unknown command %s", command);
    }
  }
  if (!ok) {
    print_usage(&args);
  }
  sl_args_destroy(&args);
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

#### Replace pattern
```
./build/debug/tools/txt replace sl_iterator it ./include/stufflib_iterator.h
```
**`stdout`**:
```
#ifndef _SL_ITERATOR_H_INCLUDED
#define _SL_ITERATOR_H_INCLUDED
#include <stdlib.h>
#include <string.h>

struct it;

typedef void* it_get_item(struct it*);
typedef void it_advance(struct it*);
typedef bool it_is_done(struct it*);

struct it {
  size_t index;
  size_t pos;
  void* data;
  it_get_item* get_item;
  it_advance* advance;
  it_is_done* is_done;
};

#endif  // _SL_ITERATOR_H_INCLUDED
```

## Combine commands by using `/dev/stdin` as input path

### Run preprocessor on source file and count 25 most common lines

```
clang-17 -std=c2x -E -I./include ./src/tools/txt.c \
  | ./build/debug/tools/txt replace '  ' '' /dev/stdin \
  | ./build/debug/tools/txt replace $'\n ' $'\n' /dev/stdin \
  | ./build/debug/tools/txt linefreq /dev/stdin \
  | ./build/debug/tools/sort numeric --reverse /dev/stdin \
  | ./build/debug/tools/txt slicelines 0 25 /dev/stdin
```
**`stdout`**:
```
249 }
24 };
18 __attribute__ ((__const__));
17 {
13 goto done;
11 } break;
10 return false;
9 __attribute__ ((__nothrow__ )) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
8 __extension__
8 return dst;
7 for (size_t i = 0; i < n; ++i) {
6 bool ok = false;
6 done:
6 __attribute__ ((__nothrow__ )) __attribute__ ((__nonnull__ (1)));
6 } else {
6 if (0x80 <= byte && byte <= 0xbf) {
6 # 1 "/usr/include/aarch64-linux-gnu/bits/libc-header-start.h" 1 3 4
5 struct sl_string content = sl_string_from_file(path);
5 ok = true;
5 sl_string_delete(&content);
5 "\n"
5 # 1 "/usr/include/aarch64-linux-gnu/bits/wordsize.h" 1 3 4
5 return ok;
5 return 0;
5 const size_t args_count = sl_args_count_positional(args) - 1;
```

### Format `NUL`-separated metadata fields in a PNG `tEXt` block
```sh
./build/debug/tools/png dump_raw ./docs/img/tokyo.png tEXt \
  | ./build/debug/tools/txt replace date: $'\n'date= /dev/stdin \
  | ./build/debug/tools/txt replace exif: $'\n'exif= /dev/stdin \
  | ./build/debug/tools/txt replace 0x00 ': ' /dev/stdin \
  && echo
```
**`stdout`**:
```

date=create: 2023-01-23T21:22:19+00:00
date=modify: 2023-01-23T21:22:19+00:00
exif=ColorSpace: 1
exif=ComponentsConfiguration: 1, 2, 3, 0
exif=ExifOffset: 90
exif=ExifVersion: 48, 50, 50, 49
exif=FlashPixVersion: 48, 49, 48, 48
exif=PixelXDimension: 2100
exif=PixelYDimension: 2100
exif=SceneCaptureType: 0
exif=YCbCrPositioning: 1: :
```
