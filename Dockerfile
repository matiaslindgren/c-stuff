FROM debian:sid-slim

ENV LANG=C.UTF-8
ARG LLVM_VERSION=18

RUN apt update --yes \
    && apt install --yes curl \
    && curl --location https://apt.llvm.org/llvm-snapshot.gpg.key >> /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && echo "deb http://apt.llvm.org/unstable/ llvm-toolchain-${LLVM_VERSION} main" >> /etc/apt/sources.list \
    && echo "deb-src http://apt.llvm.org/unstable/ llvm-toolchain-${LLVM_VERSION} main" >> /etc/apt/sources.list

RUN apt update --yes \
    && apt install --yes \
       clang-${LLVM_VERSION} \
       lld-${LLVM_VERSION} \
       clang-format-${LLVM_VERSION} \
       make \
       xxd \
       jq

WORKDIR /c-stuff
