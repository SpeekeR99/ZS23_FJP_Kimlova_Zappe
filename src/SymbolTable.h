#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <ranges>

/** Activation record size */
const std::uint32_t ACTIVATION_RECORD_SIZE = 3;

/**
 * Enum representing all possible types of values
 */
enum ValueType {
    UNDEFINED_TYPE,
    VOID,
    INTEGER,
    BOOLEAN,
    STRING,
    FLOAT
};

/**
 * Converts string to ValueType
 * @param str string to convert
 * @return ValueType
 */
ValueType str_to_val_type(const std::string &str);

/**
 * Converts ValueType to string
 * @param type ValueType to convert
 * @return string
 */
std::string val_type_to_str(ValueType type);

/**
 * Enum representing size of value types
 */
enum ValueTypeSize {
    UNDEFINED_TYPE_SIZE = -1,
    VOID_SIZE = 0,
    INTEGER_SIZE = 1,
    BOOLEAN_SIZE = 1,
    STRING_SIZE = 1,
    FLOAT_SIZE = 2
};

/**
 * Returns size of ValueType
 * @param type ValueType
 * @return size of ValueType
 */
int sizeof_val_type(ValueType type);

/**
 * Structure representing a complete Type for symbol table
 * Type consists of ValueType, size, pointer_level and pointing_to_stack flag
 */
typedef struct Type {
    /** Type of value */
    ValueType type;
    /** Size of type */
    uint32_t size;
    /** Pointer level */
    uint32_t is_pointer = 0;
    /** Flag if type is pointing to stack or heap */
    bool is_pointing_to_stack = false;

    /**
     * Default constructor
     */
    Type() = default;

    /**
     * Constructor
     * @param type ValueType
     * @param is_pointer pointer level
     * @param is_pointing_to_stack flag if type is pointing to stack or heap
     */
    Type(ValueType type, uint32_t is_pointer, bool is_pointing_to_stack) : type(type), is_pointer(is_pointer), is_pointing_to_stack(is_pointing_to_stack) {
        this->size = sizeof_val_type(type);
    }

    /**
     * Overloaded operator ==
     * @param other Type to compare with
     * @return True if types are equal; False otherwise
     */
    bool operator==(const Type &other) const;
} Type;

/**
 * Enum representing all possible types of symbols (variables, functions)
 */
enum SymbolType {
    VARIABLE,
    FUNCTION
};

/**
 * Structure representing a record in symbol table
 * Record consists of name, symbol_type, type, is_const, address, parameters and pointee
 */
typedef struct SymbolTableRecord {
    /** Name of symbol */
    std::string name;
    /** Type of symbol */
    SymbolType symbol_type;
    /** Type of value */
    Type type;
    /** Flag if symbol is constant */
    bool is_const;
    /** Address of symbol */
    std::uint32_t address;
    /** Parameters of function */
    std::vector<struct SymbolTableRecord> parameters;
    /** Pointee of symbol */
    struct SymbolTableRecord *pointee = nullptr;

    /**
     * Overloaded operator ==
     * @param other SymbolTableRecord to compare with
     * @return True if records are equal; False otherwise
     */
    bool operator==(const SymbolTableRecord &other) const;
} SymbolTableRecord;

/** Type representing undefined type */
static Type undefined_t = {UNDEFINED_TYPE, 0, false};
/** Type representing undefined pointer type  */
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type undefined_t_ptr = {UNDEFINED_TYPE, pointer_level, is_pointing_to_stack};

/** Type representing void type */
static Type void_t = {VOID, 0, false};
/** Type representing void pointer type */
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type void_t_ptr = {VOID, pointer_level, is_pointing_to_stack};

/** Type representing int type */
static Type int_t = {INTEGER, 0, false};
/** Type representing int pointer type */
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type int_t_ptr = {INTEGER, pointer_level, is_pointing_to_stack};

/** Type representing bool type */
static Type bool_t = {BOOLEAN, 0, false};
/** Type representing bool pointer type */
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type bool_t_ptr = {BOOLEAN, pointer_level, is_pointing_to_stack};

/** Type representing string type */
static Type string_t = {STRING, 0, false};
/** Type representing string pointer type */
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type string_t_ptr = {STRING, pointer_level, is_pointing_to_stack};

/** Type representing float type */
static Type float_t = {FLOAT, 0, false};
/** Type representing float pointer type */
template<uint32_t pointer_level, bool is_pointing_to_stack = false>
static Type float_t_ptr = {FLOAT, pointer_level, is_pointing_to_stack};

/** Map of size representants for each known possible value size */
static std::map<uint32_t, Type> size_representant = {
        {0, void_t},
        {1, int_t},
        {2, float_t},
};

/** Record representing undefined record */
static SymbolTableRecord undefined_record{"", VARIABLE, Type(VOID, false, false), false, 0};

/**
 * Class representing a scope in symbol table
 */
class ScopeSymbolTable {
private:
    /** Map of symbols in scope */
    std::map<std::string, SymbolTableRecord> table;
    /** Address base of scope */
    std::uint32_t address_base;
    /** Address offset of scope */
    std::uint32_t address_offset;
    /** Flag if scope is function scope */
    bool is_function_scope;

public:
    /**
     * Constructor
     * @param address_base Address base of scope
     * @param address_offset Address offset of scope
     * @param is_function_scope Flag if scope is function scope
     */
    ScopeSymbolTable(uint32_t address_base, uint32_t address_offset, bool is_function_scope = false);
    /**
     * Destructor
     */
    ~ScopeSymbolTable();

    /**
     * Inserts symbol into scope
     * @param name Name of symbol
     * @param symbol_type Type of symbol
     * @param type Type of value
     * @param is_const Flag if symbol is constant
     * @param address Address of symbol
     */
    void insert(const std::string &name, SymbolType symbol_type, Type type, bool is_const, uint32_t address = 0);
    /**
     * Checks if symbol exists in scope
     * @param name Name of symbol
     * @return True if symbol exists; False otherwise
     */
    bool exists(const std::string &name);
    /**
     * Removes symbol from scope
     * @param name Name of symbol
     */
    void remove(const std::string &name);
    /**
     * Returns symbol from scope
     * @param name Name of symbol
     * @return SymbolTableRecord
     */
    SymbolTableRecord &get(const std::string &name);

    /**
     * Returns table of symbols in scope
     * @return Map of symbols in scope
     */
    [[nodiscard]] std::map<std::string, SymbolTableRecord> &get_table() const;
    /**
     * Returns address offset of scope
     * @return Address offset of scope
     */
    [[nodiscard]] std::uint32_t get_address_offset() const;
    /**
     * Returns address base of scope
     * @return Address base of scope
     */
    [[nodiscard]] std::uint32_t get_address_base() const;
    /**
     * Returns flag if scope is function scope
     * @return True if scope is function scope; False otherwise
     */
    [[nodiscard]] bool get_is_function_scope() const;

    /**
     * Sets address offset of scope
     * @param offset Address offset of scope
     */
    void set_address_offset(std::uint32_t offset);
    /**
     * Sets address base of scope
     * @param base Address base of scope
     */
    void set_address_base(std::uint32_t base);
};

/**
 * Class representing a symbol table
 * Symbol table is basically a vector of scopes
 */
class SymbolTable {
private:
    /** Vector of scopes */
    std::vector<ScopeSymbolTable> table;
    /** Counter for unique temporary symbol names */
    uint32_t temp_counter = 0;

public:
    /** Vector of builtin functions */
    static std::vector<std::string> builtin_functions;

    /**
     * Constructor
     */
    SymbolTable();
    /**
     * Destructor
     */
    ~SymbolTable();

    /**
     * Initializes builtin functions in symbol table
     */
    void init_builtin_functions();

    /**
     * Inserts scope into symbol table
     * @param address_base Address base of scope
     * @param address_offset Address offset of scope
     * @param is_function_scope Flag if scope is function scope
     */
    void insert_scope(uint32_t address_base, uint32_t address_offset, bool is_function_scope = false);
    /**
     * Removes scope from symbol table
     */
    void remove_scope();
    /**
     * Allocates symbols in scope (creates temporary symbols)
     * @param number_of_symbols Number of symbols to allocate
     * @param size_of_symbols Vector of sizes of symbols to allocate
     */
    void allocate_symbols(uint32_t number_of_symbols, std::vector<uint32_t> size_of_symbols);
    /**
     * Returns first temporary symbol in scope
     * @param size_of_symbol Size of symbol
     * @return SymbolTableRecord of first temporary symbol in scope with given size
     */
    SymbolTableRecord &get_first_empty_symbol(uint32_t size_of_symbol);
    /**
     * Changes name of symbol
     * @param old_name Old name of symbol
     * @param new_name New name of symbol
     */
    void change_symbol_name(const std::string &old_name, const std::string &new_name);
    /**
     * Inserts symbol into symbol table
     * @param name Name of symbol
     * @param symbol_type Type of symbol
     * @param type Type of value
     * @param is_const Flag if symbol is constant
     * @param address Address of symbol
     */
    void insert_symbol(const std::string &name, SymbolType symbol_type, Type type, bool is_const, uint32_t address = 0);
    /**
     * Removes symbol from symbol table
     * @param name Name of symbol
     */
    void remove_symbol(const std::string &name);

    /**
     * Returns symbol from symbol table
     * @param name Name of symbol
     * @return SymbolTableRecord
     */
    [[nodiscard]] SymbolTableRecord &get_symbol(const std::string &name);
    /**
     * Returns relative scope level of symbol
     * @param name Name of symbol
     * @return Relative scope level of symbol
     */
    [[nodiscard]] uint32_t get_symbol_level(const std::string &name);
    /**
     * Returns scope of symbol
     * @param index Index of scope
     * @return ScopeSymbolTable
     */
    [[nodiscard]] ScopeSymbolTable &get_scope(uint32_t index);
    /**
     * Returns scope of symbol
     * @param name Name of symbol
     * @return ScopeSymbolTable
     */
    [[nodiscard]] ScopeSymbolTable &get_scope(const std::string &name);
    /**
     * Gets number of variables in current scoep
     * @return Number of variables in current scope
     */
    [[nodiscard]] uint32_t get_number_of_variables() const;
    /**
     * Gets size of variables in current scope
     * @return Size of variables in current scope
     */
    [[nodiscard]] uint32_t get_sizeof_variables() const;
    /**
     * Gets the current scope
     * @return Current scope
     */
    [[nodiscard]] ScopeSymbolTable &get_current_scope();

    /**
     * Overloaded operator <<
     * @param os Output stream
     * @param table SymbolTable to print
     * @return Output stream
     */
    friend std::ostream &operator<<(std::ostream &os, const SymbolTable &table);
};
