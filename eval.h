#ifndef PM_EVAL
#define PM_EVAL

#include "tree.h"
#include "bucket.h"
#include "types.h"
#include "myarray.h"
#include "symtab.h"
#include "message.h"
#include "bucket.h"
#include "message.h"
#include "backend-x86.h"

extern int is_asgn;
void e_return(EN root, char* current_func);
void e_eval_expr(EN root);
char* e_begin_func_block(BUCKET_PTR bck, TT root);
void e_end_func_block(char* func_name);
void initProcess(BUCKET_PTR bck, TA arr);

void install_decls_helper(BUCKET_PTR bck, TT root);

TYPE process_type(TYPE type, TT root, int DECL_TYPE);
TYPE build_type(TYPE type, TT root);
#endif
