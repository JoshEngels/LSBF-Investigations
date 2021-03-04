# Base file courtesy of https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

# Build and source directories, target executable name
TARGET_EXEC ?= runme
BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

# Find cpp source files
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Automatically generated include flags
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# C++ Flags
CPP_DEBUG_FLAS := -g -fno-omit-frame-pointer 
CPP_OPT_FLAGS := -O2
CPP_WARN_FLAGS := -Wall
CPPFLAGS ?= -std=c++11 $(INC_FLAGS) $(CPP_WARN_FLAGS) $(CPP_OPT_FLAGS) $(CPP_DEBUG_FLAS) -MMD -MP

# Make target executable, require object files to be created
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Make c++ source into object files
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean command
.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

