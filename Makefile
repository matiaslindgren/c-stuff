SHELL := /bin/sh
CC    := clang

CFLAGS_DEBUG := \
	-std=c17 \
	-Wall \
	-Werror \
	-O2 \
	-g \
	-fsanitize=address,undefined

CFLAGS_RELEASE := \
	-std=c17 \
	-Wall \
	-Werror \
	-O3

LFLAGS := -lm

OUT_DIR         := out
OUT_DIR_DEBUG   := $(OUT_DIR)/debug
OUT_DIR_RELEASE := $(OUT_DIR)/release

TOOLS_SRC   := $(wildcard src/tools/*.c)
TESTS_SRC   := $(wildcard src/tests/*.c)
TOOLS_FILES := $(notdir $(basename $(TOOLS_SRC)))
TESTS_FILES := $(notdir $(basename $(TESTS_SRC)))

TOOLS_DIR_DEBUG   := $(OUT_DIR_DEBUG)/tools
TESTS_DIR_DEBUG   := $(OUT_DIR_DEBUG)/tests
TOOLS_DIR_RELEASE := $(OUT_DIR_RELEASE)/tools
TESTS_DIR_RELEASE := $(OUT_DIR_RELEASE)/tests

TOOLS_DEBUG   := $(addprefix $(TOOLS_DIR_DEBUG)/,$(TOOLS_FILES))
TESTS_DEBUG   := $(addprefix $(TESTS_DIR_DEBUG)/,$(TESTS_FILES))
TOOLS_RELEASE := $(addprefix $(TOOLS_DIR_RELEASE)/,$(TOOLS_FILES))
TESTS_RELEASE := $(addprefix $(TESTS_DIR_RELEASE)/,$(TESTS_FILES))

.PHONY: all
all: debug release

.PHONY: debug
debug: $(TOOLS_DEBUG) $(TESTS_DEBUG)

.PHONY: release
release: $(TOOLS_RELEASE) $(TESTS_RELEASE)

.PHONY: clean
clean:
	$(RM) -r $(OUT_DIR)

$(OUT_DIR):
	mkdir $@

$(OUT_DIR_DEBUG) $(OUT_DIR_RELEASE): $(OUT_DIR)
	mkdir $@

$(TOOLS_DIR_DEBUG) $(TESTS_DIR_DEBUG): $(OUT_DIR_DEBUG)
	mkdir $@

$(TOOLS_DIR_RELEASE) $(TESTS_DIR_RELEASE): $(OUT_DIR_RELEASE)
	mkdir $@

$(TOOLS_DEBUG) $(TESTS_DEBUG): $(OUT_DIR_DEBUG)/%: src/%.c $(wildcard ./include/*.h) | $(TOOLS_DIR_DEBUG) $(TESTS_DIR_DEBUG)
	$(CC) $(CFLAGS_DEBUG) -I./include -o $@ $< $(LFLAGS)

$(TOOLS_RELEASE) $(TESTS_RELEASE): $(OUT_DIR_RELEASE)/%: src/%.c $(wildcard ./include/*.h) | $(TOOLS_DIR_RELEASE) $(TESTS_DIR_RELEASE)
	$(CC) $(CFLAGS_RELEASE) -I./include -o $@ $< $(LFLAGS)

RUN_DEBUG_TESTS   := $(addprefix run_debug_,$(TESTS_FILES))
RUN_RELEASE_TESTS := $(addprefix run_release_,$(TESTS_FILES))

.PHONY: test_debug
test_debug: $(RUN_DEBUG_TESTS)
.PHONY: test_release
test_release: $(RUN_RELEASE_TESTS)

.PHONY: $(RUN_DEBUG_TESTS)
$(RUN_DEBUG_TESTS): run_debug_%: $(TESTS_DIR_DEBUG)/%
	./$<
.PHONY: $(RUN_RELEASE_TESTS)
$(RUN_RELEASE_TESTS): run_release_%: $(TESTS_DIR_RELEASE)/%
	./$<
