# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -Iinclude
DEPFLAGS := -MMD -MP

# Directories
SRC_DIR := src
BUILD_DIR := build

# Output binary
TARGET := $(BUILD_DIR)/output

# Source files and corresponding object files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# Rule to build the final output
all: $(TARGET)

# Linking the final executable
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# Rule to compile each source file to an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Include dependency files, if they exist
-include $(DEPS)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean up the build directory and the output binary
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
