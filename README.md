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

### Examples

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


## Stream text editor: `./src/tool/txt.c`

### Example
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

### Combine commands
```
./out/tool/txt replace hello you ./hello.txt \
  | ./out/tool/txt replace \n ' ' /dev/stdin \
  | ./out/tool/txt concat /dev/stdin ./there.txt
```
#### `stdout`:
```
you there
```

## Sorting: `./src/tool/sort.c`

Create data (on macOS, use `gfind`):
```
find ./test-data -printf '%s\n' > test-data-sizes.txt
```

### Sort lines as doubles
```
./out/tool/sort doubles ./test-data-sizes.txt
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
