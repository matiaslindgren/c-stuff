# c-stuff

Useless library/toolkit/scribble that reimplements solved problems for the sake of learning.
Contains mostly solutions to the programming challenges suggested by Jens Gustedt in [Modern C](https://gustedt.gitlabpages.inria.fr/modern-c/).

## `src/tools`

### `segment_image.c`

Merge adjacent image segments by comparing the Euclidian distance between the average RGB-pixel of each segment.

#### input

![](/docs/img/tokyo.png)

#### output

![](/docs/img/tokyo_segmented.png)
