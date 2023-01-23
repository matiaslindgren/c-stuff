# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

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
