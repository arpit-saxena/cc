%{
#include <cstdio>
#include <iostream>
using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
int yyparse();
extern "C" FILE *yyin;
 
void yyerror(const char *s);
%}

%code requires {
	#include "ast.hpp"
	#include "decl_common.hpp"
	#include "declarator.hpp"
	#include "statement.hpp"
	#include "func_def.hpp"
	#include "expression.hpp"
	#include "declaration.hpp"
}

%{
#include "ast.hpp"
void unimplemented [[noreturn]] () {
	ast_node::raise_error("Unimplemented grammar rule!");
}
%}

%define api.value.type union
%define parse.trace

%token	<const char *> IDENTIFIER
%token  <const char *> I_CONSTANT F_CONSTANT STRING_LITERAL FUNC_NAME SIZEOF
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME ENUMERATION_CONSTANT

%token	TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token	CONST RESTRICT VOLATILE
%token	BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token	COMPLEX IMAGINARY 
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token	ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%nterm <param_declaration *> parameter_declaration
%nterm <param_list *> parameter_type_list parameter_list
%nterm <declaration_specs *> declaration_specifiers
%nterm <storage_specifiers::storage> storage_class_specifier
%nterm <type_specifiers::type> type_specifier
%nterm <type_qualifiers::qualifier> type_qualifier
%nterm <pointer_node *> pointer
%nterm <type_qualifiers *> type_qualifier_list
%nterm <declarator_node *> declarator
%nterm <direct_decl *> direct_declarator
%nterm <external_decl *> external_declaration
%nterm <func_def *> function_definition
%nterm <trans_unit *> translation_unit
%nterm <blk_item *> block_item
%nterm <stmt_node *> statement
%nterm <labeled_stmt *> labeled_statement
%nterm <expression_stmt *> expression_statement
%nterm <selection_stmt *> selection_statement
%nterm <iteration_stmt *> iteration_statement
%nterm <compound_stmt *> compound_statement block_item_list
%nterm <jump_stmt *> jump_statement

%nterm <primary_expr *> primary_expression
%nterm <const_expr *> constant
%nterm <string_expr *> string
%nterm <postfix_expr *> postfix_expression
%nterm <arg_expr_list *> argument_expression_list
%nterm <unary_expr *> unary_expression
%nterm <unary_op_expr::OP> unary_operator
%nterm <cast_expr *> cast_expression

%nterm <binary_expr *> multiplicative_expression
					   additive_expression
					   shift_expression
					   relational_expression
					   equality_expression
					   and_expression
					   exclusive_or_expression
					   inclusive_or_expression
					   logical_and_expression
					   logical_or_expression

%nterm <cond_expr *> conditional_expression
%nterm <assign_expr *> assignment_expression
%nterm <assign_expr_ops::OP> assignment_operator
%nterm <expr *> expression
%nterm <cond_expr *> constant_expression // TODO: Replace with own class
%nterm <initializer_node *> initializer
%nterm <initializer_lst *> initializer_list
%nterm <init_decl *> init_declarator
%nterm <init_decl_list *> init_declarator_list
%nterm <declaration_node *> declaration

%start translation_unit
%%

primary_expression
	: IDENTIFIER {$$ = new ident_expr($1);}
	| constant {$$ = $1;}
	| string {$$ = $1;}
	| '(' expression ')' {$$ = new paren_expr($2);}
	| generic_selection {unimplemented();}
	;

constant
	: I_CONSTANT {$$ = const_expr::new_int_expr($1);}		/* includes character_constant */
	| F_CONSTANT {unimplemented();}
	| ENUMERATION_CONSTANT {unimplemented();}	/* after it has been defined as such */
	;

enumeration_constant		/* before it has been defined as such */
	: IDENTIFIER {unimplemented();}
	;

string
	: STRING_LITERAL {$$ = new string_expr($1);}
	| FUNC_NAME {unimplemented();}
	;

generic_selection
	: GENERIC '(' assignment_expression ',' generic_assoc_list ')'
	;

generic_assoc_list
	: generic_association {unimplemented();}
	| generic_assoc_list ',' generic_association {unimplemented();}
	;

generic_association
	: type_name ':' assignment_expression {unimplemented();}
	| DEFAULT ':' assignment_expression {unimplemented();}
	;

postfix_expression
	: primary_expression {$$ = $1;}
	| postfix_expression '[' expression ']' {unimplemented();}
	| postfix_expression '(' ')' {$$ = new func_call($1);}
	| postfix_expression '(' argument_expression_list ')' {$$ = new func_call($1, $3);}
	| postfix_expression '.' IDENTIFIER {unimplemented();}
	| postfix_expression PTR_OP IDENTIFIER {unimplemented();}
	| postfix_expression INC_OP {$$ = new postfix_inc_dec_expr($1, postfix_inc_dec_expr::INC);}
	| postfix_expression DEC_OP {$$ = new postfix_inc_dec_expr($1, postfix_inc_dec_expr::DEC);}
	| '(' type_name ')' '{' initializer_list '}' {unimplemented();}
	| '(' type_name ')' '{' initializer_list ',' '}' {unimplemented();}
	;

argument_expression_list
	: assignment_expression {$$ = (new arg_expr_list())->add($1);}
	| argument_expression_list ',' assignment_expression {$$ = $1->add($3);}
	;

unary_expression
	: postfix_expression {$$ = $1;}
	| INC_OP unary_expression {$$ = new unary_inc_dec_expr(unary_inc_dec_expr::INC, $2);}
	| DEC_OP unary_expression {$$ = new unary_inc_dec_expr(unary_inc_dec_expr::DEC, $2);}
	| unary_operator cast_expression {$$ = new unary_op_expr($1, $2);}
	| SIZEOF unary_expression {unimplemented();}
	| SIZEOF '(' type_name ')' {unimplemented();}
	| ALIGNOF '(' type_name ')' {unimplemented();}
	;

unary_operator
	: '&' {$$ = unary_op_expr::ADDRESS_OF;}
	| '*' {$$ = unary_op_expr::INDIRECTION;}
	| '+' {$$ = unary_op_expr::PLUS;}
	| '-' {$$ = unary_op_expr::MINUS;}
	| '~' {$$ = unary_op_expr::BIT_NOT;}
	| '!' {$$ = unary_op_expr::NOT;}
	;

cast_expression
	: unary_expression {$$ = $1;}
	| '(' type_name ')' cast_expression {unimplemented();}
	;

multiplicative_expression
	: cast_expression {$$ = $1;}
	| multiplicative_expression '*' cast_expression {$$ = new binary_expr_ops($1, binary_expr_ops::MULT, $3);}
	| multiplicative_expression '/' cast_expression {$$ = new binary_expr_ops($1, binary_expr_ops::DIV, $3);}
	| multiplicative_expression '%' cast_expression {$$ = new binary_expr_ops($1, binary_expr_ops::MOD, $3);}
	;

additive_expression
	: multiplicative_expression {$$ = $1;}
	| additive_expression '+' multiplicative_expression {$$ = new binary_expr_ops($1, binary_expr_ops::PLUS, $3);}
	| additive_expression '-' multiplicative_expression {$$ = new binary_expr_ops($1, binary_expr_ops::MINUS, $3);}
	;

shift_expression
	: additive_expression {$$ = $1;}
	| shift_expression LEFT_OP additive_expression {$$ = new binary_expr_ops($1, binary_expr_ops::SHIFT_LEFT, $3);}
	| shift_expression RIGHT_OP additive_expression {$$ = new binary_expr_ops($1, binary_expr_ops::SHIFT_RIGHT, $3);}
	;

relational_expression
	: shift_expression {$$ = $1;}
	| relational_expression '<' shift_expression {$$ = new binary_expr_ops($1, binary_expr_ops::LT, $3);}
	| relational_expression '>' shift_expression {$$ = new binary_expr_ops($1, binary_expr_ops::GT, $3);}
	| relational_expression LE_OP shift_expression {$$ = new binary_expr_ops($1, binary_expr_ops::LE, $3);}
	| relational_expression GE_OP shift_expression {$$ = new binary_expr_ops($1, binary_expr_ops::GE, $3);}
	;

equality_expression
	: relational_expression {$$ = $1;}
	| equality_expression EQ_OP relational_expression {$$ = new binary_expr_ops($1, binary_expr_ops::EQ, $3);}
	| equality_expression NE_OP relational_expression {$$ = new binary_expr_ops($1, binary_expr_ops::NE, $3);}
	;

and_expression
	: equality_expression {$$ = $1;}
	| and_expression '&' equality_expression {$$ = new binary_expr_ops($1, binary_expr_ops::BIT_AND, $3);}
	;

exclusive_or_expression
	: and_expression {$$ = $1;}
	| exclusive_or_expression '^' and_expression {$$ = new binary_expr_ops($1, binary_expr_ops::BIT_XOR, $3);}
	;

inclusive_or_expression
	: exclusive_or_expression {$$ = $1;}
	| inclusive_or_expression '|' exclusive_or_expression {$$ = new binary_expr_ops($1, binary_expr_ops::BIT_OR, $3);}
	;

logical_and_expression
	: inclusive_or_expression {$$ = $1;}
	| logical_and_expression AND_OP inclusive_or_expression {$$ = new binary_expr_ops($1, binary_expr_ops::AND, $3);}
	;

logical_or_expression
	: logical_and_expression {$$ = $1;}
	| logical_or_expression OR_OP logical_and_expression {$$ = new binary_expr_ops($1, binary_expr_ops::OR, $3);}
	;

conditional_expression
	: logical_or_expression {$$ = $1;}
	| logical_or_expression '?' expression ':' conditional_expression {$$ = new cond_expr_ops($1, $3, $5);}
	;

assignment_expression
	: conditional_expression {$$ = $1;}
	| unary_expression assignment_operator assignment_expression {$$ = new assign_expr_ops($1, $2, $3);}
	;

assignment_operator
	: '='		   {$$ = assign_expr_ops::ASSIGN;}
	| MUL_ASSIGN   {$$ = assign_expr_ops::MUL_ASSIGN;}
	| DIV_ASSIGN   {$$ = assign_expr_ops::DIV_ASSIGN;}
	| MOD_ASSIGN   {$$ = assign_expr_ops::MOD_ASSIGN;}
	| ADD_ASSIGN   {$$ = assign_expr_ops::ADD_ASSIGN;}
	| SUB_ASSIGN   {$$ = assign_expr_ops::SUB_ASSIGN;}
	| LEFT_ASSIGN  {$$ = assign_expr_ops::LEFT_ASSIGN;}
	| RIGHT_ASSIGN {$$ = assign_expr_ops::RIGHT_ASSIGN;}
	| AND_ASSIGN   {$$ = assign_expr_ops::AND_ASSIGN;}
	| XOR_ASSIGN   {$$ = assign_expr_ops::XOR_ASSIGN;}
	| OR_ASSIGN    {$$ = assign_expr_ops::OR_ASSIGN;}
	;

expression
	: assignment_expression {$$ = $1;}
	| expression ',' assignment_expression
	;

constant_expression
	: conditional_expression {$$ = $1;}	/* with constraints */
	;

declaration
	: declaration_specifiers ';' {$$ = new declaration_node($1);}
	| declaration_specifiers init_declarator_list ';' {$$ = new declaration_node($1, $2);}
	| static_assert_declaration {unimplemented();}
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers {$$ = $2->add($1);}
	| storage_class_specifier {$$ = (new declaration_specs())->add($1);}
	| type_specifier declaration_specifiers {$$ = $2->add($1);}
	| type_specifier {$$ = (new declaration_specs())->add($1);}
	| type_qualifier declaration_specifiers {$$ = $2->add($1);}
	| type_qualifier {$$ = (new declaration_specs)->add($1);}
	| function_specifier declaration_specifiers {unimplemented();}
	| function_specifier {unimplemented();}
	| alignment_specifier declaration_specifiers {unimplemented();}
	| alignment_specifier {unimplemented();}
	;

init_declarator_list
	: init_declarator {$$ = (new init_decl_list())->add($1);}
	| init_declarator_list ',' init_declarator {$$ = $1->add($3);}
	;

init_declarator
	: declarator '=' initializer {$$ = new init_decl($1, $3);}
	| declarator {$$ = new init_decl($1);}
	;

storage_class_specifier
	: TYPEDEF {$$ = storage_specifiers::TYPEDEF;} /* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN {$$ = storage_specifiers::EXTERN;}
	| STATIC {$$ = storage_specifiers::STATIC;}
	| THREAD_LOCAL {$$ = storage_specifiers::THREAD_LOCAL;}
	| AUTO {$$ = storage_specifiers::AUTO;}
	| REGISTER {$$ = storage_specifiers::REGISTER;}
	;

type_specifier
	: VOID {$$ = type_specifiers::VOID;}
	| CHAR {$$ = type_specifiers::CHAR;}
	| SHORT {$$ = type_specifiers::SHORT;}
	| INT {$$ = type_specifiers::INT;}
	| LONG {$$ = type_specifiers::LONG;}
	| FLOAT {$$ = type_specifiers::FLOAT;}
	| DOUBLE {$$ = type_specifiers::DOUBLE;}
	| SIGNED {$$ = type_specifiers::SIGNED;}
	| UNSIGNED {$$ = type_specifiers::UNSIGNED;}
	| BOOL {$$ = type_specifiers::BOOL;}
	| COMPLEX {unimplemented();}
	| IMAGINARY	{unimplemented();}  	/* non-mandated extension */
	| atomic_type_specifier {unimplemented();}
	| struct_or_union_specifier {unimplemented();}
	| enum_specifier {unimplemented();}
	| TYPEDEF_NAME {unimplemented();}		/* after it has been defined as such */
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER '{' struct_declaration_list '}'
	| struct_or_union IDENTIFIER {unimplemented();}
	;

struct_or_union
	: STRUCT {unimplemented();}
	| UNION {unimplemented();}
	;

struct_declaration_list
	: struct_declaration {unimplemented();}
	| struct_declaration_list struct_declaration {unimplemented();}
	;

struct_declaration
	: specifier_qualifier_list ';'	/* for anonymous struct/union */ {unimplemented();}
	| specifier_qualifier_list struct_declarator_list ';' {unimplemented();}
	| static_assert_declaration {unimplemented();}
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list {unimplemented();}
	| type_specifier {unimplemented();}
	| type_qualifier specifier_qualifier_list {unimplemented();}
	| type_qualifier {unimplemented();}
	;

struct_declarator_list
	: struct_declarator {unimplemented();}
	| struct_declarator_list ',' struct_declarator {unimplemented();}
	;

struct_declarator
	: ':' constant_expression {unimplemented();}
	| declarator ':' constant_expression {unimplemented();}
	| declarator {unimplemented();}
	;

enum_specifier
	: ENUM '{' enumerator_list '}' {unimplemented();}
	| ENUM '{' enumerator_list ',' '}' {unimplemented();}
	| ENUM IDENTIFIER '{' enumerator_list '}' {unimplemented();}
	| ENUM IDENTIFIER '{' enumerator_list ',' '}' {unimplemented();}
	| ENUM IDENTIFIER {unimplemented();}
	;

enumerator_list
	: enumerator {unimplemented();}
	| enumerator_list ',' enumerator {unimplemented();}
	;

enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	: enumeration_constant '=' constant_expression {unimplemented();}
	| enumeration_constant {unimplemented();}
	;

atomic_type_specifier
	: ATOMIC '(' type_name ')' {unimplemented();}
	;

type_qualifier
	: CONST {$$ = type_qualifiers::CONST;}
	| RESTRICT {$$ = type_qualifiers::RESTRICT;}
	| VOLATILE {$$ = type_qualifiers::VOLATILE;}
	| ATOMIC {$$ = type_qualifiers::ATOMIC;}
	;

function_specifier
	: INLINE {unimplemented();}
	| NORETURN {unimplemented();}
	;

alignment_specifier
	: ALIGNAS '(' type_name ')' {unimplemented();}
	| ALIGNAS '(' constant_expression ')' {unimplemented();}
	;

declarator
	: pointer direct_declarator {$$ = new declarator_node($2, $1);}
	| direct_declarator {$$ = new declarator_node($1);}
	;

direct_declarator
	: IDENTIFIER {$$ = new identifier_declarator($1);}
	| '(' declarator ')' {$$ = $2;}
	| direct_declarator '[' ']'
	| direct_declarator '[' '*' ']'
	| direct_declarator '[' STATIC type_qualifier_list assignment_expression ']'
	| direct_declarator '[' STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list '*' ']'
	| direct_declarator '[' type_qualifier_list STATIC assignment_expression ']'
	| direct_declarator '[' type_qualifier_list assignment_expression ']'
	| direct_declarator '[' type_qualifier_list ']'
	| direct_declarator '[' assignment_expression ']'
	| direct_declarator '(' parameter_type_list ')' {$$ = new function_declarator($1, $3);}
	| direct_declarator '(' ')' {$$ = new function_declarator($1);}
	| direct_declarator '(' identifier_list ')' {function_declarator::old_style_error();}
	;

pointer
	: '*' type_qualifier_list pointer {$$ = $3->add($2);}
	| '*' type_qualifier_list {$$ = (new pointer_node())->add($2);}
	| '*' pointer {$$ = $2->add(new type_qualifiers());}
	| '*' {$$ = (new pointer_node())->add(new type_qualifiers());}
	;

type_qualifier_list
	: type_qualifier {$$ = (new type_qualifiers())->add_qual($1);}
	| type_qualifier_list type_qualifier {$$ = $1->add_qual($2);}
	;


parameter_type_list
	: parameter_list ',' ELLIPSIS {$$ = $1->make_vararg();}
	| parameter_list {$$ = $1;}
	;

parameter_list
	: parameter_declaration {$$ = (new param_list())->add($1);}
	| parameter_list ',' parameter_declaration {$$ = $1->add($3);}
	;

parameter_declaration
	: declaration_specifiers declarator {$$ = new param_declaration($1, $2);}
	| declaration_specifiers abstract_declarator {unimplemented();}
	| declaration_specifiers {$$ = new param_declaration($1);}
	;

identifier_list
	: IDENTIFIER {unimplemented();}
	| identifier_list ',' IDENTIFIER {unimplemented();}
	;

type_name
	: specifier_qualifier_list abstract_declarator {unimplemented();}
	| specifier_qualifier_list {unimplemented();}
	;

abstract_declarator
	: pointer direct_abstract_declarator {unimplemented();}
	| pointer {unimplemented();}
	| direct_abstract_declarator {unimplemented();}
	;

direct_abstract_declarator
	: '(' abstract_declarator ')' {unimplemented();}
	| '[' ']' {unimplemented();}
	| '[' '*' ']' {unimplemented();}
	| '[' STATIC type_qualifier_list assignment_expression ']' {unimplemented();}
	| '[' STATIC assignment_expression ']' {unimplemented();}
	| '[' type_qualifier_list STATIC assignment_expression ']' {unimplemented();}
	| '[' type_qualifier_list assignment_expression ']' {unimplemented();}
	| '[' type_qualifier_list ']' {unimplemented();}
	| '[' assignment_expression ']' {unimplemented();}
	| direct_abstract_declarator '[' ']' {unimplemented();}
	| direct_abstract_declarator '[' '*' ']' {unimplemented();}
	| direct_abstract_declarator '[' STATIC type_qualifier_list assignment_expression ']' {unimplemented();}
	| direct_abstract_declarator '[' STATIC assignment_expression ']' {unimplemented();}
	| direct_abstract_declarator '[' type_qualifier_list assignment_expression ']' {unimplemented();}
	| direct_abstract_declarator '[' type_qualifier_list STATIC assignment_expression ']' {unimplemented();}
	| direct_abstract_declarator '[' type_qualifier_list ']' {unimplemented();}
	| direct_abstract_declarator '[' assignment_expression ']' {unimplemented();}
	| '(' ')' {unimplemented();}
	| '(' parameter_type_list ')' {unimplemented();}
	| direct_abstract_declarator '(' ')' {unimplemented();}
	| direct_abstract_declarator '(' parameter_type_list ')' {unimplemented();}
	;

initializer
	: '{' initializer_list '}' {$$ = $2;}
	| '{' initializer_list ',' '}' {$$ = $2;}
	| assignment_expression {$$ = new init_assign_expr($1);}
	;

initializer_list
	: designation initializer {unimplemented();}
	| initializer {$$ = (new initializer_lst())->add($1);}
	| initializer_list ',' designation initializer {unimplemented();}
	| initializer_list ',' initializer {$$ = $1->add($3);}
	;

designation
	: designator_list '=' {unimplemented();}
	;

designator_list
	: designator {unimplemented();}
	| designator_list designator {unimplemented();}
	;

designator
	: '[' constant_expression ']' {unimplemented();}
	| '.' IDENTIFIER {unimplemented();}
	;

static_assert_declaration
	: STATIC_ASSERT '(' constant_expression ',' STRING_LITERAL ')' ';' {unimplemented();}
	;

statement
	: labeled_statement {$$ = $1;}
	| compound_statement {$$ = $1;}
	| expression_statement {$$ = $1;}
	| selection_statement {$$ = $1;}
	| iteration_statement {$$ = $1;}
	| jump_statement {$$ = $1;}
	;

labeled_statement
	: IDENTIFIER ':' statement {$$ = new prefix_labeled_stmt($1, $3);}
	| CASE constant_expression ':' statement {$$ = new case_labeled_stmt($2, $4);}
	| DEFAULT ':' statement {$$ = new default_labeled_stmt($3);}
	;

compound_statement
	: '{' '}' {$$ = new compound_stmt();}
	| '{'  block_item_list '}' {$$ = $2;}
	;

block_item_list
	: block_item {$$ = (new compound_stmt())->add($1);}
	| block_item_list block_item {$$ = $1->add($2);}
	;

block_item
	: declaration {$$ = $1;}
	| statement {$$ = $1;}
	;

expression_statement
	: ';' {$$ = new expression_stmt();}
	| expression ';' {$$ = new expression_stmt($1);}
	;

selection_statement
	: IF '(' expression ')' statement ELSE statement {$$ = new if_stmt($3, $5, $7);}
	| IF '(' expression ')' statement {$$ = new if_stmt($3, $5);}
	| SWITCH '(' expression ')' statement {$$ = new switch_stmt($3, $5);}
	;

iteration_statement
	: WHILE '(' expression ')' statement {$$ = new while_stmt($3, $5);}
	| DO statement WHILE '(' expression ')' ';' {$$ = new do_stmt($2, $5);}
	| FOR '(' expression_statement expression_statement ')' statement {unimplemented();}
	| FOR '(' expression_statement expression_statement expression ')' statement {unimplemented();}
	| FOR '(' declaration expression_statement ')' statement {unimplemented();}
	| FOR '(' declaration expression_statement expression ')' statement {unimplemented();}
	;

jump_statement
	: GOTO IDENTIFIER ';' {$$ = new goto_stmt($2);}
	| CONTINUE ';' {unimplemented();}
	| BREAK ';' {unimplemented();}
	| RETURN ';' {$$ = new return_stmt();}
	| RETURN expression ';' {$$ = new return_stmt($2);}
	;

translation_unit
	: external_declaration {ast_node::base = $$ = (new trans_unit())->add($1);}
	| translation_unit external_declaration {$$ = $1->add($2);}
	;

external_declaration
	: function_definition {$$ = $1;}
	| declaration {$$ = $1;}
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement {func_def::old_style_error();}
	| declaration_specifiers declarator compound_statement {$$ = new func_def($1, $2, $3);}
	;

declaration_list
	: declaration {unimplemented();}
	| declaration_list declaration {unimplemented();}
	;

%%
#include <stdio.h>

void yyerror(const char *s)
{
	fflush(stdout);
	fprintf(stderr, "*** %s\n", s);
}
