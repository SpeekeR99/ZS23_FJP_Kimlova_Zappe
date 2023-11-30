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
class ASTNodeExpressionStatement;
class ASTNodeIdentifier;
class ASTNodeIntLiteral;
class ASTNodeBoolLiteral;
class ASTNodeAssignExpression;
class ASTNodeBinaryOperator;
class ASTNodeUnaryOperator;
class ASTNodeCast;
class ASTNodeCallFunc;
class ASTNodeNew;
class ASTNodeDelete;
class ASTNodeDereference;
class ASTNodeReference;
class ASTNodeDynamicAssignExpression;

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
    virtual void visit(ASTNodeExpressionStatement *node) = 0;
    virtual void visit(ASTNodeIdentifier *node) = 0;
    virtual void visit(ASTNodeIntLiteral *node) = 0;
    virtual void visit(ASTNodeBoolLiteral *node) = 0;
    virtual void visit(ASTNodeAssignExpression *node) = 0;
    virtual void visit(ASTNodeBinaryOperator *node) = 0;
    virtual void visit(ASTNodeUnaryOperator *node) = 0;
    virtual void visit(ASTNodeCast *node) = 0;
    virtual void visit(ASTNodeCallFunc *node) = 0;
    virtual void visit(ASTNodeNew *node) = 0;
    virtual void visit(ASTNodeDelete *node) = 0;
    virtual void visit(ASTNodeDereference *node) = 0;
    virtual void visit(ASTNodeReference *node) = 0;
    virtual void visit(ASTNodeDynamicAssignExpression *node) = 0;
};

class ASTNode {
public:
    int line = -1;
    virtual ~ASTNode() = default;

    virtual void debug_print() = 0;
    virtual void accept(ASTVisitor *visitor) = 0;
};

class ASTNodeExpression : public ASTNode {
public:
    int line = -1;
    ~ASTNodeExpression() override = default;

    void debug_print() override = 0;
    void accept(ASTVisitor *visitor) override = 0;
};

class ASTNodeStatement : public ASTNode {
public:
    int line = -1;
    ~ASTNodeStatement() override = default;

    void debug_print() override = 0;
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

    void debug_print() override {
        for (auto &statement : statements)
            statement->debug_print();
    }
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

    ASTNodeDeclVar(const std::string &type, int is_pointer, const std::string &name, bool is_const, ASTNodeExpression *expression, int line) : name(name), is_pointer(is_pointer), type(type), is_const(is_const), expression(expression), line(line) {
        sizeof_type = sizeof_val_type(str_to_val_type(type));
    }

    ~ASTNodeDeclVar() override {
        delete expression;
    }

    void debug_print() override {
        if (is_const)
            std::cout << "const ";
        for (int i = 0; i < is_pointer; i++)
            std::cout << "*";
        std::cout << "var " << name << " : " << type << " = ";
        if (expression)
            expression->debug_print();
        std::cout << std::endl;
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

    ASTNodeDeclFunc( const std::string &return_type, const std::string &name, std::vector<ASTNodeDeclVar *> parameters, ASTNodeBlock *block, int line) : return_type(return_type), name(name), parameters(std::move(parameters)), block(block), line(line) {
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

    ASTNodeIf(ASTNodeExpression *condition, ASTNodeBlock *block, ASTNodeBlock *else_block, int line) : condition(condition), block(block), else_block(else_block), line(line) {
        /* Empty */
    }

    ~ASTNodeIf() override {
        delete condition;
        delete block;
        delete else_block;
    }

    bool contains_return_statement();

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
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeBreakContinue : public ASTNodeStatement {
public:
    int line;
    bool is_break;

    explicit ASTNodeBreakContinue(bool is_break, int line) : is_break(is_break), line(line) {
        /* Empty */
    }

    void debug_print() override {
        if (is_break)
            std::cout << "break";
        else
            std::cout << "continue";
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
    int break_number = 0;
    int continue_number = 0;

    ASTNodeWhile(ASTNodeExpression *condition, ASTNodeBlock *block, bool is_do_while, int line) : condition(condition), block(block), is_do_while(is_do_while), line(line) {
        /* Empty */
    }

    ~ASTNodeWhile() override {
        delete condition;
        delete block;
    }

    void debug_print() override {
        if (is_do_while) {
            std::cout << "do ";
            block->debug_print();
            std::cout << "while ";
            condition->debug_print();
        } else {
            std::cout << "while ";
            condition->debug_print();
            std::cout << std::endl;
            block->debug_print();
        }
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

    ASTNodeFor(ASTNodeStatement *init, ASTNodeExpression *condition, ASTNodeExpression *increment, ASTNodeBlock *block, int line) : init(init), condition(condition), increment(increment), block(block), line(line) {
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
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeReturn : public ASTNodeStatement {
public:
    int line;
    ASTNodeExpression *expression;

    explicit ASTNodeReturn(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
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
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeExpressionStatement : public ASTNodeStatement {
public:
    int line;
    ASTNodeExpression *expression;

    ASTNodeExpressionStatement(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeExpressionStatement() override {
        delete expression;
    }

    void debug_print() override {
        expression->debug_print();
        std::cout << std::endl;
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

    void debug_print() override {
        std::cout << name;
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

    void debug_print() override {
        std::cout << value;
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

    void debug_print() override {
        std::cout << value;
    }
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeAssignExpression : public ASTNodeExpression {
public:
    int line;
    std::string name;
    ASTNodeExpression *expression;

    ASTNodeAssignExpression(const std::string &name, ASTNodeExpression *expression, int line) : name(name), expression(expression), line(line) {
        /* Empty */
    }

    ~ASTNodeAssignExpression() override {
        delete expression;
    }

    void debug_print() override {
        std::cout << name << " = ";
        expression->debug_print();
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

    void debug_print() override {
        std::cout << "(";
        left->debug_print();
        std::cout << " " << op << " ";
        right->debug_print();
        std::cout << ")";
    }
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

    void debug_print() override {
        std::cout << op;
        expression->debug_print();
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

    void debug_print() override {
        std::cout << "(" << type << ")";
        expression->debug_print();
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

    void debug_print() override {
        std::cout << name << "(";
        for (auto &argument : arguments) {
            argument->debug_print();
            std::cout << ", ";
        }
        std::cout << ")";
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

    void debug_print() override {
        std::cout << "new " << type << "[";
        expression->debug_print();
        std::cout << "]";
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

    void debug_print() override {
        std::cout << "delete ";
        expression->debug_print();
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

    void debug_print() override {
        std::cout << "*";
        expression->debug_print();
    }
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

    void debug_print() override {
        std::cout << "&" << identifier;
    }
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

class ASTNodeDynamicAssignExpression : public ASTNodeExpression {
public:
    int line;
    ASTNodeExpression *left;
    ASTNodeExpression *right;

    ASTNodeDynamicAssignExpression(ASTNodeExpression *left, ASTNodeExpression *right, int line) : left(left), right(right), line(line) {
        /* Empty */
    }

    ~ASTNodeDynamicAssignExpression() override {
        delete left;
        delete right;
    }

    void debug_print() override {
        left->debug_print();
        std::cout << " = ";
        right->debug_print();
    }
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};