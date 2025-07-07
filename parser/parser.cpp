#include "parser.h"
#include "ast.h"
#include <iostream>

//前进到下一个Token并返回前一个Token
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

//记录解析错误
ParseError Parser::error(const Token& token, const std::string& message) {
    if (!isRecovering) {  // 只有在不处于恢复状态时才报告错误
    std::cerr << "[Error at line " << token.line << ", column " << token.column << "] "
        << message << std::endl;
    errorCount++;
    hadError = true;
    isRecovering = true;  // 设置恢复状态
    }
    return ParseError(message);
}

//错误恢复同步方法,当解析出错时，跳过当前语法结构，找到下一个可恢复点
void Parser::synchronize() {
    // 只有在恢复模式下才需要同步
    if (isRecovering) {
        advance();
        while (!isAtEnd()) {
            // 如果遇到以下token，认为已找到同步点
            if (previous().type == TokenType::SEMICOLON) {
                isRecovering = false;
                return;
            }

            switch (peek(0).type) {
            case TokenType::INT:
            case TokenType::VOID:
            case TokenType::IF:
            case TokenType::ELSE:
            case TokenType::WHILE:
            case TokenType::BREAK:
            case TokenType::CONTINUE:
            case TokenType::RETURN:
            case TokenType::LBRACE:
            case TokenType::RBRACE:
                isRecovering = false;
                return;
            }

            advance();
        }

        isRecovering = false; // 如果到达文件末尾，也退出恢复模式
    }
}

//检查是否已到达Token序列末尾
bool Parser::isAtEnd() const {
    return peek(0).type == TokenType::END_OF_FILE;
}

//尝试匹配一组Token类型,如果匹配成功返回true并消费Token，否则false
bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

//消费指定类型的Token
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(0), message);
}

//获取前一个已消费的Token
Token Parser::previous() const {
    return tokens[current - 1];
}

//检查当前Token是否匹配指定类型
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek(0).type == type;
}

//开始解析过程,返回解析生成的编译单元AST根节点
std::shared_ptr<CompUnit> Parser::parse() {
    try {
        auto result = compUnit();
        // 即使compUnit没有抛出异常，也检查是否有错误发生
        if (hadError) {
            return nullptr;
        }
        return result;
    }
    catch (const ParseError& error) {
        // 错误处理
        return nullptr;
    }
}

//解析编译单元(CompUnit),compUnit → {functionDef}
std::shared_ptr<CompUnit> Parser::compUnit() {
    std::vector<std::shared_ptr<FunctionDef>> functions;

    // 获取编译单元的起始位置（第一个 token）
    int line = peek(0).line;
    int column = peek(0).column;

    while (!isAtEnd()) {
        isRecovering = false; // 确保每个新函数定义开始时都不处于恢复状态

        try {
            if (check(TokenType::INT) || check(TokenType::VOID)) {
                auto func = funcDef();
                if (func) {
                    functions.push_back(func);
                }
            }
            else {
                // 如果不是以返回类型开始，报告错误并尝试同步
                throw error(peek(0), "Expected return type 'int' or 'void'.");
            }
        }
        catch (const ParseError& e) {
            synchronize();
            // 如果同步后到达了文件末尾，退出循环
            if (isAtEnd()) {
                break;
            }
        }
    }

    return std::make_shared<CompUnit>(functions, line, column);
}

//解析函数定义(functionDef)
//functionDef → type IDENT '(' [params] ')' block
//params → param {',' param}
//param → 'int' IDENT
std::shared_ptr<FunctionDef> Parser::funcDef() {
    // 记录函数定义的起始位置
    int line = peek(0).line;
    int column = peek(0).column;
    // 1. 解析返回类型
    std::string returnTypeStr;
    if (match({ TokenType::INT })) {
        returnTypeStr = "int";
    }
    else if (match({ TokenType::VOID })) {
        returnTypeStr = "void";
    }
    else {
        throw error(peek(0), "Expected return type 'int' or 'void'.");
    }

    // 2. 解析函数名
    Token nameToken = peek(0);
    try {
        nameToken = consume(TokenType::IDENTIFIER, "Expected function name.");
    }
    catch (const ParseError& e) {
        synchronize();
        return nullptr;
    }
    std::string name = nameToken.lexeme;

    // 3. 解析左括号
    try {
        consume(TokenType::LPAREN, "Expected '(' after function name.");
    }
    catch (const ParseError& e) {
        synchronize();
        // 如果下一个token是左大括号，尝试继续解析
        if (!check(TokenType::LBRACE)) {
            return nullptr;
        }
    }

    // 4. 解析参数列表
    std::vector<Param> params;

    // 检查是否直接是左大括号，这意味着可能缺少右括号
    if (check(TokenType::LBRACE)) {
        // 报告缺少右括号的错误，但不进入恢复模式
        bool temp = isRecovering;
        isRecovering = false;
        error(peek(0), "Expected ')' after parameter list.");
        isRecovering = temp;
    }
    else if (!check(TokenType::RPAREN)) {
        do {
            if (!match({ TokenType::INT })) {
                if (check(TokenType::LBRACE)) {
                    // 报告缺少右括号的错误，但允许继续解析
                    bool temp = isRecovering;
                    isRecovering = false;
                    error(peek(0), "Expected ')' after parameter list.");
                    isRecovering = temp;
                    break;
                }
                throw error(peek(0), "Parameter type must be 'int'.");
            }

            try {
                Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name.");
                params.push_back(Param(paramName.lexeme));
            }
            catch (const ParseError& e) {
                synchronize();
                if (check(TokenType::LBRACE)) {
                    break;
                }
                return nullptr;
            }
        } while (match({ TokenType::COMMA }));
    }

    // 5. 检查是否有右括号，没有就尝试继续
    if (!check(TokenType::LBRACE)) { // 如果不是左大括号开始
        try {
            consume(TokenType::RPAREN, "Expected ')' after parameters.");
        }
        catch (const ParseError& e) {
            // 如果下一个是左大括号，报告错误但继续
            if (check(TokenType::LBRACE)) {
                synchronize();
            }
            else {
                synchronize();
                if (!check(TokenType::LBRACE)) {
                    return nullptr;
                }
            }
        }
    }

    // 6. 解析函数体
    std::shared_ptr<BlockStmt> body;
    try {
        body = block();
    }
    catch (const ParseError& e) {
        synchronize();
        return nullptr;
    }

    return std::make_shared<FunctionDef>(returnTypeStr, name, params, body, line, column);
}

//解析函数参数
Param Parser::param() {
    int line = peek(0).line;
    int column = peek(0).column;

    consume(TokenType::INT, "Parameter type must be 'int'.");
    Token name = consume(TokenType::IDENTIFIER, "Expected parameter name.");
    return Param(name.lexeme, line, column);
}

//解析代码块,block → '{' {stmt} '}'
std::shared_ptr<BlockStmt> Parser::block() {
    int line = peek(0).line;
    int column = peek(0).column;
    try {
        consume(TokenType::LBRACE, "Expected '{' before block.");
    }
    catch (const ParseError& e) {
        synchronize();
        // 创建一个空的代码块作为替代
        return std::make_shared<BlockStmt>(std::vector<std::shared_ptr<Stmt>>());
    }

    std::vector<std::shared_ptr<Stmt>> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        try {
            auto statement = stmt();
            if (statement) {
                statements.push_back(statement);
            }
        }
        catch (const ParseError& e) {
            synchronize();
            // 如果同步后已经到达右大括号或文件末尾，退出循环
            if (check(TokenType::RBRACE) || isAtEnd()) {
                break;
            }
        }
    }

    try {
        consume(TokenType::RBRACE, "Expected '}' after block.");
    }
    catch (const ParseError& e) {
        synchronize();
    }

    return std::make_shared<BlockStmt>(statements, line, column);
}

//解析语句
/* stmt → exprStmt
           | block
           | varDeclStmt
           | ifStmt
           | whileStmt
           | breakStmt
           | continueStmt
           | returnStmt
           | assignStmt
*/
std::shared_ptr<Stmt> Parser::stmt() {
    if (match({TokenType::SEMICOLON})) {
        return std::make_shared<ExprStmt>(nullptr); // 空语句
    }
    
    if (check(TokenType::LBRACE)) {
        return block();
    }
    
    if (match({TokenType::INT})) {
        return varDeclStmt();
    }
    
    if (match({TokenType::IF})) {
        return ifStmt();
    }
    
    if (match({TokenType::WHILE})) {
        return whileStmt();
    }
    
    if (match({TokenType::BREAK})) {
        return breakStmt();
    }
    
    if (match({TokenType::CONTINUE})) {
        return continueStmt();
    }
    
    if (match({TokenType::RETURN})) {
        return returnStmt();
    }
    
    // 检查是否是赋值语句
    if (check(TokenType::IDENTIFIER) && peek(1).type == TokenType::ASSIGN) {
        return assignStmt();
    }
    
    // 默认为表达式语句
    return exprStmt();
}

//解析表达式语句,exprStmt → expr ';'
std::shared_ptr<Stmt> Parser::exprStmt() {
    int line = peek(0).line;
    int column = peek(0).column;
    
    auto expression = expr();
    consume(TokenType::SEMICOLON, "Expected ';' after expression.");
    return std::make_shared<ExprStmt>(expression);
}

//解析变量声明语句,varDeclStmt → 'int' IDENT '=' expr ';'
std::shared_ptr<Stmt> Parser::varDeclStmt() {
    int line = peek(0).line;
    int column = peek(0).column;

    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after 'int'.");
    
    // 必须有初始化器
    consume(TokenType::ASSIGN, "Expected '=' after variable name.");
    auto initializer = expr();
    
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_shared<VarDeclStmt>(name.lexeme, initializer, line, column);
}

//解析赋值语句,assignStmt → IDENT '=' expr ';'
std::shared_ptr<Stmt> Parser::assignStmt() {
    int line = peek(0).line;
    int column = peek(0).column;

    Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
    consume(TokenType::ASSIGN, "Expected '=' after variable name.");
    
    auto value = expr();
    
    consume(TokenType::SEMICOLON, "Expected ';' after assignment.");
    return std::make_shared<AssignStmt>(name.lexeme, value, line, column);
}

//解析if语句,ifStmt → 'if' '(' expr ')' stmt ['else' stmt]
std::shared_ptr<Stmt> Parser::ifStmt() {
    int line = previous().line;  // 'if' token 的位置
    int column = previous().column;

    consume(TokenType::LPAREN, "Expected '(' after 'if'.");
    auto condition = expr();
    consume(TokenType::RPAREN, "Expected ')' after if condition.");
    
    auto thenBranch = stmt();
    std::shared_ptr<Stmt> elseBranch = nullptr;
    
    if (match({TokenType::ELSE})) {
        elseBranch = stmt();
    }
    
    return std::make_shared<IfStmt>(condition, thenBranch, elseBranch, line, column);
}

//解析while语句,whileStmt → 'while' '(' expr ')' stmt
std::shared_ptr<Stmt> Parser::whileStmt() {
    int line = previous().line;  // 'while' token 的位置
    int column = previous().column;
    consume(TokenType::LPAREN, "Expected '(' after 'while'.");
    auto condition = expr();
    consume(TokenType::RPAREN, "Expected ')' after while condition.");
    
    auto body = stmt();
    
    return std::make_shared<WhileStmt>(condition, body, line, column);
}

//解析break语句,breakStmt → 'break' ';'
std::shared_ptr<Stmt> Parser::breakStmt() {
    int line = previous().line;  // 'break' token 的位置
    int column = previous().column;

    consume(TokenType::SEMICOLON, "Expected ';' after 'break'.");
    return std::make_shared<BreakStmt>(line, column);
}

//解析continue语句,continueStmt → 'continue' ';'
std::shared_ptr<Stmt> Parser::continueStmt() {
    int line = previous().line;  // 'continue' token 的位置
    int column = previous().column;
    consume(TokenType::SEMICOLON, "Expected ';' after 'continue'.");
    return std::make_shared<ContinueStmt>(line, column);
}

//解析return语句,returnStmt → 'return' [expr] ';'
std::shared_ptr<Stmt> Parser::returnStmt() {
    int line = previous().line;  // 'return' token 的位置
    int column = previous().column;

    std::shared_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expr();
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after return value.");
    return std::make_shared<ReturnStmt>(value, line, column);
}

// 表达式解析示例实现,expr → lorExpr
std::shared_ptr<Expr> Parser::expr() {
    return lorExpr();
}

//逻辑或表达式,lorExpr → landExpr { '||' landExpr }
std::shared_ptr<Expr> Parser::lorExpr() {
    auto expr = landExpr();
    
    while (match({TokenType::OR})) {
        std::string op = previous().lexeme;
        auto right = landExpr();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

//逻辑与表达式,landExpr → relExpr { '&&' relExpr }
std::shared_ptr<Expr> Parser::landExpr() {
    auto expr = relExpr();
    
    while (match({TokenType::AND})) {
        std::string op = previous().lexeme;
        auto right = relExpr();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

//关系表达式,relExpr → addExpr { ('<' | '>' | '<=' | '>=' | '==' | '!=') addExpr }
std::shared_ptr<Expr> Parser::relExpr() {
    auto expr = addExpr();
    
    while (match({TokenType::LT, TokenType::GT, TokenType::LE, 
                 TokenType::GE, TokenType::EQ, TokenType::NEQ})) {
        std::string op = previous().lexeme;
        auto right = addExpr();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

//加减表达式,addExpr → mulExpr { ('+' | '-') mulExpr }
std::shared_ptr<Expr> Parser::addExpr() {
    auto expr = mulExpr();
    
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        std::string op = previous().lexeme;
        auto right = mulExpr();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    
    return expr;
}

//乘除模表达式,mulExpr → unaryExpr { ('*' | '/' | '%') unaryExpr }
std::shared_ptr<Expr> Parser::mulExpr() {
    auto expr = unaryExpr();
    
    while (match({TokenType::MULTIPLY, TokenType::DIVIDE, TokenType::MODULO})) {
        std::string op = previous().lexeme;
        int line = previous().line;
        int column = previous().column;
        auto right = unaryExpr();
        expr = std::make_shared<BinaryExpr>(expr, op, right, line, column);
    }
    
    return expr;
}

//一元表达式,unaryExpr → { ('+' | '-' | '!') } primaryExpr
std::shared_ptr<Expr> Parser::unaryExpr() {
    if (match({TokenType::PLUS, TokenType::MINUS, TokenType::NOT})) {
        std::string op = previous().lexeme;
        int line = previous().line;
        int column = previous().column;
        auto right = unaryExpr();
        return std::make_shared<UnaryExpr>(op, right, line, column);
    }
    
    return primaryExpr();
}

//基础表达式
/* primaryExpr → NUM
                   | IDENT
                   | IDENT '(' [args] ')'
                   | '(' expr ')'
*/
std::shared_ptr<Expr> Parser::primaryExpr() {
    if (match({TokenType::NUMBER})) {
        int value = std::stoi(previous().lexeme);
        int line = previous().line;
        int column = previous().column;
        return std::make_shared<NumberExpr>(value, line, column);
    }
    
    if (match({TokenType::IDENTIFIER})) {
        std::string name = previous().lexeme;
        int line = previous().line;
        int column = previous().column;
        
        // 检查是否是函数调用
        if (match({TokenType::LPAREN})) {
            std::vector<std::shared_ptr<Expr>> arguments;
            
            if (!check(TokenType::RPAREN)) {
                do {
                    arguments.push_back(expr());
                } while (match({TokenType::COMMA}));
            }
            
            consume(TokenType::RPAREN, "Expected ')' after arguments.");
            
            return std::make_shared<CallExpr>(name, arguments, line, column);
        }
        
        // 否则是变量引用
        return std::make_shared<VariableExpr>(name, line, column);
    }
    
    if (match({TokenType::LPAREN})) {
        auto expression = expr();
        consume(TokenType::RPAREN, "Expected ')' after expression.");
        return expression;
    }
    
    throw error(peek(0), "Expected expression.");
}
