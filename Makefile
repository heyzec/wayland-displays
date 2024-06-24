########################################
# Variables
########################################
# Overridable variables
# Installation prefix (where to install)
PREFIX ?= /usr/local
# Directory to place build artifacts
BUILD_DIR ?= build
# Whether to build test functions
BUILD_TESTING ?= OFF

# Target-specific variables
test: BUILD_TESTING := ON

# Sources
SRC_DIR := src
SRC = $(wildcard $(SRC_DIR)/*)

########################################
# Targets
########################################

# The target for building the project
# First target is default target, called by build phase of nix
all: build

# Configure the project using cmake
configure: $(SRC)
	# Add flag to generate compile_commands.json for LSP to find headers
	cmake -DBUILD_TESTING=$(BUILD_TESTING) -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B ${BUILD_DIR}

# Build the project
build: configure
	cd $(BUILD_DIR) && $(MAKE)

# Build the project with test functions, then run it
test: build
	cd $(BUILD_DIR) && $(MAKE) test

# This target is called by the install phase of nix
install:
	echo $(PREFIX)
	mkdir -p $(PREFIX)/bin
	cp $(BUILD_DIR)/wayland-displays $(PREFIX)/bin

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)


# Declare targets are just labels for commands to be executed
# Prevent make from skipping targets due to presence of already built files
.PHONY: all clean

