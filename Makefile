SHELL  := /bin/sh
CLANG  := clang
CFLAGS := \
	-std=c17 \
	-Wall \
	-Werror \
	-O3 \
	-g
LFLAGS := -lm

OUT_DIR   := out
SRC_FILES := $(wildcard src/*.c)
BIN_FILES := $(notdir $(basename $(SRC_FILES)))
BIN_PATHS := $(addprefix $(OUT_DIR)/,$(BIN_FILES))

$(BIN_PATHS): $(OUT_DIR)/%: src/%.c | $(OUT_DIR)
	$(CLANG) $(CFLAGS) -o $@ $< $(LFLAGS)

$(OUT_DIR):
	mkdir $@
