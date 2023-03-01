#!/usr/bin/env sh
set -ue

docker run \
	--pull never \
	--rm \
	-v ${PWD}/include:/c-stuff/include \
	-v ${PWD}/src:/c-stuff/src \
	-v ${PWD}/Makefile:/c-stuff/Makefile \
	-v ${PWD}/README.md:/c-stuff/README.md \
	-v ${PWD}/docs:/c-stuff/docs \
	-v ${PWD}/test-data:/c-stuff/test-data \
	-v ${PWD}/tests:/c-stuff/tests \
	--interactive \
	--tty \
	c-stuff:1 \
	bash
