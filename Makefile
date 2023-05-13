SHELL := /bin/sh

INCLUDE_DIR := $(abspath ./include)
SOURCE_DIR  := $(abspath ./src)
BUILD_DIR   := $(abspath ./build)

HEADERS     := $(wildcard $(INCLUDE_DIR)/*.h)
TOOLS_SRC   := $(wildcard $(SOURCE_DIR)/tools/*.c)
TESTS_SRC   := $(wildcard $(SOURCE_DIR)/tests/*.c)
TOOLS_FILES := $(notdir $(basename $(TOOLS_SRC)))
TESTS_FILES := $(notdir $(basename $(TESTS_SRC)))

BUILD_DIR_DEBUG   := $(BUILD_DIR)/debug
BUILD_DIR_RELEASE := $(BUILD_DIR)/release
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

ALL_DIRS := \
	$(BUILD_DIR) \
	$(BUILD_DIR_DEBUG) \
	$(BUILD_DIR_RELEASE) \
	$(TOOLS_DIR_DEBUG) \
	$(TOOLS_DIR_RELEASE) \
	$(TESTS_DIR_DEBUG) \
	$(TESTS_DIR_RELEASE)

$(ALL_DIRS):
	mkdir -p $@

$(TOOLS_DEBUG) $(TESTS_DEBUG): $(BUILD_DIR_DEBUG)/%: $(SOURCE_DIR)/%.c $(HEADERS) | $(TOOLS_DIR_DEBUG) $(TESTS_DIR_DEBUG)
	$(CLANG) $(CFLAGS_DEBUG) -I$(INCLUDE_DIR) -o $@ $< $(LFLAGS)

$(TOOLS_RELEASE) $(TESTS_RELEASE): $(BUILD_DIR_RELEASE)/%: $(SOURCE_DIR)/%.c $(HEADERS) | $(TOOLS_DIR_RELEASE) $(TESTS_DIR_RELEASE)
	$(CLANG) $(CFLAGS_RELEASE) -I$(INCLUDE_DIR) -o $@ $< $(LFLAGS)

RUN_DEBUG_TESTS       := $(addprefix run_debug_,$(TESTS_FILES))
RUN_RELEASE_TESTS     := $(addprefix run_release_,$(TESTS_FILES))
RUN_INTEGRATION_TESTS := $(addprefix run_integration_test_,$(TOOLS_FILES))

ifeq (${STUFFLIB_TEST_VERBOSE},1)
	TEST_ARGS := -v
else
	TEST_ARGS :=
endif

.PHONY: run_debug_tests
run_debug_tests: $(RUN_DEBUG_TESTS)
.PHONY: run_release_tests
run_release_tests: $(RUN_RELEASE_TESTS)
.PHONY: run_integration_tests
run_integration_tests: $(RUN_INTEGRATION_TESTS)

.PHONY: $(RUN_DEBUG_TESTS)
$(RUN_DEBUG_TESTS): run_debug_%: $(TESTS_DIR_DEBUG)/%
	$< $(TEST_ARGS)
.PHONY: $(RUN_RELEASE_TESTS)
$(RUN_RELEASE_TESTS): run_release_%: $(TESTS_DIR_RELEASE)/%
	$< $(TEST_ARGS)
.PHONY: $(RUN_INTEGRATION_TESTS)
$(RUN_INTEGRATION_TESTS): run_integration_test_%: ./tests/test_%_tool.bash $(TOOLS_DEBUG) $(TOOLS_RELEASE)
	timeout --kill-after=4m 2m $< $(TEST_ARGS)
