FROM debian:sid-slim

RUN apt update --yes \
  && apt install --yes curl \
  && curl --location https://apt.llvm.org/llvm-snapshot.gpg.key >> /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
  && echo 'deb http://apt.llvm.org/unstable/ llvm-toolchain-16 main' >> /etc/apt/sources.list \
  && echo 'deb-src http://apt.llvm.org/unstable/ llvm-toolchain-16 main' >> /etc/apt/sources.list \
  && apt update --yes \
  && apt install --yes clang-16 make xxd jq

WORKDIR /c-stuff
