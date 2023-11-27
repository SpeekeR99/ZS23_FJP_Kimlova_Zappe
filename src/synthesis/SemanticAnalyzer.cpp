#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer(ASTNodeBlock *global_block) : global_block(global_block), symtab() {
    /* Empty */
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::analyze() {
    this->symtab.insert_scope(0, 0); /* No need to care about addressing here */

    for (auto &statement: this->global_block->statements)
        statement->accept(this);

    auto main_func = symtab.get_symbol("main");
    if (main_func == undefined_record) {
        std::cerr << "Semantic error: main function not found" << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeBlock *node) {
    for (auto &statement: node->statements)
        statement->accept(this);

    /* TODO: very naive way of checking for return statement, needs to be improved */
//    if (symtab.get_current_scope().get_is_function_scope()) {
//        auto current_function = this->current_functions.back();
//
//        if (node->statements.empty()) {
//            std::cerr << "Semantic error: function \"" << current_function.first << "\" does not contain a return statement, error on line " << current_function.second << std::endl;
//            exit(1);
//        }
//
//        auto last_statement = node->statements.back();
//
//        if (dynamic_cast<ASTNodeReturn *>(last_statement) == nullptr) {
//            std::cerr << "Semantic error: function \"" << current_function.first << "\" does not contain a return statement, error on line " << current_function.second << std::endl;
//            exit(1);
//        }
//    }

    this->symtab.remove_scope();
}

void SemanticAnalyzer::visit(ASTNodeDeclVar *node) {
    if (this->symtab.get_current_scope().exists(node->name)) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" already declared in this scope, error on line " << node->line << std::endl;
        exit(1);
    }

    this->symtab.insert_symbol(node->name, VARIABLE, node->type, node->is_const);
    if (node->expression) {
        this->assigned_constants[node->name] = true;
        node->expression->accept(this);
    }
    else
        this->assigned_constants[node->name] = false;
}

void SemanticAnalyzer::visit(ASTNodeDeclFunc *node) {
    auto func_symbol = this->symtab.get_symbol(node->name);
    if (func_symbol != undefined_record && this->declared_functions[node->name]) {
        std::cerr << "Semantic error: function \"" << node->name << "\" already declared, error on line " << node->line << std::endl;
        exit(1);
    }

    if (func_symbol == undefined_record)
        this->symtab.insert_symbol(node->name, FUNCTION, node->return_type, false, 0);

    if (node->block) {
        this->symtab.insert_scope(0, 0, true); /* No need to care about addressing here */
        this->current_functions.emplace_back(node->name, node->line);
        this->declared_functions[node->name] = true;
        node->block->accept(this);
        this->current_functions.pop_back();
    }
    else
        this->declared_functions[node->name] = false;
}

void SemanticAnalyzer::visit(ASTNodeIf *node) {
    this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */

    node->condition->accept(this);
    node->block->accept(this);

    if (node->else_block) {
        this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */
        node->else_block->accept(this);
    }
}

void SemanticAnalyzer::visit(ASTNodeWhile *node) {
    this->symtab.insert_scope(0, 0, false); /* No need to care about addressing here */

    if (node->is_do_while) {
        node->block->accept(this);
        node->condition->accept(this);
    } else {
        node->condition->accept(this);
        node->block->accept(this);
    }
}

void SemanticAnalyzer::visit(ASTNodeFor *node) { /* TODO: for loop is not implemented yet */
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeReturn *node) {
    if (node->expression)
        node->expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeExpressionStatement *node) {
    node->expression->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeIdentifier *node) {
    auto &symbol = this->symtab.get_symbol(node->name);
    if (symbol == undefined_record) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" not declared, error on line " << node->line << std::endl;
        exit(1);
    }
}

void SemanticAnalyzer::visit(ASTNodeIntLiteral *node) {
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeBoolLiteral *node) {
    /* Empty */
}

void SemanticAnalyzer::visit(ASTNodeAssignExpression *node) {
    auto &symbol = this->symtab.get_symbol(node->name);
    if (symbol == undefined_record) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" not declared, error on line " << node->line << std::endl;
        exit(1);
    }

    if (symbol.is_const && this->assigned_constants[node->name]) {
        std::cerr << "Semantic error: variable \"" << node->name << "\" is constant and can only be assigned once, error on line " << node->line << std::endl;
        exit(1);
    }

    node->expression->accept(this);

    if (symbol.is_const)
        this->assigned_constants[node->name] = true;
}

void SemanticAnalyzer::visit(ASTNodeBinaryOperator *node) {
    node->left->accept(this);
    node->right->accept(this);
}

void SemanticAnalyzer::visit(ASTNodeUnaryOperator *node) {
    node->expression->accept(this);
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

    /* TODO: forward reference is not implemented yet (here, code generator handles it well) */
//    if (!this->declared_functions[node->name]) {
//        std::cerr << "Semantic error: function \"" << node->name << "\" is not defined, error on line " << node->line << std::endl;
//        exit(1);
//    }
}
