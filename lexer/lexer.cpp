#include "lexer.h"
#include <cctype>
#include <iostream>
#include <stdexcept>

/*
 * Lexer 类的构造函数
 * 初始化词法分析器的状态和关键字映射表
 * @param source 源代码字符串
*/

// 默认构造函数
Lexer::Lexer() : source(""), position(0), line(1), column(1) {
    // 初始化关键字映射表
    keywords["int"] = TokenType::INT;
    keywords["void"] = TokenType::VOID;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["while"] = TokenType::WHILE;
    keywords["break"] = TokenType::BREAK;
    keywords["continue"] = TokenType::CONTINUE;
    keywords["return"] = TokenType::RETURN;
    
    // 初始化运算符映射表
    initOperators();
}

// 带参数的构造函数
Lexer::Lexer(const std::string& source) : source(source), position(0), line(1), column(1) {
    // 初始化关键字映射表
    keywords["int"] = TokenType::INT;
    keywords["void"] = TokenType::VOID;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["while"] = TokenType::WHILE;
    keywords["break"] = TokenType::BREAK;
    keywords["continue"] = TokenType::CONTINUE;
    keywords["return"] = TokenType::RETURN;

    // 初始化运算符映射表
    initOperators();
}

// 初始化运算符映射表
// 将所有ToyC支持的运算符和标点符号映射到对应的标记类型
void Lexer::initOperators() {
    operators["="] = TokenType::ASSIGN;
    operators["+"] = TokenType::PLUS;
    operators["-"] = TokenType::MINUS;
    operators["*"] = TokenType::MULTIPLY;
    operators["/"] = TokenType::DIVIDE;
    operators["%"] = TokenType::MODULO;
    operators["<"] = TokenType::LT;
    operators[">"] = TokenType::GT;
    operators["<="] = TokenType::LE;
    operators[">="] = TokenType::GE;
    operators["=="] = TokenType::EQ;
    operators["!="] = TokenType::NEQ;
    operators["&&"] = TokenType::AND;
    operators["||"] = TokenType::OR;
    operators["!"] = TokenType::NOT;
    operators["("] = TokenType::LPAREN;
    operators[")"] = TokenType::RPAREN;
    operators["{"] = TokenType::LBRACE;
    operators["}"] = TokenType::RBRACE;
    operators[";"] = TokenType::SEMICOLON;
    operators[","] = TokenType::COMMA;
}

/*
 * 将源代码转换为标记序列
 * @return 包含所有标记的向量
*/
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    // 循环直到处理完所有源代码
    while (!isAtEnd()) {
        // 跳过空白字符和注释
        skipWhitespace();
        if (isAtEnd()) break;
        
        // 获取下一个标记并添加到序列中
        Token token = scanToken();
        tokens.push_back(token);
    }
    
    // 添加文件结束标记
    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
    return tokens;
}

/*
 * 查看当前位置向前偏移量的字符，不改变当前位置
 * @param offset 向前的偏移量
 * @return 偏移位置的字符，如果超出范围则返回 '\0'
*/
char Lexer::peek(int offset) const {
    if (position + offset >= source.length()) {
        return '\0';
    }
    return source[position + offset];
}

/*
 * 读取当前字符并将位置前进一步
 * @return 当前字符
*/
char Lexer::advance() {
    char current = source[position++];
    // 更新行列信息
    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return current;
}


/*
 * 检查是否已到达源代码的末尾
 * @return 如果到达末尾则返回 true，否则返回 false
*/
bool Lexer::isAtEnd() const {
    return position >= source.length();
}

/*
 * 跳过空白字符和注释
 * 处理空格、制表符、回车符、换行符以及单行和多行注释
*/
void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            //空白字符
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;
            case '\n':
                advance();
                break;
            
            case '/':
                if (peek(1) == '/' || peek(1) == '*') {
                    skipComment();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

// 跳过注释
void Lexer::skipComment() {
    char c = peek();
    if (c == '/') {
        // 单行注释
        if (peek(1) == '/') {
            position += 2;
            column += 2;
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        }
        // 多行注释
        else if (peek(1) == '*') {
            position += 2;
            column += 2;
            while (!isAtEnd()) {
                if (peek() == '*' && peek(1) == '/') {
                    position += 2;
                    column += 2;
                    break;
                }
                if (peek() == '\n') {
                    line++;
                    column = 1;
                    position++;
                } else {
                    position++;
                    column++;
                }
            }
        }
    }
}


/*
 * 扫描并返回下一个标记
 * 主要负责确定当前位置的字符类型并调用相应的专用扫描函数
 * @return 下一个标记
*/
Token Lexer::scanToken() {
    char c = advance();
    
    // 标识符或关键字（以字母或下划线开头）
    if (isalpha(c) || c == '_') {
        position--; // 回退一步，让scanIdentifier处理
        column--;
        return scanIdentifier();
    }
    
    // 数字（以数字开头）
    if (isdigit(c)) {
        position--; // 回退一步，让scanNumber处理
        column--;
        return scanNumber();
    }
    //记录当前标记的行列位置
    int tokenLine = line;
    int tokenColumn = column - 1; // 调整列号，因为advance已经增加了
    
    //处理各种符号
    switch (c) {
        //单字符标记
        case '(': return Token(TokenType::LPAREN, "(", tokenLine, tokenColumn);
        case ')': return Token(TokenType::RPAREN, ")", tokenLine, tokenColumn);
        case '{': return Token(TokenType::LBRACE, "{", tokenLine, tokenColumn);
        case '}': return Token(TokenType::RBRACE, "}", tokenLine, tokenColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);
        case ',': return Token(TokenType::COMMA, ",", tokenLine, tokenColumn);
        
        //算术运算符
        case '+': return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);
        case '-': return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);
        case '*': return Token(TokenType::MULTIPLY, "*", tokenLine, tokenColumn);
        case '%': return Token(TokenType::MODULO, "%", tokenLine, tokenColumn);
        
        // 可能是单字符或双字符的运算符
        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::EQ, "==", tokenLine, tokenColumn);
            }
            return Token(TokenType::ASSIGN, "=", tokenLine, tokenColumn);
            
        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::NEQ, "!=", tokenLine, tokenColumn);
            }
            return Token(TokenType::NOT, "!", tokenLine, tokenColumn);
            
        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::LE, "<=", tokenLine, tokenColumn);
            }
            return Token(TokenType::LT, "<", tokenLine, tokenColumn);
            
        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::GE, ">=", tokenLine, tokenColumn);
            }
            return Token(TokenType::GT, ">", tokenLine, tokenColumn);
            
        case '&':
            if (peek() == '&') {
                advance();
                return Token(TokenType::AND, "&&", tokenLine, tokenColumn);
            }
            return Token(TokenType::UNKNOWN, "&", tokenLine, tokenColumn);
            
        case '|':
            if (peek() == '|') {
                advance();
                return Token(TokenType::OR, "||", tokenLine, tokenColumn);
            }
            return Token(TokenType::UNKNOWN, "|", tokenLine, tokenColumn);
            
        case '/':
            return Token(TokenType::DIVIDE, "/", tokenLine, tokenColumn);
    }
    
    //未识别的字符
    return Token(TokenType::UNKNOWN, std::string(1, c), tokenLine, tokenColumn);
}

/*
 * 扫描标识符或关键字
 * 识别以字母或下划线开头，后跟字母、数字或下划线的序列
 * @return 标识符或关键字的标记
*/
Token Lexer::scanIdentifier() {
    int startPos = position;
    int startColumn = column;
    int startLine = line;
    
    // 第一个字符必须是字母或下划线
    if (isalpha(peek()) || peek() == '_') {
        advance();
    }
    
    // 后续字符可以是字母、数字或下划线
    while (!isAtEnd() && (isalnum(peek()) || peek() == '_')) {
        advance();
    }

    //提取词素
    std::string lexeme = source.substr(startPos, position - startPos);
    
    // 检查是否是关键字
    if (keywords.find(lexeme) != keywords.end()) {
        return Token(keywords[lexeme], lexeme, startLine, startColumn);
    }
    
    //不是关键字，则是标识符
    return Token(TokenType::IDENTIFIER, lexeme, startLine, startColumn);
}

/*
 * 扫描数字
 * 识别由数字组成的序列
 * @return 数字的标记
*/
Token Lexer::scanNumber() {
    int startPos = position;
    int startColumn = column;
    int startLine = line;
    
    // 处理可能的负号
    if (peek() == '-') {
        advance();
    }

    // 读取所有连续的数字字符
    while (!isAtEnd() && isdigit(peek())) {
        advance();
    }
    
    // 提取词素
    std::string lexeme = source.substr(startPos, position - startPos);
    return Token(TokenType::NUMBER, lexeme, startLine, startColumn);
}

// 读取运算符或标点符号
Token Lexer::readOperatorOrPunctuator() {
    int start = position;
    int startLine = line;
    int startColumn = column;

    char c = peek();
    position++;
    column++;

    // 检查双字符符号
    if (position < source.length()) {
        char next = peek();
        std::string twoCharOp = std::string(1, c) + next;
        if (operators.find(twoCharOp) != operators.end()) {
            position++;
            column++;
            return Token(operators.at(twoCharOp), twoCharOp, startLine, startColumn);
        }
    }

    // 单字符符号
    std::string singleCharOp = std::string(1, c);
    if (operators.find(singleCharOp) != operators.end()) {
        return Token(operators.at(singleCharOp), singleCharOp, startLine, startColumn);
    }

    // 未知符号
    return Token(TokenType::UNKNOWN, singleCharOp, startLine, startColumn);
}

// 获取下一个标记
Token Lexer::nextToken() {
    while (true) {
        // 跳过空格、制表、换行、回车符
        skipWhitespace();
        // 检查是否已到末尾
        if (isAtEnd()) {
            return Token(TokenType::END_OF_FILE, "", line, column);
        }
        
        // 使用原有的scanToken函数来获取下一个标记
        return scanToken();
    }
}

// 查看下一个标记但不消耗它
Token Lexer::peekToken() {
    int savedPosition = position;
    int savedLine = line;
    int savedColumn = column;

    Token token = nextToken();

    position = savedPosition;
    line = savedLine;
    column = savedColumn;

    return token;
}

// 带参数的标记化函数
std::vector<Token> Lexer::tokenize(const std::string& source) {
    Lexer lexer(source);
    return lexer.tokenize();
}