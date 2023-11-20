#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>

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
    std::uint32_t address;
} SymbolTableRecord;

class ScopeSymbolTable {
private:
    std::map<std::string, SymbolTableRecord> table;
    std::uint32_t address_base;
    std::uint32_t address_offset;

public:
    ScopeSymbolTable(uint32_t address_base, uint32_t address_offset);
    ~ScopeSymbolTable();

    void insert(const std::string &name, SymbolType symbol_type, ValueType type);
    bool exists(const std::string &name);
    SymbolTableRecord get(const std::string &name);

    [[nodiscard]] std::map<std::string, SymbolTableRecord> &get_table() const;
    [[nodiscard]] std::uint32_t get_address_offset() const;
    [[nodiscard]] std::uint32_t get_address_base() const;

    void set_address_offset(std::uint32_t offset);
    void set_address_base(std::uint32_t base);
};

class SymbolTable {
private:
    std::vector<ScopeSymbolTable> table;

public:
    SymbolTable();
    ~SymbolTable();

    void insert_scope(uint32_t address_base, uint32_t address_offset);
    void remove_scope();
    void insert_symbol(const std::string &name, SymbolType symbol_type, ValueType type);
    SymbolTableRecord get_symbol(const std::string &name);
    ScopeSymbolTable &get_scope(uint32_t index);

    friend std::ostream &operator<<(std::ostream &os, const SymbolTable &table);
};

extern SymbolTable global_symbol_table;
