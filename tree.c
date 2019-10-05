#include "tree.h"
#include <limits.h>
LABELS* make_labels(char* start, char* end){
    LABELS* ret;
    ret = (LABELS*) malloc(sizeof(LABELS));
    if(ret == NULL)
        exit(1);
    ret->start = start;
    ret->end = end;
    return ret;
}

TT make_id_node(ST_ID id){
    TT ret;
	ret = (TT) malloc(sizeof(TYPE_TREE));
	if(ret == NULL)
		exit(1);
	ret->tag = ID;
	ret->u.id = id;
	return ret;
}

TT make_arr_node(int dim, TT next){
	TT ret;
	ret = (TT) malloc(sizeof(TYPE_TREE));
	if(ret == NULL)
		exit(1);
	ret->tag = ARR_DIM;
	ret->u.dim = dim;
	ret->next = next;
	return ret;
}

TT make_fn_node(TT next){
	    TT ret;
	    ret = (TT) malloc(sizeof(TYPE_TREE));
	    if(ret == NULL)
	        exit(1);
	    ret->tag = FN;
		ret->next = next;
	    return ret;
}
TT make_ptr_node(TT next){
	TT ret;
	ret = (TT) malloc(sizeof(TYPE_TREE));
	if(ret == NULL)
		exit(1);
	ret->tag = POINTER;
	ret->next = next;
	return ret;
}


EN make_fn_call_node(EN identifier){
    if(identifier == NULL){
        return NULL;   
    }
    EN ret;
    ret = (EN) malloc(sizeof(EXPR_NODE));
    if(ret == NULL)
        exit(1);


    int junk;
    ST_DR fn_dr = st_lookup(identifier->u.var, &junk);
    if(fn_dr == NULL){
        error("could not find ST_DR");
        return NULL;
    }else{
    
    if(fn_dr->u.decl.type != identifier->type){
        error("Duplicate declaration for %s", st_get_id_str(identifier->u.var));
        error("Duplicate definition for %s", st_get_id_str(identifier->u.var));
    }

    ret->u.func_call.fn_id = identifier->u.var;
    PARAMSTYLE paramstyle;
    PARAM_LIST params;
    if(ty_query(fn_dr->u.decl.type ) != TYFUNC){ 
        error("expression not of function type"); 
        return NULL;
    }
    ret->type = ty_query_func(fn_dr->u.decl.type, &paramstyle, &params);
    ret->tag = FCALL;
    return ret;
    }
}


EN make_int_node(int val){
   EN ret;
   ret = (EN) malloc(sizeof(EXPR_NODE));
   if(ret == NULL)
        exit(1);
   ret->type = ty_build_basic(TYSIGNEDINT); 
   ret->tag = INT_CONST;
   ret->u.int_val = val;
   return ret;

}
EN make_double_node(double val){
   EN ret;
   ret = (EN) malloc(sizeof(EXPR_NODE));
   if(ret == NULL)
        exit(1);
    ret->type = ty_build_basic(TYDOUBLE);
    ret->tag = D_CONST;
    ret->u.double_val = val;
    return ret;
}

EN make_var_node(ST_ID var){
   EN ret;
   ret = (EN) malloc(sizeof(EXPR_NODE));
   if(ret == NULL)
        exit(1);
    int junk;
    ST_DR dr = st_lookup(var, &junk);
    if(dr == NULL){
        error("'%s' is undefined", st_get_id_str(var));
        // return NULL
    }else{
        ret->type = dr->u.decl.type;
    }
    ret->tag = VAR;
    ret->u.var = var;
    return ret;
}
EN make_unop_node(EN node, OP_TAG op){
    if(node == NULL)
        return NULL;
    EN ret;
    ret = (EN) malloc(sizeof(EXPR_NODE));
    if(ret == NULL) exit(1);
    
    if(node->tag == INT_CONST)
    {
        if(op == DEREF || op == PTR){
            error("illegal unary operation on constant operand");
            return NULL;
        }
        ret->type = node->type;
        ret->tag = node->tag;
        ret->u.int_val = -1*node->u.int_val;
    }else if(node->tag == D_CONST){
        if(op == DEREF || op == PTR){
            error("illegal unary operation on constant operand");
            return NULL;
        }
        ret->type = node->type;
        ret->tag = node->tag;
        ret->u.double_val = -1*node->u.double_val;
    }else if(node->tag == VAR){
        if(op == PTR){
            if(!(ty_query(node->type) == TYPTR || ty_query(node->type) == TYFUNC) ){
                error("indirection on nonpointer type");
                return NULL;
            }
           
            node = make_deref_node(node);
            
            // will fail if node->type is not a pointer
            TYPE_QUALIFIER junk;
            if(ty_query(node->type) == TYPTR){
                ret->type = ty_query_pointer(node->type, &junk);
            }else{
                error("left side of assignment is not an l-value");
                return NULL;
            }
            ret->tag = UNOP;
            ret->u.unop.op = PTR;
            ret->u.unop.child = node;
        }else if(op == DEREF){
             if((node->tag == INT_CONST && node->u.int_val !=0) || node->tag == D_CONST || node->tag == C_CONST){
                error("illegal unary operation on constant operand");
                return NULL;
             }
             ret->type = ty_build_ptr(node->type, NO_QUAL);
             ret->u.unop.op = op;
             ret->u.unop.child = node;
             ret->tag = UNOP;
        }   
    }else if(node->tag == UNOP){
         if(node->u.unop.op == DEREF && op == PTR){
            return node->u.unop.child;
         }
         if(op == PTR){
            if(ty_query(node->type) != TYPTR){
                error("indirection on nonpointer type");
                return NULL;
            }
            node = make_deref_node(node);
            
            // will fail if node->type is not a pointer
            TYPE_QUALIFIER junk;
            ret->type = ty_query_pointer(node->type, &junk);
            ret->tag = UNOP;
            ret->u.unop.op = PTR;
            ret->u.unop.child = node;
  
        }
    }else if(node->tag == BINOP && (op == DEREF || op == PTR)){
        if(op == DEREF){
        if(ty_query(node->type) != TYPTR){
            error("address operator requires function designator or l-value");
            return NULL;
        }
        }else{
            error("indirection on nonpointer type");
            return NULL;
        }
        
    }else{
        ret->type = node->type;
        ret->tag = UNOP;
        ret->u.unop.op = op;
        ret->u.unop.child = node;
    }
    return ret;

}
EN make_binop_node(EN left, EN right, OP_TAG binop){
    if(left == NULL || right == NULL)
        return NULL;
    EN ret;
    ret = (EN) malloc(sizeof(EXPR_NODE));
    if(ret == NULL)
        exit(1);
    


    // CONSTANT FOLDING
    // If the left and right are constant expressions, evaluate it at compile time
    double ret_val;
    if((left->tag == INT_CONST || left->tag == D_CONST) && (right->tag == INT_CONST || right->tag == D_CONST)){
        double left_val;
        double right_val;

        if(left->tag == INT_CONST)
            left_val = left->u.int_val;
        if(right->tag == INT_CONST)
            right_val = right->u.int_val;
        if(left->tag == D_CONST)
            left_val = left->u.double_val;
        if(right->tag == D_CONST)
            right_val = right->u.double_val;
        int cond_op = 0; 
        switch(binop){
            case ADD:
                ret_val = left_val + right_val;
                break;
            case SUB:
                ret_val = left_val - right_val;
                break;
            case MUL:
                ret_val = left_val * right_val;
                break;
            case DIV:
                ret_val = left_val / right_val;
                break;
            case MOD:
                ret_val = (int) left_val % (int) right_val;
                break;
            case ASGN:
                error("Left value is not LVAL");
                return NULL;
            case LT:
                ret_val = (int) (left_val < right_val);
                cond_op = 1;
                break;
            case LTE:
                ret_val = (int) (left_val <= right_val);
                cond_op = 1;
                break;
            case GT:
                ret_val = (int) (left_val > right_val);
                cond_op = 1;
                break;
            case GTE:
                ret_val = (int) (left_val >= right_val);
                cond_op = 1;
                break;
            case EQ:
                ret_val = (int) (left_val == right_val);
                cond_op = 1;
                break;
            case NEQ:
                ret_val = (int) (left_val != right_val);
                cond_op = 1;
                break;
            default:
                bug("Fed in wrong op type");
                break;
        }
        if(!cond_op){
            if(right->tag == INT_CONST || left->tag == INT_CONST){
                ret->type = ty_build_basic(TYSIGNEDINT);
                ret->tag = INT_CONST;
                ret->u.int_val = (int) ret_val;
            }else{
            if(right->tag == D_CONST){
                ret->type = right->type;
                ret->tag = right->tag;
                ret->u.double_val = ret_val;
            }
            else if(left->tag == D_CONST){ 
                ret->type = left->type;
                ret->tag = left->tag; 
                ret->u.double_val = ret_val;
            }
            else{
                ret->type = left->type;
                ret->tag = left->tag;
                ret->u.int_val = (int) ret_val;
            }}
        }else{
            ret->type = ty_build_basic(TYSIGNEDINT);
            ret->tag = INT_CONST;
            ret->u.int_val = (int) ret_val;
        }
        return ret;
    }
    // end constant folding 

    // deref variable nodes, except for lval
    if(left->tag == VAR){
        if(binop != ASGN){
            left = make_deref_node(left);
        }
    }
    if(right->tag == VAR){
        right = make_deref_node(right);

    }
    if((ty_query(right->type) == TYPTR || ty_query(left->type) == TYPTR) && (binop == MUL || binop == DIV)){
        error("illegal pointer operation");
        return NULL;
    }
    if(binop == ASGN && left->tag == UNOP && left->u.unop.op == DEREF){
        error("left side of assignment is not an l-value");
        return NULL;
    }
    if(binop == ASGN && ty_query(left->type) == TYPTR && ty_query(right->type) != TYPTR){
        if(right->u.int_val != 0){
            error("type mismatch in pointer conversion");
            return NULL;
            }
    }
    if(right->tag == UNOP && right->u.unop.op == PTR){
        right = make_deref_node(right);
    }
    
    if(binop != ASGN && left->tag == UNOP && left->u.unop.op == PTR){
        left = make_deref_node(left);
    }


    // These are not const nodes
    // add necessary conversion nodes where needed
    if(ty_test_equality(left->type, right->type)){
        // they are the same, so it doesn't matter
        if(binop > ASGN){
            ret->type = ty_build_basic(TYSIGNEDINT);
        }else{
            if(ty_query(left->type) == TYSIGNEDCHAR){
                left = make_conv_node(left, ty_build_basic(TYSIGNEDINT));
                right = make_conv_node(right, ty_build_basic(TYSIGNEDINT));
            }
        ret->type = left->type;  
        }
    }else{
         if(binop < ASGN){
            if(ty_query(left->type) == TYSIGNEDCHAR){
                left = make_conv_node(left, ty_build_basic(TYSIGNEDINT));
                }
             if(ty_query(right->type) == TYSIGNEDCHAR){
                right = make_conv_node(right, ty_build_basic(TYSIGNEDINT));
             }
             if(handle_conversion(left->type, right->type)){
                 if(ty_query(left->type) == TYFLOAT && (ty_query(right->type) == TYSIGNEDINT || ty_query(right->type) == TYSIGNEDCHAR)){
                    left = make_conv_node(left, ty_build_basic(TYDOUBLE));
                    right = make_conv_node(right, ty_build_basic(TYDOUBLE));
                 }else{
                    right = make_conv_node(right, left->type);
                 }
                 ret->type = left->type;
             }else{
                 ret->type = right->type;
                 left = make_conv_node(left, right->type);
             }
             
         }else{
            if(binop > ASGN || binop == LT || binop == GT){
                
                if(ty_query(left->type) == TYSIGNEDCHAR){
                    left = make_conv_node(left, ty_build_basic(TYSIGNEDINT));
                }
                
                if(ty_query(right->type) == TYSIGNEDCHAR){
                    right = make_conv_node(left, ty_build_basic(TYSIGNEDINT));
                }

                ret->type = ty_build_basic(TYSIGNEDINT);

                if(ty_query(left->type) == TYPTR && ty_query(right->type) == TYPTR){
                    if(!ty_test_equality(left->type, right->type)){
                        error("type mismatch in pointer operation");
                        return NULL;
                    }
                }
                if(ty_query(left->type) == TYPTR){
                    if(ty_query(right->type) == TYSIGNEDINT)
                        right = make_conv_node(right, left->type);
                }else if(ty_query(right->type) == TYPTR){
                    if(ty_query(left->type) == TYSIGNEDINT){
                        left = make_conv_node(left, right->type);
                    }
                } 
                if(ty_query(left->type) == TYFLOAT){
                    left = make_conv_node(left, ty_build_basic(TYDOUBLE));
                    ret->type = ty_build_basic(TYDOUBLE);
                }
                //if(ty_query(right->type) == TYFLOAT){
                if(ty_query(right->type) == TYFLOAT || ty_query(right->type) == TYSIGNEDINT && ty_query(left->type) != TYSIGNEDINT){
                    right = make_conv_node(right, ty_build_basic(TYDOUBLE));
                    ret->type = ty_build_basic(TYDOUBLE);
                }
            }else{
                if(ty_query(left->type) == TYPTR && ty_query(right->type) == TYPTR){
                    TYPE_QUALIFIER junk;
                    TYPE_QUALIFIER junk2;
                    if(ty_query_pointer(left->type, &junk) != ty_query_pointer(right->type, &junk2)){
                        error("type mismatch in pointer conversion");
                    }
                }
                if(ty_query(left->type) == TYPTR){
                    if(ty_query(right->type) != TYPTR && (right->tag == INT_CONST && right->u.int_val != 0)){
                            error("left side of assignment is not an l-value");
                        }
                    }
                
             ret->type = left->type;
             right = make_conv_node(right, left->type);
            }
         }
    }

   
    
    ret->tag = BINOP;
    ret->u.binop.op = binop;
    ret->u.binop.left = left;
    ret->u.binop.right = right;

    if(binop > ASGN && ty_query(ret->type) != TYSIGNEDINT){
        ret->type = ty_build_basic(TYSIGNEDINT);
    }
    return ret;
}



int handle_conversion(TYPE left, TYPE right){
    int cfloat = 0;
    int cdouble = 1;
    int cint = 2;
    int cchar = 3;
    int vleft = 0, vright = 0;
        
    switch(ty_query(left)){
        case TYFLOAT:
            vleft = 3;
            break;
        case TYDOUBLE:
            vleft = 4;
            break;
        case TYSIGNEDINT:
            vleft = 2;
            break;
        case TYSIGNEDCHAR:
            vleft = 1;
            break;
        default:
            bug("Undefined TYPE");
    }
    switch(ty_query(right)){
        case TYFLOAT:
            vright = 3;
            break;
        case TYDOUBLE:
            vright = 4;
            break;
        case TYSIGNEDINT:
            vright = 2;
            break;
        case TYSIGNEDCHAR:
            vright = 1;
            break;
        default:
            bug("Undefined TYPE");
    }

    return vleft > vright;


}

EN make_deref_node(EN node){
    if(node == NULL) return NULL;
    EN ret;
    ret = (EN) malloc(sizeof(EXPR_NODE));
    if(ret == NULL) exit(1);
   
    ret->type = node->type;
    ret->tag = DRF;
    ret->u.child = node;
    return ret;
}

EN make_conv_node(EN right, TYPE left_type){
    if(right == NULL) return NULL;
    EN ret;
    ret = (EN) malloc(sizeof(EXPR_NODE));
    if(ret == NULL)
        exit(1);
    ret->type = left_type;
    ret->tag = CONV;
    ret->u.child = right;
    return ret;
}
