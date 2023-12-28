#pragma once

#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"
#include "InstructionsGenerator.h"

/**
 * Class for optimizations
 * Optimizations are done on the AST and on the generated instructions
 * Inherits from ASTVisitor, so it can traverse the AST
 */
class Optimizer : public ASTVisitor {
private:

    /**
     * Modifies the AST by replacing the node with the new child and updating the corresponding pointers
     * @param node Node to replace
     * @param parent Parent of the node
     * @param new_child New child of the node
     */
    void modify_ast_parent_child(ASTNode *node, ASTNode *parent, ASTNodeExpression *new_child);
    /**
     * Checks if the binary operator contains the given node with the given value and replaces it if the
     * conditions are met (uses modify_ast_parent_child)
     * @tparam T Type of the node to check
     * @param node Binary operator node
     * @param node_to_check Node to check
     * @param value_to_check Value to check
     * @param node_to_set Node to set if the conditions are met
     */
    template<typename T>
    void binary_op_check_case(ASTNodeBinaryOperator *node, ASTNode *node_to_check, int value_to_check, ASTNodeExpression *node_to_set);

public:
    /**
     * Constructor
     */
    Optimizer();
    /**
     * Destructor
     */
    ~Optimizer() override;

    /**
     * Optimize the AST
     * @param global_block Root of the AST
     */
    void optimize_ast(ASTNodeBlock* global_block);
    /**
     * Optimize the instructions
     * @param instructions Instructions to optimize
     */
    void optimize_instructions(std::vector<Instruction> &instructions);

    /* Visit methods */
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
