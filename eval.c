// AUTHOR PRANAV MINASANDRAM (PRANAVM@EMAIL.SC.EDU)
#include "eval.h"
#include <string.h>


void e_return(EN root, char* current_func){
    ST_ID id = st_enter_id(current_func);
    int junk;
    ST_DR dr = st_lookup(id, &junk);
    PARAM_LIST junk2;
    PARAMSTYLE junk1;
    TYPE ret_type = ty_query_func(dr->u.decl.type, &junk1, &junk2);
    EN new_root = root;
    if(root->tag == VAR)
        new_root = make_deref_node(root);
    if(!ty_test_equality(root->type, ret_type))
        new_root = make_conv_node(root, ret_type);

    e_eval_expr(new_root);
    b_encode_return(ty_query(ret_type));
}


int is_asgn = 0;
// takes and expression tree and evaluates it, spitting out necessary x86 code
void e_eval_expr(EN root){
    if(root == NULL)
        return;
    int junk;
    switch(root->tag){
        case FCALL:
            b_alloc_arglist(0);
            b_funcall_by_name(st_get_id_str(root->u.func_call.fn_id), ty_query(root->type));
            if(ty_query(root->type) != TYVOID)
                is_asgn = 1;
            break;
        case VAR:
            if(st_lookup(root->u.var, &junk))
                b_push_ext_addr(st_get_id_str(root->u.var));
            else
                error("Variable %s not in symbol table", st_get_id_str(root->u.var));
            break;
        case DRF:
            e_eval_expr(root->u.child);
            b_deref(ty_query(root->type));
            break;
        case CONV:
            e_eval_expr(root->u.child);
            b_convert(ty_query(root->u.child->type), ty_query(root->type));
            break;
        case UNOP:
            e_eval_expr(root->u.unop.child);
            switch(root->u.unop.op){
                case NEG:
                    b_negate(ty_query(root->type));
                    break;
                default:
                    break;
            }
            break;
        case BINOP:
            e_eval_expr(root->u.binop.left);
            e_eval_expr(root->u.binop.right);
            switch(root->u.binop.op){
                case ASGN:
                    is_asgn = 1;
                    b_assign(ty_query(root->type));
                    break;
                case ADD:
                    b_arith_rel_op(B_ADD, ty_query(root->type));
                    break;
                case SUB:
                    b_arith_rel_op(B_SUB, ty_query(root->type));
                    break;
                case DIV:
                    b_arith_rel_op(B_DIV, ty_query(root->type));
                    break;
                case MOD:
                    b_arith_rel_op(B_MOD, ty_query(root->type));
                    break;
                case MUL:
                    b_arith_rel_op(B_MULT,ty_query(root->type));
                    break;
                case LT:

                    b_arith_rel_op(B_LT, ty_query(root->u.binop.right->type));

                    break;
                case LTE:
                    b_arith_rel_op(B_LE, ty_query(root->u.binop.right->type));
                    break;
                case GT:
                    b_arith_rel_op(B_GT, ty_query(root->u.binop.right->type));
                    break;
                case GTE:
                    b_arith_rel_op(B_GE, ty_query(root->u.binop.right->type));
                    break;
                case EQ:
                    b_arith_rel_op(B_EQ, ty_query(root->u.binop.right->type));
                    break;
                case NEQ:
                    b_arith_rel_op(B_NE, ty_query(root->u.binop.right->type));
                    break;

                default:
                    bug("You forgot a bin op, bud");
                    break;
            }
            break;
        case INT_CONST:
            b_push_const_int(root->u.int_val);
            break;
        case D_CONST:
            b_push_const_double(root->u.double_val);
            break;
        default:
            bug("What just happened to me?");
    }


}



char* e_begin_func_block(BUCKET_PTR bck, TT root){

    // let's build that type, brother  B)
    TYPE base = build_base(bck);
    TYPE func_type = build_type(base, root);


    if(func_type == NULL){
        bug("how did i get here");
        exit(1);
    }
    // TYPE should be TYFUNC, else error
    if(ty_query(func_type) != TYFUNC){
        error("Declarator is not of type TYFUNC");
        exit(1);
    }


    //retrieves the ST_ID
    TT id = root;
    while(id->tag != ID){
        id = id->next;
    }

    char* func_name = st_get_id_str(id->u.id);
    b_func_prologue(func_name);

    // has it been installed already?
    int junk;
    ST_DR new_dr = stdr_alloc();
    new_dr->tag = FDECL;
    new_dr->u.decl.type = func_type;
    new_dr->u.decl.sc = NO_SC;
    new_dr->u.decl.is_ref = FALSE;

    ST_DR dr = st_lookup(id->u.id, &junk);


    // if dr is null, it hasn't been installed yet, install it.
    if(dr == NULL){

	 	if(!st_install(id->u.id, new_dr)){
            // st_install failed, meaning it has already been installed
            error("Duplicate definition for %s.", st_get_id_str(id));
     	}else{
        }
     }else{
        if(!ty_test_equality(dr->u.decl.type, new_dr->u.decl.type)){
            error("duplicate or incompatible function declaration '%s'", st_get_id_str(id->u.id));
            }
        if(dr->tag == GDECL && dr->u.decl.type == func_type){
            if(!st_replace(id, new_dr)){
                error("st_replace failed in reinstalling a fdecl");
            }
        }
    }

    st_enter_block();
    return func_name;
}

void e_end_func_block(char* func_name){
    b_label(new_symbol());
    b_func_epilogue(func_name);
    st_exit_block();
}



// takes bucket pointer and array of declarators and begins processing them one by one
void initProcess(BUCKET_PTR bck, TA arr){
	for(int i = 0; i < arr->used; i++){
		install_decls_helper(bck, arr->array[i]);
	}
}

// helper method for processing, builds the base and then processes
void install_decls_helper(BUCKET_PTR bck, TT root){
	TYPE base = build_base(bck);
	process_type(base, root, GDECL);
}




TYPE process_type(TYPE type, TT root, int DECL_TYPE){
	TYPE new_type;
	ST_DR dr;
	unsigned int ret_mul = 1;
	switch(root->tag){
		case ID:
			//install action
			dr = stdr_alloc();
			dr->tag = DECL_TYPE;
			dr->u.decl.type = type;
			dr->u.decl.sc = NO_SC;
			dr->u.decl.is_ref = FALSE;
			if(st_install(root->u.id, dr)){
				char * id = st_get_id_str(root->u.id);
				TYPETAG tag = ty_query(type);
				unsigned int decl_size;
				TYPETAG new_tag = tag;
				TYPE new_type = type;
                if(new_tag == TYFUNC)
                    return new_type;
				while(new_tag == TYARRAY){
					DIMFLAG dimflag;
					unsigned int dim;
					new_type = ty_query_array(new_type, &dimflag, &dim);
					new_tag = ty_query(new_type);
					ret_mul *= dim;
			    }
			    decl_size = get_size_basic(new_tag);
				b_global_decl(id, decl_size, decl_size*ret_mul);
				b_skip(decl_size*ret_mul);
				return new_type;
			}
			else{
				error("duplicate declaration for %s", st_get_id_str(root->u.id));
			return NULL;}
			break;
		case POINTER:
			//pointer action
			new_type = ty_build_ptr(type, NO_QUAL);
			return process_type(new_type, root->next, DECL_TYPE);
			break;
		case FN:
			//fn action
			new_type = ty_build_func(type, OLDSTYLE, NULL);
			return process_type(new_type, root->next, DECL_TYPE);
			break;
		case ARR_DIM:
			//arr action
			if(root->u.dim <= 0)
				error("illegal array dimension");
			new_type = ty_build_array(type, DIM_PRESENT, root->u.dim);
			return process_type(new_type, root->next, DECL_TYPE);
		default:
			error("Undefined tag %c", root->tag);
	}

}

// Takes a type tree and recursively builds the type object
TYPE build_type(TYPE type, TT root){
   TYPE new_type;
   switch(root->tag){
        case ID:
            return type;
        case POINTER:
            new_type = ty_build_ptr(type, NO_QUAL);
            return build_type(new_type, root->next);
        case FN:
            new_type = ty_build_func(type, OLDSTYLE, NULL);
            return build_type(new_type, root->next);
        case ARR_DIM:
            if(root->u.dim <= 0)
                error("Illegal array dimension");
            new_type = ty_build_array(type, DIM_PRESENT, root->u.dim);
            return build_type(new_type, root->next);
        default:
            error("Undefined tag");
    }
}
