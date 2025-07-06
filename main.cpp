#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "ir/irgen.h"
#include "codegen/codegen.h"
#include <fstream>
#include <iostream>
#include <sstream>


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.tc> <output.s>" << std::endl;
        return 1;
    }
    
    // 读取输入文件
    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "Error: Could not open input file " << argv[1] << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << input.rdbuf();
    std::string source = buffer.str();
    input.close();
    
    // 词法分析
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    
    // 语法分析
    Parser parser(tokens);
    std::shared_ptr<CompUnit> ast = parser.parse();
    if (!ast) {
        std::cerr << "Error: Parsing failed." << std::endl;
        return 1;
    }
    
    // 语义分析
    SemanticAnalyzer semanticAnalyzer;
    if (!semanticAnalyzer.analyze(ast)) {
        std::cerr << "Error: Semantic analysis failed." << std::endl;
        return 1;
    }
    
    // IR生成
    IRGenerator irGenerator;
    irGenerator.generate(ast);
    
    // 可选：打印IR用于调试
    std::ofstream irOutput(std::string(argv[2]) + ".ir");
    IRPrinter::print(irGenerator.getInstructions(), irOutput);
    irOutput.close();
    
    // 代码生成
    // 需要修改CodeGenerator使其接收IR而不是AST
    CodeGenerator generator(argv[2], irGenerator.getInstructions());
    generator.generate();
    
    std::cout << "Compilation successful. Output written to " << argv[2] << std::endl;
    return 0;
}