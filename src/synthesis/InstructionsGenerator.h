#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"

enum InstructionIndex {
    LIT = 0,
    OPR,
    LOD,
    STO,
    CAL,
    INT,
    JMP,
    JMC,
    RET,
    NUM_OF_INSTRUCTIONS
};

enum Oprs {
    PL0_NEG = 1,
    PL0_ADD,
    PL0_SUB,
    PL0_MUL,
    PL0_DIV,
    PL0_MOD,
    PL0_ODD,
    PL0_EQ,
    PL0_NEQ,
    PL0_LT,
    PL0_GEQ,
    PL0_GRT,
    PL0_LEQ
};

static const std::map<std::string, Oprs> OperatorsTable = {
    {"+", PL0_ADD},
    {"-", PL0_SUB},
    {"*", PL0_MUL},
    {"/", PL0_DIV},
    {"%", PL0_MOD},
    {"==", PL0_EQ},
    {"!=", PL0_NEQ},
    {"<", PL0_LT},
    {">=", PL0_GEQ},
    {">", PL0_GRT},
    {"<=", PL0_LEQ}
};

static const char * const InstructionsTable[] = {
    [LIT] = "LIT",
    [OPR] = "OPR",
    [LOD] = "LOD",
    [STO] = "STO",
    [CAL] = "CAL",
    [INT] = "INT",
    [JMP] = "JMP",
    [JMC] = "JMC",
    [RET] = "RET"
};

typedef struct Instruction {
    uint32_t line;
    std::string instruction;
    int level;
    int parameter;
} Instruction;

class InstructionsGenerator : public ASTVisitor {
private:
    ASTNodeBlock* global_block;
    std::vector<Instruction> instructions;
    std::uint32_t instruction_counter;
    SymbolTable symtab;
    std::vector<int> number_of_declared_variables;
    std::map<std::string, int> declared_functions;
    std::vector<uint32_t> break_stack;
    std::vector<uint32_t> continue_stack;
    std::vector<uint32_t> sizeof_params_stack;

    void generate(const std::string &instruction, int level, int parameter);
    void generate(InstructionIndex instruction, int level, int parameter);

    [[nodiscard]] Instruction &get_instruction(std::uint32_t index);

    [[nodiscard]] std::uint32_t get_instruction_counter() const;
    void set_instruction_counter(std::uint32_t counter);

public:
    explicit InstructionsGenerator(ASTNodeBlock* global_block);
    ~InstructionsGenerator() override;

    void generate();
    [[nodiscard]] std::vector<Instruction> &get_instructions();

    void visit(ASTNodeBlock *node) override;
    void visit(ASTNodeDeclVar *node) override;
    void visit(ASTNodeDeclFunc *node) override;
    void visit(ASTNodeIf *node) override;
    void visit(ASTNodeWhile *node) override;
    void visit(ASTNodeFor *node) override;
    void visit(ASTNodeBreakContinue *node) override;
    void visit(ASTNodeReturn *node) override;
    void visit(ASTNodeExpressionStatement *node) override;
    void visit(ASTNodeIdentifier *node) override;
    void visit(ASTNodeIntLiteral *node) override;
    void visit(ASTNodeBoolLiteral *node) override;
    void visit(ASTNodeAssignExpression *node) override;
    void visit(ASTNodeBinaryOperator *node) override;
    void visit(ASTNodeUnaryOperator *node) override;
    void visit(ASTNodeCast *node) override;
    void visit(ASTNodeCallFunc *node) override;
};
