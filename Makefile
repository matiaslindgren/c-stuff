SHELL := /bin/sh

BUILD_DIR         := build
BUILD_DIR_DEBUG   := $(BUILD_DIR)/debug
BUILD_DIR_RELEASE := $(BUILD_DIR)/release

HEADERS     := $(wildcard include/*.h)
TOOLS_SRC   := $(wildcard src/tools/*.c)
TESTS_SRC   := $(wildcard src/tests/*.c)
TOOLS_FILES := $(notdir $(basename $(TOOLS_SRC)))
TESTS_FILES := $(notdir $(basename $(TESTS_SRC)))

TOOLS_DIR_DEBUG   := $(BUILD_DIR_DEBUG)/tools
TESTS_DIR_DEBUG   := $(BUILD_DIR_DEBUG)/tests
TOOLS_DIR_RELEASE := $(BUILD_DIR_RELEASE)/tools
TESTS_DIR_RELEASE := $(BUILD_DIR_RELEASE)/tests

TOOLS_DEBUG   := $(addprefix $(TOOLS_DIR_DEBUG)/,$(TOOLS_FILES))
TESTS_DEBUG   := $(addprefix $(TESTS_DIR_DEBUG)/,$(TESTS_FILES))
TOOLS_RELEASE := $(addprefix $(TOOLS_DIR_RELEASE)/,$(TOOLS_FILES))
TESTS_RELEASE := $(addprefix $(TESTS_DIR_RELEASE)/,$(TESTS_FILES))

CLANG ?= clang-16

CSTANDARD := -std=c2x
CWARNINGS := -Wall -Wpedantic -Werror
LFLAGS    := -lm

CFLAGS_DEBUG   := $(CSTANDARD) $(CWARNINGS) -O2 -g -fsanitize=address,undefined
CFLAGS_RELEASE := $(CSTANDARD) $(CWARNINGS) -O3

.PHONY: all
all: debug release

.PHONY: debug
debug: $(TOOLS_DEBUG) $(TESTS_DEBUG)

.PHONY: release
release: $(TOOLS_RELEASE) $(TESTS_RELEASE)

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $@

$(BUILD_DIR_DEBUG) $(BUILD_DIR_RELEASE): $(BUILD_DIR)
	mkdir $@

$(TOOLS_DIR_DEBUG) $(TESTS_DIR_DEBUG): $(BUILD_DIR_DEBUG)
	mkdir $@

$(TOOLS_DIR_RELEASE) $(TESTS_DIR_RELEASE): $(BUILD_DIR_RELEASE)
	mkdir $@

$(TOOLS_DEBUG) $(TESTS_DEBUG): $(BUILD_DIR_DEBUG)/%: src/%.c $(HEADERS) | $(TOOLS_DIR_DEBUG) $(TESTS_DIR_DEBUG)
	$(CLANG) $(CFLAGS_DEBUG) -I./include -o $@ $< $(LFLAGS)

$(TOOLS_RELEASE) $(TESTS_RELEASE): $(BUILD_DIR_RELEASE)/%: src/%.c $(HEADERS) | $(TOOLS_DIR_RELEASE) $(TESTS_DIR_RELEASE)
	$(CLANG) $(CFLAGS_RELEASE) -I./include -o $@ $< $(LFLAGS)

RUN_DEBUG_TESTS   := $(addprefix run_debug_,$(TESTS_FILES))
RUN_RELEASE_TESTS := $(addprefix run_release_,$(TESTS_FILES))

.PHONY: test_debug
test_debug: $(RUN_DEBUG_TESTS)
.PHONY: test_release
test_release: $(RUN_RELEASE_TESTS)
.PHONY: test_tools
test_tools:
	@bash -c 'timeout --kill-after=2m 1m ./tests/test_tools.bash'

.PHONY: $(RUN_DEBUG_TESTS)
$(RUN_DEBUG_TESTS): run_debug_%: $(TESTS_DIR_DEBUG)/%
	./$<
.PHONY: $(RUN_RELEASE_TESTS)
$(RUN_RELEASE_TESTS): run_release_%: $(TESTS_DIR_RELEASE)/%
	./$<
