#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <ranges>

enum ValueType {
    VOID,
    INTEGER,
    BOOLEAN
};

enum SymbolType {
    VARIABLE,
    FUNCTION
};

typedef struct SymbolTableRecord {
    std::string name;
    SymbolType symbol_type;
    ValueType type;
    bool is_const;
    std::uint32_t address;
} SymbolTableRecord;

class ScopeSymbolTable {
private:
    std::map<std::string, SymbolTableRecord> table;
    std::uint32_t address_base;
    std::uint32_t address_offset;
    bool is_function_scope;

public:
    ScopeSymbolTable(uint32_t address_base, uint32_t address_offset, bool is_function_scope = false);
    ~ScopeSymbolTable();

    void insert(const std::string &name, SymbolType symbol_type, ValueType type, bool is_const, uint32_t address = 0);
    void insert(const std::string &name, SymbolType symbol_type, const std::string &type, bool is_const, uint32_t address = 0);
    bool exists(const std::string &name);
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

public:
    SymbolTable();
    ~SymbolTable();

    void insert_scope(uint32_t address_base, uint32_t address_offset, bool is_function_scope = false);
    void remove_scope();
    void insert_symbol(const std::string &name, SymbolType symbol_type, ValueType type, bool is_const, uint32_t address = 0);
    void insert_symbol(const std::string &name, SymbolType symbol_type, const std::string &type, bool is_const, uint32_t address = 0);
    [[nodiscard]] SymbolTableRecord &get_symbol(const std::string &name);
    [[nodiscard]] uint32_t get_symbol_level(const std::string &name);
    [[nodiscard]] ScopeSymbolTable &get_scope(uint32_t index);
    [[nodiscard]] uint32_t get_number_of_variables() const;

    friend std::ostream &operator<<(std::ostream &os, const SymbolTable &table);
};

extern SymbolTable global_symbol_table;
