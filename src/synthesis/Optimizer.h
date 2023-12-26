#pragma once

#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"
#include "InstructionsGenerator.h"

class Optimizer : public ASTVisitor {
private:

public:
    Optimizer();
    ~Optimizer() override;

    void optimize_ast(ASTNodeBlock* global_block);
    void optimize_instructions(std::vector<Instruction> &instructions);

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