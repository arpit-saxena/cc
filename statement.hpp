#include "ast.hpp"

class statement_node : public ast_node {};
class labeled_statement : public statement_node {};
class compound_statement : public statement_node {};
class expression_statement : public statement_node {};
class selection_statement : public statement_node {};
class iteration_statement : public statement_node {};
class jump_statement : public statement_node {};