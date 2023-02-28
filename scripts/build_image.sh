#!/usr/bin/env sh
set -ue

docker build \
	--no-cache \
	--tag c-stuff:1 \
	.
