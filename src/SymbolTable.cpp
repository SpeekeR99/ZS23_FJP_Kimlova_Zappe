#include "SymbolTable.h"

SymbolTableRecord undefined_record{"", VARIABLE, VOID, false, 0};

ValueType str_to_val_type(const std::string &str) {
    if (str == "void")
        return VOID;
    else if (str == "int")
        return INTEGER;
    else if (str == "bool")
        return BOOLEAN;
    else {
        std::cout << "Invalid value type: " << str << std::endl;
        throw std::runtime_error("Invalid value type!");
    }
}

ScopeSymbolTable::ScopeSymbolTable(uint32_t address_base, uint32_t address_offset) : table(), address_base(address_base), address_offset(address_offset) {
    /* Empty */
}

ScopeSymbolTable::~ScopeSymbolTable() = default;

void ScopeSymbolTable::insert(const std::string &name, SymbolType symbol_type, ValueType type, bool is_const) {
    this->table[name] = SymbolTableRecord{name, symbol_type, type, is_const, this->address_base + this->address_offset++};
}

void ScopeSymbolTable::insert(const std::string &name, SymbolType symbol_type, const std::string &type, bool is_const) {
    this->insert(name, symbol_type, str_to_val_type(type), is_const);
}

bool ScopeSymbolTable::exists(const std::string &name) {
    return this->table.find(name) != this->table.end();
}

SymbolTableRecord &ScopeSymbolTable::get(const std::string &name) {
    if (this->exists(name))
        return this->table[name];
    else
        return undefined_record;
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

void SymbolTable::insert_symbol(const std::string &name, SymbolType symbol_type, ValueType type, bool is_const) {
    this->table.back().insert(name, symbol_type, type, is_const);
}

void SymbolTable::insert_symbol(const std::string &name, SymbolType symbol_type, const std::string &type, bool is_const) {
    this->insert_symbol(name, symbol_type, str_to_val_type(type), is_const);
}

SymbolTableRecord &SymbolTable::get_symbol(const std::string &name, uint32_t *level) {
    for (auto &it : std::ranges::reverse_view(this->table)) {
        if (it.exists(name))
            return it.get(name);
        (*level)++;
    }
    return undefined_record;
}

ScopeSymbolTable &SymbolTable::get_scope(uint32_t index) {
    return this->table[index];
}

uint32_t SymbolTable::get_number_of_variables() const {
    uint32_t number_of_variables = 0;
    for (const auto & [key, value] : this->table.back().get_table())
        if (value.symbol_type == VARIABLE)
            number_of_variables++;
    return number_of_variables;
}

std::ostream &operator<<(std::ostream &os, const SymbolTable &table) {
    for (const auto &scope : table.table) {
        os << "Scope: " << scope.get_address_base() << " " << scope.get_address_offset() << std::endl;
        for (const auto &record : scope.get_table()) {
            os << "\tName: " << record.second.name << std::endl;
            os << "\tSymbol Type: " << record.second.symbol_type << std::endl;
            os << "\tValue Type: " << record.second.type << std::endl;
            os << "\tIs Const: " << record.second.is_const << std::endl;
            os << "\tAddress: " << record.second.address << std::endl << std::endl;
        }
    }
    return os;
}
