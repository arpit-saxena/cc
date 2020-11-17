#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include "ast.hpp"

class stmt_node : public ast_node {};
class labeled_stmt : public stmt_node {};
class compound_stmt : public stmt_node {};
class expression_stmt : public stmt_node {};
class selection_stmt : public stmt_node {};
class iteration_stmt : public stmt_node {};
class jump_stmt : public stmt_node {};

#endif /* STATEMENT_HPP */