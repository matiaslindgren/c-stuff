# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly over-engineered solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

## `src/tools`

### `segment_image.c`

Merge adjacent image segments by comparing the Euclidian distance between the average RGB-pixel of each segment.

#### input

![](/docs/img/tokyo.png)

#### output (`--threshold-percent=10`)

![](/docs/img/tokyo_segmented_10p.png)

#### output (`--threshold-percent=20`)

![](/docs/img/tokyo_segmented_20p.png)

#### output (`--threshold-percent=30`)

![](/docs/img/tokyo_segmented_30p.png)

### `txt.c`

Stream text editor.

Create 2 files:
```
echo hello > hello.txt
echo there > there.txt
```

#### Concatenate
```
./out/tool/txt concat ./hello.txt ./there.txt
```
`stdout`:
```
hello
there
```

#### Replace
```
./out/tool/txt replace hello you ./hello.txt
```
`stdout`:
```
you
```

#### Count pattern occurrence
```
./out/tool/txt count l ./hello.txt
```
`stdout`:
```
2
```

#### Combine commands
```
./out/tool/txt replace hello you ./hello.txt \
  | ./out/tool/txt replace \n ' ' /dev/stdin \
  | ./out/tool/txt concat /dev/stdin ./there.txt
```
`stdout`:
```
you there
```
