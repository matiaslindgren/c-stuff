#!/usr/bin/env sh
set -ue

docker run \
	--pull never \
	--rm \
	-v ${PWD}/Makefile:/c-stuff/Makefile \
	-v ${PWD}/README.md:/c-stuff/README.md \
	-v ${PWD}/docs:/c-stuff/docs \
	-v ${PWD}/stufflib:/c-stuff/stufflib \
	-v ${PWD}/test-data:/c-stuff/test-data \
	-v ${PWD}/tests:/c-stuff/tests \
	-v ${PWD}/tools:/c-stuff/tools \
	--interactive \
	--tty \
	c-stuff:1 \
	bash
