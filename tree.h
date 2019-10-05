#ifndef _PM_TREE_
#define _PM_TREE_

#include <stdlib.h>
#include "symtab.h"
#include "bucket.h"
#include "message.h"

typedef struct endstart{
    char* start;
    char* end;
}   LABELS;
enum {INT_TYPE, REAL_TYPE, FN, POINTER, ARR_DIM, ID};

LABELS* make_labels(char* start, char* end);


typedef struct tt{
	int tag;
	union{
		double real_val;
		ST_ID id;
		int dim;
	}u;
	struct tt *next;
} TYPE_TREE, *TT;


TT make_id_node(ST_ID id);
TT make_arr_node(int dim, TT next);
TT make_fn_node(TT next);
TT make_ptr_node(TT next);





typedef enum {VAR, INT_CONST, D_CONST, C_CONST, UNOP, BINOP, FCALL, CONV, DRF} EXPR_TAG;
typedef enum {DEREF, PTR, POS, NEG, ADD, SUB, MUL, MOD, DIV, ASGN, LT, GT, LTE, GTE, EQ, NEQ} OP_TAG;


typedef struct exprnode EXPR_NODE, *EN;
typedef struct exprlist EXPR_LIST, *EL;

typedef struct exprlist{
    EN exprnode;
    struct el *next;
}EXPR_LIST, *EL;

// Used to build up expressions for evaluating and outputting asm
typedef struct exprnode{
    TYPE type;
    EXPR_TAG tag;
    
    union{
        ST_ID var;
        int int_val;
        double double_val;
        char char_val;
        struct exprnode *child;

        struct {
            OP_TAG op;
            struct exprnode *child;
        } unop;

        struct {
            OP_TAG op;
            struct exprnode *left, *right;
        } binop;

        struct {
            ST_ID fn_id;
            EL arglist; 
        } func_call;
    }u;
} EXPR_NODE, *EN;

void print_node(EN node);
int handle_conversion(TYPE left, TYPE right);
EN make_fn_call_node(EN identifier);
EN make_char_node(char c);
EN make_int_node(int val);
EN make_double_node(double val);
EN make_var_node(ST_ID var);
EN make_unop_node(EN node, OP_TAG);
EN make_binop_node(EN left, EN right, OP_TAG binop);
EN make_deref_node(EN node);
EN make_conv_node(EN right, TYPE left_type);

#endif
