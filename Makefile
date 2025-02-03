# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Iexternal/install/include
LDFLAGS = -Lexternal/install/lib64 -ligraph -llibleidenalg

# Source files
SRC_DIR = src
BUILD_DIR = build
SRC_FILES = $(SRC_DIR)/leiden_wrapper.cpp $(SRC_DIR)/run_leiden.cpp
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
TARGET = $(BUILD_DIR)/leiden_test

# Default rule: Build the project
all: $(TARGET)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link everything into the executable
$(TARGET): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the executable
run: all
	LD_LIBRARY_PATH=external/install/lib64:$(LD_LIBRARY_PATH) ./$(TARGET)
