#include "SymbolTable.h"

#include <ranges>

ScopeSymbolTable::ScopeSymbolTable(uint32_t address_base, uint32_t address_offset) : table(), address_base(address_base), address_offset(address_offset) {
    /* Empty */
}

ScopeSymbolTable::~ScopeSymbolTable() = default;

void ScopeSymbolTable::insert(const std::string &name, SymbolType symbol_type, ValueType type) {
    this->table[name] = SymbolTableRecord{name, symbol_type, type, this->address_base + this->address_offset++};
}

bool ScopeSymbolTable::exists(const std::string &name) {
    return this->table.find(name) != this->table.end();
}

SymbolTableRecord ScopeSymbolTable::get(const std::string &name) {
    if (this->exists(name))
        return this->table[name];
    else
        return SymbolTableRecord{"", VARIABLE, VOID, 0};
}

std::map<std::string, SymbolTableRecord> &ScopeSymbolTable::get_table() const {
    return const_cast<std::map<std::string, SymbolTableRecord> &>(this->table);
}

std::uint32_t ScopeSymbolTable::get_address_offset() const {
    return this->address_offset;
}

std::uint32_t ScopeSymbolTable::get_address_base() const {
    return this->address_base;
}

void ScopeSymbolTable::set_address_offset(std::uint32_t offset) {
    this->address_offset = offset;
}

void ScopeSymbolTable::set_address_base(std::uint32_t base) {
    this->address_base = base;
}

SymbolTable::SymbolTable() : table() {
    /* Empty */
}

SymbolTable::~SymbolTable() = default;

void SymbolTable::insert_scope(uint32_t address_base, uint32_t address_offset) {
    this->table.emplace_back(address_base, address_offset);
}

void SymbolTable::remove_scope() {
    this->table.pop_back();
}

void SymbolTable::insert_symbol(const std::string &name, SymbolType symbol_type, ValueType type) {
    this->table.back().insert(name, symbol_type, type);
}

SymbolTableRecord SymbolTable::get_symbol(const std::string &name) {
    for (auto & it : std::ranges::reverse_view(this->table)) {
        if (it.exists(name))
            return it.get(name);
    }
    return SymbolTableRecord{"", VARIABLE, VOID, 0};
}

ScopeSymbolTable &SymbolTable::get_scope(uint32_t index) {
    return this->table[index];
}

std::ostream &operator<<(std::ostream &os, const SymbolTable &table) {
    for (const auto &scope : table.table) {
        os << "Scope: " << scope.get_address_base() << " " << scope.get_address_offset() << std::endl;
        for (const auto &record : scope.get_table()) {
            os << "\t" << record.second.name << " " << record.second.symbol_type << " " << record.second.type << " " << record.second.address << std::endl;
        }
    }
    return os;
}
