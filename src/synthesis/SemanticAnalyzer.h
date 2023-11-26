#pragma once

#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"

class SemanticAnalyzer : public ASTVisitor {
private:
    ASTNodeBlock* global_block;
    SymbolTable symtab;
    std::map<std::string, bool> declared_functions;
    std::map<std::string, bool> assigned_constants;
    std::vector<std::pair<std::string, int>> current_functions;

public:
    explicit SemanticAnalyzer(ASTNodeBlock* global_block);
    ~SemanticAnalyzer() override;

    void analyze();

    void visit(ASTNodeBlock *node) override;
    void visit(ASTNodeDeclVar *node) override;
    void visit(ASTNodeDeclFunc *node) override;
    void visit(ASTNodeIf *node) override;
    void visit(ASTNodeWhile *node) override;
    void visit(ASTNodeFor *node) override;
    void visit(ASTNodeReturn *node) override;
    void visit(ASTNodeExpressionStatement *node) override;
    void visit(ASTNodeIdentifier *node) override;
    void visit(ASTNodeIntLiteral *node) override;
    void visit(ASTNodeAssignExpression *node) override;
    void visit(ASTNodeBinaryOperator *node) override;
    void visit(ASTNodeUnaryOperator *node) override;
    void visit(ASTNodeCast *node) override;
    void visit(ASTNodeCallFunc *node) override;
};