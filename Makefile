# Base file courtesy of https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

# Build and source directories, target executable name
TARGET_EXEC ?= runme
TARGET_TEST ?= testme
TARGET_PYBIND ?= lsbf.so
BUILD_DIR ?= ./build
SRC_DIRS ?= src
TEST_DIRS ?= test
MAIN_FILE ?= src/Main.cpp # Do this to avoid including main file when doing tests

SRCS_WITHOUT_MAIN := $(shell find $(SRC_DIRS) -name '*.cpp' -not -path $(MAIN_FILE))

# Find cpp source files
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TESTSRCS := $(SRCS_WITHOUT_MAIN) $(shell find $(TEST_DIRS) -name '*.cpp')
TESTOBJS := $(TESTSRCS:%=$(BUILD_DIR)/%.o)
TESTDEPS := $(TESTOBJS:.o=.d)

# Automatically generated include flags
INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(shell find $(TEST_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# C++ Flags
CPP_DEBUG_FLAS := -g -fno-omit-frame-pointer 
CPP_OPT_FLAGS := -O2
CPP_WARN_FLAGS := -Wall -Werror
CPPFLAGS ?= -std=c++11 $(INC_FLAGS) $(CPP_WARN_FLAGS) $(CPP_OPT_FLAGS) $(CPP_DEBUG_FLAS) -MMD -MP -fopenmp

all: $(BUILD_DIR)/$(TARGET_TEST) $(BUILD_DIR)/$(TARGET_EXEC) $(BUILD_DIR)/$(TARGET_PYBIND)

# Make target pybind
$(BUILD_DIR)/$(TARGET_PYBIND): ./pybindings/lsbf.cpp
	$(CXX) $(CPPFLAGS)  $(shell python3 -m pybind11 --includes) -shared -fPIC ./pybindings/lsbf.cpp $(SRCS_WITHOUT_MAIN) -o $(BUILD_DIR)/$(TARGET_PYBIND)


# Make target test, require object files to be created
$(BUILD_DIR)/$(TARGET_TEST): $(TESTOBJS)
	$(CXX) $(TESTOBJS) -o $@ $(LDFLAGS)

# Make target executable, require object files to be created
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Make c++ source into object files
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean command
.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
-include $(TESTDEPS)

MKDIR_P ?= mkdir -p

