#include	"CodeGenerator.h"

typedef struct variable {

	/* Think! what does a Variable contain? */
	char *var_name,*var_type;
	int var_adress,var_size;
	struct variable *var_next,*var_prev;
} Variable;

typedef struct symbol_table {

	/* Think! what does a symbol_table contain?
	 * table_adress_counter is for tracking last variable adress, we start with adress 5
	 * table_content is a pointer pointing to the head of the variables linked list */
	int table_size;
	int table_adress_counter;
	Variable *table_content;
} Symbol_table;

Symbol_table *table;
int label_counter=1;
/*
*	You need to build a data structure for the symbol table
*	I recommend to use linked list.
*
*	You also need to build some functions that add/remove/find element in the symbol table
*/

Variable* findVarByName(Variable *head,char *var_name){
	if(head==NULL){
		return NULL;
	}
	if(strcmp(head->var_name,var_name)==0) return head;
	return findVarByName(head->var_next,var_name);
}

Variable* findVarByAdress(Variable *head,char *var_adress){
	if(head==NULL){
		return NULL;
	}
	if(head->var_adress==var_adress) return head;
	return findVarByName(head->var_next,var_adress);
}

void insertVar(Variable *head,Variable *new_var){

	int i=0;
	Variable *tmp=head;
	if(new_var == NULL)
		return;

	if(head == NULL){
		if(table == NULL){
			table=(Symbol_table *)malloc(sizeof(Symbol_table));
		}
		table->table_content=new_var;
		return;
	}

	if(findVarByName(head,new_var->var_name) != NULL)
		return;
	for(i=0;i<table->table_size;i++){
		if(tmp->var_next==NULL)
			break;
		tmp=tmp->var_next;
	}
	if(tmp != NULL){
		tmp->var_next=new_var;
		new_var->var_prev=tmp;
	}
	return;
}

Variable * createVar(char *var_name,char *var_type,int var_size,int var_adress,Variable *var_next,Variable *var_prev){

	Variable *new_var;
	new_var=(Variable *)malloc(sizeof(Variable));
	if(new_var == NULL){
		printf("Couldn't allocate memory, variable was not created!");
		return NULL;
	}
	new_var->var_name=var_name;
	new_var->var_type=var_type;
	new_var->var_adress=var_adress;
	new_var->var_size=var_size;
	new_var->var_next=var_next;
	new_var->var_prev=var_prev;
	return new_var;
}

int removeVar(Variable *var_to_remove){
	if(var_to_remove == NULL)
		return -1;
	Variable *tmp_next=var_to_remove->var_next;
	Variable *tmp_prev=var_to_remove->var_prev;
	free(var_to_remove->var_name);
	free(var_to_remove->var_type);
	if(var_to_remove->var_adress == table->table_content->var_adress)
		table->table_content=NULL;
	free(var_to_remove);
	if(tmp_next != NULL)
		tmp_next->var_prev=tmp_prev;
	if(tmp_prev != NULL)
		tmp_prev->var_next=tmp_next;
	return 1;
}

/*
*	This recursive function is the main method for Code Generation
*	Input: treenode (AST)
*	Output: prints the Pcode on the console
*/
int  code_recur(treenode *root)
{
	if_node  *ifn;
	for_node *forn;
	leafnode *leaf;
	
    if (!root)
        return SUCCESS;

    switch (root->hdr.which){
		case LEAF_T:
			leaf = (leafnode *) root;
			switch (leaf->hdr.type) {
				case TN_LABEL:
					/* Maybe you will use it later */
					break;

				case TN_IDENT:
					/* variable case */
					/*
					*	In order to get the identifier name you have to use:
					*	leaf->data.sval->str
					*/
					break;

				case TN_COMMENT:
					/* Maybe you will use it later */
					break;

				case TN_ELLIPSIS:
					/* Maybe you will use it later */
					break;

				case TN_STRING:
					/* Maybe you will use it later */
					break;

				case TN_TYPE:
					/* Maybe you will use it later */
					break;

				case TN_INT:
					/* Constant case */
					/* 
					*	In order to get the int value you have to use: 
					*	leaf->data.ival 
					*/
					printf("ldc %d\n", leaf->data.ival);
					break;

				case TN_REAL:
					/* Constant case */
					/*
					*	In order to get the real value you have to use:
					*	leaf->data.dval
					*/
					printf("ldc %lf\n", leaf->data.ival);
					break;
			}
			break;

		case IF_T:
			ifn = (if_node *) root;
			switch (ifn->hdr.type) {

			case TN_IF:
				if (ifn->else_n == NULL) {
					/* if case (without else)*/
					code_recur(ifn->cond);
					printf("fjp label%d\n",label_counter);
					code_recur(ifn->then_n);
					printf("label%d:\n",label_counter);
					label_counter++;
				}
				else {
					/* if - else case*/ 
					code_recur(ifn->cond);
					printf("fjp label%d\n",label_counter);
					code_recur(ifn->then_n);
					printf("ujp label%d\n",label_counter+1);
					printf("label%d:\n",label_counter);
					code_recur(ifn->else_n);
					label_counter++;
					printf("label%d:\n",label_counter);
				}
				break;
				
			case TN_COND_EXPR:
				/* (cond)?(exp):(exp); */
				code_recur(ifn->cond);
				printf("fjp label%d\n",label_counter);
				code_recur(ifn->then_n);
				printf("ujp label%d\n",label_counter+1);
				printf("label%d:\n",label_counter);
				code_recur(ifn->else_n);
				label_counter++;
				printf("label%d:\n",label_counter);
				break;

			default:
				/* Maybe you will use it later */
				code_recur(ifn->cond);
				code_recur(ifn->then_n);
				code_recur(ifn->else_n);
			}
			break;

		case FOR_T:
			forn = (for_node *) root;
			switch (forn->hdr.type) {

			case TN_FUNC_DEF:
				/* Function definition */
				/* e.g. int main(...) { ... } */
				/* Look at the output AST structure! */
				code_recur(forn->init);
				code_recur(forn->test);
				code_recur(forn->incr);
				code_recur(forn->stemnt);
				break;

			case TN_FOR:
				/* For case*/
				/* e.g. for(i=0;i<5;i++) { ... } */
				/* Look at the output AST structure! */
				code_recur(forn->init);
				code_recur(forn->test);
				code_recur(forn->stemnt);
				code_recur(forn->incr);
				break;

			default:
				/* Maybe you will use it later */
				code_recur(forn->init);
				code_recur(forn->test);
				code_recur(forn->stemnt);
				code_recur(forn->incr);
			}
			break;

		case NODE_T:
			switch (root->hdr.type) {
				case TN_PARBLOCK:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;
				
				case TN_PARBLOCK_EMPTY:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;
					
				case TN_TRANS_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_FUNC_DECL:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_FUNC_CALL:
					/* Function call */
					if (strcmp(((leafnode*)root->lnode)->data.sval->str, "printf") == 0) {
						/* printf case */
						/* The expression that you need to print is located in */
						/* the currentNode->right->right sub tree */
						/* Look at the output AST structure! */
						code_recur(root->rnode->rnode);
						printf("print\n");
					}
					else {
						/* other function calls - for HW3 */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					break;

				case TN_BLOCK:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_ARRAY_DECL:
					/* array declaration - for HW2 */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_EXPR_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_NAME_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_ENUM_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_FIELD_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_PARAM_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_IDENT_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_TYPE_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_COMP_DECL:
					/* struct component declaration - for HW2 */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;
					
				case TN_DECL:
					/* structs declaration - for HW2 */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_DECL_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_DECLS:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_STEMNT_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_STEMNT:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_BIT_FIELD:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_PNTR:
					/* pointer - for HW2! */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_TYPE_NME:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_INIT_LIST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_INIT_BLK:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_OBJ_DEF:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_OBJ_REF:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_CAST:
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_JUMP:
					if (root->hdr.tok == RETURN) {
						/* return jump - for HW2! */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == BREAK) {
						/* break jump - for HW2! */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == GOTO) {
						/* GOTO jump - for HW2! */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					break;

				case TN_SWITCH:
					/* Switch case - for HW2! */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_INDEX: 
					/* call for array - for HW2! */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_DEREF:
					/* pointer derefrence - for HW2! */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_SELECT:
					/* struct case - for HW2! */
					if (root->hdr.tok == ARROW){
						/* Struct select case "->" */
						/* e.g. struct_variable->x; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else{
						/* Struct select case "." */
						/* e.g. struct_variable.x; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					break;

				case TN_ASSIGN:
					if(root->hdr.tok == EQ){
						/* Regular assignment "=" */
						/* e.g. x = 5; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == PLUS_EQ){
						/* Plus equal assignment "+=" */
						/* e.g. x += 5; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == MINUS_EQ){
						/* Minus equal assignment "-=" */
						/* e.g. x -= 5; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == STAR_EQ){
						/* Multiply equal assignment "*=" */
						/* e.g. x *= 5; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == DIV_EQ){
						/* Divide equal assignment "/=" */
						/* e.g. x /= 5; */
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					break;

				case TN_EXPR:
					switch (root->hdr.tok) {
					  case CASE:
					      /* you should not get here */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  break;

					  case INCR:
						  /* Increment token "++" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  break;

					  case DECR:
						  /* Decrement token "--" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  break;

					  case PLUS:
					  	  /* Plus token "+" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("add\n");
						  break;

					  case MINUS:
					  	  /* Minus token "-" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("sub\n");
						  break;

					  case DIV:
					  	  /* Divide token "/" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("div\n");
						  break;

					  case STAR:
					  	  /* multiply token "*" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("mul\n");
						  break;

					  case AND:
					  	  /* And token "&&" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("and\n");
						  break;

					  case OR:
					  	  /* Or token "||" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("or\n");
						  break;
						
					  case NOT:
					  	  /* Not token "!" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("not\n");
						  break;

					  case GRTR:
					  	  /* Greater token ">" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("grt\n");
						  break;

					  case LESS:
					  	  /* Less token "<" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("les\n");
						  break;
						  
					  case EQUAL:
					  	  /* Equal token "==" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("equ\n");
						  break;

					  case NOT_EQ:
					  	  /* Not equal token "!=" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("neq\n");
						  break;

					  case LESS_EQ:
					  	  /* Less or equal token "<=" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("leq\n");
						  break;

					  case GRTR_EQ:
					  	  /* Greater or equal token ">=" */
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("geq\n");
						  break;

					  default:
						code_recur(root->lnode);
						code_recur(root->rnode);
						printf("ldc\n");
						break;
					}
					break;

				case TN_WHILE:
					/* While case */
					printf("fjp lable%d\n",label_counter);
					code_recur(root->lnode);
					printf("fjp lable%d\n",label_counter+1);
					code_recur(root->rnode);
					printf("ujp lable%d\n",label_counter);
					label_counter++;
					printf("lable%d\n",label_counter);
					break;

				case TN_DOWHILE:
					/* Do-While case */
					printf("lable%d\n",label_counter);
					code_recur(root->rnode);
					code_recur(root->lnode);
					printf("fjp lable%d\n",label_counter+1);
					printf("ujp lable%d\n",label_counter);
					label_counter++;
					printf("lable%d\n",label_counter);
					break;

				case TN_LABEL:
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				default:
					code_recur(root->lnode);
					code_recur(root->rnode);
			}
			break;

		case NONE_T:
			printf("Error: Unknown node type!\n");
			exit(FAILURE);	
    }

    return SUCCESS;
}


/*
*	This function prints all the variables on your symbol table with their data
*	Input: treenode (AST)
*	Output: prints the Sumbol Table on the console
*/
void print_symbol_table(treenode *root) {
	printf("---------------------------------------\n");
	printf("Showing the Symbol Table:\n");
	/*
	*	add your code here
	*/
}
