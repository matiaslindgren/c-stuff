SHELL := /bin/sh

CLANG    := clang-18
CFLAGS   ?= \
	-std=c23 \
	-Weverything \
	-Werror \
	-Wno-missing-prototypes \
	-Wno-pre-c23-compat \
	-Wno-c99-compat \
	-Wno-vla \
	-Wno-declaration-after-statement \
	-Wno-padded \
	-Wno-unsafe-buffer-usage \
	-Wno-switch-default \
	-Wno-switch-enum \
	-Wno-double-promotion \
	-Wno-gnu-zero-variadic-macro-arguments
LDFLAGS  ?= -lm -fuse-ld=lld -lc
INCLUDES ?= ./include

ifeq ($(shell uname), Darwin)
	SDK_PATH := $(shell xcrun --show-sdk-path)
	LLVM_DIR := $(shell brew --prefix llvm)
	CLANG    := $(LLVM_DIR)/bin/$(CLANG)
	LDFLAGS  += -Wl,-syslibroot,$(SDK_PATH),-framework,Accelerate
	INCLUDES += -isysroot $(SDK_PATH)
else
	LDFLAGS += -lopenblas
endif

INCLUDE_DIR := $(abspath ./include)
SOURCE_DIR  := $(abspath ./src)
OUTPUT_DIR  := ./build
TMP_DIR     := $(shell mktemp --directory)

# todo separate: asan, ubsan, valgrind
DEBUG=0
ifeq ($(DEBUG), 1)
	BUILD_DIR := $(OUTPUT_DIR)/debug
	CFLAGS    += -O1 -g -fsanitize=address,undefined
else
	BUILD_DIR := $(OUTPUT_DIR)/release
	CFLAGS    += -O3 -march=native
endif

TRACE=0
ifeq ($(TRACE), 1)
	CFLAGS += -DSL_VERBOSITY=3
else
	CFLAGS += -DSL_VERBOSITY=2
endif

HEADERS     := $(wildcard $(INCLUDE_DIR)/*.h)
TOOLS_SRC   := $(wildcard $(SOURCE_DIR)/tools/*.c)
TESTS_SRC   := $(wildcard $(SOURCE_DIR)/tests/*.c)
TOOLS_FILES := $(notdir $(basename $(TOOLS_SRC)))
TESTS_FILES := $(notdir $(basename $(TESTS_SRC)))
TOOLS_DIR   := $(BUILD_DIR)/tools
TESTS_DIR   := $(BUILD_DIR)/tests
TOOLS       := $(addprefix $(TOOLS_DIR)/,$(TOOLS_FILES))
TESTS       := $(addprefix $(TESTS_DIR)/,$(TESTS_FILES))

.PHONY: all
all: $(TOOLS) $(TESTS)

.PHONY: clean
clean:
	$(RM) -r $(OUTPUT_DIR)

.PHONY: fmt
fmt: $(HEADERS) $(TOOLS_SRC) $(TESTS_SRC)
	@clang-format --verbose -i $^

$(BUILD_DIR) $(TOOLS_DIR) $(TESTS_DIR):
	mkdir -p $@

$(TOOLS) $(TESTS): $(BUILD_DIR)/%: $(SOURCE_DIR)/%.c $(HEADERS) | $(TOOLS_DIR) $(TESTS_DIR)
	$(CLANG) $(CFLAGS) -I $(INCLUDES) -o $@ $< $(LDFLAGS)


JQ_MAKE_COMPILE_COMMANDS := [inputs|{\
	directory: "$(abspath .)", \
	command: ., \
	file: match("('$(SOURCE_DIR)'[^ ]*)").captures[0].string, \
	output: match("-o ([^ ]+)").captures[0].string \
	}]

compile_commands.json:
	@$(MAKE) --always-make --dry-run \
		| grep -wE '^\S*clang' \
		| jq -nR '$(JQ_MAKE_COMPILE_COMMANDS)' > $@


TEST_ARGS :=
ifeq (${STUFFLIB_TEST_VERBOSE}, 1)
	TEST_ARGS += -v
endif

RUN_TESTS := $(addprefix run_,$(TESTS_FILES))
.PHONY: test
test: $(RUN_TESTS)
.PHONY: $(RUN_TESTS)
$(RUN_TESTS): run_%: $(TESTS_DIR)/%
	@env SL_TMP_DIR=$(TMP_DIR) $< $(TEST_ARGS)

TIMEOUT_CMD := $(shell which timeout)
ifeq ($(TIMEOUT_CMD),)
	TIMEOUT_CMD :=
else
	TIMEOUT_CMD += --kill-after=4m 2m
endif

RUN_INTEGRATION_TESTS := $(addprefix integration_test_,$(TOOLS_FILES))
.PHONY: integration_test
integration_test: $(RUN_INTEGRATION_TESTS)
.PHONY: $(RUN_INTEGRATION_TESTS)
$(RUN_INTEGRATION_TESTS): integration_test_%: ./tests/test_%_tool.bash $(BUILD_DIR)/tools/%
	$(TIMEOUT_CMD) $^ $(TEST_ARGS)
