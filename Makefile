SHELL := /bin/bash

MODULE_DIR  := ./stufflib
PROGRAM_DIR := ./src
OUTPUT_DIR  := ./build
TEMP_DIR    := $(shell mktemp --directory)

CLANG    := clang-18
CFLAGS   ?= \
	-std=c23 \
	-Weverything \
	-Werror \
	-Wstrict-prototypes \
	-Wno-c99-compat \
	-Wno-declaration-after-statement \
	-Wno-gnu-zero-variadic-macro-arguments \
	-Wno-padded \
	-Wno-pre-c23-compat \
	-Wno-switch-default \
	-Wno-unsafe-buffer-usage \
	-Wno-vla
LDFLAGS  ?= -lm -fuse-ld=lld -lc
INCLUDES ?= $(MODULE_DIR)

ifeq ($(shell uname), Darwin)
	SDK_PATH := $(shell xcrun --show-sdk-path)
	LLVM_DIR := $(shell brew --prefix llvm)
	CLANG    := $(LLVM_DIR)/bin/$(CLANG)
	LDFLAGS  += -Wl,-syslibroot,$(SDK_PATH),-framework,Accelerate
	INCLUDES += -isysroot $(SDK_PATH)
else
	LDFLAGS += -lopenblas
endif

# todo separate: asan, ubsan, valgrind
DEBUG := 1
ifeq ($(DEBUG), 1)
	BUILD_DIR := $(OUTPUT_DIR)/debug
	CFLAGS    += -O1 -g -fsanitize=address,undefined
else
	BUILD_DIR := $(OUTPUT_DIR)/release
	CFLAGS    += -O3 -march=native
endif

TRACE := 0
ifeq ($(TRACE), 1)
	CFLAGS += -DSL_VERBOSITY=3
else
	CFLAGS += -DSL_VERBOSITY=2
endif

MODULE_DIRS  := $(wildcard $(MODULE_DIR)/stufflib/*)
HEADER_PATHS := $(foreach dir,$(MODULE_DIRS),$(wildcard $(dir)/*.h))
MODULES_SRC  := $(foreach dir,$(MODULE_DIRS),$(wildcard $(dir)/*.c))
MODULES_OUT  := $(subst .c,,$(subst $(MODULE_DIR),$(BUILD_DIR),$(MODULES_SRC)))
MODULES_DEP  := $(subst .c,.d,$(subst $(MODULE_DIR),$(BUILD_DIR),$(MODULES_SRC)))
MODULES_OBJ  := $(subst .c,.o,$(subst $(MODULE_DIR),$(BUILD_DIR),$(MODULES_SRC)))

PROGRAM_DIRS := $(wildcard $(PROGRAM_DIR)/*)
PROGRAMS_SRC := $(foreach dir,$(PROGRAM_DIRS),$(wildcard $(dir)/*.c))
PROGRAMS_OUT := $(subst .c,,$(subst $(PROGRAM_DIR),$(BUILD_DIR),$(PROGRAMS_SRC)))
PROGRAMS_DEP := $(subst .c,.d,$(subst $(PROGRAM_DIR),$(BUILD_DIR),$(PROGRAMS_SRC)))
PROGRAMS_OBJ := $(subst .c,.o,$(subst $(PROGRAM_DIR),$(BUILD_DIR),$(PROGRAMS_SRC)))

.PHONY: all
all: $(PROGRAMS_OUT)

.PHONY: clean
clean:
	$(RM) -r $(OUTPUT_DIR)

.PHONY: fmt
fmt: $(HEADER_PATHS) $(MODULES_SRC) $(PROGRAMS_SRC)
	@clang-format --verbose -i $^


JQ_MAKE_COMPILE_COMMANDS := [inputs|{\
	directory: "$(abspath .)", \
	command: ., \
	file: match("(('$(MODULE_DIR)'|'$(PROGRAM_DIR)')[^ ]*)").captures[0].string, \
	output: match("-o ([^ ]+)").captures[0].string \
	}]

compile_commands.json:
	@$(MAKE) --always-make --dry-run \
		| grep -wE '^\S*clang' \
		| jq -nR '$(JQ_MAKE_COMPILE_COMMANDS)' > $@

$(BUILD_DIRS):
	mkdir -p $@

# todo: maybe use implicit rules
$(MODULES_OBJ): $(BUILD_DIR)/%.o: $(MODULE_DIR)/%.c $(HEADER_PATHS) | $(BUILD_DIRS)
	$(CLANG) $(CFLAGS) -I $(INCLUDES) -c $< -o $@

$(MODULES_DEP): $(BUILD_DIR)/%.d: $(MODULE_DIR)/%.c | $(BUILD_DIRS)
	$(CLANG) $(CFLAGS) -I $(INCLUDES) -MMD -MF $@ $<

$(PROGRAMS_OBJ): $(BUILD_DIR)/%.o: $(PROGRAMS_DIR)/%.c $(HEADER_PATHS) | $(BUILD_DIRS)
	$(CLANG) $(CFLAGS) -I $(INCLUDES) -c $< -o $@

$(PROGRAMS_DEP): $(BUILD_DIR)/%.d: $(PROGRAMS_DIR)/%.c $(HEADER_PATHS) | $(BUILD_DIRS)
	$(CLANG) $(CFLAGS) -I $(INCLUDES) -MMD -MF $@ $<

$(MODULES_OUT) $(PROGRAMS_OUT): $(BUILD_DIR)/%: $(BUILD_DIR)/%.d $(BUILD_DIR)/%.o
	$(CLANG) $(CFLAGS) $< -o $@ $(LDFLAGS)

# https://www.gnu.org/software/make/manual/make.html#Automatic-Prerequisites

# TEST_ARGS :=
# ifeq (${STUFFLIB_TEST_VERBOSE}, 1)
# 	TEST_ARGS += -v
# endif

# RUN_TESTS := $(addprefix run_,$(TESTS_FILES))
# .PHONY: test
# test: $(RUN_TESTS)
# .PHONY: $(RUN_TESTS)
# $(RUN_TESTS): run_%: $(TESTS_DIR)/%
# 	@env SL_TEMP_DIR=$(TEMP_DIR) $< $(TEST_ARGS)

# TIMEOUT_CMD := $(shell which timeout)
# ifeq ($(TIMEOUT_CMD),)
# 	TIMEOUT_CMD :=
# else
# 	TIMEOUT_CMD += --kill-after=4m 2m
# endif

# RUN_INTEGRATION_TESTS := $(addprefix integration_test_,$(TOOLS_FILES))
# .PHONY: integration_test
# integration_test: $(RUN_INTEGRATION_TESTS)
# .PHONY: $(RUN_INTEGRATION_TESTS)
# $(RUN_INTEGRATION_TESTS): integration_test_%: ./tests/test_%_tool.bash $(BUILD_DIR)/tools/%
# 	$(TIMEOUT_CMD) $^ $(TEST_ARGS)

#.SECONDEXPANSION:

#SED_GET_DEPS = $(shell sed -En 's|\#include [<"]stufflib/([^/]+/[^.]+).h[">]|\1|p' ./src/$*.c)

#$(OUT_PATHS): $(BUILD_DIR)/%: $(BUILD_DIR)/%.o $$(addsuffix .o,$$(addprefix $(BUILD_DIR)/,$${SED_GET_DEPS}))
#	@echo $@ $^
#	#$(CLANG) $(CFLAGS) $< -o $@ $(LDFLAGS)
