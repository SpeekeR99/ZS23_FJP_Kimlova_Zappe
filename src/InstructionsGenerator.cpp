#include "InstructionsGenerator.h"

InstructionsGenerator::InstructionsGenerator() : instructions(), instruction_counter(0) {
    /* Empty */
}

InstructionsGenerator::~InstructionsGenerator() = default;

void InstructionsGenerator::generate(const std::string &instruction, int level, int parameter) {
    this->instructions.push_back(Instruction{this->instruction_counter++, instruction, level, parameter});
}

void InstructionsGenerator::generate(InstructionIndex instruction, int level, int parameter) {
    this->generate(InstructionsTable[instruction], level, parameter);
}

std::vector<Instruction> &InstructionsGenerator::get_instructions() {
    return this->instructions;
}

Instruction &InstructionsGenerator::get_instruction(std::uint32_t index) {
    return this->instructions[index];
}

std::uint32_t InstructionsGenerator::get_instruction_counter() const {
    return this->instruction_counter;
}

void InstructionsGenerator::set_instruction_counter(std::uint32_t counter) {
    this->instruction_counter = counter;
}
