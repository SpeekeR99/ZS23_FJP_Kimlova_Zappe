#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer(ASTNodeBlock *global_block) : global_block(global_block), symtab(),
                                                                 declared_functions(), problematic_forward_referenced_functions(), assigned_constants(),
                                                                 current_functions(), current_loop_level(0), used_builtin_functions(), declared_labels(), used_labels() {
    /* Empty */
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

std::vector<std::string> SemanticAnalyzer::get_used_builtin_functions() {
    return this->used_builtin_functions;
}

void SemanticAnalyzer::register_label(ASTNode *node) {
    if (!node->label.empty()) {
        if (std::find(this->declared_labels.begin(), this->declared_labels.end(), node->label) != this->declared_labels.end()) {
            std::cerr << "Semantic error: label \"" << node->label << "\" already declared, error on line " << node->line << std::endl;
            exit(1);
        }
        this->declared_labels.push_back(node->label);
    }
}

void SemanticAnalyzer::analyze() {
    this->symtab.insert_scope(0, 0); /* No need to care about addressing here */
    this->symtab.init_builtin_functions();
    for (auto &builtin_function: this->symtab.get_current_scope().get_table())
        this->declared_functions[builtin_function.first] = true;

    for (auto &statement: this->global_block->statements)
        statement->accept(this);

    for (auto &used_label : this->used_labels) {
        bool found = std::find(this->declared_labels.begin(), this->declared_labels.end(), used_label.first) != this->declared_labels.end();
        if (!found) {
            std::cerr << "Semantic error: label \"" << used_label.first << "\" not declared, error on line " << used_label.second << std::endl;
            exit(1);
        }
    }

    auto main_func = symtab.get_symbol("main");
    if (main_func == undefined_record) {
        std::cerr << "Semantic error: main function not found" << std::endl;
        exit(1);
    }

    if (main_func.type != int_t) {
        std::cerr << "Semantic error: main function must return integer" << std::endl;
        exit(1);
    }

    if (!problematic_forward_referenced_functions.empty()) {
        for (auto &problematic_forward_referenced_function: problematic_forward_referenced_functions)
            std::cerr << "Semantic error: function \"" << problematic_forward_referenced_function.first << "\" is not defined, error on line " << problematic_forward_referenced_function.second << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeBlock *node) {
    for (auto &statement: node->statements)
        statement->accept(this);

    /* This is kind of naive way of checking if a function contains a return statement */
    if (symtab.get_current_scope().get_is_function_scope()) {
        auto current_function = this->current_functions.back();

        if (node->statements.empty()) {
            std::cerr << "Semantic error: function \"" << current_function.first << "\" does not contain a return statement, error on line " << current_function.second << std::endl;
            exit(1);
        }

        auto last_statement = node->statements.back();
        bool contains_return_statement = false;
        if (dynamic_cast<ASTNodeReturn *>(last_statement))
            contains_return_statement = true;
        else if (auto if_statement = dynamic_cast<ASTNodeIf *>(last_statement))
            contains_return_statement = if_statement->contains_return_statement();

        if (!contains_return_statement) {
            std::cerr << "Semantic error: function \"" << current_function.first << "\" does not contain a return statement, error on line " << current_function.second << std::endl;
            exit(1);
        }
    }
}

void SemanticAnalyzer::visit(ASTNodeDeclVar *node) {
    if (this->symtab.get_current_scope().exists(node->name)) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" already declared in this scope, error on line " << node->line << std::endl;
        exit(1);
    }

    if (node->type == "void") {
        std::cerr << "Semantic error: variable \"" << node->name << "\" cannot be of type void, error on line " << node->line << std::endl;
        exit(1);
    }

    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    Type type{str_to_val_type(node->type), node->is_pointer, false};
    this->symtab.insert_symbol(node->name, VARIABLE, type, node->is_const);
    auto &symbol = this->symtab.get_symbol(node->name);

    if (node->expression) {
        if (dynamic_cast<ASTNodeNew *>(node->expression))
            symbol.type.is_pointing_to_stack = false;
        else
            symbol.type.is_pointing_to_stack = true;

        this->defined_variables[node->name] = true;
        this->assigned_constants[node->name] = true;

        node->expression->accept(this);

        bool is_rvalue_ptr = false;
        if (dynamic_cast<ASTNodeReference *>(node->expression)) {
            is_rvalue_ptr = true;
        }
        else if (dynamic_cast<ASTNodeNew *>(node->expression)) {
            is_rvalue_ptr = true;
        }
        else if (auto binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(node->expression)) {
            if (binary_operator->contains_reference()) {
                is_rvalue_ptr = true;
            }
        }
        else if (auto ternary_operator = dynamic_cast<ASTNodeTernaryOperator *>(node->expression)) {
            if (dynamic_cast<ASTNodeReference *>(ternary_operator->true_expression) || dynamic_cast<ASTNodeReference *>(ternary_operator->false_expression)) {
                is_rvalue_ptr = true;
            } else if (dynamic_cast<ASTNodeNew *>(ternary_operator->true_expression) || dynamic_cast<ASTNodeNew *>(ternary_operator->false_expression)) {
                is_rvalue_ptr = true;
            }
        }

        if (symbol.type.is_pointer && !is_rvalue_ptr) {
            std::cerr << "Semantic error: variable \"" << node->name << "\" is a pointer and must be assigned with a reference or new, error on line " << node->line << std::endl;
            exit(1);
        }
        else if (!symbol.type.is_pointer && is_rvalue_ptr) {
            std::cerr << "Semantic error: variable \"" << node->name << "\" is not a pointer, error on line " << node->line << std::endl;
            exit(1);
        }
    }
    else {
        this->assigned_constants[node->name] = false;
    }

    /* TODO: type checking is pain */
//    if (symbol.type != str_to_val_type("string") && dynamic_cast<ASTNodeStringLiteral *>(node->expression)) {
//        std::cerr << "Semantic error: string literals cannot be assigned to non-string variables, error on line " << node->line << std::endl;
//        exit(1);
//    }
//    else if (symbol.type != str_to_val_type("string") && dynamic_cast<ASTNodeCallFunc *>(node->expression)) {
//        auto *call_func = dynamic_cast<ASTNodeCallFunc *>(node->expression);
//        auto &func_symbol = this->symtab.get_symbol(call_func->name);
//        if (func_symbol.type == str_to_val_type("string")) {
//            std::cerr << "Semantic error: function \"" << call_func->name << "\" returns string, error on line " << node->line << std::endl;
//            exit(1);
//        }
//    }
//    else if (symbol.type == str_to_val_type("string") && symbol.is_pointer == false && !dynamic_cast<ASTNodeStringLiteral *>(node->expression) && !dynamic_cast<ASTNodeCallFunc *>(node->expression)) {
//        std::cerr << "Semantic error: non-string literals cannot be assigned to string variables, error on line " << node->line << std::endl;
//        exit(1);
//    }
//    else if (symbol.type == str_to_val_type("string") && dynamic_cast<ASTNodeCallFunc *>(node->expression)) {
//        auto *call_func = dynamic_cast<ASTNodeCallFunc *>(node->expression);
//        auto &func_symbol = this->symtab.get_symbol(call_func->name);
//        if (func_symbol.type != str_to_val_type("string")) {
//            std::cerr << "Semantic error: function \"" << call_func->name << "\" does not return string, error on line " << node->line << std::endl;
//            exit(1);
//        }
//    }
}

void SemanticAnalyzer::visit(ASTNodeDeclFunc *node) {
    auto func_symbol = this->symtab.get_symbol(node->name);
    if (func_symbol != undefined_record && this->declared_functions[node->name]) {
        std::cerr << "Semantic error: function \"" << node->name << "\" already declared, error on line " << node->line << std::endl;
        exit(1);
    }

    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    if (func_symbol == undefined_record) {
        Type type{str_to_val_type(node->return_type), 0, false};
        this->symtab.insert_symbol(node->name, FUNCTION, type, false, 0);
    }

    if (node->block) {
        this->symtab.insert_scope(0, 0, true); /* No need to care about addressing here */

        for (auto &parameter: node->parameters) {
            auto &decl_func_symbol = this->symtab.get_symbol(node->name);
            parameter->accept(this);
            decl_func_symbol.parameters.push_back(this->symtab.get_symbol(parameter->name));
            defined_variables[parameter->name] = true;
        }

        this->current_functions.emplace_back(node->name, node->line);
        this->declared_functions[node->name] = true;

        if (this->problematic_forward_referenced_functions.find(node->name) != this->problematic_forward_referenced_functions.end())
            this->problematic_forward_referenced_functions.erase(node->name);

        node->block->accept(this);

        this->current_functions.pop_back();

        this->symtab.remove_scope();
    }
    else {
        this->declared_functions[node->name] = false;
    }
}

void SemanticAnalyzer::visit(ASTNodeIf *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */

    node->condition->accept(this);
    node->block->accept(this);

    this->symtab.remove_scope();

    if (node->else_block) {
        this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */
        node->else_block->accept(this);
        this->symtab.remove_scope();
    }
}

void SemanticAnalyzer::visit(ASTNodeWhile *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    this->current_loop_level++;
    this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */

    node->condition->accept(this);
    node->block->accept(this);

    this->symtab.remove_scope();
    this->current_loop_level--;
}

void SemanticAnalyzer::visit(ASTNodeFor *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    this->current_loop_level++;
    this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */

    if (!dynamic_cast<ASTNodeDeclVar *>(node->init) && !dynamic_cast<ASTNodeAssignExpression *>(node->init)) {
        std::cerr << "Semantic error: invalid for loop initialization, error on line " << node->line << std::endl;
        exit(1);
    }

    node->init->accept(this);
    node->condition->accept(this);
    node->block->accept(this);
    node->increment->accept(this);

    this->symtab.remove_scope();
    this->current_loop_level--;
}

void SemanticAnalyzer::visit(ASTNodeBreakContinue *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    if (!this->current_loop_level) {
        std::cerr << "Semantic error: break/continue statement outside of loop, error on line " << node->line << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeReturn *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    if (node->expression)
        node->expression->accept(this);

    /* TODO: type check */
}

void SemanticAnalyzer::visit(ASTNodeGoto *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    this->used_labels.emplace_back(node->label_to_go_to, node->line);
}

void SemanticAnalyzer::visit(ASTNodeExpressionStatement *node) {
    node->label = node->ASTNodeStatement::label;
    this->register_label(node);

    node->expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeIdentifier *node) {
    auto &symbol = this->symtab.get_symbol(node->name);
    if (symbol == undefined_record) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" not declared, error on line " << node->line << std::endl;
        exit(1);
    }
    if (!this->defined_variables[node->name]) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" used before definition, error on line " << node->line << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeIntLiteral *node) {
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeBoolLiteral *node) {
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeStringLiteral *node) {
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeFloatLiteral *node) {
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeAssignExpression *node) {
    auto &symbol = this->symtab.get_symbol(node->name);

    if (node->lvalue) {
        if (!dynamic_cast<ASTNodeDereference *>(node->lvalue)) {
            std::cerr << "Semantic error: lvalue required as left operand of assignment, error on line " << node->line << std::endl;
            exit(1);
        }
        node->lvalue->accept(this);
    }

    node->expression->accept(this);

    this->defined_variables[node->name] = true;

    if (dynamic_cast<ASTNodeAssignExpression *>(node->expression)) { /* TODO: will be supported :) */
        std::cerr << "Multi assignment is not supported, error on line " << node->line << std::endl;
        exit(1);
    }

    if (!node->lvalue) {
        if (symbol == undefined_record) {
            std::cerr << "Semantic error: variable \"" << node->name << "\" not declared, error on line " << node->line << std::endl;
            exit(1);
        }

        if (symbol.is_const && this->assigned_constants[node->name]) {
            std::cerr << "Semantic error: variable \"" << node->name << "\" is constant and can only be assigned once, error on line " << node->line << std::endl;
            exit(1);
        }

        if (symbol.is_const)
            this->assigned_constants[node->name] = true;
    }

    bool is_rvalue_ptr = false;
    if (dynamic_cast<ASTNodeReference *>(node->expression)) {
        is_rvalue_ptr = true;
    }
    else if (dynamic_cast<ASTNodeNew *>(node->expression)) {
        is_rvalue_ptr = true;
    }
    else if (auto binary_operator = dynamic_cast<ASTNodeBinaryOperator *>(node->expression)) {
        if (binary_operator->contains_reference()) {
            is_rvalue_ptr = true;
        }
    }
    else if (auto ternary_operator = dynamic_cast<ASTNodeTernaryOperator *>(node->expression)) {
        if (dynamic_cast<ASTNodeReference *>(ternary_operator->true_expression) || dynamic_cast<ASTNodeReference *>(ternary_operator->false_expression)) {
            is_rvalue_ptr = true;
        } else if (dynamic_cast<ASTNodeNew *>(ternary_operator->true_expression) || dynamic_cast<ASTNodeNew *>(ternary_operator->false_expression)) {
            is_rvalue_ptr = true;
        }
    }

    if (symbol.type.is_pointer && !is_rvalue_ptr) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" is a pointer and must be assigned with a reference or new, error on line " << node->line << std::endl;
        exit(1);
    }
    else if (!symbol.type.is_pointer && is_rvalue_ptr) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" is not a pointer, error on line " << node->line << std::endl;
        exit(1);
    }

    /* TODO: type checking is pain */
//    if (symbol.type != str_to_val_type("string") && dynamic_cast<ASTNodeStringLiteral *>(node->expression)) {
//        std::cerr << "Semantic error: string literals cannot be assigned to non-string variables, error on line " << node->line << std::endl;
//        exit(1);
//    }
//    else if (symbol.type != str_to_val_type("string") && dynamic_cast<ASTNodeCallFunc *>(node->expression)) {
//        auto *call_func = dynamic_cast<ASTNodeCallFunc *>(node->expression);
//        auto &func_symbol = this->symtab.get_symbol(call_func->name);
//        if (func_symbol.type == str_to_val_type("string")) {
//            std::cerr << "Semantic error: function \"" << call_func->name << "\" returns string, error on line " << node->line << std::endl;
//            exit(1);
//        }
//    }
//    else if (symbol.type == str_to_val_type("string") && symbol.is_pointer == false &&  !dynamic_cast<ASTNodeStringLiteral *>(node->expression) && !dynamic_cast<ASTNodeCallFunc *>(node->expression)) {
//        std::cerr << "Semantic error: non-string literals cannot be assigned to string variables, error on line " << node->line << std::endl;
//        exit(1);
//    }
//    else if (symbol.type == str_to_val_type("string") && dynamic_cast<ASTNodeCallFunc *>(node->expression)) {
//        auto *call_func = dynamic_cast<ASTNodeCallFunc *>(node->expression);
//        auto &func_symbol = this->symtab.get_symbol(call_func->name);
//        if (func_symbol.type != str_to_val_type("string")) {
//            std::cerr << "Semantic error: function \"" << call_func->name << "\" does not return string, error on line "
//                      << node->line << std::endl;
//            exit(1);
//        }
//    }
}

void SemanticAnalyzer::visit(ASTNodeTernaryOperator *node) {
    node->condition->accept(this);
    node->true_expression->accept(this);
    node->false_expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeBinaryOperator *node) {
    node->left->accept(this);
    node->right->accept(this);

    if (dynamic_cast<ASTNodeStringLiteral *>(node->left) || dynamic_cast<ASTNodeStringLiteral *>(node->right)) {
        std::cerr << "Semantic error: string literals cannot be used in binary operators, error on line " << node->line << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeUnaryOperator *node) {
    node->expression->accept(this);

    if (dynamic_cast<ASTNodeStringLiteral *>(node->expression)) {
        std::cerr << "Semantic error: string literals cannot be used in unary operators, error on line " << node->line << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeCast *node) { /* TODO: cast is not implemented yet */
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeCallFunc *node) {
    auto &symbol = this->symtab.get_symbol(node->name);

    if (symbol == undefined_record) {
        std::cerr << "Semantic error: function \"" << node->name << "\" not declared, error on line " << node->line << std::endl;
        exit(1);
    }

    if (symbol.symbol_type != FUNCTION) {
        std::cerr << "Semantic error: \"" << node->name << "\" is not a function, error on line " << node->line << std::endl;
        exit(1);
    }

    if (symbol.parameters.size() != node->arguments.size()) {
        std::cerr << "Semantic error: function \"" << node->name << "\" expects " << symbol.parameters.size() << " arguments, " << node->arguments.size() << " given, error on line " << node->line << std::endl;
        exit(1);
    }

    if (!this->declared_functions[node->name])
        problematic_forward_referenced_functions[node->name] = node->line;

    if (std::find(SymbolTable::builtin_functions.begin(), SymbolTable::builtin_functions.end(), node->name) != SymbolTable::builtin_functions.end())
        this->used_builtin_functions.push_back(node->name);
}

void SemanticAnalyzer::visit(ASTNodeNew *node) {
    if (str_to_val_type(node->type) == void_t.type) {
        std::cerr << "Semantic error: cannot allocate void, error on line " << node->line << std::endl;
        exit(1);
    }

    node->expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeDelete *node) {
    node->expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeDereference *node) {
    node->expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeReference *node) {
    auto &symbol = this->symtab.get_symbol(node->identifier);
    if (symbol == undefined_record) {
        std::cerr << "Semantic error: variable \"" << node->identifier << "\" not declared, error on line " << node->line << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeSizeof *node) {
    if (str_to_val_type(node->type) == undefined_t.type) {
        std::cerr << "Semantic error: type \"" << node->type << "\" not declared, error on line " << node->line << std::endl;
        exit(1);
    }
}
