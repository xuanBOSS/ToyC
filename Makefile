# ==== Makefile ====

# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I.

# 源码路径（自动查找所有子目录的 .cpp 文件）
SRC = $(wildcard */*.cpp) main.cpp
OBJ = $(SRC:.cpp=.o)

# 输出可执行文件名
TARGET = toycc

# 默认目标
all: $(TARGET)

# 添加依赖规则，使修改 .h 文件后会重新编译
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 链接（增加 -lstdc++fs 支持 <filesystem>）
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lstdc++fs

# 清理目标
clean:
	rm -f $(TARGET) $(OBJ) *.s *.ir

# 运行测试
test: all
	./$(TARGET) test/sample.toyc sample.s

# 伪目标
.PHONY: all clean test
