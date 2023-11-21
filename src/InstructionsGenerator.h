#pragma once

#include <iostream>
#include <vector>
#include <string>

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

class InstructionsGenerator {
private:
    std::vector<Instruction> instructions;
    std::uint32_t instruction_counter;

public:
    InstructionsGenerator();
    ~InstructionsGenerator();

    void generate(const std::string &instruction, int level, int parameter);
    void generate(InstructionIndex instruction, int level, int parameter);

    [[nodiscard]] std::vector<Instruction> &get_instructions();
    [[nodiscard]] Instruction &get_instruction(std::uint32_t index);

    [[nodiscard]] std::uint32_t get_instruction_counter() const;
    void set_instruction_counter(std::uint32_t counter);
};

extern InstructionsGenerator global_instructions_generator;
