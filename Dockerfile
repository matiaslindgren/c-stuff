FROM debian:sid-slim

ENV LANG=C.UTF-8
ARG CLANG_VERSION=17

RUN apt update --yes \
    && apt install --yes curl \
    && curl --location https://apt.llvm.org/llvm-snapshot.gpg.key >> /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && echo "deb http://apt.llvm.org/unstable/ llvm-toolchain-${CLANG_VERSION} main" >> /etc/apt/sources.list \
    && echo "deb-src http://apt.llvm.org/unstable/ llvm-toolchain-${CLANG_VERSION} main" >> /etc/apt/sources.list

RUN apt update --yes \
    && apt install --yes \
       clang-${CLANG_VERSION} \
       lld-${CLANG_VERSION} \
       clang-format-${CLANG_VERSION} \
       make \
       xxd \
       jq

WORKDIR /c-stuff
