#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/semantic.h"
#include "ir/irgen.cpp"
#include "codegen/codegen.h"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./toycc <input_file>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto tokens = lexer::tokenize(source);
    auto ast = parser::parse(tokens);
    semantic::check(ast);
    auto ir = irgen::generateIR(ast);
    codegen::generate(ir, "output/output.s");

    return 0;
}