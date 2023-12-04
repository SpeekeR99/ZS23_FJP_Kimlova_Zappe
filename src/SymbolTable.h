#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <ranges>

const std::uint32_t ACTIVATION_RECORD_SIZE = 3;

enum ValueType {
    UNDEFINED_TYPE,
    VOID,
    INTEGER,
    BOOLEAN,
    STRING
};

ValueType str_to_val_type(const std::string &str);

enum ValueTypeSize {
    UNDEFINED_TYPE_SIZE = -1,
    VOID_SIZE = 0,
    INTEGER_SIZE = 1,
    BOOLEAN_SIZE = 1,
    STRING_SIZE = 1
};

int sizeof_val_type(ValueType type);

typedef struct Type {
    ValueType type;
    uint32_t size;
    uint32_t is_pointer = 0;
    bool is_pointing_to_stack = false;

    Type() = default;

    Type(ValueType type, uint32_t is_pointer, bool is_pointing_to_stack) : type(type), is_pointer(is_pointer), is_pointing_to_stack(is_pointing_to_stack) {
        this->size = sizeof_val_type(type);
    }

    bool operator==(const Type &other) const;
} Type;

enum SymbolType {
    VARIABLE,
    FUNCTION
};

typedef struct SymbolTableRecord {
    std::string name;
    SymbolType symbol_type;
    Type type;
    bool is_const;
    std::uint32_t address;
    std::vector<struct SymbolTableRecord> parameters;

    bool operator==(const SymbolTableRecord &other) const;
} SymbolTableRecord;

static Type undefined_t = {UNDEFINED_TYPE, 0, false};
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type undefined_t_ptr = {UNDEFINED_TYPE, pointer_level, is_pointing_to_stack};
static Type void_t = {VOID, 0, false};
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type void_t_ptr = {VOID, pointer_level, is_pointing_to_stack};
static Type int_t = {INTEGER, 0, false};
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type int_t_ptr = {INTEGER, pointer_level, is_pointing_to_stack};
static Type bool_t = {BOOLEAN, 0, false};
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type bool_t_ptr = {BOOLEAN, pointer_level, is_pointing_to_stack};
static Type string_t = {STRING, 0, false};
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type string_t_ptr = {STRING, pointer_level, is_pointing_to_stack};

static std::map<uint32_t, Type> size_representant = {
        {0, void_t},
        {1, int_t},
};

static SymbolTableRecord undefined_record{"", VARIABLE, Type(VOID, false, false), false, 0};

class ScopeSymbolTable {
private:
    std::map<std::string, SymbolTableRecord> table;
    std::uint32_t address_base;
    std::uint32_t address_offset;
    bool is_function_scope;

public:
    ScopeSymbolTable(uint32_t address_base, uint32_t address_offset, bool is_function_scope = false);
    ~ScopeSymbolTable();

    void insert(const std::string &name, SymbolType symbol_type, Type type, bool is_const, uint32_t address = 0);
    bool exists(const std::string &name);
    void remove(const std::string &name);
    SymbolTableRecord &get(const std::string &name);

    [[nodiscard]] std::map<std::string, SymbolTableRecord> &get_table() const;
    [[nodiscard]] std::uint32_t get_address_offset() const;
    [[nodiscard]] std::uint32_t get_address_base() const;
    [[nodiscard]] bool get_is_function_scope() const;

    void set_address_offset(std::uint32_t offset);
    void set_address_base(std::uint32_t base);
};

class SymbolTable {
private:
    std::vector<ScopeSymbolTable> table;
    uint32_t temp_counter = 0;

public:
    static std::vector<std::string> builtin_functions;

    SymbolTable();
    ~SymbolTable();

    void init_builtin_functions();

    void insert_scope(uint32_t address_base, uint32_t address_offset, bool is_function_scope = false);
    void remove_scope();
    void allocate_symbols(uint32_t number_of_symbols, std::vector<uint32_t> size_of_symbols);
    SymbolTableRecord &get_first_empty_symbol(uint32_t size_of_symbol);
    void change_symbol_name(const std::string &old_name, const std::string &new_name);
    void insert_symbol(const std::string &name, SymbolType symbol_type, Type type, bool is_const, uint32_t address = 0);
    void remove_symbol(const std::string &name);

    [[nodiscard]] SymbolTableRecord &get_symbol(const std::string &name);
    [[nodiscard]] uint32_t get_symbol_level(const std::string &name);
    [[nodiscard]] ScopeSymbolTable &get_scope(uint32_t index);
    [[nodiscard]] ScopeSymbolTable &get_scope(const std::string &name);
    [[nodiscard]] uint32_t get_number_of_variables() const;
    [[nodiscard]] uint32_t get_sizeof_variables() const;
    [[nodiscard]] ScopeSymbolTable &get_current_scope();

    friend std::ostream &operator<<(std::ostream &os, const SymbolTable &table);
};
