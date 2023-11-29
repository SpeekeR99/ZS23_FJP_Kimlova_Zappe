#include "SymbolTable.h"

std::vector<std::string> SymbolTable::builtin_functions = {
        "print_num",
        "read_num"
};

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

int sizeof_val_type(ValueType type) {
    if (type == VOID)
        return VOID_SIZE;
    else if (type == INTEGER)
        return INTEGER_SIZE;
    else if (type == BOOLEAN)
        return BOOLEAN_SIZE;
    else {
        std::cout << "Invalid value type: " << type << std::endl;
        throw std::runtime_error("Invalid value type!");
    }
}

bool SymbolTableRecord::operator==(const SymbolTableRecord &other) const {
    return this->name == other.name &&
           this->symbol_type == other.symbol_type &&
           this->type == other.type &&
           this->is_const == other.is_const &&
           this->address == other.address;
}

ScopeSymbolTable::ScopeSymbolTable(uint32_t address_base, uint32_t address_offset, bool is_function_scope) : table(), address_base(address_base), address_offset(address_offset), is_function_scope(is_function_scope) {
    /* Empty */
}

ScopeSymbolTable::~ScopeSymbolTable() = default;

void ScopeSymbolTable::insert(const std::string &name, SymbolType symbol_type, ValueType type, bool is_const, uint32_t address) {
    if (symbol_type == VARIABLE) {
        this->table[name] = SymbolTableRecord{name, symbol_type, type, is_const, this->address_base + this->address_offset};
        this->address_offset += sizeof_val_type(type);
    }
    else
        this->table[name] = SymbolTableRecord{name, symbol_type, type, is_const, address};
}

void ScopeSymbolTable::insert(const std::string &name, SymbolType symbol_type, const std::string &type, bool is_const, uint32_t address) {
    this->insert(name, symbol_type, str_to_val_type(type), is_const, address);
}

bool ScopeSymbolTable::exists(const std::string &name) {
    return this->table.find(name) != this->table.end();
}


void ScopeSymbolTable::remove(const std::string &name) {
    auto &symbol = this->get(name);
    this->address_offset -= sizeof_val_type(symbol.type);
    this->table.erase(name);
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

bool ScopeSymbolTable::get_is_function_scope() const {
    return this->is_function_scope;
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

void SymbolTable::init_builtin_functions() {
    this->insert_symbol("print_num", FUNCTION, "void", false, 0);
    auto &print_num = this->get_symbol("print_num");
    auto print_param = SymbolTableRecord{"__print_num_param__", VARIABLE, str_to_val_type("int"), false};
    print_num.parameters.emplace_back(print_param);
    this->insert_symbol("read_num", FUNCTION, "int", false, 0);
}

SymbolTable::~SymbolTable() = default;

void SymbolTable::insert_scope(uint32_t address_base, uint32_t address_offset, bool is_function_scope) {
    this->table.emplace_back(address_base, address_offset, is_function_scope);
}

void SymbolTable::remove_scope() {
    this->table.pop_back();
}

void SymbolTable::insert_symbol(const std::string &name, SymbolType symbol_type, ValueType type, bool is_const, uint32_t address) {
    this->table.back().insert(name, symbol_type, type, is_const, address);
}

void SymbolTable::insert_symbol(const std::string &name, SymbolType symbol_type, const std::string &type, bool is_const, uint32_t address) {
    this->insert_symbol(name, symbol_type, str_to_val_type(type), is_const, address);
}

void SymbolTable::change_symbol_name(const std::string &old_name, const std::string &new_name) {
    auto &record = this->get_symbol(old_name);
    auto &scope = this->get_scope(old_name);

    auto record_deep_copy = SymbolTableRecord{new_name, record.symbol_type, record.type, record.is_const, record.address};

    scope.get_table().erase(old_name);
    scope.get_table()[new_name] = record_deep_copy;
}

void SymbolTable::remove_symbol(const std::string &name) {
    auto &scope = this->get_scope(name);
    scope.remove(name);
}

SymbolTableRecord &SymbolTable::get_symbol(const std::string &name) {
    for (auto &it : std::ranges::reverse_view(this->table)) {
        if (it.exists(name))
            return it.get(name);
    }
    return undefined_record;
}

uint32_t SymbolTable::get_symbol_level(const std::string &name) {
    uint32_t level = 0;
    ScopeSymbolTable *last_scope;
    for (auto &it : std::ranges::reverse_view(this->table)) {
        if (it.exists(name))
            break;
        last_scope = &it;
        if (last_scope && last_scope->get_is_function_scope())
            level++;
    }
    return level;
}

ScopeSymbolTable &SymbolTable::get_scope(uint32_t index) {
    return this->table[index];
}

ScopeSymbolTable &SymbolTable::get_scope(const std::string &name) {
    for (auto &it : std::ranges::reverse_view(this->table)) {
        if (it.exists(name))
            return it;
    }
    throw std::runtime_error("Scope not found!");
}

uint32_t SymbolTable::get_number_of_variables() const {
    uint32_t number_of_variables = 0;
    for (const auto & [key, value] : this->table.back().get_table())
        if (value.symbol_type == VARIABLE)
            number_of_variables++;
    return number_of_variables;
}

uint32_t SymbolTable::get_sizeof_variables() const {
    uint32_t sizeof_variables = 0;
    for (const auto & [key, value] : this->table.back().get_table())
        if (value.symbol_type == VARIABLE)
            sizeof_variables += sizeof_val_type(value.type);
    return sizeof_variables;
}

ScopeSymbolTable &SymbolTable::get_current_scope() {
    return this->table.back();
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
