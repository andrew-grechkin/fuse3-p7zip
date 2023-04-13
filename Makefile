BUILD_TYPE ?= "Debug"
DIR         = ".build-$(BUILD_TYPE)"

.PHONY:   \
	build

$(DIR):
	mkdir -p "$(DIR)"

build:
	@cmake -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -S . -B "$(DIR)"
	@make -j -C "$(DIR)" install

tidy:
	@cp -f "$(shell git rev-parse --show-toplevel)/.clang-format" /tmp
	@tidyall -a -j 4
