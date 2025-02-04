# Compiler and Flags
CXX = g++
CFLAGS = -O3 -w -fPIC -I./external/install/include -I./external/install/include/igraph -c
LDFLAGS = -L./external/install/lib64 -Wl,-rpath,$(PWD)/external/install/lib64 -ligraph -llibleidenalg

# Directories
BIN_DIR = bin
SRC_DIR = src
LIB_DIR = external/install/lib64

# Targets
OBJECTS = $(BIN_DIR)/run_leiden.o
EXECUTABLE = $(BIN_DIR)/leiden_test

# Default Target: Build both .o file and executable
all: set_library_path $(OBJECTS) $(EXECUTABLE)

# Set LD_LIBRARY_PATH dynamically
set_library_path:
	@export LD_LIBRARY_PATH=$(PWD)/$(LIB_DIR):$$LD_LIBRARY_PATH
	@echo "LD_LIBRARY_PATH set to: $(PWD)/$(LIB_DIR)"

# Compile run_leiden.cpp into an object file for Chapel
$(BIN_DIR)/run_leiden.o: $(SRC_DIR)/run_leiden.cpp $(SRC_DIR)/run_leiden.h
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CFLAGS) $< -o $@

# Compile leiden_wrapper.cpp into an executable for testing
$(BIN_DIR)/leiden_test: $(SRC_DIR)/leiden_wrapper.cpp $(SRC_DIR)/run_leiden.o
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Run the executable with correct LD_LIBRARY_PATH
run: all
	@LD_LIBRARY_PATH=$(PWD)/$(LIB_DIR) ./$(BIN_DIR)/leiden_test

# Clean Build Artifacts
clean:
	@echo "Removing objects and executables..."
	rm -f $(BIN_DIR)/*.o $(BIN_DIR)/leiden_test *.log src/*~ include/*~ *~ core
