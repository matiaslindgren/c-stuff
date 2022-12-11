SHELL  := /bin/sh
CLANG  := clang
CFLAGS := \
	-std=c17 \
	-Wall \
	-Werror \
	-O3 \
	-g \
	-fsanitize=address,undefined
LFLAGS := -lm

OUT_DIR  := out
BIN_DIR  := $(OUT_DIR)/bin
TEST_DIR := $(OUT_DIR)/test

SRC_FILES := $(wildcard src/bin/*.c)
BIN_FILES := $(notdir $(basename $(SRC_FILES)))
BIN_PATHS := $(addprefix $(BIN_DIR)/,$(BIN_FILES))

TEST_SRC_FILES := $(wildcard src/test/*.c)
TEST_BIN_FILES := $(notdir $(basename $(TEST_SRC_FILES)))
TEST_BIN_PATHS := $(addprefix $(TEST_DIR)/,$(TEST_BIN_FILES))

.PHONY: all
all: $(BIN_PATHS) $(TEST_BIN_PATHS)

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

$(OUT_DIR):
	mkdir $@

$(BIN_DIR) $(TEST_DIR): $(OUT_DIR)
	mkdir $@

$(BIN_PATHS): $(BIN_DIR)/%: src/bin/%.c $(wildcard ./include/*.h) | $(BIN_DIR)
	$(CLANG) $(CFLAGS) -I./include -o $@ $< $(LFLAGS)

$(TEST_BIN_PATHS): $(TEST_DIR)/%: src/test/%.c $(wildcard ./include/*.h) | $(TEST_DIR)
	$(CLANG) $(CFLAGS) -I./include -o $@ $< $(LFLAGS)

RUN_TESTS := $(addprefix run_,$(TEST_BIN_FILES))

.PHONY: test
test: $(RUN_TESTS)

.PHONY: $(RUN_TESTS)
$(RUN_TESTS): run_%: $(TEST_DIR)/%
	./$<
