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

# 链接
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 清理目标
clean:
	rm -f $(TARGET) $(OBJ) output/*.s

# 创建输出目录
output:
	mkdir -p output

# 运行单个测试
test_sample: $(TARGET) output
	./$(TARGET) < test/sample.toyc > output/sample.s
	@echo "Sample test completed. Output in output/sample.s"

# 运行所有测试
test_all: $(TARGET) output
	@echo "Running all tests..."
	./$(TARGET) < test/01_minimal.tc > output/01.s
	./$(TARGET) < test/02_assignment.tc > output/02.s
	./$(TARGET) < test/03_if_else.tc > output/03.s
	./$(TARGET) < test/04_while_break.tc > output/04.s
	./$(TARGET) < test/05_function_call.tc > output/05.s
	./$(TARGET) < test/10_void_fn.tc > output/10.s
	./$(TARGET) < test/15_multiple_return_paths.tc> output/15.s
	@echo "All tests completed. Outputs in output/ directory"

# 运行所有测试（带优化）
test_all_opt: $(TARGET) output
	@echo "Running all tests with optimization..."
	./$(TARGET) -opt < test/complex.toyc > output/complex_opt.s
	./$(TARGET) -opt < test/function.toyc > output/function_opt.s
	./$(TARGET) -opt < test/loop.toyc > output/loop_opt.s
	./$(TARGET) -opt < test/sample.toyc > output/sample_opt.s
	./$(TARGET) -opt < test/t_semantic.toyc > output/t_semantic_opt.s
	@echo "All optimized tests completed. Outputs in output/ directory"

# 添加一个调试目标
debug_test: $(TARGET) output
	@echo "Running first test with debug output..."
	./$(TARGET) < test/18_many_variables.tc > output/18.s
	./$(TARGET) < test/19_many_arguments.tc > output/19.s
	./$(TARGET) < test/20_comprehensive.tc > output/20.s
	@echo "Debug test completed."

# 伪目标
.PHONY: all clean test_sample test_all test_all_opt output