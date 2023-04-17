THIS_FILE     := $(abspath $(firstword $(MAKEFILE_LIST)))
THIS_DIR      := $(dir $(THIS_FILE))

GIT_ROOT      := $(shell git -C "$(THIS_DIR)" rev-parse --show-toplevel)
GIT_REPO      := $(shell git config --file $(THIS_DIR)/.git/config --get remote.origin.url)
GIT_REPO_NAME := $(shell basename -s .git $(GIT_REPO))

BUILD_TYPE    ?= Debug
BUILD_DIR      = $(GIT_ROOT)/.build-$(BUILD_TYPE)

.PHONY:   \
	build \
	clean \
	init  \
	tidy

$(BUILD_DIR):
	@mkdir -p "$(BUILD_DIR)"

build: $(BUILD_DIR)
	@cmake -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -S . -B "$(BUILD_DIR)"
	@make -j -C "$(BUILD_DIR)" install

clean:
	@rm -rf "$(BUILD_DIR)"
	@rm -rf "$(GIT_ROOT)/.tidyall.d"

init:
	@git -C "$(GIT_ROOT)" submodule update --init

tidy:
	@cp -f "$(GIT_ROOT)/.clang-format" /tmp
	@tidyall -a -j 4
