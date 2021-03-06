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
	Variable *table_content;
} Symbol_table;

Symbol_table *table=NULL;
int label_counter=0;
int if_label_counter=0;
int if_else_label_counter=0;
int while_loop_counter=0;
int do_while_loop_counter=0;
int for_loop_counter=0;
int switch_loop_counter=0;
int case_counter=0;
int table_adress_counter=5;
int Declaration_Flag=0;
int Dereference_Flag=0;
int EQ_Left_Flag=0;
int Array_D_Flag=0;
int Array_INDEX_Flag=0;	
int Array_Decl_Flag=0;	
int Decleration_Flag=0;	
int ixa=1;
/*
*	You need to build a data structure for the symbol table
*	I recommend to use linked list.
*
*	You also need to build some functions that add/remove/find element in the symbol table
*/

/*A function for finding a variable by var_name field - returns Variable* */
Variable* findVarByName(Variable *head,char *var_name){
	if(head==NULL){
		return NULL;
	}
	if(strcmp(head->var_name,var_name)==0) return head;
	return findVarByName(head->var_next,var_name);
}

/*void print_case(treenode *root,int case_num){
	leafnode *leaf;
	if(root == NULL)
		return;
	case TN_EXPR:
					switch (root->hdr.tok) {
					  case CASE:
	if(root->hdr.which == LEAF_T){
			leaf = (leafnode *) root;
			//////////////////////////LEAF CASES////////////////
			if(leaf->hdr.type == TN_LABEL) {

			}
	}
}*/

char* getVarFromTree(treenode *root){
		leafnode *leaf;
		char *tmp1,*tmp2;
		if(root == NULL)
			return NULL;

		if(root->hdr.which == LEAF_T){
			leaf = (leafnode *) root;
			//////////////////////////LEAF CASES////////////////
			if(leaf->hdr.type == TN_IDENT)
				return leaf->data.sval->str;
		}
		tmp1=getVarFromTree(root->lnode);
		tmp2=getVarFromTree(root->rnode);
		if((tmp1 != NULL && tmp2 != NULL) || (tmp1 == NULL && tmp2 == NULL))
			return NULL;
		else{
			if(tmp1 != NULL)
				return tmp1;
			else
				return tmp2;
		}

}
/*get a variable by var_adress field - returns Variable* */
/*Variable* findVarByAdress(Variable *head,char *var_adress){
	if(head==NULL){
		return NULL;
	}
	if(head->var_adress==var_adress) return head;
	return findVarByAdress(head->var_next,var_adress);
}*/


/*A function used to inserts Variables into the symbol table*/
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
		table->table_size++;
	}
	return;
}

/*function for creating a new var and filling all its fields*/
Variable * createVar(char *var_name,char *var_type,int var_size,Variable *var_next,Variable *var_prev){

	Variable *new_var;
	new_var=(Variable *)malloc(sizeof(Variable));
	if(new_var == NULL){
		printf("Couldn't allocate memory, variable was not created!");
		return NULL;
	}

	new_var->var_name=var_name;
	new_var->var_type=var_type;
	new_var->var_adress=table_adress_counter++;
	new_var->var_size=var_size;
	new_var->var_next=var_next;
	new_var->var_prev=var_prev;
	return new_var;
}

/*a function for removing a variable from the symbol table - returns 1/-1 to indecate success/failure*/
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

int is_const(treenode *root){
	int right_result=0,left_result=0;
	leafnode *leaf1,*leaf2;
	if_node  *ifn;
	if(root == NULL)
		return -1;
	if(root->hdr.which == NODE_T){
		leaf1 = (leafnode *)root->lnode;
		leaf2 = (leafnode *)root->rnode;
		switch (root->hdr.tok) {
					  case PLUS:
						  /* Plus token "+" */
						  left_result= is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result == -1 || right_result == -1)
							  return -1;
						  else
							  return right_result+left_result;

					  case MINUS:
						  /* Minus token "-" */
						  left_result= is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(right_result == left_result){
							  return 0;
						  }
						  if(left_result == -1 || right_result == -1)
							  return -1;
						  else
							  return left_result-right_result;

					  case DIV:
						  /* Divide token "/" */
						  left_result = is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(left_result == 0)
							  return 0;
						  else if(left_result == -1 || right_result == -1)
							  return -1;
						  else
							  return left_result/right_result;

					  case STAR:
						  /* multiply token "*" */
						  left_result = is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == 0 || left_result == 0){
							  return 0;
						  }else if(left_result == -1 || right_result == -1)
							  return -1;
						  else
							  return left_result*right_result;
					  case AND:
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == 0 || left_result ==0)
							  return 0;
						  else if(left_result == -1 || right_result == -1)
							  return -1;
						  else
							  return is_const(root->lnode)&&is_const(root->rnode);

					  case OR:
						  /* Or token "||" */
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == 1 || left_result == 1)
							  return 1;
						  else if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival||leaf2->data.ival;

					  case GRTR:
						  /* Greater token ">" */
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival>leaf2->data.ival;

					  case LESS:
						  /* Less token "<" */
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival<leaf2->data.ival;

					  case EQUAL:
						  /* Equal token "==" */
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival==leaf2->data.ival;
					  case NOT_EQ:
						  /* Not equal token "!=" */
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival != leaf2->data.ival;

					  case LESS_EQ:
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival <= leaf2->data.ival;
					  case GRTR_EQ:
						  /* Greater or equal token ">=" */
						  left_result= is_const(root->lnode);
						  right_result= is_const(root->rnode);
						  if(right_result == -1 || left_result == -1)
							  return -1;
						  else
							  return leaf1->data.ival >= leaf2->data.ival;
					  default:
						  return -1;
					}
	}else if(root->hdr.which == LEAF_T){
		if(root->hdr.type == TN_IDENT){
			return -1;
		}else {
			leaf1 = (leafnode *)root;
			return leaf1->data.ival;
		}
	}else if(root->hdr.which == IF_T){
		ifn = (if_node *) root;
		if(ifn->hdr.type ==TN_COND_EXPR) {
			if(is_const(ifn->cond) == 1)
				return 1;
			else if(is_const(ifn->cond) == 0)
					return 0;
			else return -1;
		}
	}
	return -1;

}


/*
*	This recursive function is the main method for Code Generation
*	Input: treenode (AST)
*	Output: prints the Pcode on the console
*/
int  code_recur(treenode *root)
{
	Variable *test ;
	char *tmp_name,*tmp_type,*name;
	if_node  *ifn;
	for_node *forn;
	leafnode *leaf;
	 Variable* t,*tmp;
	 int label1,label2;
	 int left_result=0,right_result=0;
	 if(table == NULL){
		 table=(Symbol_table *)malloc(sizeof(Symbol_table));
		 if(table == NULL){
			 printf("Couldn't allocate memory!\n");
			 exit(FAILURE);
		 }
	 }
    if (!root)
        return SUCCESS;
//////////////////////////////////////////WITCH NODE??///////////////////////
    switch (root->hdr.which){
		case LEAF_T:
			leaf = (leafnode *) root;
			//////////////////////////LEAF CASES////////////////
			switch (leaf->hdr.type) {
				case TN_LABEL:
					/* Maybe you will use it later */
					//printf("I'm at label <TN_LABEL> (L) line 158!\n");
					break;

				case TN_IDENT:
					/* variable case */
					/*
					*	In order to get the identifier name you have to use:
					*	leaf->data.sval->str
					*/
					//printf("I'm at Identifier (L)!! \n");
					tmp_name=leaf->data.sval->str;
					//printf("%s\n",tmp_name);
					////////////////////////Declaration Flag////////////////
					if ((Declaration_Flag)&&(!Array_Decl_Flag))
					{
						//printf("TN_IDEN >>>Dec Flag is on<<<\n");
					Variable *new_var=createVar(tmp_name,tmp_type,1,NULL,NULL);
					insertVar(table->table_content,new_var);
					}	
					if (Array_Decl_Flag){	
					tmp_name=leaf->data.sval->str;
					//printf("TN_IDEN >>>Dec Flag is off<<<\n");
					}
					///////////////////////Dereferance Flag//////////////////
					else if (Dereference_Flag){

				    test = findVarByName(table->table_content,leaf->data.sval->str);
                        if( test != NULL)
			            	{
                        	//printf("TN_IDEN >>>Deref Flag is on<<<\n");
		                     printf("ldc %d\n" ,test->var_adress);
		                     printf("ind\n");
			            	}
					}
					///////////////////////////Else Cases/////////////////////////
					else{
				    test = findVarByName(table->table_content,leaf->data.sval->str);
                        if( test != NULL)
			            {
                        	//printf("TN_IDEN >>>else<<<\n");
                        		if (!Declaration_Flag)	
                        		{	
                        			printf("ldc %d\n" ,test->var_adress);	
                        			//////EQ is on//////	
                        			if(!EQ_Left_Flag)	
                        			{	
                        				printf("ind\n");	
                        			}	
                        		}
			            }
						}
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
					//printf("I'm at Type (L) line 184!\n");
					/* Maybe you will use it later */
					break;

				case TN_INT:
					/* Constant case */
					/* 
					*	In order to get the int value you have to use: 
					*	leaf->data.ival 
					*/
					//printf("constant case INT (L)\n");
					if (!Array_Decl_Flag)	
						printf("ldc %d\n", leaf->data.ival);
					else	
					{	
						//int tmp_size=leaf->data.ival;	
						int tmp_size=ixa;	
						printf("tmp_size = %d\n",tmp_size);	
						for (int i=tmp_size;i>0;i--)	
						{	
						Variable *new_var=createVar(tmp_name,tmp_type,1,NULL,NULL);	
						insertVar(table->table_content,new_var);	
						}	
					}	
					if(Array_INDEX_Flag){
						printf("ixa ? \n");
						if (Array_INDEX_Flag==1)
							printf("dec 0\n");
						if (!EQ_Left_Flag)
							printf("ind\n");
					}
					break;

				case TN_REAL:
					/* Constant case */
					/*
					*	In order to get the real value you have to use:
					*	leaf->data.dval
					*/
					//printf("constant case REAL (L)\n");
					printf("ldc %f\n", leaf->data.dval);
					break;
			}
			break;

		case IF_T:
			ifn = (if_node *) root;
			switch (ifn->hdr.type) {

			case TN_IF:
				if (ifn->else_n == NULL) {
					/* if case (without else)*/
					if(is_const(ifn->cond) < 0){
						label1=if_label_counter++;
						code_recur(ifn->cond);
						printf("fjp if_label%d\n",label1);
						code_recur(ifn->then_n);
						printf("if_label%d:\n",label1);
						break;
					}else if(is_const(ifn->cond) == 0){
						break;
					} else {
						code_recur(ifn->then_n);
						break;
					}
				}else {
					/* if - else case*/ 
					if(is_const(ifn->cond) > 0){
						code_recur(ifn->then_n);
						break;
					}else if(is_const(ifn->cond) == 0){
						code_recur(ifn->else_n);
						break;
					}else{
						label1=if_else_label_counter++;
						code_recur(ifn->cond);
						printf("fjp if_else_label%d\n",label1);
						code_recur(ifn->then_n);
						printf("ujp if_else_end%d\n",label1);
						printf("if_else_label%d:\n",label1);
						code_recur(ifn->else_n);
						printf("if_else_end%d:\n",label1);
						break;
					}
				}
				
			case TN_COND_EXPR:
				/* (cond)?(exp):(exp); */
				if(is_const(ifn->cond) == 1)
				{
					code_recur(ifn->then_n);
					break;
				}
				if(is_const(ifn->cond) == 0){
					code_recur(ifn->else_n);
					break;
				}
				code_recur(ifn->cond);
				printf("fjp cond_else%d\n",label_counter);
				code_recur(ifn->then_n);
				printf("ujp condLabel_end%d\n",label_counter);
				printf("cond_else%d:\n",label_counter);
				code_recur(ifn->else_n);
				printf("condLabel_end%d:\n",label_counter);
				label_counter++;
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
				label1=for_loop_counter++;
				code_recur(forn->init);
				printf("for_loop%d:\n",label1);
				code_recur(forn->test);
				printf("fjp for_end%d\n",label1);
				code_recur(forn->stemnt);
				code_recur(forn->incr);
				printf("ujp for_loop%d\n",label1);
				printf("for_end%d:\n",label1);
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
					//printf("I'm at Function Call line 320!\n");
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
					//printf("I'm here at Block line 337!\n");
					/* Maybe you will use it later */
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_ARRAY_DECL:
					/* array declaration - for HW2 */
     					//printf("I'm at Array Declaration <TN_ARRAY_DECL> \n");
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_EXPR_LIST:
					//printf("I'm here at Expr lis line 363!\n");
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
					//printf("I'm at Type list line 398!\n");
					leaf = (leafnode*)root->lnode;
					switch(leaf->hdr.tok){
									case INT:
										tmp_type="int";
										//printf("int is good!\n");
										break;
									case FLOAT:
										tmp_type="float";
										//printf("float is good!\n");
										break;
									case DOUBLE:
										tmp_type="double";
										//printf("double is good!\n");
										break;
									default:
										tmp_type=NULL;
										//printf("NULL is good!\n");
										break;
												}
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_COMP_DECL:
					/* struct component declaration - for HW2 */
					//printf("I'm at SC Declaration <TN_COMP_DECL> line 458!\n");
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;
					
				case TN_DECL:
					Declaration_Flag=1;
					//printf("I'm at Declaration <TN_DEC> line 409!\n");
					code_recur(root->lnode);
					code_recur(root->rnode);
					Declaration_Flag=0;
					break;

				case TN_DECL_LIST:
					/* Maybe you will use it later */
					//printf("I'm at TN_DECL_LIST line 434!\n");
					Decleration_Flag=1;
					code_recur(root->lnode);
					code_recur(root->rnode);
					Decleration_Flag=0;
					break;

				case TN_DECLS:
					/* Declaration - Maybe you will use it later */
					//printf("I'm at TN_DECLS line 510!\n");
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_STEMNT_LIST:
					/* Maybe you will use it later */
					//printf("I'm at Statement List line 449!\n");
					code_recur(root->lnode);
					code_recur(root->rnode);
					break;

				case TN_STEMNT:
					//printf("I'm at Statement line 455!\n");
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
					//printf("I'm at Pointer line 467!\n");
					code_recur(root->lnode);
					//printf("ldc %d\n" ,test->var_adress);
					//printf("ind\n");
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
						//printf("I'm at RETURN \n");
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					else if (root->hdr.tok == BREAK) {
						/* break jump - for HW2! */
						//printf("I'm at BREAK \n");
						label1=switch_loop_counter;
						label2=case_counter-1;
						code_recur(root->lnode);
						code_recur(root->rnode);
						printf("ujp switch_end%d\n",--label1);
						printf("switch%d_case%d:\n",label1,label2);
					}
					else if (root->hdr.tok == GOTO) {
						/* GOTO jump - for HW2! */
						//printf("I'm at GoTO \n");
						code_recur(root->lnode);
						code_recur(root->rnode);
					}
					break;

				case TN_SWITCH:
					/* Switch case - for HW2! */
					/*if(is_const(root->lnode) >= 0){
						print_case(root->rnode);
						break;
					}*/
					label1=switch_loop_counter++;
					case_counter=0;
					code_recur(root->lnode);
					printf("switch%d_case%d:\n",label1,case_counter++);/*JK: in place*/
					code_recur(root->rnode);
					printf("switch_end%d:\n",label1);
					break;

				case TN_INDEX: 
					/* call for array - for HW2! */
					//printf("I'm at Call for Array <TN_INDEX> \n");
					Array_INDEX_Flag++;
					printf("Array Count: %d \n",Array_D_Flag);
					code_recur(root->lnode);
					printf("Array Count: %d \n",Array_D_Flag);
					code_recur(root->rnode);
					Array_INDEX_Flag--;
					//printf("ixa XX\n");


					break;

				case TN_DEREF:
					/* pointer derefrence - for HW2! */
					Dereference_Flag++;
					//printf("I'm at Dereference_Flag <TN_DEREF> line 600!\n");
					code_recur(root->lnode);
					code_recur(root->rnode);
					Dereference_Flag--;
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
					//printf("(=)\n");
					if(root->hdr.tok == EQ){
						/* Regular assignment "=" */
						/* e.g. x = 5; */
						//printf("EQ going left [<<<<<<] \n");
						//printf(">>>>>>> EQ_Left_Flag On <<<<<< \n");
						EQ_Left_Flag=1;
						code_recur(root->lnode);
						EQ_Left_Flag=0;
						//printf(">>>>>>> EQ_Left_Flag Off <<<<<< \n");
						//printf("EQ going Right [>>>>>>]\n");
						code_recur(root->rnode);
						printf("sto\n");
					}
					else if (root->hdr.tok == PLUS_EQ){
						/* Plus equal assignment "+=" */
						/* e.g. x += 5; */
						//printf("(+=)\n");
						if(is_const(root->rnode) == 0){
							break;
						 }
					     leaf = (leafnode*)root->lnode;
					     test = findVarByName(table->table_content,leaf->data.sval->str);
					       if( test != NULL)
					      		printf("ldc %d\n" ,test->var_adress);
						code_recur(root->lnode);
						code_recur(root->rnode);
						printf("add\n");
					        printf("sto\n");
					}
					else if (root->hdr.tok == MINUS_EQ){
						/* Minus equal assignment "-=" */
						/* e.g. x -= 5; */
						//printf("(-=)\n");
						if(is_const(root->rnode) == 0){
							break;
						}
					leaf = (leafnode*)root->lnode;
					test = findVarByName(table->table_content,leaf->data.sval->str);
					 if( test != NULL)
				        printf("ldc %d\n" ,test->var_adress );
				        code_recur(root->lnode);
				        code_recur(root->rnode);
				        printf("sub\n");
				        printf("sto\n");
					   
					}
					else if (root->hdr.tok == STAR_EQ){
						/* Multiply equal assignment "*=" */
						/* e.g. x *= 5; */
						//printf("(*=)\n");
						if(is_const(root->rnode) == 1){
							break;
						 }
						if(is_const(root->rnode) == 0){
							leaf = (leafnode*)root->lnode;
							test = findVarByName(table->table_content,leaf->data.sval->str);
							if( test != NULL)
								printf("ldc %d\nldc 0\nsto\n" ,test->var_adress);
							break;
						}
					    leaf = (leafnode*)root->lnode;
					    test = findVarByName(table->table_content,leaf->data.sval->str);
					    if( test != NULL)
					    	printf("ldc %d\n" ,test->var_adress);
	     		                     code_recur(root->lnode);
	     		                     code_recur(root->rnode);
				             printf("mul\n");
				             printf("sto\n");
					}
					else if (root->hdr.tok == DIV_EQ){
						/* Divide equal assignment "/=" */
						/* e.g. x /= 5; */
						//printf("(/=)\n");
						if(is_const(root->rnode) == 1){
							break;
						 }
			        	leaf = (leafnode*)root->lnode;
					test = findVarByName(table->table_content,leaf->data.sval->str);
					 if( test != NULL)
    			        	   printf("ldc %d\n" ,test->var_adress);
				           code_recur(root->lnode);
				           code_recur(root->rnode);
				           printf("div\n");
				           printf("sto\n");
					}
					break;

				case TN_EXPR:
					switch (root->hdr.tok) {
					  case CASE:
					      /* you should not get here */
						  label1=switch_loop_counter;
						  label2=case_counter++;
						  printf("dpl\n");
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("equ\n");
						  printf("fjp switch%d_case%d\n",--label1,label2);
						  break;

					  case INCR:
				       if(root->lnode)
					   {
			      		   leaf = (leafnode*)root->lnode;
						   test = findVarByName(table->table_content,leaf->data.sval->str);
						   if( test != NULL)
						   {
						   printf("ldc %d\n" ,test->var_adress);
						   printf("ind\n");
						   printf("ldc %d\n" ,test->var_adress);
						   printf("ldc %d\n" ,test->var_adress);
						   printf("ind\n");
						   printf("inc 1\n");
						   printf("sto\n");
						   }
                        }else{
				             leaf = (leafnode*)root->rnode;
				             test = findVarByName(table->table_content,leaf->data.sval->str);
				             if( test != NULL)
	                         {
	                          printf("ldc %d\n" ,test->var_adress);
					          printf("ldc %d\n" ,test->var_adress);
					          printf("ind\n");
						  printf("inc 1\n");
						  printf("sto\n");
						  printf("ldc %d\n" ,test->var_adress);
				       		  printf("ind\n");
                                }
			              }
						  break;

					  case DECR:
						  /* Decrement token "--" */
						  if(root->lnode)
						  {
					         leaf = (leafnode*)root->lnode;
					test = findVarByName(table->table_content,leaf->data.sval->str);
					 if( test != NULL)
						 {
						      printf("ldc %d\n" ,test->var_adress );
					              printf("ind\n");
					              printf("ldc %d\n" ,test->var_adress);
					              printf("ldc %d\n" ,test->var_adress);
					              printf("ind\n");
					              printf("dec 1\n");
					              printf("sto\n");
						  }
                                               }
					        else {
						leaf = (leafnode*)root->rnode;
						test = findVarByName(table->table_content,leaf->data.sval->str);
						 if( test != NULL)
					            {
					            	printf("ldc %d\n" ,test->var_adress);
					                printf("ldc %d\n" ,test->var_adress);
				                    	printf("ind\n");
                                    			printf("dec 1\n");
						        printf("sto\n");
					                printf("ldc %d\n" ,test->var_adress);
					                printf("ind\n");
					             }
			                          }
						  break;

					  case PLUS:
					  	  /* Plus token "+" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);

						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result+right_result);
							  break;
						  }
						  if(right_result == 0){
							  code_recur(root->lnode);
							  break;
						  }
						  if(left_result == 0){
							  code_recur(root->rnode);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("add\n");
						  break;

					  case MINUS:
					  	  /* Minus token "-" */
						if((leafnode*)root->lnode)
						  {
							  left_result = is_const(root->lnode);
							  right_result = is_const(root->rnode);
							  if(left_result >= 0 && right_result >= 0){
								  printf("ldc %d\n",left_result-right_result);
								  break;
							  }
							  if(right_result == 0 || left_result ==0){
								  if(right_result == 0){
									  code_recur(root->lnode);
									  break;
								  }else{
									  code_recur(root->rnode);
									  printf("neg\n");
									  break;
								  }
							  }
						     code_recur(root->lnode);
						     code_recur(root->rnode);
						     printf("sub\n");
						  }else {
							code_recur(root->rnode);
							printf("neg\n");
						  }
						  break;

					  case DIV:
					  	  /* Divide token "/" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result/right_result);
							  break;
						  }
						  if(left_result == 0 ){
							  printf("ldc 0\n");
							  break;
						  }
						  if(right_result == 1){
							  if(left_result >= 0)
								  printf("ldc %d\n",left_result);
							  else
								  printf("ldc %d\nind\n", findVarByName(table->table_content,getVarFromTree(root))->var_adress);

							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("div\n");
						  break;

					  case STAR:
					  	  /* multiply token "*" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result == 0 || right_result == 0){
							  printf("ldc 0\n");
							  break;
						  }
						  if(left_result > 0 && right_result > 0){
							  printf("ldc %d\n",left_result*right_result);
							  break;
						  }
						  if(left_result == 1 && right_result == 1){
							  printf("ldc 1\n");
							  break;
						  }
						  if(left_result == 1){
							  if( right_result >= 0){
								  printf("ldc %d\n",right_result);
								  break;
							  }else{
								  printf("ldc %d\nind\n",findVarByName(table->table_content,getVarFromTree(root))->var_adress);
								  break;
							  }
						  }
						  if(right_result == 1){
							  if( left_result >= 0){
								  printf("ldc %d\n",left_result);
								  break;
							  }else{
								  printf("ldc %d\nind\n",findVarByName(table->table_content,getVarFromTree(root))->var_adress);
								  break;
							  }
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("mul\n");
						  break;

					  case AND:
					  	  /* And token "&&" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result&&right_result);
							  break;
						  }
						  if(left_result == 0 || right_result == 0){
							  printf("ldc 0\n");
							  break;
						  }
						  if(left_result == 1 ){
							  printf("ldc %d\nind\n",findVarByName(table->table_content,getVarFromTree(root))->var_adress);
							  break;
						  }
						  if(right_result == 1){
							  printf("ldc %d\nind\n",findVarByName(table->table_content,getVarFromTree(root))->var_adress);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("and\n");
						  break;

					  case OR:
					  	  /* Or token "||" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result||right_result);
							  break;
						  }
						  if(left_result > 0 || right_result > 0){
							  printf("ldc 1\n");
							  break;
						  }
						  if(left_result == 0 ){
							  printf("ldc %d\nind\n",findVarByName(table->table_content,getVarFromTree(root))->var_adress);
							  break;
						  }
						  if(right_result == 0){
							  printf("ldc %d\nind\n",findVarByName(table->table_content,getVarFromTree(root))->var_adress);
							  break;
						  }
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
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result>right_result);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("grt\n");
						  break;

					  case LESS:
					  	  /* Less token "<" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result<right_result);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("les\n");
						  break;
						  
					  case EQUAL:
					  	  /* Equal token "==" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result == right_result);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("equ\n");
						  break;

					  case NOT_EQ:
					  	  /* Not equal token "!=" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result != right_result);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("neq\n");
						  break;

					  case LESS_EQ:
					  	  /* Less or equal token "<=" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result<=right_result);
							  break;
						  }
						  code_recur(root->lnode);
						  code_recur(root->rnode);
						  printf("leq\n");
						  break;

					  case GRTR_EQ:
					  	  /* Greater or equal token ">=" */
						  left_result = is_const(root->lnode);
						  right_result = is_const(root->rnode);
						  if(left_result >= 0 && right_result >= 0){
							  printf("ldc %d\n",left_result>=right_result);
							  break;
						  }
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
					label1=while_loop_counter++;
					printf("while_loop%d:\n",label1);
					code_recur(root->lnode);
					printf("fjp while_end%d\n",label1);
					code_recur(root->rnode);
					printf("ujp while_loop%d\n",label1);
					printf("while_end%d:\n",label1);
					break;

				case TN_DOWHILE:
					/* Do-While case */
					label1=do_while_loop_counter++;
					printf("do_while_loop%d:\n",label1);
					code_recur(root->rnode);
					code_recur(root->lnode);
					printf("fjp do_while_end%d\n",label1);
					printf("ujp do_while_loop%d\n",label1);
					printf("do_while_end%d:\n",label1);
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
	printf("---------------------------------------\n");
	Variable *tmp=table->table_content;
	printf("%s\n",tmp->var_name);
	while(tmp){
		printf("name: %s| type: %s| adress: %d| size: %d|\n",tmp->var_name,tmp->var_type,tmp->var_adress,tmp->var_size);
	}
	printf("---------------------------------------\n");


}
