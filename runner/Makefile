CXX = g++
CXXFLAGS = -std=c++17 -Wall -Isrc

SRC = src/runner.cpp src/compiler.cpp src/executor.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = runner

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)
	rm -f user_program user_program.exe temp_output.txt
