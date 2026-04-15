.PHONY: help
help:
	@echo "STUFFLIB!"
	@echo "clang on Linux or macOS only."
	@echo ""
	@echo "Requires:"
	@echo "  clang, lld, OpenBLAS, jq"
	@echo "  (optional) clang-format, clang-tidy"
	@echo ""
	@echo "Main targets:"
	@echo "  all                    build all tools and tests"
	@echo "  check                  run unit and integration tests"
	@echo "  clean                  remove build artifacts"
	@echo "  help                   print this (default)"
	@echo "  compile_commands.json  generate compilation database"
	@echo ""
	@echo "Other targets:"
	@echo "  objects                build objects, partial compile commands, and dep files"
	@echo "  test                   run only unit tests"
	@echo "  integration_test       run only integration tests"
	@echo "  fmt                    format source files with clang-format"
	@echo "  tidy                   lint with clang-tidy"
	@echo "  printvars              print all make variables"
	@echo "  macro-expand-%         run preprocessor on % and print result"
	@echo ""
	@echo "Variables:"
	@echo "  OPTIMIZE=O0|O1|O2|O3                    (default: O2)"
	@echo "  SANITIZE=none|address|undefined|memory  (default: none)"
	@echo "  TRACE=0|1                               (default: 0)"
	@echo "  STUFFLIB_TEST_VERBOSE=0|1               (default: 0)"

# disable builtin suffix rules: we use custom explicit pattern rules
.SUFFIXES:
# delete (possibly partially) built targets on error
.DELETE_ON_ERROR:

# defaults and parameters
SHELL := /bin/bash
LLVM_VERSION := 21
AR := ar
RM := rm -f

STANDARD ?= -std=gnu23
WARNINGS ?= \
	-Weverything \
	-Werror \
	-Wstrict-prototypes \
	-Wno-c++-compat \
	-Wno-c99-compat \
	-Wno-declaration-after-statement \
	-Wno-disabled-macro-expansion \
	-Wno-gnu-zero-variadic-macro-arguments \
	-Wno-implicit-int-enum-cast \
	-Wno-implicit-void-ptr-cast \
	-Wno-jump-misses-init \
	-Wno-missing-prototypes \
	-Wno-nrvo \
	-Wno-padded \
	-Wno-pre-c11-compat \
	-Wno-pre-c23-compat \
	-Wno-sign-conversion \
	-Wno-switch-default \
	-Wno-unsafe-buffer-usage \
	-Wno-vla \
	-Wno-global-constructors
CFLAGS   ?= -g
LDFLAGS  ?=
INCLUDES ?=


# platform dependent config
UNAME := $(shell uname)
CLANG :=
PLATFORM_LDFLAGS := -lm -fuse-ld=lld
PLATFORM_INCLUDES := -I.

ifeq ($(UNAME), Darwin)
	LLVM_PATH := $(shell brew --prefix llvm@$(LLVM_VERSION))
	SDK_PATH  := $(shell xcrun --show-sdk-path)
	CLANG        := $(LLVM_PATH)/bin/clang-$(LLVM_VERSION)
	CLANG_FORMAT := $(LLVM_PATH)/bin/clang-format
	CLANG_TIDY   := $(LLVM_PATH)/bin/clang-tidy
	PLATFORM_LDFLAGS  += -Wl,-syslibroot,$(SDK_PATH),-framework,Accelerate
	PLATFORM_INCLUDES += -isysroot $(SDK_PATH)
else ifeq ($(UNAME), Linux)
	CLANG        := clang-$(LLVM_VERSION)
	CLANG_FORMAT := clang-format
	CLANG_TIDY   := clang-tidy
	PLATFORM_LDFLAGS += -lopenblas
else
	$(error Unsupported platform: $(UNAME))
endif

CC := $(CLANG)

CFLAGS   := $(CFLAGS) $(STANDARD) $(WARNINGS)
LDFLAGS  := $(LDFLAGS) $(PLATFORM_LDFLAGS)
INCLUDES := $(INCLUDES) $(PLATFORM_INCLUDES)

OPTIMIZE ?= O2
SANITIZE ?= none
MODULE_DIR := ./stufflib
OUTPUT_DIR := ./build
BUILD_DIR  := $(OUTPUT_DIR)/$(OPTIMIZE)-$(SANITIZE)

ifeq ($(OPTIMIZE),O0)
	CFLAGS += -O0
else ifeq ($(OPTIMIZE),O1)
	CFLAGS += -O1
else ifeq ($(OPTIMIZE),O2)
	CFLAGS += -O2
else ifeq ($(OPTIMIZE),O3)
	CFLAGS += -O3
else
	$(error Unknown OPTIMIZE: $(OPTIMIZE). Use O0, O1, O2, or O3)
endif

ifeq ($(SANITIZE),none)
else ifeq ($(SANITIZE),address)
	CFLAGS  += -fsanitize=address
	LDFLAGS += -fsanitize=address
else ifeq ($(SANITIZE),undefined)
	CFLAGS  += -fsanitize=undefined
	LDFLAGS += -fsanitize=undefined
else ifeq ($(SANITIZE),memory)
	CFLAGS  += -fsanitize=memory
	LDFLAGS += -fsanitize=memory
else
	$(error Unknown SANITIZE: $(SANITIZE). Use none, address, undefined, or memory)
endif


TRACE ?= 0

ifeq ($(TRACE), 1)
	CFLAGS += -DSL_VERBOSITY=3
else
	CFLAGS += -DSL_VERBOSITY=2
endif


# functions to find files in given list of dirs
find_headers = $(foreach dir,$1,$(wildcard $(dir)/*.h))
find_sources = $(foreach dir,$1,$(wildcard $(dir)/*.c))

# source discovery
MODULE_DIRS    := $(wildcard $(MODULE_DIR)/*)
MODULE_HEADERS := $(call find_headers,$(MODULE_DIRS))
MODULE_SOURCES := $(patsubst ./%,%,$(call find_sources,$(MODULE_DIRS)))

PROGRAM_DIRS    := tools tests
PROGRAM_SOURCES := $(call find_sources,$(PROGRAM_DIRS))

ALL_READABLE_FILES := $(MODULE_HEADERS) $(MODULE_SOURCES) $(PROGRAM_SOURCES)

# derived paths
MODULE_OBJECTS  := $(patsubst %.c,$(BUILD_DIR)/%.o,$(MODULE_SOURCES))
PROGRAM_OBJECTS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(PROGRAM_SOURCES))
PROGRAM_PATHS   := $(patsubst %.o,%,$(PROGRAM_OBJECTS))

TEST_PATHS := $(filter $(BUILD_DIR)/tests/test_%,$(PROGRAM_PATHS))
TOOL_PATHS := $(filter $(BUILD_DIR)/tools/%,$(PROGRAM_PATHS))

STUFFLIB     := $(BUILD_DIR)/libstufflib.a
OBJECT_PATHS := $(MODULE_OBJECTS) $(PROGRAM_OBJECTS)
DEPEND_PATHS := $(patsubst %.o,%.d,$(OBJECT_PATHS))
BUILD_DIRS   := $(sort $(dir $(OBJECT_PATHS)))

COMPILE_COMMANDS_DIR := $(BUILD_DIR)/compile_commands


.PHONY: all
all: $(PROGRAM_PATHS)

.PHONY: clean
clean:
	$(RM) -r $(OUTPUT_DIR)

.PHONY: fmt
fmt: format

.PHONY: format
format: $(ALL_READABLE_FILES)
	@$(CLANG_FORMAT) --verbose -i $^

.PHONY: tidy
tidy: $(ALL_READABLE_FILES)
	@$(CLANG_TIDY) --quiet $^

.PHONY: check
check: test integration_test

MACRO_EXPAND_TARGETS := $(addprefix macro-expand-,$(ALL_READABLE_FILES))
.PHONY: $(MACRO_EXPAND_TARGETS)
$(MACRO_EXPAND_TARGETS): macro-expand-%: %
	$(CC) $(CFLAGS) $(INCLUDES) -E $< | $(CLANG_FORMAT) --assume-filename=$<

$(BUILD_DIR) $(BUILD_DIRS) $(COMPILE_COMMANDS_DIR):
	mkdir -p $@

.PHONY: objects
objects: $(OBJECT_PATHS)

TEST_ARGS :=
ifeq (${STUFFLIB_TEST_VERBOSE}, 1)
	TEST_ARGS += -v
endif

make_tmp_dir = $(shell mktemp --directory)

RUN_TESTS := $(addprefix run_,$(notdir $(TEST_PATHS)))
.PHONY: test
test: $(RUN_TESTS)

.PHONY: $(RUN_TESTS)
$(RUN_TESTS): run_%: $(BUILD_DIR)/tests/%
	@export SL_TEMP_DIR=$(call make_tmp_dir); \
		trap "rm -rf $$SL_TEMP_DIR" EXIT SIGINT; \
		$< $(TEST_ARGS)

TIMEOUT_CMD := $(shell which timeout)
ifeq ($(TIMEOUT_CMD),)
	TIMEOUT_CMD :=
else
	TIMEOUT_CMD += --kill-after=4m 2m
endif

RUN_INTEGRATION_TESTS := $(addprefix integration_test_,$(notdir $(TOOL_PATHS)))
.PHONY: integration_test
integration_test: $(RUN_INTEGRATION_TESTS)

.PHONY: $(RUN_INTEGRATION_TESTS)
$(RUN_INTEGRATION_TESTS): integration_test_%: ./tests/integration/test_%_tool.bash $(BUILD_DIR)/tools/%
	$(TIMEOUT_CMD) $^ $(TEST_ARGS)

-include $(wildcard DEPEND_PATHS)

$(OBJECT_PATHS): $(BUILD_DIR)/%.o: %.c | $(BUILD_DIRS) $(COMPILE_COMMANDS_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(subst .o,.d,$@) -MJ $(COMPILE_COMMANDS_DIR)/$(subst /,-,$@).json -c -o $@ $<

$(STUFFLIB): $(MODULE_OBJECTS)
	$(AR) rcs $@ $^

$(PROGRAM_PATHS): %: %.o $(STUFFLIB)
	$(CC) $(CFLAGS) -o $@ $(STUFFLIB) $(LDFLAGS) $<

.PHONY: printvars
printvars:
	@$(foreach V,$(sort $(.VARIABLES)),            \
	$(if $(filter-out environ% default automatic,  \
	$(origin $V)),$(info $V=$($V))))

compile_commands.json: $(OBJECT_PATHS)
	sed 's/,$$//' $(COMPILE_COMMANDS_DIR)/*.json | jq -s '.' > $@+ && mv $@+ $@
