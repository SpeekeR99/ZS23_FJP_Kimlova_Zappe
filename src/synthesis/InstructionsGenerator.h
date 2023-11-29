#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"

enum InstructionIndex {
    PL0_LIT = 0,
    PL0_OPR,
    PL0_LOD,
    PL0_STO,
    PL0_CAL,
    PL0_INT,
    PL0_JMP,
    PL0_JMC,
    PL0_RET,
    PL0_REA,
    PL0_WRI,
    PL0_NEW,
    PL0_DEL,
    PL0_LDA,
    PL0_STA,
    PL0_PLD,
    PL0_PST,
    PL0_NUM_OF_INSTRUCTIONS
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
    [PL0_LIT] = "LIT",
    [PL0_OPR] = "OPR",
    [PL0_LOD] = "LOD",
    [PL0_STO] = "STO",
    [PL0_CAL] = "CAL",
    [PL0_INT] = "INT",
    [PL0_JMP] = "JMP",
    [PL0_JMC] = "JMC",
    [PL0_RET] = "RET",
    [PL0_REA] = "REA",
    [PL0_WRI] = "WRI",
    [PL0_NEW] = "NEW",
    [PL0_DEL] = "DEL",
    [PL0_LDA] = "LDA",
    [PL0_STA] = "STA",
    [PL0_PLD] = "PLD",
    [PL0_PST] = "PST"
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
    std::vector<std::string> used_builtin_functions;
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

    void init_builtin_functions();
    void gen_print_num();
    void gen_read_num();

    [[nodiscard]] Instruction &get_instruction(std::uint32_t index);

    [[nodiscard]] std::uint32_t get_instruction_counter() const;
    void set_instruction_counter(std::uint32_t counter);

public:
    InstructionsGenerator(ASTNodeBlock* global_block, std::vector<std::string> &used_builtin_functions);
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
    void visit(ASTNodeNew *node) override;
    void visit(ASTNodeDelete *node) override;
    void visit(ASTNodeDereference *node) override;
    void visit(ASTNodeReference *node) override;
    void visit(ASTNodeDynamicAssignExpression *node) override;
};
