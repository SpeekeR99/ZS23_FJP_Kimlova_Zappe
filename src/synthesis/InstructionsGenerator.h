#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "AbstractSyntaxTree.h"
#include "SymbolTable.h"

/**
 * Enum for instructions
 */
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
    PL0_ITR,
    PL0_RTI,
    PL0_OPF,
    PL0_NUM_OF_INSTRUCTIONS /* This is fine trick, but it is unused in this project :( */
};

/**
 * Enum for different OPR parameters
 */
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

/**
 * Map for operators and their OPR parameters
 */
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

/**
 * Map for instructions and their names
 */
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
    [PL0_PST] = "PST",
    [PL0_ITR] = "ITR",
    [PL0_RTI] = "RTI",
    [PL0_OPF] = "OPF"
};

/**
 * Struct for instruction
 */
typedef struct Instruction {
    /** Line number */
    uint32_t line;
    /** Instruction */
    std::string instruction;
    /** Level */
    int level;
    /** Parameter */
    int parameter;
} Instruction;

/**
 * Class for instructions generation (PL/0 instructions)
 * Inherits from ASTVisitor to traverse the AST
 */
class InstructionsGenerator : public ASTVisitor {
private:
    /** Root of the AST */
    ASTNodeBlock* global_block;
    /** Used builtin functions */
    std::vector<std::string> used_builtin_functions;
    /** Instructions */
    std::vector<Instruction> instructions;
    /** Instruction counter */
    uint32_t instruction_counter;
    /** Symbol table */
    SymbolTable symtab;
    /** Map for declared functions */
    std::map<std::string, int> declared_functions;
    /** Stack for break statements */
    std::vector<uint32_t> break_stack;
    /** Stack for continue statements */
    std::vector<uint32_t> continue_stack;
    /** Stack for parameters sizes */
    std::vector<uint32_t> sizeof_params_stack;
    /** Stack for return type sizes */
    std::vector<uint32_t> sizeof_return_type_stack;
    /** Stack for arguments sizes */
    std::vector<uint32_t> sizeof_arguments_stack;
    /** Map for labels and their line numbers */
    std::map<std::string, uint32_t> labels_to_line;
    /** Map for goto instructions and their line numbers */
    std::map<std::string, uint32_t> goto_labels_line;
    /** Array of strings flag */
    bool is_array_of_strings = false;

    /**
     * Generate instruction
     * @param instruction String instruction
     * @param level Level
     * @param parameter Parameter
     */
    void generate(const std::string &instruction, int level, int parameter);
    /**
     * Generate instruction
     * @param instruction InstructionIndex instruction
     * @param level Level
     * @param parameter Parameter
     */
    void generate(InstructionIndex instruction, int level, int parameter);

    /**
     * Register label
     * @param node AST node with label to be registered
     */
    void register_label(ASTNodeStatement *node);

    /**
     * Generate instructions for builtin functions
     */
    void init_builtin_functions();
    /* Builtin functions */
    void gen_print_int();
    void gen_read_int();
    void gen_print_string();
    void gen_read_string();
    void gen_strcmp();
    void gen_strcat();
    void gen_strlen();
    void gen_print_float();
    void gen_read_float();

    /**
     * Get instruction
     * @param index Index of the instruction
     * @return Instruction
     */
    [[nodiscard]] Instruction &get_instruction(std::uint32_t index);
    /**
     * Get instruction counter
     * @return Instruction counter
     */
    [[nodiscard]] std::uint32_t get_instruction_counter() const;
    /**
     * Set instruction counter
     * @param counter Instruction counter
     */
    void set_instruction_counter(std::uint32_t counter);

public:
    /**
     * Constructor
     * @param global_block Root of the AST
     * @param used_builtin_functions Used builtin functions
     */
    InstructionsGenerator(ASTNodeBlock* global_block, std::vector<std::string> &used_builtin_functions);
    /**
     * Destructor
     */
    ~InstructionsGenerator() override;

    /**
     * Generate instructions
     */
    void generate();
    /**
     * Get instructions
     * @return Instructions
     */
    [[nodiscard]] std::vector<Instruction> &get_instructions();

    /* ASTVisitor methods */
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
