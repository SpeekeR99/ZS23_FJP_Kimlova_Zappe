#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "SymbolTable.h"

/* Forward declarations */
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
class ASTNodeFloatLiteral;
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
class ASTNodeSizeof;

/**
 * Class representing a generic visitor pattern for the AST
 */
class ASTVisitor {
public:
    /**
     * Destructor
     */
    virtual ~ASTVisitor() = default;

    /* Visit methods */
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
    virtual void visit(ASTNodeFloatLiteral *node) = 0;
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
    virtual void visit(ASTNodeSizeof *node) = 0;
};

/**
 * Class representing a generic AST node
 */
class ASTNode {
public:
    /** Line for error reporting */
    int line = -1;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Destructor
     */
    virtual ~ASTNode() = default;

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    virtual void accept(ASTVisitor *visitor) = 0;
};

/**
 * Class representing a generic AST node for expressions
 */
class ASTNodeExpression : public ASTNode {
public:
    /** Line for error reporting */
    int line = -1;
    /** Label for the goto instruction */
    std::string label;
    /** Parent node for AST optimizations */
    ASTNode *parent = nullptr;

    /**
     * Destructor
     */
    ~ASTNodeExpression() override = default;

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    void accept(ASTVisitor *visitor) override = 0;
};

/**
 * Class representing a generic AST node for statements
 */
class ASTNodeStatement : public ASTNode {
public:
    /** Line for error reporting */
    int line = -1;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Destructor
     */
    ~ASTNodeStatement() override = default;

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    void accept(ASTVisitor *visitor) override = 0;
};

/**
 * Class representing an AST node for block
 */
class ASTNodeBlock : public ASTNode {
public:
    /** Line for error reporting */
    int line = -1;
    /** Number of break statements in the block */
    int break_number = 0;
    /** Number of continue statements in the block */
    int continue_number = 0;
    /** Statements in the block */
    std::vector<ASTNodeStatement *> statements{};
    /** Label for the goto instruction */
    std::string label;

    /**
     * Default constructor
     */
    ASTNodeBlock() = default;

    /**
     * Destructor
     */
    ~ASTNodeBlock() override {
        for (auto &statement : statements)
            delete statement;
    }

    /**
     * Counts the number of break and continue statements in the block
     */
    void count_breaks_and_continues();
    /**
     * Counts the number of declared variables in the block
     * @return Number of declared variables
     */
    int get_number_of_declared_variables();
    /**
     * Counts the size of declared functions in the block
     * @return Size of declared functions
     */
    std::vector<uint32_t> get_sizeof_variables();
    /**
     * Checks if the block contains a return statement
     * @return True if the block contains a return statement, false otherwise
     */
    bool contains_return_statement();

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for variable declaration
 */
class ASTNodeDeclVar : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Type of the variable */
    std::string type;
    /** Size of the type of the variable */
    uint32_t sizeof_type;
    /** Name of the variable */
    std::string name;
    /** True if the variable is constant, false otherwise */
    bool is_const;
    /** Expression for the initialization of the variable; can be null */
    ASTNodeExpression *expression;
    /** Pointer level of the variable */
    uint32_t is_pointer;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param type Type of the variable
     * @param is_pointer Pointer level of the variable
     * @param name Name of the variable
     * @param is_const True if the variable is constant, false otherwise
     * @param expression Expression for the initialization of the variable; can be null
     * @param line Line for error reporting
     */
    ASTNodeDeclVar(const std::string &type, int is_pointer, const std::string &name, bool is_const, ASTNodeExpression *expression, int line) : name(name), is_pointer(is_pointer), type(type), is_const(is_const), expression(expression), line(line) {
        this->sizeof_type = sizeof_val_type(str_to_val_type(type));
    }

    /**
     * Destructor
     */
    ~ASTNodeDeclVar() override {
        delete expression;
    }

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for function declaration
 */
class ASTNodeDeclFunc : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Return type of the function */
    std::string return_type;
    /** Name of the function */
    std::string name;
    /** Parameters of the function */
    std::vector<ASTNodeDeclVar *> parameters;
    /** Block of the function; can be null */
    ASTNodeBlock *block;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param return_type Return type of the function
     * @param name Name of the function
     * @param parameters Parameters of the function
     * @param block Block of the function; can be null
     * @param line Line for error reporting
     */
    ASTNodeDeclFunc( const std::string &return_type, const std::string &name, std::vector<ASTNodeDeclVar *> parameters, ASTNodeBlock *block, int line) : return_type(return_type), name(name), parameters(std::move(parameters)), block(block), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeDeclFunc() override {
        delete block;
        for (auto &parameter : parameters)
            delete parameter;
    }

    /**
     * Counts the number of declared variables in the function
     * @return Number of declared variables
     */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for if statement
 */
class ASTNodeIf : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Condition of the if statement */
    ASTNodeExpression *condition;
    /** Block of the if statement */
    ASTNodeBlock *block;
    /** Else block of the if statement; can be null */
    ASTNodeBlock *else_block;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param condition Condition of the if statement
     * @param block Block of the if statement
     * @param else_block Else block of the if statement; can be null
     * @param line Line for error reporting
     */
    ASTNodeIf(ASTNodeExpression *condition, ASTNodeBlock *block, ASTNodeBlock *else_block, int line) : condition(condition), block(block), else_block(else_block), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeIf() override {
        delete condition;
        delete block;
        delete else_block;
    }

    /**
     * Checks if the if statement contains a return statement
     * @return True if the if statement contains a return statement, false otherwise
     */
    bool contains_return_statement();

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for break and continue statements
 */
class ASTNodeBreakContinue : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** True if the statement is a break statement, false if it is a continue statement */
    bool is_break;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param is_break True if the statement is a break statement, false if it is a continue statement
     * @param line Line for error reporting
     */
    explicit ASTNodeBreakContinue(bool is_break, int line) : is_break(is_break), line(line) {
        /* Empty */
    }

    /**
     * Accept method for the visitor pattern
     * @param visitor Visitor to accept
     */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for while statement
 */
class ASTNodeWhile : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Condition of the while statement */
    ASTNodeExpression *condition;
    /** Block of the while statement */
    ASTNodeBlock *block;
    /** True if the while statement is a do-while statement, false otherwise */
    bool is_do_while = false;
    /** True if the while statement is a repeat-until statement, false otherwise */
    bool is_repeat_until = false;
    /** Number of break statements in the while statement */
    int break_number = 0;
    /** Number of continue statements in the while statement */
    int continue_number = 0;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param condition Condition of the while statement
     * @param block Block of the while statement
     * @param is_do_while True if the while statement is a do-while statement, false otherwise
     * @param is_repeat_until True if the while statement is a repeat-until statement, false otherwise
     * @param line Line for error reporting
     */
    ASTNodeWhile(ASTNodeExpression *condition, ASTNodeBlock *block, bool is_do_while, bool is_repeat_until, int line) : condition(condition), block(block), is_do_while(is_do_while), is_repeat_until(is_repeat_until), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeWhile() override {
        delete condition;
        delete block;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for for statement
 */
class ASTNodeFor : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Initialization of the for statement */
    ASTNodeStatement *init;
    /** Condition of the for statement */
    ASTNodeExpression *condition;
    /** Increment of the for statement */
    ASTNodeExpression *increment;
    /** Block of the for statement */
    ASTNodeBlock *block;
    /** Number of break statements in the for statement */
    int break_number = 0;
    /** Number of continue statements in the for statement */
    int continue_number = 0;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param init Initialization of the for statement
     * @param condition Condition of the for statement
     * @param increment Increment of the for statement
     * @param block Block of the for statement
     * @param line Line for error reporting
     */
    ASTNodeFor(ASTNodeStatement *init, ASTNodeExpression *condition, ASTNodeExpression *increment, ASTNodeBlock *block, int line) : init(init), condition(condition), increment(increment), block(block), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeFor() override {
        delete init;
        delete condition;
        delete increment;
        delete block;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for return statement
 */
class ASTNodeReturn : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Expression of the return statement; can be null (void funcs) */
    ASTNodeExpression *expression;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param expression Expression of the return statement; can be null (void funcs)
     * @param line Line for error reporting
     */
    explicit ASTNodeReturn(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeReturn() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for goto statement
 */
class ASTNodeGoto : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Label for the goto instruction */
    std::string label;
    /** Label to go to */
    std::string label_to_go_to;

    /**
     * Constructor
     * @param label_to_go_to Label to go to
     * @param line Line for error reporting
     */
    explicit ASTNodeGoto(const std::string &label, int line) : label_to_go_to(label), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeGoto() override = default;

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for expression statement
 * Basically just a statement wrapper for an expression
 */
class ASTNodeExpressionStatement : public ASTNodeStatement {
public:
    /** Line for error reporting */
    int line;
    /** Expression of the statement */
    ASTNodeExpression *expression;
    /** Label for the goto instruction */
    std::string label;

    /**
     * Constructor
     * @param expression Expression of the statement
     * @param line Line for error reporting
     */
    ASTNodeExpressionStatement(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeExpressionStatement() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for identifier
 */
class ASTNodeIdentifier : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Name of the identifier */
    std::string name;

    /**
     * Constructor
     * @param name Name of the identifier
     * @param line Line for error reporting
     */
    explicit ASTNodeIdentifier(const std::string &name, int line) : name(name), line(line) {
        /* Empty */
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for integer literal
 */
class ASTNodeIntLiteral : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Value of the integer literal */
    int value;

    /**
     * Constructor
     * @param value Value of the integer literal
     * @param line Line for error reporting
     */
    explicit ASTNodeIntLiteral(int value, int line) : value(value), line(line) {
        /* Empty */
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for boolean literal
 */
class ASTNodeBoolLiteral : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Value of the boolean literal */
    bool value;

    /**
     * Constructor
     * @param value Value of the boolean literal
     * @param line Line for error reporting
     */
    explicit ASTNodeBoolLiteral(bool value, int line) : value(value), line(line) {
        /* Empty */
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for string literal
 */
class ASTNodeStringLiteral : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Value of the string literal */
    std::string value;

    /**
     * Constructor
     * @param value Value of the string literal
     * @param line Line for error reporting
     */
    explicit ASTNodeStringLiteral(const std::string &value, int line) : value(value), line(line) {
        /* Empty */
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for float literal
 */
class ASTNodeFloatLiteral : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Value of the float literal */
    float value;

    /**
     * Constructor
     * @param value Value of the float literal
     * @param line Line for error reporting
     */
    explicit ASTNodeFloatLiteral(float value, int line) : value(value), line(line) {
        /* Empty */
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for new expression
 */
class ASTNodeNew : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Type of the new expression */
    std::string type;
    /** Expression for the size of the new expression */
    ASTNodeExpression *expression;

    /**
     * Constructor
     * @param type Type of the new expression
     * @param expression Expression for the size of the new expression
     * @param line Line for error reporting
     */
    ASTNodeNew(const std::string &type, ASTNodeExpression *expression, int line) : type(type), expression(expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeNew() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for delete expression
 */
class ASTNodeDelete : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Expression for the delete expression */
    ASTNodeExpression *expression;

    /**
     * Constructor
     * @param expression Expression for the delete expression
     * @param line Line for error reporting
     */
    explicit ASTNodeDelete(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeDelete() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for dereference expression
 */
class ASTNodeDereference : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Identifier of the dereference expression */
    std::string identifier;
    /** Expression of the dereference expression */
    ASTNodeExpression *expression;
    /** True if the dereference expression is an lvalue, false otherwise */
    bool is_lvalue = false;
    /** True if the dereference expression is pointing to the stack, false otherwise */
    bool is_pointing_to_stack = false;

    /**
     * Constructor
     * @param expression Expression of the dereference expression
     * @param line Line for error reporting
     */
    ASTNodeDereference(ASTNodeExpression *expression, int line) : expression(expression), line(line) {
        this->what_do_i_dereference();
    }

    /**
     * Destructor
     */
    ~ASTNodeDereference() override {
        delete expression;
    }

    /**
     * Finds and sets the identifier of the dereference expression
     */
    void what_do_i_dereference();

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for reference expression
 */
class ASTNodeReference : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Identifier of the reference expression */
    std::string identifier;

    /**
     * Constructor
     * @param identifier Identifier of the reference expression
     * @param line Line for error reporting
     */
    explicit ASTNodeReference(const std::string &identifier, int line) : identifier(identifier), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeReference() override = default;

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for assign expression
 */
class ASTNodeAssignExpression : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Name of the assign expression */
    std::string name;
    /** Lvalue of the assign expression */
    ASTNodeExpression *lvalue;
    /** Expression of the assign expression */
    ASTNodeExpression *expression;

    /**
     * Constructor
     * @param name Name of the assign expression
     * @param lvalue Lvalue of the assign expression
     * @param expression Expression of the assign expression
     * @param line Line for error reporting
     */
    ASTNodeAssignExpression(const std::string &name, ASTNodeExpression *lvalue, ASTNodeExpression *expression, int line) : name(name), lvalue(lvalue), expression(expression), line(line) {
        if (auto *deref = dynamic_cast<ASTNodeDereference *>(lvalue)) { /* If the lvalue is a dereference */
            deref->is_lvalue = true;
            if (dynamic_cast<ASTNodeNew *>(deref->expression)) /* If the dereference is a new expression */
                deref->is_pointing_to_stack = false;
            else
                deref->is_pointing_to_stack = true;
        }
    }

    /**
     * Destructor
     */
    ~ASTNodeAssignExpression() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for ternary operator
 */
class ASTNodeTernaryOperator : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Condition of the ternary operator */
    ASTNodeExpression *condition;
    /** Expression if the condition is true */
    ASTNodeExpression *true_expression;
    /** Expression if the condition is false */
    ASTNodeExpression *false_expression;

    /**
     * Constructor
     * @param condition Condition of the ternary operator
     * @param true_expression Expression if the condition is true
     * @param false_expression Expression if the condition is false
     * @param line Line for error reporting
     */
    ASTNodeTernaryOperator(ASTNodeExpression *condition, ASTNodeExpression *true_expression, ASTNodeExpression *false_expression, int line) : condition(condition), true_expression(true_expression), false_expression(false_expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeTernaryOperator() override {
        delete condition;
        delete true_expression;
        delete false_expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for binary operator
 */
class ASTNodeBinaryOperator : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Left expression of the binary operator */
    ASTNodeExpression *left;
    /** Right expression of the binary operator */
    ASTNodeExpression *right;
    /** Operator of the binary operator */
    std::string op;
    /** True if the binary operator is a pointer arithmetic, false otherwise */
    bool is_pointer_arithmetic = false;
    /** True if the binary operator is a float arithmetic, false otherwise */
    bool is_float_arithmetic = false;

    /**
     * Constructor
     * @param left Left expression of the binary operator
     * @param op Operator of the binary operator
     * @param right Right expression of the binary operator
     * @param line Line for error reporting
     */
    ASTNodeBinaryOperator(ASTNodeExpression *left, const std::string &op, ASTNodeExpression *right, int line) : left(left), op(op), right(right), line(line) {
        this->is_float_arithmetic_check();
    }

    /**
     * Destructor
     */
    ~ASTNodeBinaryOperator() override {
        delete left;
        delete right;
    }

    /**
     * Checks if the binary operator contains a reference
     * @return True if the binary operator contains a reference, false otherwise
     */
    bool contains_reference();
    /**
     * Checks if the binary operator contains a dereference and returns the name of the dereference
     * @return Name of the dereference if the binary operator contains a dereference, empty string otherwise
     */
    std::string find_dereference();
    /**
     * Checks if the binary operator is floating point arithmetic
     * Sets the is_float_arithmetic flag
     */
    void is_float_arithmetic_check();
    /**
     * Propagates the float arithmetic flag to the children if its compound binary operator
     */
    void propagate_float();

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for unary operator
 */
class ASTNodeUnaryOperator : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Expression of the unary operator */
    ASTNodeExpression *expression;
    /** Operator of the unary operator */
    std::string op;

    /**
     * Constructor
     * @param op Operator of the unary operator
     * @param expression Expression of the unary operator
     * @param line Line for error reporting
     */
    ASTNodeUnaryOperator(const std::string &op, ASTNodeExpression *expression, int line) : op(op), expression(expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeUnaryOperator() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for cast expression
 */
class ASTNodeCast : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Type of the cast expression */
    std::string type;
    /** Expression of the cast expression */
    ASTNodeExpression *expression;

    /**
     * Constructor
     * @param type Type of the cast expression
     * @param expression Expression of the cast expression
     * @param line Line for error reporting
     */
    ASTNodeCast(const std::string &type, ASTNodeExpression *expression, int line) : type(type), expression(expression), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeCast() override {
        delete expression;
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Class representing an AST node for call function expression
 */
class ASTNodeCallFunc : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Name of the call function expression */
    std::string name;
    /** Arguments of the call function expression */
    std::vector<ASTNodeExpression *> arguments;

    /**
     * Constructor
     * @param name Name of the call function expression
     * @param arguments Arguments of the call function expression
     * @param line Line for error reporting
     */
    ASTNodeCallFunc(const std::string &name, std::vector<ASTNodeExpression *> arguments, int line) : name(name), arguments(std::move(arguments)), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeCallFunc() override {
        for (auto &argument : arguments) {
            delete argument;
        }
    }

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};

/**
 * Struct representing an AST node for sizeof expression
 */
class ASTNodeSizeof : public ASTNodeExpression {
public:
    /** Line for error reporting */
    int line;
    /** Type to get the size of */
    std::string type;

    /**
     * Constructor
     * @param type Type to get the size of
     * @param line Line for error reporting
     */
    explicit ASTNodeSizeof(const std::string &type, int line) : type(type), line(line) {
        /* Empty */
    }

    /**
     * Destructor
     */
    ~ASTNodeSizeof() override = default;

    /**
    * Accept method for the visitor pattern
    * @param visitor Visitor to accept
    */
    void accept(ASTVisitor *visitor) override {
        visitor->visit(this);
    }
};
