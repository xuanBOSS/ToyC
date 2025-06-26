#include "lexer.h"
#include <cctype>
#include <iostream>
#include <stdexcept>

/*
 * Lexer ��Ĺ��캯��
 * ��ʼ���ʷ���������״̬�͹ؼ���ӳ���
 * @param source Դ�����ַ���
*/

// Ĭ�Ϲ��캯��
Lexer::Lexer() : source(""), position(0), line(1), column(1) {
    // ��ʼ���ؼ���ӳ���
    keywords["int"] = TokenType::INT;
    keywords["void"] = TokenType::VOID;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["while"] = TokenType::WHILE;
    keywords["break"] = TokenType::BREAK;
    keywords["continue"] = TokenType::CONTINUE;
    keywords["return"] = TokenType::RETURN;

    // ��ʼ�������ӳ���
    initOperators();
}
Lexer::Lexer(const std::string& source) : source(source), position(0), line(1), column(1) {
    // ��ʼ���ؼ���ӳ���
    keywords["int"] = TokenType::INT;
    keywords["void"] = TokenType::VOID;
    keywords["if"] = TokenType::IF;
    keywords["else"] = TokenType::ELSE;
    keywords["while"] = TokenType::WHILE;
    keywords["break"] = TokenType::BREAK;
    keywords["continue"] = TokenType::CONTINUE;
    keywords["return"] = TokenType::RETURN;

    // ��ʼ�������ӳ���
    initOperators();
}

// ��ʼ�������ӳ���
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
 * ��Դ����ת��Ϊ�������
 * @return �������б�ǵ�����
*/
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    // ѭ��ֱ������������Դ����
    while (!isAtEnd()) {
        // �����հ��ַ���ע��
        skipWhitespace();
        if (isAtEnd()) break;

        // ��ȡ��һ����ǲ���ӵ�������
        Token token = scanToken();
        tokens.push_back(token);
    }

    // ����ļ��������
    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
    return tokens;
}

/*
 * �鿴��ǰλ����ǰƫ�������ַ������ı䵱ǰλ��
 * @param offset ��ǰ��ƫ����
 * @return ƫ��λ�õ��ַ������������Χ�򷵻� '\0'
*/
char Lexer::peek(int offset) const {
    if (position + offset >= source.length()) {
        return '\0';
    }
    return source[position + offset];
}

/*
 * ��ȡ��ǰ�ַ�����λ��ǰ��һ��
 * @return ��ǰ�ַ�
*/
char Lexer::advance() {
    char current = source[position++];
    // ����������Ϣ
    if (current == '\n') {
        line++;
        column = 1;
    }
    else {
        column++;
    }
    return current;
}


/*
 * ����Ƿ��ѵ���Դ�����ĩβ
 * @return �������ĩβ�򷵻� true�����򷵻� false
*/
bool Lexer::isAtEnd() const {
    return position >= source.length();
}

/*
 * �����հ��ַ���ע��
 * ����ո��Ʊ�����س��������з��Լ����кͶ���ע��
*/
void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            //�հ��ַ�
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
            }
            else {
                return;
            }
            break;
        default:
            return;
        }
    }
}

// ����ע��
void Lexer::skipComment() {
    char c = peek();
    if (c == '/') {
        // ����ע��
        if (peek(1) == '/') {
            position += 2;
            column += 2;
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        }
        // ����ע��
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
                }
                else {
                    position++;
                    column++;
                }
            }
        }
    }
}


/*
 * ɨ�貢������һ�����
 * ��Ҫ����ȷ����ǰλ�õ��ַ����Ͳ�������Ӧ��ר��ɨ�躯��
 * @return ��һ�����
*/
Token Lexer::scanToken() {
    char c = advance();

    // ��ʶ����ؼ��֣�����ĸ���»��߿�ͷ��
    if (isalpha(c) || c == '_') {
        position--; // ����һ������scanIdentifier����
        column--;
        return scanIdentifier();
    }

    // ���֣������ֿ�ͷ��
    if (isdigit(c)) {
        position--; // ����һ������scanNumber����
        column--;
        return scanNumber();
    }
    //��¼��ǰ��ǵ�����λ��
    int tokenLine = line;
    int tokenColumn = column - 1; // �����кţ���Ϊadvance�Ѿ�������

    //������ַ���
    switch (c) {
        //���ַ����
    case '(': return Token(TokenType::LPAREN, "(", tokenLine, tokenColumn);
    case ')': return Token(TokenType::RPAREN, ")", tokenLine, tokenColumn);
    case '{': return Token(TokenType::LBRACE, "{", tokenLine, tokenColumn);
    case '}': return Token(TokenType::RBRACE, "}", tokenLine, tokenColumn);
    case ';': return Token(TokenType::SEMICOLON, ";", tokenLine, tokenColumn);
    case ',': return Token(TokenType::COMMA, ",", tokenLine, tokenColumn);

        //���������
    case '+': return Token(TokenType::PLUS, "+", tokenLine, tokenColumn);
    case '-': return Token(TokenType::MINUS, "-", tokenLine, tokenColumn);
    case '*': return Token(TokenType::MULTIPLY, "*", tokenLine, tokenColumn);
    case '%': return Token(TokenType::MODULO, "%", tokenLine, tokenColumn);

        // �����ǵ��ַ���˫�ַ��������
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

    //δʶ����ַ�
    return Token(TokenType::UNKNOWN, std::string(1, c), tokenLine, tokenColumn);
}

/*
 * ɨ���ʶ����ؼ���
 * ʶ������ĸ���»��߿�ͷ�������ĸ�����ֻ��»��ߵ�����
 * @return ��ʶ����ؼ��ֵı��
*/
Token Lexer::scanIdentifier() {
    int startPos = position;
    int startColumn = column;
    int startLine = line;

    // ��һ���ַ���������ĸ���»���
    if (isalpha(peek()) || peek() == '_') {
        advance();
    }

    // �����ַ���������ĸ�����ֻ��»���
    while (!isAtEnd() && (isalnum(peek()) || peek() == '_')) {
        advance();
    }

    //��ȡ����
    std::string lexeme = source.substr(startPos, position - startPos);

    // ����Ƿ��ǹؼ���
    if (keywords.find(lexeme) != keywords.end()) {
        return Token(keywords[lexeme], lexeme, startLine, startColumn);
    }

    //���ǹؼ��֣����Ǳ�ʶ��
    return Token(TokenType::IDENTIFIER, lexeme, startLine, startColumn);
}

/*
 * ɨ������
 * ʶ����������ɵ�����
 * @return ���ֵı��
*/
Token Lexer::scanNumber() {
    int startPos = position;
    int startColumn = column;
    int startLine = line;

    // ��ȡ���������������ַ�
    while (!isAtEnd() && isdigit(peek())) {
        advance();
    }

    // ��ȡ����
    std::string lexeme = source.substr(startPos, position - startPos);
    return Token(TokenType::NUMBER, lexeme, startLine, startColumn);
}

// ��ȡ������������
Token Lexer::readOperatorOrPunctuator() {
    int start = position;
    int startLine = line;
    int startColumn = column;

    char c = peek();
    position++;
    column++;

    // ���˫�ַ�����
    if (position < source.length()) {
        char next = peek();
        std::string twoCharOp = std::string(1, c) + next;
        if (operators.find(twoCharOp) != operators.end()) {
            position++;
            column++;
            return Token(operators.at(twoCharOp), twoCharOp, startLine, startColumn);
        }
    }

    // ���ַ�����
    std::string singleCharOp = std::string(1, c);
    if (operators.find(singleCharOp) != operators.end()) {
        return Token(operators.at(singleCharOp), singleCharOp, startLine, startColumn);
    }

    // δ֪����
    return Token(TokenType::UNKNOWN, singleCharOp, startLine, startColumn);
}

// ��ȡ��һ�����
Token Lexer::nextToken() {
    while (true) {
        // �����ո��Ʊ����С��س���
        skipWhitespace();
        // ����Ƿ��ѵ�ĩβ
        if (isAtEnd()) {
            return Token(TokenType::END_OF_FILE, "", line, column);
        }

        // ʹ��ԭ�е�scanToken��������ȡ��һ�����
        return scanToken();
    }
}

// �鿴��һ����ǵ���������
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

// �������ı�ǻ�����
std::vector<Token> Lexer::tokenize(const std::string& source) {
    Lexer lexer(source);
    return lexer.tokenize();
}