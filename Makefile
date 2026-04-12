SHELL := /bin/bash

MODULE_DIR := ./stufflib
OUTPUT_DIR := ./build

LLVM_VERSION  := 21
CFLAGS ?= \
	-std=gnu23 \
	-Weverything \
	-Werror \
	-Wstrict-prototypes \
	-Wno-c++-compat \
	-Wno-nrvo \
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
LDFLAGS  ?= -lm -lc
INCLUDES := -I.

ifeq ($(shell uname), Darwin)
	LLVM_PATH := $(shell brew --prefix llvm@$(LLVM_VERSION))
	SDK_PATH  := $(shell xcrun --show-sdk-path)
	CC        := $(LLVM_PATH)/bin/clang-$(LLVM_VERSION)
	LDFLAGS   += -Wl,-syslibroot,$(SDK_PATH),-framework,Accelerate
	INCLUDES  += -isysroot $(SDK_PATH)
	CLANG_FORMAT :=  $(LLVM_PATH)/bin/clang-format
	CLANG_TIDY   :=  $(LLVM_PATH)/bin/clang-tidy
else
	CC      := clang-$(LLVM_VERSION)
	LDFLAGS += -lopenblas
	CLANG_FORMAT := clang-format
	CLANG_TIDY   := clang-tidy
endif

BUILD_TYPE ?= debug

ifeq ($(BUILD_TYPE),fast)
	BUILD_DIR := $(OUTPUT_DIR)/fast
	CFLAGS    += -O3 -march=native
else ifeq ($(BUILD_TYPE),asan)
	BUILD_DIR := $(OUTPUT_DIR)/asan
	CFLAGS    += -O1 -g -fsanitize=address
else ifeq ($(BUILD_TYPE),ubsan)
	BUILD_DIR := $(OUTPUT_DIR)/ubsan
	CFLAGS    += -O1 -g -fsanitize=undefined
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

# https://www.gnu.org/software/make/manual/make.html#Automatic-Prerequisites
# 2025-04-19
$(DEPEND_PATHS): $(BUILD_DIR)/%.d: %.c | $(BUILD_DIRS)
	@set -e; rm -f $@; \
		trap "rm -f '$@.$$$$'" EXIT SIGINT; \
		$(CC) $(CFLAGS) $(INCLUDES) -MM -MT $(subst .d,.o,$@) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@

# TODO how to avoid dynamic make
-include $(DEPEND_PATHS)

$(OBJECT_PATHS): $(BUILD_DIR)/%.o: %.c | $(BUILD_DIRS)
	$(CC) $< $(CFLAGS) $(INCLUDES) -c -o $@

$(STUFFLIB): $(MODULE_OBJECTS)
	$(AR) rcs $@ $^

$(PROGRAM_PATHS): %: %.o $(STUFFLIB)
	$(CC) $< $(CFLAGS) -o $@ $(STUFFLIB) $(LDFLAGS)

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
