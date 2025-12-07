SHELL := /bin/bash

# TODO dedicated subdir, not repo root
MODULE_DIR := ./stufflib
OUTPUT_DIR := ./build
TEMP_DIR   = $(eval TEMP_DIR := $(shell mktemp --directory))

CLANG  := clang-21
CFLAGS ?= \
	-std=gnu23 \
	-Weverything \
	-Werror \
	-Wstrict-prototypes \
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
	-Wno-pre-c23-compat \
	-Wno-sign-conversion \
	-Wno-switch-default \
	-Wno-unsafe-buffer-usage \
	-Wno-vla
LDFLAGS  ?= -lm -lc
INCLUDES := -I.

ifeq ($(shell uname), Darwin)
	CC       := $(shell brew --prefix llvm)/bin/$(CLANG)
	SDK_PATH := $(shell xcrun --show-sdk-path)
	LDFLAGS  += -Wl,-syslibroot,$(SDK_PATH),-framework,Accelerate
	INCLUDES += -isysroot $(SDK_PATH)
else
	CC      := $(CLANG)
	LDFLAGS += -lopenblas
endif

# todo separate: asan, ubsan, valgrind
RELEASE ?= 0
ifeq ($(RELEASE), 1)
	BUILD_DIR := $(OUTPUT_DIR)/release
	CFLAGS    += -O3 -march=native
else
	BUILD_DIR := $(OUTPUT_DIR)/debug
	CFLAGS    += -O1 -g -fsanitize=address,undefined
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

PROGRAM_DIRS    := $(wildcard ./*)
PROGRAM_SOURCES := $(call find_sources,$(PROGRAM_DIRS))
PROGRAM_OBJECTS := $(subst /./,/,$(addprefix $(BUILD_DIR)/,$(subst .c,.o,$(PROGRAM_SOURCES))))
PROGRAM_PATHS   := $(subst .o,,$(PROGRAM_OBJECTS))

TEST_PATHS := $(filter $(BUILD_DIR)/tests/test_%,$(PROGRAM_PATHS))
TOOL_PATHS := $(filter $(BUILD_DIR)/tools/%,$(PROGRAM_PATHS))

OBJECT_PATHS := $(MODULE_OBJECTS) $(PROGRAM_OBJECTS)
DEPEND_PATHS := $(subst .o,.d,$(OBJECT_PATHS))
BUILD_DIRS   := $(sort $(dir $(OBJECT_PATHS)))

.PHONY: clean
clean:
	$(RM) -r $(OUTPUT_DIR)

.PHONY: fmt
fmt: $(MODULE_HEADERS) $(MODULE_SOURCES) $(PROGRAM_SOURCES)
	@clang-format --verbose -i $^

$(BUILD_DIR) $(BUILD_DIRS):
	mkdir -p $@

.PHONY: all
all: $(PROGRAM_PATHS)

.PHONY: objects
objects: $(OBJECT_PATHS)

TEST_ARGS :=
ifeq (${STUFFLIB_TEST_VERBOSE}, 1)
	TEST_ARGS += -v
endif

RUN_TESTS := $(addprefix run_,$(notdir $(TEST_PATHS)))
.PHONY: test
test: $(RUN_TESTS)
.PHONY: $(RUN_TESTS)
$(RUN_TESTS): run_%: $(BUILD_DIR)/tests/%
	@env SL_TEMP_DIR=$(TEMP_DIR) $< $(TEST_ARGS)

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
	$(CC) -x c $< $(CFLAGS) $(INCLUDES) -c -o $@

$(PROGRAM_PATHS): %: %.o
	$(CC) $< $(CFLAGS) -o $@ $(LDFLAGS)

.PHONY: printvars
printvars:
	@$(foreach V,$(sort $(.VARIABLES)),            \
	$(if $(filter-out environ% default automatic,  \
	$(origin $V)),$(info $V=$($V))))

JQ_MAKE_COMPILE_COMMANDS := [inputs|{\
	directory: "$(abspath .)", \
	command: ., \
	file: match("^$(CC)[ ]+([^ ]+)").captures[0].string, \
	output: match("-o[ ]+([^ ]+)").captures[0].string \
	}]

compile_commands.json: ALWAYS_BUILD
	@$(MAKE) --always-make --dry-run all objects \
		| grep -wE '^$(CC)' \
		| jq -nR '$(JQ_MAKE_COMPILE_COMMANDS)' > $@

.PHONY: ALWAYS_BUILD
ALWAYS_BUILD:
