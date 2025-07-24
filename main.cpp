// main.cpp - 编译器主程序
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "ir/ir.h"
#include "ir/irgen.h"
#include "codegen/codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
    // 检查是否有 -opt 参数
    bool enableOptimization = false;
    //bool enableOptimization = true; // 默认启用优化
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-opt") {
            enableOptimization = true;
            std::cerr << "Optimization enabled." << std::endl;
            break;
        }
    }

    // 是否打印生成的中间代码
    bool enablePrintIR = true;
    
    std::cerr << "程序开始执行\n";
    // 从标准输入读取源代码
    /*std::stringstream buffer;
    buffer << std::cin.rdbuf();
    std::string source = buffer.str();*/

    //从指定文件读入源代码
    std::stringstream buffer;
    std::string filename;
    
    // 处理命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-opt") {
            enableOptimization = true;
        } else {
            filename = arg; // 将不是 -opt 的参数作为文件名处理
        }
    }
    
    // 从文件或标准输入读取源代码
    if (!filename.empty()) {
        std::ifstream inputFile(filename);
        if (!inputFile) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return 1;
        }
        buffer << inputFile.rdbuf();
    } else {
        buffer << std::cin.rdbuf();
    }
    
    std::string source = buffer.str();
    

    std::cerr << "初始化完成，开始编译\n";

    // 词法分析
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    std::cerr << "词法分析完成\n";

    // 语法分析
    Parser parser(tokens);
    std::shared_ptr<CompUnit> ast = parser.parse();
    if (!ast) {
        std::cerr << "Error: Parsing failed." << std::endl;
        return 1;
    }
    std::cerr << "语法分析完成\n";

    // 语义分析
    SemanticAnalyzer semanticAnalyzer;
    if (!semanticAnalyzer.analyze(ast)) {
        std::cerr << "Error: Semantic analysis failed." << std::endl;
        return 1;
    }

    // IR生成配置
    IRGenConfig irConfig;
    if(enableOptimization) {
        // 启用优化选项
        irConfig.enableOptimizations = true;
    }
    
    // IR生成
    IRGenerator irGenerator(irConfig);
    irGenerator.generate(ast);
    
    // 可选：打印IR用于调试（输出到stderr不影响标准输出）
    if (enablePrintIR) {
        std::cerr << "IR生成完成，开始打印IR\n";
        IRPrinter::print(irGenerator.getInstructions(), std::cerr);
    }
    std::cerr << "IR生成完成\n";

    // 代码生成配置
    CodeGenConfig config;
    if (enableOptimization) {
        // 启用优化选项
        config.optimizeStackLayout = true;
        //config.eliminateDeadStores = true;
        //config.enablePeepholeOptimizations = true;
        //config.regAllocStrategy = RegisterAllocStrategy::GRAPH_COLOR; // 使用图着色算法
    }
    
    // 创建临时字符串流用于收集输出
    std::stringstream outputStream;
    std::cerr << "代码生成开始\n";
    std::cerr << "准备创建CodeGenerator\n";
    // 代码生成
    CodeGenerator generator(outputStream, irGenerator.getInstructions(), config);
    std::cerr << "CodeGenerator创建完成\n";
    std::cerr << "开始生成代码\n";
    generator.generate();
    
    std::cerr << "代码生成完成\n";
    // 将生成的汇编代码输出到标准输出
    std::cout << outputStream.str();
    
    return 0;
}