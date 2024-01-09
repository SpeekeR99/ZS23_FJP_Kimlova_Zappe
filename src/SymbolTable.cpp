#include "SymbolTable.h"

std::vector<std::string> SymbolTable::builtin_functions = {
        "print_int",
        "read_int",
        "print_str",
        "read_str",
        "strcmp",
        "strcat",
        "strlen",
        "print_float",
        "read_float",
};

ValueType str_to_val_type(const std::string &str) {
    if (str == "void")
        return VOID;
    else if (str == "int")
        return INTEGER;
    else if (str == "bool")
        return BOOLEAN;
    else if (str == "string")
        return STRING;
    else if (str == "float")
        return FLOAT;
    else {
        return UNDEFINED_TYPE;
    }
}

std::string val_type_to_str(ValueType type) {
    if (type == VOID)
        return "void";
    else if (type == INTEGER)
        return "int";
    else if (type == BOOLEAN)
        return "bool";
    else if (type == STRING)
        return "string";
    else if (type == FLOAT)
        return "float";
    else {
        return "undefined";
    }
}

int sizeof_val_type(ValueType type) {
    if (type == VOID)
        return VOID_SIZE;
    else if (type == INTEGER)
        return INTEGER_SIZE;
    else if (type == BOOLEAN)
        return BOOLEAN_SIZE;
    else if (type == STRING)
        return STRING_SIZE;
    else if (type == FLOAT)
        return FLOAT_SIZE;
    else {
        return UNDEFINED_TYPE_SIZE;
    }
}

bool Type::operator==(const Type &other) const {
    return this->type == other.type && this->size == other.size && this->is_pointer == other.is_pointer;
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

void ScopeSymbolTable::insert(const std::string &name, SymbolType symbol_type, Type type, bool is_const, uint32_t address) {
    if (symbol_type == VARIABLE) {
        this->table[name] = SymbolTableRecord{name, symbol_type, type, is_const, this->address_base + this->address_offset};
        this->address_offset += type.size;
    }
    else
        this->table[name] = SymbolTableRecord{name, symbol_type, type, is_const, address};
}

bool ScopeSymbolTable::exists(const std::string &name) {
    return this->table.find(name) != this->table.end();
}


void ScopeSymbolTable::remove(const std::string &name) {
    auto &symbol = this->get(name);
    this->address_offset -= symbol.type.size;
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
    /* Insert builtin functions into the symbol table */
    this->insert_symbol("print_int", FUNCTION, void_t, false, 0);
    auto &print_num = this->get_symbol("print_int");
    auto print_param = SymbolTableRecord{"__print_int_param__", VARIABLE, int_t, false};
    print_num.parameters.emplace_back(print_param);

    this->insert_symbol("read_int", FUNCTION, int_t, false, 0);

    this->insert_symbol("print_str", FUNCTION, void_t, false, 0);
    auto &print_str = this->get_symbol("print_str");
    print_param = SymbolTableRecord{"__print_str_param__", VARIABLE, string_t, false};
    print_str.parameters.emplace_back(print_param);

    this->insert_symbol("read_str", FUNCTION, string_t, false, 0);

    this->insert_symbol("strcmp", FUNCTION, int_t, false, 0);
    auto &strcmp = this->get_symbol("strcmp");
    print_param = SymbolTableRecord{"__strcmp_param1__", VARIABLE, string_t, false};
    strcmp.parameters.emplace_back(print_param);
    print_param = SymbolTableRecord{"__strcmp_param2__", VARIABLE, string_t, false};
    strcmp.parameters.emplace_back(print_param);

    this->insert_symbol("strcat", FUNCTION, string_t, false, 0);
    auto &strcat = this->get_symbol("strcat");
    print_param = SymbolTableRecord{"__strcat_param1__", VARIABLE, string_t, false};
    strcat.parameters.emplace_back(print_param);
    print_param = SymbolTableRecord{"__strcat_param2__", VARIABLE, string_t, false};
    strcat.parameters.emplace_back(print_param);

    this->insert_symbol("strlen", FUNCTION, int_t, false, 0);
    auto &strlen = this->get_symbol("strlen");
    print_param = SymbolTableRecord{"__strlen_param__", VARIABLE, string_t, false};
    strlen.parameters.emplace_back(print_param);

    this->insert_symbol("print_float", FUNCTION, void_t, false, 0);
    auto &print_float = this->get_symbol("print_float");
    print_param = SymbolTableRecord{"__print_float_param__", VARIABLE, float_t, false};
    print_float.parameters.emplace_back(print_param);

    this->insert_symbol("read_float", FUNCTION, float_t, false, 0);
}

SymbolTable::~SymbolTable() = default;

void SymbolTable::insert_scope(uint32_t address_base, uint32_t address_offset, bool is_function_scope) {
    this->table.emplace_back(address_base, address_offset, is_function_scope);
}

void SymbolTable::remove_scope() {
    this->table.pop_back();
}

void SymbolTable::allocate_symbols(uint32_t number_of_symbols, std::vector<uint32_t> size_of_symbols) {
    for (int i = 0; i < number_of_symbols; i++)
        this->table.back().insert("__TEMP__" + std::to_string(temp_counter++), VARIABLE, size_representant[size_of_symbols[i]], false);
}

SymbolTableRecord &SymbolTable::get_first_empty_symbol(uint32_t size_of_symbol) {
    for (auto &it : std::ranges::reverse_view(this->table)) {
        for (auto & [key, value] : it.get_table()) {
            if (value.name.contains("__TEMP__") && value.type.size == size_of_symbol)
                return value;
        }
    }

    return undefined_record;
}

void SymbolTable::change_symbol_name(const std::string &old_name, const std::string &new_name) {
    auto &record = this->get_symbol(old_name);
    auto &scope = this->get_scope(old_name);

    auto record_deep_copy = SymbolTableRecord{new_name, record.symbol_type, record.type, record.is_const, record.address};

    scope.get_table().erase(old_name);
    scope.get_table()[new_name] = record_deep_copy;
}

void SymbolTable::insert_symbol(const std::string &name, SymbolType symbol_type, Type type, bool is_const, uint32_t address) {
    this->table.back().insert(name, symbol_type, type, is_const, address);
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
            sizeof_variables += value.type.size;
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
            os << "\tValue Type: " << record.second.type.type << std::endl;
            os << "\tSize: " << record.second.type.size << std::endl;
            os << "\tIs Pointer: " << record.second.type.is_pointer << std::endl;
            os << "\tTo stack: " << record.second.type.is_pointing_to_stack << std::endl;
            os << "\tIs Const: " << record.second.is_const << std::endl;
            os << "\tAddress: " << record.second.address << std::endl << std::endl;
        }
    }
    return os;
}
