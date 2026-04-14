.SUFFIXES:
.DELETE_ON_ERROR:

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
CFLAGS   ?=
LDFLAGS  ?=
INCLUDES ?=


CLANG :=
PLATFORM_LDFLAGS := -lm -fuse-ld=lld
PLATFORM_INCLUDES := -I.

ifeq ($(shell uname), Darwin)
	LLVM_PATH := $(shell brew --prefix llvm@$(LLVM_VERSION))
	SDK_PATH := $(shell xcrun --show-sdk-path)
	CLANG        := $(LLVM_PATH)/bin/clang-$(LLVM_VERSION)
	CLANG_FORMAT := $(LLVM_PATH)/bin/clang-format
	CLANG_TIDY   := $(LLVM_PATH)/bin/clang-tidy
	PLATFORM_LDFLAGS += -Wl,-syslibroot,$(SDK_PATH),-framework,Accelerate
	PLATFORM_INCLUDES += -isysroot $(SDK_PATH)
else
	CLANG        := clang-$(LLVM_VERSION)
	CLANG_FORMAT := clang-format
	CLANG_TIDY   := clang-tidy
	PLATFORM_LDFLAGS += -lopenblas
endif

CC := $(CLANG)

ifeq ($(CFLAGS),)
	CFLAGS := $(STANDARD) $(WARNINGS)
endif

ifeq ($(LDFLAGS),)
	LDFLAGS := $(PLATFORM_LDFLAGS)
endif

ifeq ($(INCLUDES),)
	INCLUDES := $(PLATFORM_INCLUDES)
endif

BUILD_TYPE ?= debug
MODULE_DIR := ./stufflib
OUTPUT_DIR := ./build

ifeq ($(BUILD_TYPE),fast)
	BUILD_DIR := $(OUTPUT_DIR)/fast
	CFLAGS    += -O3 -march=native
else ifeq ($(BUILD_TYPE),asan)
	BUILD_DIR := $(OUTPUT_DIR)/asan
	CFLAGS    += -O1 -g -fsanitize=address
	LDFLAGS   += -fsanitize=address
else ifeq ($(BUILD_TYPE),ubsan)
	BUILD_DIR := $(OUTPUT_DIR)/ubsan
	CFLAGS    += -O1 -g -fsanitize=undefined
	LDFLAGS   += -fsanitize=undefined
else ifeq ($(BUILD_TYPE),msan)
	BUILD_DIR := $(OUTPUT_DIR)/msan
	CFLAGS    += -O1 -g -fsanitize=memory
	LDFLAGS   += -fsanitize=memory
else
	BUILD_DIR := $(OUTPUT_DIR)/debug
	CFLAGS    += -O1 -g
endif


TRACE ?= 0

ifeq ($(TRACE), 1)
	CFLAGS += -DSL_VERBOSITY=3
else
	CFLAGS += -DSL_VERBOSITY=2
endif


find_headers = $(foreach dir,$1,$(wildcard $(dir)/*.h))
find_sources = $(foreach dir,$1,$(wildcard $(dir)/*.c))

MODULE_DIRS    := $(wildcard $(MODULE_DIR)/*)
MODULE_HEADERS := $(call find_headers,$(MODULE_DIRS))
MODULE_SOURCES := $(call find_sources,$(MODULE_DIRS))
MODULE_OBJECTS := $(subst /./,/,$(addprefix $(BUILD_DIR)/,$(subst .c,.o,$(MODULE_SOURCES))))

PROGRAM_DIRS    := tools tests
PROGRAM_SOURCES := $(call find_sources,$(PROGRAM_DIRS))
PROGRAM_OBJECTS := $(subst /./,/,$(addprefix $(BUILD_DIR)/,$(subst .c,.o,$(PROGRAM_SOURCES))))
PROGRAM_PATHS   := $(subst .o,,$(PROGRAM_OBJECTS))

ALL_READABLE_FILES := $(MODULE_HEADERS) $(MODULE_SOURCES) $(PROGRAM_SOURCES)

TEST_PATHS := $(filter $(BUILD_DIR)/tests/test_%,$(PROGRAM_PATHS))
TOOL_PATHS := $(filter $(BUILD_DIR)/tools/%,$(PROGRAM_PATHS))

STUFFLIB := $(BUILD_DIR)/libstufflib.a

OBJECT_PATHS := $(MODULE_OBJECTS) $(PROGRAM_OBJECTS)
DEPEND_PATHS := $(subst .o,.d,$(OBJECT_PATHS))
BUILD_DIRS   := $(sort $(dir $(OBJECT_PATHS)))


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

.PHONY: tidy-fix
tidy-fix: $(ALL_READABLE_FILES)
	@$(CLANG_TIDY) --fix --quiet $^

.PHONY: check
check: test integration_test

MACRO_EXPAND_TARGETS := $(addprefix macro-expand-,$(ALL_READABLE_FILES))
.PHONY: $(MACRO_EXPAND_TARGETS)
$(MACRO_EXPAND_TARGETS): macro-expand-%: %
	$(CC) $(CFLAGS) $(INCLUDES) -E $< | $(CLANG_FORMAT) --assume-filename=$<

$(BUILD_DIR) $(BUILD_DIRS):
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

$(OBJECT_PATHS): $(BUILD_DIR)/%.o: %.c | $(BUILD_DIRS)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -MF $(subst .o,.d,$@) -c -o $@ $<

$(STUFFLIB): $(MODULE_OBJECTS)
	$(AR) rcs $@ $^

$(PROGRAM_PATHS): %: %.o $(STUFFLIB)
	$(CC) $(CFLAGS) -o $@ $(STUFFLIB) $(LDFLAGS) $<

.PHONY: printvars
printvars:
	@$(foreach V,$(sort $(.VARIABLES)),            \
	$(if $(filter-out environ% default automatic,  \
	$(origin $V)),$(info $V=$($V))))

JQ_MAKE_COMPILE_COMMANDS := [inputs|{\
	directory: "$(abspath .)", \
	command: ., \
	file: match("([^ ]+\\.[co])").captures[0].string, \
	output: match("-o[ ]+([^ ]+)").captures[0].string \
	}]

compile_commands.json: ALWAYS_BUILD
	@$(MAKE) --always-make --dry-run all objects \
		| grep -wE '^$(CC)' \
		| jq -nR '$(JQ_MAKE_COMPILE_COMMANDS)' > $@

.PHONY: ALWAYS_BUILD
ALWAYS_BUILD:
