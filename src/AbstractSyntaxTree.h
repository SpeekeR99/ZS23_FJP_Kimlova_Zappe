#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "SymbolTable.h"

class ASTNodeBlock;
class ASTNodeDeclVar;
class ASTNodeDeclFunc;
class ASTNodeIf;
class ASTNodeWhile;
class ASTNodeFor;
class ASTNodeBreakContinue;
class ASTNodeReturn;
class ASTNodeGoto;
class ASTNodeExpressionStatement;
class ASTNodeIdentifier;
class ASTNodeIntLiteral;
class ASTNodeBoolLiteral;
class ASTNodeStringLiteral;
class ASTNodeAssignExpression;
class ASTNodeTernaryOperator;
class ASTNodeBinaryOperator;
class ASTNodeUnaryOperator;
class ASTNodeCast;
class ASTNodeCallFunc;
class ASTNodeNew;
class ASTNodeDelete;
class ASTNodeDereference;
class ASTNodeReference;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ASTNodeBlock *node) = 0;
    virtual void visit(ASTNodeDeclVar *node) = 0;
    virtual void visit(ASTNodeDeclFunc *node) = 0;
    virtual void visit(ASTNodeIf *node) = 0;
    virtual void visit(ASTNodeWhile *node) = 0;
    virtual void visit(ASTNodeFor *node) = 0;
    virtual void visit(ASTNodeBreakContinue *node) = 0;
    virtual void visit(ASTNodeReturn *node) = 0;
    virtual void visit(ASTNodeGoto *node) = 0;
    virtual void visit(ASTNodeExpressionStatement *node) = 0;
    virtual void visit(ASTNodeIdentifier *node) = 0;
    virtual void visit(ASTNodeIntLiteral *node) = 0;
    virtual void visit(ASTNodeBoolLiteral *node) = 0;
    virtual void visit(ASTNodeStringLiteral *node) = 0;
    virtual void visit(ASTNodeAssignExpression *node) = 0;
    virtual void visit(ASTNodeTernaryOperator *node) = 0;
    virtual void visit(ASTNodeBinaryOperator *node) = 0;
    virtual void visit(ASTNodeUnaryOperator *node) = 0;
    virtual void visit(ASTNodeCast *node) = 0;
    virtual void visit(ASTNodeCallFunc *node) = 0;
    virtual void visit(ASTNodeNew *node) = 0;
    virtual void visit(ASTNodeDelete *node) = 0;
    virtual void visit(ASTNodeDereference *node) = 0;
    virtual void visit(ASTNodeReference *node) = 0;
};

class ASTNode {
public:
    int line = -1;
    virtual ~ASTNode() = default;

    virtual void accept(ASTVisitor *visitor) = 0;
};

class ASTNodeExpression : public ASTNode {
public:
    int line = -1;
    ~ASTNodeExpression() override = default;

    void accept(ASTVisitor *visitor) override = 0;
};

class ASTNodeStatement : public ASTNode {
public:
    int line = -1;
    std::string label;
    ~ASTNodeStatement() override = default;

    void accept(ASTVisitor *visitor) override = 0;
};

class ASTNodeBlock : public ASTNode {
public:
    int line = -1;
    int break_number = 0;
    int continue_number = 0;
    std::vector<ASTNodeStatement *> statements{};

    ASTNodeBlock() = default;

    ~ASTNodeBlock() override {
        for (auto &statement : statements)
            delete statement;
    }

    void count_breaks_and_continues();
    int get_number_of_declared_variables();
    std::vector<int> get_sizeof_variables();
    bool contains_return_statement();

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeDeclVar : public ASTNodeStatement {
public:
    int line;
    std::string type;
    int sizeof_type;
    std::string name;
    bool is_const;
    ASTNodeExpression *expression;
    int is_pointer;
    std::string label;

    ASTNodeDeclVar(const std::string &type, int is_pointer, const std::string &name, bool is_const, ASTNodeExpression *expression, int line) : name(name), is_pointer(is_pointer), type(type), is_const(is_const), expression(expression), line(line) {
        sizeof_type = sizeof_val_type(str_to_val_type(type));
    }

    ~ASTNodeDeclVar() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeDeclFunc : public ASTNodeStatement {
public:
    int line;
    std::string return_type;
    std::string name;
    std::vector<ASTNodeDeclVar *> parameters;
    ASTNodeBlock *block;
    std::string label;

    ASTNodeDeclFunc( const std::string &return_type, const std::string &name, std::vector<ASTNodeDeclVar *> parameters, ASTNodeBlock *block, int line) : return_type(return_type), name(name), parameters(std::move(parameters)), block(block), line(line) {
        /* Empty */
    }

    ~ASTNodeDeclFunc() override {
        delete block;
        for (auto &parameter : parameters)
            delete parameter;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeIf : public ASTNodeStatement {
public:
    int line;
    ASTNodeExpression *condition;
    ASTNodeBlock *block;
    ASTNodeBlock *else_block;
    std::string label;

    ASTNodeIf(ASTNodeExpression *condition, ASTNodeBlock *block, ASTNodeBlock *else_block, int line) : condition(condition), block(block), else_block(else_block), line(line) {
        /* Empty */
    }

    ~ASTNodeIf() override {
        delete condition;
        delete block;
        delete else_block;
    }

    bool contains_return_statement();

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeBreakContinue : public ASTNodeStatement {
public:
    int line;
    bool is_break;
    std::string label;

    explicit ASTNodeBreakContinue(bool is_break, int line) : is_break(is_break), line(line) {
        /* Empty */
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeWhile : public ASTNodeStatement {
public:
    int line;
    ASTNodeExpression *condition;
    ASTNodeBlock *block;
    bool is_do_while = false;
    bool is_repeat_until = false;
    int break_number = 0;
    int continue_number = 0;
    std::string label;

    ASTNodeWhile(ASTNodeExpression *condition, ASTNodeBlock *block, bool is_do_while, bool is_repeat_until, int line) : condition(condition), block(block), is_do_while(is_do_while), is_repeat_until(is_repeat_until), line(line) {
        /* Empty */
    }

    ~ASTNodeWhile() override {
        delete condition;
        delete block;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeFor : public ASTNodeStatement {
public:
    int line;
    ASTNodeStatement *init;
    ASTNodeExpression *condition;
    ASTNodeExpression *increment;
    ASTNodeBlock *block;
    int break_number = 0;
    int continue_number = 0;
    std::string label;

    ASTNodeFor(ASTNodeStatement *init, ASTNodeExpression *condition, ASTNodeExpression *increment, ASTNodeBlock *block, int line) : init(init), condition(condition), increment(increment), block(block), line(line) {
        /* Empty */
    }

    ~ASTNodeFor() override {
        delete init;
        delete condition;
        delete increment;
        delete block;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeReturn : public ASTNodeStatement {
public:
    int line;
    ASTNodeExpression *expression;
    std::string label;

    explicit ASTNodeReturn(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeReturn() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeGoto : public ASTNodeStatement {
public:
    int line;
    std::string label;
    std::string label_to_go_to;

    explicit ASTNodeGoto(const std::string &label, int line) : label_to_go_to(label), line(line) {
        /* Empty */
    }

    ~ASTNodeGoto() override = default;

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeExpressionStatement : public ASTNodeStatement {
public:
    int line;
    ASTNodeExpression *expression;
    std::string label;

    ASTNodeExpressionStatement(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeExpressionStatement() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeIdentifier : public ASTNodeExpression {
public:
    int line;
    std::string name;

    explicit ASTNodeIdentifier(const std::string &name, int line) : name(name), line(line) {
        /* Empty */
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeIntLiteral : public ASTNodeExpression {
public:
    int line;
    int value;

    explicit ASTNodeIntLiteral(int value, int line) : value(value), line(line) {
        /* Empty */
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeBoolLiteral : public ASTNodeExpression {
public:
    int line;
    bool value;

    explicit ASTNodeBoolLiteral(bool value, int line) : value(value), line(line) {
        /* Empty */
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeStringLiteral : public ASTNodeExpression {
public:
    int line;
    std::string value;

    explicit ASTNodeStringLiteral(const std::string &value, int line) : value(value), line(line) {
        /* Empty */
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeAssignExpression : public ASTNodeExpression {
public:
    int line;
    std::string name;
    ASTNodeExpression *lvalue;
    ASTNodeExpression *expression;

    ASTNodeAssignExpression(const std::string &name, ASTNodeExpression *lvalue, ASTNodeExpression *expression, int line) : name(name), lvalue(lvalue), expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeAssignExpression() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeTernaryOperator : public ASTNodeExpression {
public:
    int line;
    ASTNodeExpression *condition;
    ASTNodeExpression *true_expression;
    ASTNodeExpression *false_expression;

    ASTNodeTernaryOperator(ASTNodeExpression *condition, ASTNodeExpression *true_expression, ASTNodeExpression *false_expression, int line) : condition(condition), true_expression(true_expression), false_expression(false_expression), line(line) {
        /* Empty */
    }

    ~ASTNodeTernaryOperator() override {
        delete condition;
        delete true_expression;
        delete false_expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeBinaryOperator : public ASTNodeExpression {
public:
    int line;
    ASTNodeExpression *left;
    ASTNodeExpression *right;
    std::string op;

    ASTNodeBinaryOperator(ASTNodeExpression *left, const std::string &op, ASTNodeExpression *right, int line) : left(left), op(op), right(right), line(line) {
        /* Empty */
    }

    ~ASTNodeBinaryOperator() override {
        delete left;
        delete right;
    }

    bool contains_reference();
    std::string find_dereference();

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeUnaryOperator : public ASTNodeExpression {
public:
    int line;
    ASTNodeExpression *expression;
    std::string op;

    ASTNodeUnaryOperator(const std::string &op, ASTNodeExpression *expression, int line) : op(op), expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeUnaryOperator() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeCast : public ASTNodeExpression {
public:
    int line;
    std::string type;
    ASTNodeExpression *expression;

    ASTNodeCast(const std::string &type, ASTNodeExpression *expression, int line) : type(type), expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeCast() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeCallFunc : public ASTNodeExpression {
public:
    int line;
    std::string name;
    std::vector<ASTNodeExpression *> arguments;

    ASTNodeCallFunc(const std::string &name, std::vector<ASTNodeExpression *> arguments, int line) : name(name), arguments(std::move(arguments)), line(line) {
        /* Empty */
    }

    ~ASTNodeCallFunc() override {
        for (auto &argument : arguments) {
            delete argument;
        }
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeNew : public ASTNodeExpression {
public:
    int line;
    std::string type;
    ASTNodeExpression *expression;

    ASTNodeNew(const std::string &type, ASTNodeExpression *expression, int line) : type(type), expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeNew() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeDelete : public ASTNodeExpression {
public:
    int line;
    ASTNodeExpression *expression;

    explicit ASTNodeDelete(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeDelete() override {
        delete expression;
    }

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeDereference : public ASTNodeExpression {
public:
    int line;
    std::string identifier;
    ASTNodeExpression *expression;
    bool is_lvalue = false;
    bool is_pointing_to_stack = false;

    ASTNodeDereference(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        this->what_do_i_dereference();
    }

    ~ASTNodeDereference() override {
        delete expression;
    }

    void what_do_i_dereference();

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeReference : public ASTNodeExpression {
public:
    int line;
    std::string identifier;

    explicit ASTNodeReference(const std::string &identifier, int line) : identifier(identifier), line(line) {
        /* Empty */
    }

    ~ASTNodeReference() override = default;

    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};
