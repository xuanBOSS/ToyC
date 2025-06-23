CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRC = $(wildcard */*.cpp) main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = toycc

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) */*.o
