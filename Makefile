SHELL  := /bin/sh
CC     := clang
CFLAGS := \
	-std=c17 \
	-Wall \
	-Werror \
	-O3 \
	-g \
	-fsanitize=address,undefined
LFLAGS := -lm

OUT_DIR  := out
TOOL_DIR := $(OUT_DIR)/tool
TEST_DIR := $(OUT_DIR)/test

TOOL_SRCS  := $(wildcard src/tool/*.c)
TOOL_FILES := $(notdir $(basename $(TOOL_SRCS)))
TOOL_PATHS := $(addprefix $(TOOL_DIR)/,$(TOOL_FILES))

TEST_SRCS  := $(wildcard src/test/*.c)
TEST_FILES := $(notdir $(basename $(TEST_SRCS)))
TEST_PATHS := $(addprefix $(TEST_DIR)/,$(TEST_FILES))

.PHONY: all
all: $(TOOL_PATHS) $(TEST_PATHS)

.PHONY: clean
clean:
	$(RM) -r $(OUT_DIR)

$(OUT_DIR):
	mkdir $@

$(TOOL_DIR) $(TEST_DIR): $(OUT_DIR)
	mkdir $@

$(TOOL_PATHS) $(TEST_PATHS): $(OUT_DIR)/%: src/%.c $(wildcard ./include/*.h) | $(TOOL_DIR) $(TEST_DIR)
	$(CC) $(CFLAGS) -I./include -o $@ $< $(LFLAGS)

RUN_TESTS := $(addprefix run_,$(TEST_FILES))

.PHONY: test
test: $(RUN_TESTS)

.PHONY: $(RUN_TESTS)
$(RUN_TESTS): run_%: $(TEST_DIR)/%
	./$<
