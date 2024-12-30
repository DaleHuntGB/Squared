# Compiler
CXX = g++

# Source and output
SRC = Main.cpp
OUT = Squared

# Libraries
LIBS = -lraylib -lGL -lm -ldl -pthread

# Default target
all: build run

# Build target
build:
	$(CXX) $(SRC) $(LIBS) -o $(OUT)

# Run target
run:
	./$(OUT)

# Clean target
clean:
	rm -f $(OUT)