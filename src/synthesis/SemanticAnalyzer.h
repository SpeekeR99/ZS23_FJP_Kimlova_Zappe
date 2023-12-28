#pragma once

#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"

/**
 * Class for semantic analysis of the AST
 * Inherits from ASTVisitor, so it can traverse the AST
 */
class SemanticAnalyzer : public ASTVisitor {
private:
    /** The global block of the AST, essentially the root of the AST */
    ASTNodeBlock* global_block;
    /** The symbol table */
    SymbolTable symtab;
    /** Map of all declared functions */
    std::map<std::string, bool> declared_functions;
    /** Map of problematic forward referenced functions */
    std::map<std::string, int> problematic_forward_referenced_functions;
    /** Map of all assigned constants */
    std::map<std::string, bool> assigned_constants;
    /** Vector of processed functions (because nested functions are a thing, this needs to be a vector) */
    std::vector<std::pair<std::string, int>> current_functions;
    /** Loop level (for break/continue) */
    int current_loop_level;
    /** Used builtin functions */
    std::vector<std::string> used_builtin_functions;
    /** Declared labels */
    std::vector<std::string> declared_labels;
    /** Used labels */
    std::vector<std::pair<std::string, int>> used_labels;
    /** Map of defined variables */
    std::map<std::string, bool> defined_variables;
    /** Vector of return types (because of nested functions once again) */
    std::vector<Type> return_types;

    /**
     * Register a label as existing
     * @param node The labeled node
     */
    void register_label(ASTNodeStatement *node);
    /**
     * Simple type checking between strings and numeric types
     * @param type The type to check
     * @param expr The expression to check
     * @param line The line number for error reporting
     * @param is_assignment_check Whether this is an assignment type check or return type check
     */
    void check_expr_type(Type type, ASTNodeExpression *expr, int line, bool is_assignment_check = true);

public:
    /**
     * Constructor
     * @param global_block The global block of the AST
     */
    explicit SemanticAnalyzer(ASTNodeBlock* global_block);
    /**
     * Destructor
     */
    ~SemanticAnalyzer() override;

    /**
     * Analyze the AST
     */
    void analyze();

    /**
     * Get the vector of used builtin functions
     * @return The vector of used builtin functions
     */
    std::vector<std::string> get_used_builtin_functions();

    /* Visitor methods */
    void visit(ASTNodeBlock *node) override;
    void visit(ASTNodeDeclVar *node) override;
    void visit(ASTNodeDeclFunc *node) override;
    void visit(ASTNodeIf *node) override;
    void visit(ASTNodeWhile *node) override;
    void visit(ASTNodeFor *node) override;
    void visit(ASTNodeBreakContinue *node) override;
    void visit(ASTNodeReturn *node) override;
    void visit(ASTNodeGoto *node) override;
    void visit(ASTNodeExpressionStatement *node) override;
    void visit(ASTNodeIdentifier *node) override;
    void visit(ASTNodeIntLiteral *node) override;
    void visit(ASTNodeBoolLiteral *node) override;
    void visit(ASTNodeStringLiteral *node) override;
    void visit(ASTNodeFloatLiteral *node) override;
    void visit(ASTNodeAssignExpression *node) override;
    void visit(ASTNodeTernaryOperator *node) override;
    void visit(ASTNodeBinaryOperator *node) override;
    void visit(ASTNodeUnaryOperator *node) override;
    void visit(ASTNodeCast *node) override;
    void visit(ASTNodeCallFunc *node) override;
    void visit(ASTNodeNew *node) override;
    void visit(ASTNodeDelete *node) override;
    void visit(ASTNodeDereference *node) override;
    void visit(ASTNodeReference *node) override;
    void visit(ASTNodeSizeof *node) override;
};