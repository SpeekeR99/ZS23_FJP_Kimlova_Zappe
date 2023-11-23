#pragma once

#include <iostream>
#include <vector>
#include <string>

class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual void debug_print() = 0;
};

class ASTNodeExpression : public ASTNode {
public:
    ~ASTNodeExpression() override = default;

    void debug_print() override = 0;
};

class ASTNodeStatement : public ASTNode {
public:
    ~ASTNodeStatement() override = default;

    void debug_print() override = 0;
};

class ASTNodeBlock : public ASTNode {
public:
    std::vector<ASTNodeStatement *> statements{};

    ASTNodeBlock() = default;

    ~ASTNodeBlock() override {
        for (auto &statement : statements)
            delete statement;
    }

    void debug_print() override {
        for (auto &statement : statements)
            statement->debug_print();
    }
};

class ASTNodeDeclVar : public ASTNodeStatement {
public:
    std::string type;
    std::string name;
    bool is_const;
    ASTNodeExpression *expression;

    ASTNodeDeclVar(const std::string &type, const std::string &name, bool is_const, ASTNodeExpression *expression) : name(name), type(type), is_const(is_const), expression(expression) {
        /* Empty */
    }

    ~ASTNodeDeclVar() override {
        delete expression;
    }

    void debug_print() override {
        std::cout << "var " << name << " : " << type << " = ";
        if (expression)
            expression->debug_print();
        std::cout << std::endl;
    }
};

class ASTNodeDeclFunc : public ASTNodeStatement {
public:
    std::string return_type;
    std::string name;
    std::vector<ASTNodeDeclVar *> parameters;
    ASTNodeBlock *block;

    ASTNodeDeclFunc( const std::string &return_type, const std::string &name, std::vector<ASTNodeDeclVar *> parameters, ASTNodeBlock *block) : return_type(return_type), name(name), parameters(std::move(parameters)), block(block) {
        /* Empty */
    }

    ~ASTNodeDeclFunc() override {
        delete block;
        for (auto &parameter : parameters)
            delete parameter;
    }

    void debug_print() override {
        std::cout << "func " << name << "(";
        for (auto &parameter : parameters)
            std::cout << parameter->name << " : " << parameter->type << ", ";
        std::cout << ") : " << return_type << std::endl;
        block->debug_print();
    }
};

class ASTNodeIf : public ASTNodeStatement {
public:
    ASTNodeExpression *condition;
    ASTNodeBlock *block;
    ASTNodeBlock *else_block;

    ASTNodeIf(ASTNodeExpression *condition, ASTNodeBlock *block, ASTNodeBlock *else_block) : condition(condition), block(block), else_block(else_block) {
        /* Empty */
    }

    ~ASTNodeIf() override {
        delete condition;
        delete block;
        delete else_block;
    }

    void debug_print() override {
        std::cout << "if ";
        condition->debug_print();
        std::cout << std::endl;
        block->debug_print();
        if (else_block) {
            std::cout << "else" << std::endl;
            else_block->debug_print();
        }
    }
};

class ASTNodeWhile : public ASTNodeStatement {
public:
    ASTNodeExpression *condition;
    ASTNodeBlock *block;

    ASTNodeWhile(ASTNodeExpression *condition, ASTNodeBlock *block) : condition(condition), block(block) {
        /* Empty */
    }

    ~ASTNodeWhile() override {
        delete condition;
        delete block;
    }

    void debug_print() override {
        std::cout << "while ";
        condition->debug_print();
        std::cout << std::endl;
        block->debug_print();
    }
};

class ASTNodeFor : public ASTNodeStatement {
public:
    ASTNodeStatement *init;
    ASTNodeExpression *condition;
    ASTNodeExpression *increment;
    ASTNodeBlock *block;

    ASTNodeFor(ASTNodeStatement *init, ASTNodeExpression *condition, ASTNodeExpression *increment, ASTNodeBlock *block) : init(init), condition(condition), increment(increment), block(block) {
        /* Empty */
    }

    ~ASTNodeFor() override {
        delete init;
        delete condition;
        delete increment;
        delete block;
    }

    void debug_print() override {
        std::cout << "for ";
        init->debug_print();
        std::cout << " ";
        condition->debug_print();
        std::cout << " ";
        increment->debug_print();
        std::cout << std::endl;
        block->debug_print();
    }
};

class ASTNodeReturn : public ASTNodeStatement {
public:
    ASTNodeExpression *expression;

    explicit ASTNodeReturn(ASTNodeExpression *expression) : expression(expression) {
        /* Empty */
    }

    ~ASTNodeReturn() override {
        delete expression;
    }

    void debug_print() override {
        std::cout << "return ";
        if (expression)
            expression->debug_print();
        std::cout << std::endl;
    }
};

class ASTNodeExpressionStatement : public ASTNodeStatement {
public:
    ASTNodeExpression *expression;

    explicit ASTNodeExpressionStatement(ASTNodeExpression *expression) : expression(expression) {
        /* Empty */
    }

    ~ASTNodeExpressionStatement() override {
        delete expression;
    }

    void debug_print() override {
        expression->debug_print();
        std::cout << std::endl;
    }
};

class ASTNodeIdentifier : public ASTNodeExpression {
public:
    std::string name;

    explicit ASTNodeIdentifier(const std::string &name) : name(name) {
        /* Empty */
    }

    void debug_print() override {
        std::cout << name;
    }
};

class ASTNodeIntLiteral : public ASTNodeExpression {
public:
    int value;

    explicit ASTNodeIntLiteral(int value) : value(value) {
        /* Empty */
    }

    void debug_print() override {
        std::cout << value;
    }
};

class ASTNodeAssignExpression : public ASTNodeExpression {
public:
    std::string name;
    ASTNodeExpression *expression;

    ASTNodeAssignExpression(const std::string &name, ASTNodeExpression *expression) : name(name), expression(expression) {
        /* Empty */
    }

    ~ASTNodeAssignExpression() override {
        delete expression;
    }

    void debug_print() override {
        std::cout << name << " = ";
        expression->debug_print();
    }
};

class ASTNodeBinaryOperator : public ASTNodeExpression {
public:
    ASTNodeExpression *left;
    ASTNodeExpression *right;
    std::string op;

    ASTNodeBinaryOperator(ASTNodeExpression *left, const std::string &op, ASTNodeExpression *right) : left(left), op(op), right(right) {
        /* Empty */
    }

    ~ASTNodeBinaryOperator() override {
        delete left;
        delete right;
    }

    void debug_print() override {
        std::cout << "(";
        left->debug_print();
        std::cout << " " << op << " ";
        right->debug_print();
        std::cout << ")";
    }
};

class ASTNodeUnaryOperator : public ASTNodeExpression {
public:
    ASTNodeExpression *expression;
    std::string op;

    ASTNodeUnaryOperator(const std::string &op, ASTNodeExpression *expression) : op(op), expression(expression) {
        /* Empty */
    }

    ~ASTNodeUnaryOperator() override {
        delete expression;
    }

    void debug_print() override {
        std::cout << op;
        expression->debug_print();
    }
};

class ASTNodeCast : public ASTNodeExpression {
public:
    std::string type;
    ASTNodeExpression *expression;

    ASTNodeCast(const std::string &type, ASTNodeExpression *expression) : type(type), expression(expression) {
        /* Empty */
    }

    ~ASTNodeCast() override {
        delete expression;
    }

    void debug_print() override {
        std::cout << "(" << type << ")";
        expression->debug_print();
    }
};

class ASTNodeCallFunc : public ASTNodeExpression {
public:
    std::string name;
    std::vector<ASTNodeExpression *> arguments;

    ASTNodeCallFunc(const std::string &name, std::vector<ASTNodeExpression *> arguments) : name(name), arguments(std::move(arguments)) {
        /* Empty */
    }

    ~ASTNodeCallFunc() override {
        for (auto &argument : arguments) {
            delete argument;
        }
    }

    void debug_print() override {
        std::cout << name << "(";
        for (auto &argument : arguments) {
            argument->debug_print();
            std::cout << ", ";
        }
        std::cout << ")";
    }
};
