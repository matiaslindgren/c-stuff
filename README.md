# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly over-engineered solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

## Build

```
make -j
make -j test
```

## Image segmentation: `./src/tool/segment_image.c`

Merge adjacent image segments by comparing the Euclidian distance between the average RGB-pixel of each segment.

#### Input: `./docs/img/tokyo.png`

![](/docs/img/tokyo.png)

#### Output: `./docs/img/tokyo_segmented_10p.png`
```
./out/tool/segment_image \
  --threshold-percent=10 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_10p.png
```
![](/docs/img/tokyo_segmented_10p.png)

#### Output: `./docs/img/tokyo_segmented_20p.png`
```
./out/tool/segment_image \
  --threshold-percent=20 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_20p.png
```
![](/docs/img/tokyo_segmented_20p.png)

#### Output: `./docs/img/tokyo_segmented_30p.png`
```
./out/tool/segment_image \
  --threshold-percent=30 \
  ./docs/img/tokyo.png \
  ./docs/img/tokyo_segmented_30p.png
```
![](/docs/img/tokyo_segmented_30p.png)


## Sorting: `./src/tool/sort.c`

Simple and slow sorting.

Create data (on macOS, use `gfind`) by calculating the size of each input file used during testing:
```
find ./test-data -printf '%s\n' > test-data-sizes.txt
```

### Sort lines as numeric
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

### Sort lines as strings
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

### Sort lines as numeric in descending order
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

## Stream text editor: `./src/tool/txt.c`

Create 2 files:
```
echo hello > hello.txt
echo there > there.txt
```

### Concatenate
```
./out/tool/txt concat ./hello.txt ./there.txt
```
#### `stdout`:
```
hello
there
```

### Replace
```
./out/tool/txt replace hello you ./hello.txt
```
#### `stdout`:
```
you
```

### Count pattern occurrence
```
./out/tool/txt count '##' ./README.md
```
#### `stdout`:
```
33
```

### Slice lines
```
./out/tool/txt slicelines 159 182 ./src/tool/txt.c
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

### Get top 20 words by frequency from `./include/stufflib_png.h`
```
./out/tool/txt count_words ./include/stufflib_png.h \
  | ./out/tool/sort numeric --reverse /dev/stdin \
  | ./out/tool/txt slicelines 0 20 /dev/stdin
```
#### `stdout`:
```
56 }
55 {
44 const
25 =
25 size_t
22 }
20 return
18 *
18 }
16 =
16 =
15 if
15 error;
12 {
12 unsigned
12 fprintf(stream,
11 STUFFLIB_PRINT_ERROR("failed
11 row
11 size_t
10 int
```
