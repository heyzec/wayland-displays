# Specify the build directory
BUILD_DIR := build

# The target for building the project
all: configure build

# Configure the project using cmake
configure:
	cmake -S . -B ${BUILD_DIR}

# Build the project
build:
	cd $(BUILD_DIR) && $(MAKE)

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all configure build clean
