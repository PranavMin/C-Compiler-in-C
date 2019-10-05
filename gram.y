/*
 *
 * yacc/bison input for simplified C++ parser
 *
 */

%{
#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "bucket.h"
#include "message.h"
#include "myarray.h"
#include "eval.h"
	int yylex();
	int yyerror(char *s);
	
	
	// Array for holding multiple declarators in a line
	size_t size = 10000;		// initial size of dynamic array
	TreeArray arr;			// dynamic array for storing declarators

    WHILE_STACK stack;
    char* current_func;     // holds the name of the function we are currently in, specifically for returns.
%}

%initial-action{
    initArray(&arr, 20);
    createStack(&stack, 20);
}

%union {
	int	y_int;
	double	y_double;
	char *	y_string;
	ST_ID	y_stid;
	TT		y_typetree;
	EN		y_exprnode;
	BUCKET_PTR y_bucket;
	TYPE 	y_type;
	TYPE_SPECIFIER y_type_spec;
    LABELS* y_label;
};

%token IDENTIFIER INT_CONSTANT DOUBLE_CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token BAD

%type<y_string> IDENTIFIER if_action
%type<y_int> INT_CONSTANT unary_operator
%type<y_double> DOUBLE_CONSTANT
%type<y_bucket> declaration_specifiers
%type<y_exprnode> primary_expr postfix_expr unary_expr cast_expr multiplicative_expr additive_expr shift_expr relational_expr equality_expr and_expr exclusive_or_expr inclusive_or_expr logical_and_expr conditional_expr assignment_expr constant_expr expr expr_opt 
%type<y_stid> identifier
%type<y_typetree> declarator direct_declarator init_declarator 
%type<y_type_spec> type_specifier
%start translation_unit
%%

 /*******************************
  * Expressions                 *
  *******************************/

primary_expr
	: identifier			{$$ = make_var_node($1);}
	| INT_CONSTANT			{$$ = make_int_node($1);}
	| DOUBLE_CONSTANT		{$$ = make_double_node($1);}
	| STRING_LITERAL		{}
	| '(' expr ')'		    {$$ = $2;}	
	;

postfix_expr
	: primary_expr
	| postfix_expr '[' expr ']'
	| postfix_expr '(' argument_expr_list_opt ')'       {$$ = make_fn_call_node($1);}
	| postfix_expr '.' identifier
	| postfix_expr PTR_OP identifier
	| postfix_expr INC_OP
	| postfix_expr DEC_OP
	;

argument_expr_list_opt
	: /* null derive */
	| argument_expr_list
	;

argument_expr_list
	: assignment_expr
	| argument_expr_list ',' assignment_expr
	;

unary_expr
	: postfix_expr
	| INC_OP unary_expr {}
	| DEC_OP unary_expr	{}
	| unary_operator cast_expr                  {$$ = make_unop_node($2, $1);}
	| SIZEOF unary_expr {}
	| SIZEOF '(' type_name ')' {}
	;

unary_operator
	: '&'       {$$ = DEREF;} 
    | '*'       {$$ = PTR;}
    | '+'       {$$ = POS;}
    | '-'       {$$ = NEG;} 
    | '~'       {$$ = POS;} 
    | '!'       {$$ = POS;}
	;

cast_expr
	: unary_expr
	| '(' type_name ')' cast_expr {}
	;

multiplicative_expr
	: cast_expr
	| multiplicative_expr '*' cast_expr         {$$ = make_binop_node($1, $3, MUL);}
	| multiplicative_expr '/' cast_expr         {$$ = make_binop_node($1, $3, DIV);}
	| multiplicative_expr '%' cast_expr         {$$ = make_binop_node($1, $3, MOD);}
	;

additive_expr
	: multiplicative_expr
	| additive_expr '+' multiplicative_expr     {$$ = make_binop_node($1, $3, ADD);}
	| additive_expr '-' multiplicative_expr     {$$ = make_binop_node($1, $3, SUB);}
	;

shift_expr
	: additive_expr
	| shift_expr LEFT_OP additive_expr
	| shift_expr RIGHT_OP additive_expr
	;

relational_expr
	: shift_expr
	| relational_expr '<' shift_expr            {$$ = make_binop_node($1, $3, LT);}
	| relational_expr '>' shift_expr            {$$ = make_binop_node($1, $3, GT);}
	| relational_expr LE_OP shift_expr          {$$ = make_binop_node($1, $3, LTE);}
	| relational_expr GE_OP shift_expr          {$$ = make_binop_node($1, $3, GTE);}
	;

equality_expr
	: relational_expr
	| equality_expr EQ_OP relational_expr      {$$ = make_binop_node($1, $3, EQ);} 
	| equality_expr NE_OP relational_expr      {$$ = make_binop_node($1, $3, NEQ);} 
	;

and_expr
	: equality_expr
	| and_expr '&' equality_expr
	;

exclusive_or_expr
	: and_expr
	| exclusive_or_expr '^' and_expr
	;

inclusive_or_expr
	: exclusive_or_expr
	| inclusive_or_expr '|' exclusive_or_expr
	;

logical_and_expr
	: inclusive_or_expr
	| logical_and_expr AND_OP inclusive_or_expr
	;

logical_or_expr
	: logical_and_expr
	| logical_or_expr OR_OP logical_and_expr
	;

conditional_expr
	: logical_or_expr    {}
	| logical_or_expr '?' expr ':' conditional_expr   {}
	;

assignment_expr
	: conditional_expr
	| unary_expr assignment_operator assignment_expr  {$$ = make_binop_node($1, $3, ASGN);}
	;

assignment_operator
	: '='  
    | MUL_ASSIGN | DIV_ASSIGN | MOD_ASSIGN | ADD_ASSIGN | SUB_ASSIGN
	| LEFT_ASSIGN | RIGHT_ASSIGN | AND_ASSIGN | XOR_ASSIGN | OR_ASSIGN
	;

expr
	: assignment_expr
	| expr ',' assignment_expr
	;

constant_expr
	: conditional_expr
	;

expr_opt
	: /* null derive */   {$$ = NULL;}
	| expr
	;

 /*******************************
  * Declarations                *
  *******************************/

declaration
	: declaration_specifiers ';'						{error("no declarator in declaration");}
	| declaration_specifiers init_declarator_list ';'	{initProcess($1, &arr); freeArray(&arr); initArray(&arr, size);} 	 
	;

declaration_specifiers
	: storage_class_specifier							{}
	| storage_class_specifier declaration_specifiers	{}
	| type_specifier									{$$ = update_bucket(NULL, $1, NULL);}					
	| type_specifier declaration_specifiers				{$$ = update_bucket($2, $1, NULL);}
	| type_qualifier									{}
	| type_qualifier declaration_specifiers				{}
	;

init_declarator_list
	: init_declarator	     						{insertArray(&arr, $1);}
	| init_declarator_list ',' init_declarator		{insertArray(&arr, $3);}
	;

init_declarator
	: declarator						
	| declarator '=' initializer							
	;

storage_class_specifier
	: TYPEDEF | EXTERN | STATIC | AUTO | REGISTER
	;

type_specifier
	: VOID		{$$ = VOID_SPEC ;}
	| CHAR 		{$$ = CHAR_SPEC;}
	| SHORT 	{$$ = SHORT_SPEC;}
	| INT 		{$$ = INT_SPEC;}
	| LONG		{$$ = LONG_SPEC;}
	| FLOAT 	{$$ = FLOAT_SPEC;}
	| DOUBLE 	{$$ = DOUBLE_SPEC;}
	| SIGNED 	{$$ = SIGNED_SPEC;}
	| UNSIGNED	{$$ = UNSIGNED_SPEC;}
	| struct_or_union_specifier	{$$ = STRUCT_SPEC;}
	| enum_specifier	{$$ = ENUM_SPEC;}
	| TYPE_NAME 	{}
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union identifier '{' struct_declaration_list '}'
	| struct_or_union identifier
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list_opt
	| type_qualifier specifier_qualifier_list_opt
	;

specifier_qualifier_list_opt
	: /* null derive */
	| specifier_qualifier_list
        ;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expr
	| declarator ':' constant_expr
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM identifier '{' enumerator_list '}'
	| ENUM identifier
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: identifier
	| identifier '=' constant_expr
	;

type_qualifier
	: CONST | VOLATILE
	;

declarator
	: direct_declarator
	| pointer declarator							{$$ = make_ptr_node($2);}
	;

direct_declarator
	: identifier									{$$ = make_id_node($1);}
	| '(' declarator ')'							{$$ = $2;}
	| direct_declarator '[' ']'	
	| direct_declarator '[' constant_expr ']'		{$$ = make_arr_node($3->u.int_val, $1);}
	| direct_declarator '(' parameter_type_list ')' {$$ = make_fn_node($1);}
	| direct_declarator '(' ')'						{$$ = make_fn_node($1);}	
	;

pointer
	: '*' specifier_qualifier_list_opt
    | '&'
	;


parameter_type_list
	: parameter_list
	| parameter_list ',' ELIPSIS
	;

parameter_list
	: parameter_declaration
	| parameter_list ',' parameter_declaration    
	;

parameter_declaration
	: declaration_specifiers declarator          
	| declaration_specifiers                      {error("In paramater_declaration: No declarator given for declspec");}
	| declaration_specifiers abstract_declarator  {}
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expr ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expr ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expr
	| '{' initializer_list comma_opt '}'
	;

comma_opt
	: /* Null derive */
	| ','
	;

initializer_list
	: initializer
	| initializer_list ',' initializer
	;

 /*******************************
  * Statements                  *
  *******************************/

statement
	: labeled_statement
	| {st_enter_block();} compound_statement {st_exit_block();}
	| expression_statement  {}
	| selection_statement   {} 
	| iteration_statement   {}
	| jump_statement        {}
	;

labeled_statement
	: identifier ':' statement
	| CASE constant_expr ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'       {}
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: expr_opt ';' {e_eval_expr($1); if(is_asgn) b_pop(); is_asgn = 0;}
	;

selection_statement
	: IF '(' expr ')' if_action statement   {b_label($5);}
	| IF '(' expr ')' if_action statement ELSE {char* temp = new_symbol(); b_jump(temp); $<y_string>$ = temp; b_label($5);} statement {b_label($<y_string>8);}
	| SWITCH '(' expr ')' statement
	;

if_action
    :/*empty*/ { 
                    EN new_root = $<y_exprnode>-1;

                    if(new_root->tag == VAR)
                        new_root = make_deref_node($<y_exprnode>-1);

                    e_eval_expr(new_root);
                    char* temp = new_symbol();
                    b_cond_jump(TYSIGNEDINT, B_ZERO, temp);
                    $$ = temp;      
               }
    ;

iteration_statement
	: WHILE '(' expr ')' {
                            char* start = new_symbol();
                            char* exit = new_symbol();

                            push(&stack, exit);
                            LABELS* lab = make_labels(start, exit);
                            b_label(start);
                            e_eval_expr($3);
                            b_cond_jump(TYSIGNEDINT, B_ZERO, exit);
                            $<y_label>$ = lab;
                         } 
      statement {pop(&stack); b_jump($<y_label>5->start); b_label($<y_label>5->end);}
	| DO statement WHILE '(' expr ')' ';'
	| FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' for_action  statement {e_eval_expr($7); if(is_asgn){ b_pop(); is_asgn = 0;} pop(&stack); b_jump($<y_label>9->start); b_label($<y_label>9->end);}
	;

for_action
    :/*empty*/ {
                    char* start = new_symbol();
                    char* exit = new_symbol();
                    if($<y_exprnode>-5 != NULL)
                        e_eval_expr($<y_exprnode>-5);
                    if(is_asgn){
                        b_pop();
                        is_asgn = 0;
                    }
                    b_label(start);
                    push(&stack, exit);
                    LABELS* lab = make_labels(start, exit);
                    
                    if($<y_exprnode>-5 != NULL){
                        e_eval_expr($<y_exprnode>-3);
                        b_cond_jump(TYSIGNEDINT, B_ZERO, exit);
                    }
                    $<y_label>$ = lab;
               }
    ;
    
jump_statement
	: GOTO identifier ';'
	| CONTINUE ';'
	| BREAK ';' {char* pop_elem = pop(&stack); b_jump(pop_elem); if(pop_elem != NULL) push(&stack, pop_elem);}
	| RETURN expr_opt ';' {e_return($2, current_func);}
	;

 /*******************************
  * Top level                   *
  *******************************/

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declarator {$<y_string>$ = e_begin_func_block( update_bucket(NULL, INT_SPEC, NULL), $1); current_func = $<y_string>$;}  compound_statement {e_end_func_block($<y_string>2);}  
	| declaration_specifiers declarator {$<y_string>$ = e_begin_func_block($1, $2); current_func = $<y_string>$;} compound_statement {e_end_func_block($<y_string>3);}
	;

 /*******************************
  * Identifiers                 *
  *******************************/

identifier
	: IDENTIFIER		{$$ = st_enter_id($1);}
	;
%%


extern int column;

int yyerror(char *s)
{
	error("%s (column %d)",s,column);
        return 0;  /* never reached */
}
