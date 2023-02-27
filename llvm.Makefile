SHELL   := /bin/sh
OUTPUT  := ./llvm-build
SRC_DIR := ./llvm-project

SRC_TAR_URL := https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0-rc3/llvm-project-16.0.0rc3.src.tar.xz
SRC_SHA256  := 8fa9792a1a04f78d023b87c5e5bd25010d5f73a5c0ae1830e897d85821ee9325

ifeq ($(shell uname),Darwin)
	SHA256 := shasum -a 256
else
	SHA256 := sha256sum
endif

.PHONY: all
all: $(OUTPUT)

.PHONY: clean
clean:
	$(RM) -r $(OUTPUT)

$(SRC_DIR).tar.xz:
	curl --output $@ --location $(SRC_TAR_URL)

$(SRC_DIR): $(SRC_DIR).tar.xz
	[ "$$($(SHA256) $< | tr -d ' ')" = "$(SRC_SHA256)$<" ] || exit 1
	mkdir -p $@
	cp $< $@/
	tar --directory $@ --strip-components 1 -xf $@/$<
	rm $@/$<

$(OUTPUT): $(SRC_DIR)
	mkdir -p $@
	cd $@ && cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt" \
		-G "Unix Makefiles" \
		-DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
		../$</llvm
	make --directory=$@ --jobs 2
	make --directory=$@ --jobs 2 check-all
