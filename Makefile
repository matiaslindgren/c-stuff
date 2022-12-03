SHELL  := /bin/sh
CLANG  := clang
CFLAGS := \
	-std=c17 \
	-Wall \
	-Werror \
	-O3 \
	-g \
	-fsanitize=undefined
LFLAGS := -lm

OUT_DIR   := out
SRC_FILES := $(wildcard src/*.c)
BIN_FILES := $(notdir $(basename $(SRC_FILES)))
BIN_PATHS := $(addprefix $(OUT_DIR)/,$(BIN_FILES))

.PHONY: all
all: $(BIN_PATHS)

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

$(BIN_PATHS): $(OUT_DIR)/%: src/%.c | $(OUT_DIR)
	$(CLANG) $(CFLAGS) -I./include -o $@ $< $(LFLAGS)

$(OUT_DIR):
	mkdir $@
