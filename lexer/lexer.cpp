#include "lexer.h"
#include <cctype>

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
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;
        
        Token token = scanToken();
        tokens.push_back(token);
    }
    
    // 添加文件结束标记
    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
    return tokens;
}

char Lexer::peek(int offset) const {
    if (position + offset >= source.length()) {
        return '\0';
    }
    return source[position + offset];
}

char Lexer::advance() {
    char current = source[position++];
    if (current == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return current;
}

bool Lexer::isAtEnd() const {
    return position >= source.length();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;
            case '\n':
                advance();
                break;
            case '/':
                if (peek(1) == '/') {
                    // 单行注释
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else if (peek(1) == '*') {
                    // 多行注释
                    advance(); // 跳过 /
                    advance(); // 跳过 *
                    
                    while (!isAtEnd() && !(peek() == '*' && peek(1) == '/')) {
                        advance();
                    }
                    
                    if (!isAtEnd()) {
                        advance(); // 跳过 *
                        advance(); // 跳过 /
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token Lexer::scanToken() {
    char c = advance();
    
    // 标识符
    if (isalpha(c) || c == '_') {
        position--; // 回退一步，让scanIdentifier处理
        column--;
        return scanIdentifier();
    }
    
    // 数字
    if (isdigit(c)) {
        position--; // 回退一步，让scanNumber处理
        column--;
        return scanNumber();
    }
    
    int tokenLine = line;
    int tokenColumn = column - 1; // 调整列号，因为advance已经增加了
    
    switch (c) {
        case '(': return Token(TokenType::LPAREN, "(", tokenLine, tokenColumn);
        case ')': return Token(TokenType::RPAREN, ")", tokenLine, tokenColumn);
        case '{': return Token(TokenType::LBRACE, "{", tokenLine, tokenColumn);
        case '}': return Token(TokenType::RBRACE, "}", tokenLine, tokenColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);
        case ',': return Token(TokenType::COMMA, ",", tokenLine, tokenColumn);
        
        case '+': return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);
        case '-': return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);
        case '*': return Token(TokenType::MULTIPLY, "*", tokenLine, tokenColumn);
        case '%': return Token(TokenType::MODULO, "%", tokenLine, tokenColumn);
        
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
    
    return Token(TokenType::UNKNOWN, std::string(1, c), tokenLine, tokenColumn);
}

Token Lexer::scanIdentifier() {
    int startPos = position;
    int startColumn = column;
    int startLine = line;
    
    while (!isAtEnd() && (isalnum(peek()) || peek() == '_')) {
        advance();
    }
    
    std::string lexeme = source.substr(startPos, position - startPos);
    
    // 检查是否是关键字
    if (keywords.find(lexeme) != keywords.end()) {
        return Token(keywords[lexeme], lexeme, startLine, startColumn);
    }
    
    return Token(TokenType::IDENTIFIER, lexeme, startLine, startColumn);
}

Token Lexer::scanNumber() {
    int startPos = position;
    int startColumn = column;
    int startLine = line;
    
    while (!isAtEnd() && isdigit(peek())) {
        advance();
    }
    
    std::string lexeme = source.substr(startPos, position - startPos);
    return Token(TokenType::NUMBER, lexeme, startLine, startColumn);
}