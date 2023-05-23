#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <Windows.h>  //gotoxy() �Լ��� ����ϱ� ���� ���

#define Max_symbols			100  // Number of terminals and nonterminals must be smaller than this number.
#define Max_size_rule_table		200  //  actual number of rules must be smaller than this number.
#define Max_string_of_RHS	20	// �� �������� �ִ� ����
#define MaxNumberOfStates	200  // �ִ� ������ ������Ʈ ��.
/////

typedef struct tkt { // �ϳ��� ��ū�� ��Ÿ���� ����ü.
	int index;  // index : ��ū�� ������ ��Ÿ��, �� ��ū ��ȣ�� ����.
	char typ[16]; // ��ū �̸�.
	char sub_kind[3];  // relop ��ū�� ��� �ٽ� �����ϴµ� �̿��.
						 // NUM ��ū�� ���� ��������("in") �Ǽ�������("do") ����.
	int sub_data; // ID ��ū�� �ɺ����̺���Ʈ�� ��ȣ, ���� ���� ���� ������.
	double rnum;  // �Ǽ��� ���
	char str[100]; // �� ��ū�� �����ϴ� input string/input sentence ���� �κн�Ʈ��.
	char string_constant[100]; // ���ڿ������ ����
	char char_constant; // ���ڻ���� ����
} tokentype;

typedef struct ssym {
	 int kind ; // �ܸ�/��ܸ� ����. (0:�ܸ���ȣ, 1:��ܸ���ȣ)
	 int no ; // �ܸ�, ��ܸ� �� ��쿡�� �ɺ��� ��ȣ.
	 char str[30]; // �ܸ���ȣ�� ��� get_next_token �Լ��� lexan ���� ���� token�� ǥ�� ��Ʈ���� ���⿡ ������.
} sym ;  // �����ɺ� ����

// �� �ϳ��� ��Ÿ���� ����ü.
typedef struct struc_rule {  // rule �� ���� ����.
    sym leftside ;
    sym rightside [Max_string_of_RHS] ;
    int  rleng ;  // ���� RHS �� �ɺ��� ��.  0 is given if righthand side is epsilon.
} ty_rule ;

typedef struct {
	int r, X, i, Yi;
} type_recompute;	// first ��꿡�� �̿��ϴ� ������

//  Item list���� ����� ����ü
typedef struct struc_itemnode* ty_ptr_item_node ;
typedef struct struc_itemnode {
	int		RuleNum ;
	int		DotNum ;
	ty_ptr_item_node	link ;
} ty_item_node ;

// state node : Goto graph���� ����� state node ����ü
typedef struct struc_state_node* ty_ptr_state_node;
typedef struct struc_state_node {
	int					id;				// �ڽ��� ID(��ȣ)
	int					item_cnt;		// �����ϰ� �ִ� Item ���� ��
	ty_ptr_item_node	first_item;		// �����ϰ� �ִ� Item_list �� �����ּ�
	ty_ptr_state_node	next;		// ���� state���� link
} ty_state_node;

typedef struct struc_arc_node* ty_ptr_arc_node;
typedef struct struc_arc_node {
	int		from_state;
	int		to_state;
	sym		symbol;
	ty_ptr_arc_node	link;
} ty_arc_node;

// types for goto graph
typedef struct struc_goto_graph* ty_ptr_goto_graph;
typedef struct struc_goto_graph {
	ty_ptr_arc_node		first_arc;				// ��ũ ����Ʈ�� ù ��带 ����Ŵ.
	ty_ptr_state_node	first_state;	// ������Ʈ ��� ����Ʈ�� ù ��带 ����Ŵ.
} ty_goto_graph;

typedef struct cell_action_tbl {
	char Kind ; // s, r, a, e �� �� ���� /
	int num ;	// s �̸� ������Ʈ ��ȣ, r �̸� �� ��ȣ�� ��Ÿ��.
} ty_cell_action_table ;

// �Ľ�Ʈ�� ��� ����. ���� �����ɺ��� ����. �׷���, �� ���� LR stack ���� ����.
// �׷��� LR stack ���� state ��ȣ�� ���� ��. �׷��� node �� �����ɺ��� state �� �ִ� 2 ���� �뵵�� �����.
typedef struct struc_tree_node* ty_ptr_tree_node;
typedef struct struc_tree_node { //
	sym	nodesym;	// �ܸ� ��ȣ�� ��ܸ� ��ȣ�� ����.
	int	child_cnt;	// number of children
	ty_ptr_tree_node	children[10]; // �ڽ� ���鿡 ���� ������.
	int rule_no_used; // ��ܸ���ȣ ����� ��� �� ��带 ����µ� ���� ���ȣ.
}  ty_tree_node;

// LR �ļ��� ����ϴ� stack �� �� ����.
// state ������ �����ٸ�: state >= 0 �̻�(&& symbol_node �� NULL); �����ɺ� ������� state==-1 .
typedef struct struc_stk_element {
	int state;	// state ��ȣ�� ��Ÿ��.
	ty_ptr_tree_node symbol_node;	// �����ɺ��� ������ Ʈ����忡 ���� ������.
} ty_stk_element;

/////////// Function prototypes
ty_ptr_state_node	add_a_state_node_if_it_does_not_exist(ty_ptr_state_node State_Node_List_Header, ty_ptr_item_node To_list);
void	add_arc_if_it_does_not_exist(ty_ptr_arc_node* Arc_List_Header, int from_num, int to_num, sym Symbol);
ty_ptr_item_node closure(ty_ptr_item_node IS);
int		delete_and_free_items_in_item_list(ty_ptr_item_node item_list);
void	display_tree(ty_ptr_tree_node nptr, int px, int py, int first_child_flag, int last_child_flag, int root_flag);
void		first(sym X);
void	first_all();
void	first_of_string(sym alpha[], int length, int first_result[]);
int		find_arc_in_arc_list(ty_ptr_arc_node arc_list, int from_id, sym symbal);
void	fitemListPrint(ty_ptr_item_node IS, FILE* fpw);
int		follow_nonterminal(int idx_NT);
ty_ptr_item_node	getLastItem(ty_ptr_item_node it_list);
sym		get_next_token(FILE* fps);
ty_ptr_item_node	goto_function(ty_ptr_item_node IS, sym sym_val);
void gotoxy(int x, int y);
void	initialize_action_table(void);
void	initialize_goto_table(void);
int is_item_in_itemlist(ty_ptr_item_node item_list, ty_ptr_item_node item);
int is_nonterminal_in_recompute_list(int Y);
int iswhitespace(char c);
void	itemListPrint(ty_ptr_item_node IS);
int	length_of_item_list(ty_ptr_item_node IS);
tokentype lexan_mini_grammar(FILE* fps);
int lookup_keyword_tbl(char* str);
int lookup_symtbl(char* str);
int		lookUp_nonterminal(char* str);
int		lookUp_terminal(char* str);
void	make_action_table();
void	make_goto_table();
ty_ptr_arc_node	makeArcNode(void);
void	make_goto_graph(ty_ptr_item_node IS_0);
ty_ptr_state_node	makeStateNode(void);
int nonterminal_is_in_stack(int Y);
ty_ptr_tree_node	parsing_a_sentence(FILE* fps);
void	print_Action_Table(void);
void	printGotoGraph(ty_ptr_goto_graph gsp);
void	print_Goto_Table(void);
void	push_state(ty_stk_element LR_stack[], int state);
void	push_symbol(ty_stk_element LR_stack[], ty_ptr_tree_node node);
ty_stk_element pop();	// pop from LR parsing stack.
void	read_grammar(char[]);
int same_test_of_two_item_lists(ty_ptr_item_node list1, ty_ptr_item_node list2);

///////////   GLOBAL VARIABLES  /////////////////////////////////////////////////////////////////////////////////

int Max_terminal; //  $ �� ������ �� ������ �� �ܸ���ȣ�� ��.
int Max_nonterminal; // �� ������ ���� �� ��ܸ� ��ȣ �� (��: augmented starting nonterminal ����).
sym Nonterminals_list [Max_symbols];	// ��� ��ܸ���ȣ ���� �̸��� ����
sym Terminals_list [Max_symbols];		// ��� �ܸ���ȣ ���� �̸��� ����

int Max_rules; // �� ���� ȭ�Ͽ� ���ǵ� ��Ģ�� �� ��
ty_rule rules[Max_size_rule_table]; // ��Ģ ���� ���̺�

int First_table[Max_symbols][Max_symbols]; // actual region:  Max_nonterminal  X (MaxTerminals+2) : 2 is for epsilbon and done_first flag
int done_first[Max_symbols];	// ��ܸ���ȣ�� first ��� �Ϸ� �÷���
int Follow_table[Max_symbols][Max_symbols]; // actual region:  Max_nonterminal  X (MaxTerminals+1) : 1 is for done_first flag
int done_follow[Max_symbols];  // ��ܸ���ȣ�� follow ��� �Ϸ� �÷���

type_recompute recompute_list[500];    // ���������� ����Ʈ (first ��꿡��)
int num_recompute = 0; // recompute_list�� ����� �������� �� ��.
type_recompute a_recompute;

//first ��꿡�� �̿�ȴ� call history stack. -1 �� ���� ��. �ʱ�ȭ ���ص� �������.
int ch_stack[100] = { -1, };
int top_ch_stack = -1;		// ch_stack �� top.

// goto_graph  ����ü�� ���� ������ (�� ����ü�� arc list�� state list �� ������� ����)
ty_ptr_goto_graph	Header_goto_graph = NULL;
int total_num_states = 0; // the actual number of states of go-to graph
int total_num_arcs = 0; // the actual number of arcs

ty_cell_action_table	action_table [MaxNumberOfStates][Max_symbols]; // the number of columns actually used is Max_terminal
int	goto_table [MaxNumberOfStates][Max_symbols]; // the number of columns actually used is MaxNonerminal

ty_stk_element  LR_stack[1000]; // declaration of LR parsing stack
int top_LR_stack = -1; // top of stack.

ty_ptr_tree_node Root_parse_tree = NULL ; // pointer to the root node of the parse tree.
int last_y;	// �Ľ�Ʈ�� �׸��⿡�� ����ϴ� �������� (�ٷ� ���� ����� ����� y ��ǥ�� ����)

FILE *fps; // file pointer to the file having input string(input sentence)

void gotoxy(int x, int y) {
	COORD Pos = { x ,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int iswhitespace(char c){
	if (c == ' ' || c == '\n' || c == '\t')
		return 1;
	else
		return 0;
}


// mini grammar �� ���� lexical analyzer.
// input sentence ���� ��ū�� �ϳ� �ν��Ͽ� ��ȯ�Ѵ�.
// input sentence ���� ��ū���� 1�� �̻��� white space ���ڷ� ���еȴ�.
// ��ū�� ��Ÿ���� ��Ʈ���� ���Ǵ� grammar �� terminals list �� �ִ�.
// end of file �� ������ �Ŀ��� ��ū $ �� ��ȯ�ȴ�.
tokentype lexan_mini_grammar (FILE* fps) {
	tokentype token;
	int i = 0, j;
	char ch, str[50];

	// take out one string.
	ch = fgetc(fps);
	while (ch == ' ' || ch == '\n' || ch == '\t') // ��� ���� ch �� non white ���ڸ� ����.
		ch = fgetc(fps);

	while (ch != EOF && ch != ' ' && ch != '\n' && ch != '\t') {
		str[i] = ch;
		i++;
		ch = fgetc(fps);
	}
	str[i] = '\0';

	if (i == 0) {	// �� ���ڵ� �� ����. �� ���� eof �� ������ ��츸 ����.
		token.index = Max_terminal - 1; // $ ��ū (�� ������ �ܸ���ȣ�� �����)�� ��ȯ.
		strcpy(token.str, Terminals_list[Max_terminal - 1].str);
		return token;
	}
	else {
		// look up terminals list to decide terminal id number.
		for (j = 0; j < Max_terminal; j++) {
			if (strcmp(str, Terminals_list[j].str) == 0) {
				// j �� �ܸ���ȣ�� ������ȣ��
				token.index = j;
				strcpy(token.str, Terminals_list[j].str);
				return token;
			}
		}
		// ���� ��ū. �� ���� ȭ�� ����� ������!
		printf("�Է¹��忡 �ҹ� ��ū�� ����: %s", str);
		ch = getchar();
		token.index = -1;
		return token;
	}
} // lexan_mini_grammar

// ���� ��ū�� input sentence ���� ���� �ͼ�(�̸� ���� lexan_mini_grammar �� ȣ���Ѵ�),
// �̸� terminal �ɺ��� Ÿ���� ��ȯ�Ͽ� ��ȯ�Ѵ�.
sym get_next_token(FILE * fps)  {
	sym a_terminal_sym;
	tokentype a_tok; // the token produced by the  lexical analyer we developed before.
	a_tok = lexan_mini_grammar (fps);
	a_terminal_sym.kind = 0;  // this is a terminal symbol

	a_terminal_sym.no = a_tok.index ;	// �ܸ���ȣ�� ��ȣ�� ������ (lexan �� �˾� ������).

	strcpy(a_terminal_sym.str, a_tok.str);

	return a_terminal_sym;
} // get_next_token

ty_ptr_tree_node  parsing_a_sentence (FILE * fps)
{	int i, kind, temp_state, Finished = 0, State, RuleNo, RuleLeng;
	kind = 0;  // state �� push �� ���� ����.
	sym next_terminal; // lexical analyzer ���� ���� �ִ� ��ū ����Ÿ.
	ty_ptr_tree_node Root_parse_tree, new_node_ptr;
	ty_stk_element stk_element1, stk_element2;

	// ���ÿ� �� ó���� ������Ʈ 0 �� �־� ����.
	push_state(LR_stack, 0); // push state 0 to LR stack.
	next_terminal = get_next_token (fps); // ù ��ū�� �о� �´�.

	do	{
		State = LR_stack[top_LR_stack].state;	// ���� top ������ state �� �˾� ���� (��: pop ������ �ʰ�).

		// Action[state][a] �� ���� ���� �׼��� ���� �� ������.
		switch (action_table[State][next_terminal.no].Kind) 	{
			case 's':
				// shift action �� ����:  ���� ��ū(�ܸ���ȣ)���� ��带 ����� �������� shift �Ѵ�.
				new_node_ptr = (ty_ptr_tree_node)malloc(sizeof (ty_tree_node));
				new_node_ptr->nodesym = next_terminal; // shift �� �ܸ���ȣ�� ��Ʈ�� tree node ����.
				new_node_ptr->child_cnt = 0; new_node_ptr->children[0] = NULL;
				new_node_ptr->rule_no_used = -1;  // �͹̳� ���� �� �̿����� ����.
				push_symbol(LR_stack, new_node_ptr); // ���� ��ū�� �ش��ϴ� �ܸ���ȣ ��忡 ���� �����͸� ���ÿ� shift ��.
				// action table ���� ������ ���� state �� ���ÿ� �־�� ��
				temp_state = action_table[State][next_terminal.no].num;
				push_state(LR_stack, temp_state);
				next_terminal = get_next_token (fps); // ���� ��ū�� �о� ���´�.
				break;
			case 'r':
				// ���� LHS ��ܸ���ȣ�� �ش��ϴ� ��� �غ�.
				new_node_ptr = (ty_ptr_tree_node)malloc(sizeof (ty_tree_node));
				RuleNo = action_table[State][next_terminal.no].num;
				new_node_ptr->nodesym = rules[RuleNo].leftside;		// ���� ���� �ɺ��� ��Ʈ ���� �Ѵ�.
				// ���ÿ��� ��带 ������ �ڽĵ��� �غ���.
				RuleLeng = rules[RuleNo].rleng;
				for (i = 0; i < RuleLeng; i++)		{
					stk_element1 = pop(); // state �� �ϳ� ����.
					stk_element2 = pop(); // ���� �ɺ��� �ϳ� ����.
					// ���� �����ɺ��� �����ɺ��� �ڽ����� �Ŵܴ�.
					new_node_ptr->children[RuleLeng - 1 - i] = stk_element2.symbol_node ;
				}
				new_node_ptr->children[RuleLeng] = NULL;
				new_node_ptr->child_cnt = RuleLeng;
				new_node_ptr->rule_no_used = RuleNo;
				State = LR_stack[top_LR_stack].state; // ���� ���� ������ 2 �� ��ŭ�� ���Ҹ� ������ �� �� ������ ž�� ������Ʈ.
				// ���� ž�� state ���� ���� �����ɺ��� goto �� ������Ʈ�� �����´�.
				temp_state = goto_table[State][new_node_ptr->nodesym.no];
				push_symbol(LR_stack, new_node_ptr); // ����� �� ���� ���� ��ܸ� ��ȣ�� �ش��ϴ� ����� ���ÿ� ����.
				push_state(LR_stack, temp_state); // goto �� ������Ʈ��  ����.
				break;
			case 'a':
				// ���ÿ��� 0, S, State �� ��� �ִ� ��Ȳ. ��ü �Ľ�Ʈ���� ����� ����.
				Root_parse_tree = LR_stack[1].symbol_node; // ���� 1�� starting nonterminal �� ����.
				return Root_parse_tree;
				break;
			case 'e':
				printf("Error: Parser is attempting to access an error cell. \n");
				getchar();
		} // switch
	} while (1);
}// parsing_a_sentence

//   LR stack �� state �� ��Ÿ���� ���� ���Ҹ� ����. 2nd parameter �� ���� state ��ȣ��.
void push_state(ty_stk_element LR_stack[], int state)
{
	top_LR_stack++;
	LR_stack[top_LR_stack].state = state;
	LR_stack[top_LR_stack].symbol_node = NULL;
}

// LR stack �� �����ɺ� ����� �������� ����. ȣ��ÿ� �����ɺ��� ������ ��带 �غ��Ͽ� ���� �����͸� 2nd parameter �� ������.
void push_symbol(ty_stk_element LR_stack[], ty_ptr_tree_node tree_node)
{
	top_LR_stack++;
	LR_stack[top_LR_stack].symbol_node = tree_node;
	LR_stack[top_LR_stack].state = -1;
}

// LR stack ���� ���� �ϳ��� ������.
ty_stk_element pop()
{
	ty_stk_element element;
	if (top_LR_stack < 0)	{
		printf("Pop error. Empty stack.\n");
		getchar();
	}

	element = LR_stack[top_LR_stack];
	top_LR_stack--;
	return element;
} //pop

void	initialize_goto_table(void) {
	int i_0, i_1;

	for (i_0 = 0; i_0 < MaxNumberOfStates; i_0++){
		for (i_1 = 0; i_1 < Max_nonterminal; i_1++){
			goto_table[i_0][i_1] = -1;
		} // for : i_1
	} // for : i_0
} // initialize_goto_table ( )

void	make_goto_table() {
	ty_ptr_arc_node arc_curr;

	initialize_goto_table();

	// arc_list �� �����Ѵ�. ��ũ���� ��ܸ� ��ȣ�� ���̺��̸� �̸�
	// �̿��Ͽ� goto_table �� �ش� ���� ä���.
	arc_curr = Header_goto_graph->first_arc;
	while (arc_curr) {
		if (arc_curr->symbol.kind == 1) {	// arc �� ���̺��� ��ܸ���ȣ�̸�.
			goto_table[arc_curr->from_state][arc_curr->symbol.no] = arc_curr->to_state;
		}
		arc_curr = arc_curr->link;
	}

} // make_goto_table ( )

void	print_Goto_Table(void) {
	int		i_0, i_1;
	FILE*	file_ptr = NULL;

	file_ptr = fopen("goto_table.txt", "w");

	fprintf(file_ptr, "   \t");
	for (i_0 = 0; i_0 < Max_nonterminal; i_0++)
		fprintf(file_ptr, "%3s\t", Nonterminals_list [i_0].str );
	fprintf(file_ptr, "\n");

	for (i_0 = 0; i_0 < total_num_states; i_0++){
		fprintf(file_ptr, "%3d\t", i_0);
		for (i_1 = 0; i_1 < Max_nonterminal; i_1++){
			if (goto_table[i_0][i_1] != -1)
				fprintf(file_ptr, "%3d\t", goto_table[i_0][i_1]);
			else
				fprintf(file_ptr, " -1\t");
		} // for : i_1
		fprintf(file_ptr, "\n");
	} // for : i_0

	fclose(file_ptr);
} // print_Goto_Table ( )

void	print_Action_Table(void) {
	int		i_0, i_1;
	FILE*	file_ptr = NULL;

	file_ptr = fopen("action_table.txt", "w");

	fprintf(file_ptr, "   \t");
	for (i_0 = 0; i_0 < Max_terminal; i_0++)
		fprintf(file_ptr, "%2s\t", Terminals_list [i_0].str);
	fprintf(file_ptr, "\n");

	for (i_0 = 0; i_0 < total_num_states; i_0++){
		fprintf(file_ptr, "%3d\t", i_0);
		for (i_1 = 0; i_1 < Max_terminal; i_1++){
			fprintf(file_ptr, "%c", action_table[i_0][i_1].Kind);
			if (action_table[i_0][i_1].Kind == 's' || action_table[i_0][i_1].Kind == 'r')
				fprintf(file_ptr, "%2d\t",  action_table[i_0][i_1].num);
			else
				fprintf(file_ptr, "\t");

		} // for : i_1
		fprintf(file_ptr, "\n");
	} // for : i_0

	fclose(file_ptr);
} // print_Action_Table ( )

void	initialize_action_table(void) {
	int i_0, i_1;

	for (i_0 = 0; i_0 < total_num_states; i_0++){
		for (i_1 = 0; i_1 < Max_terminal; i_1++){
			action_table[i_0][i_1].Kind = 'e';
			action_table[i_0][i_1].num = 0;
		} // for : i_1
	} // for : i_0
} // initialize_action_table ( )

void	make_action_table() {
	// �������� ����
	int		to_state_id = -1;
	int		i_0 = 0;

	ty_ptr_state_node	next_state = Header_goto_graph->first_state;
	ty_ptr_item_node	curr_item = NULL;

	sym					symbol;

	// action table �ʱ�ȭ
	initialize_action_table();

	while (next_state) {
		curr_item = next_state->first_item;

		while (curr_item) {

			if (curr_item->DotNum < rules[curr_item->RuleNum].rleng) 	{ // ��Ʈ ������ �ɺ��� ������,
				symbol = rules[curr_item->RuleNum].rightside[curr_item->DotNum];	// �� �ɺ��� ������.
				if (!symbol.kind) {  // �׸��� �װ��� �ܸ���ȣ��� shift �۾��� ����.
					// shift �� �� state ��ȣ �˾ƿ�.
					to_state_id = find_arc_in_arc_list(Header_goto_graph->first_arc, next_state->id, symbol);

					if (to_state_id == -1) { // to ������Ʈ�� ����. (�� ���� ������ �Ͼ �� ����.)
						printf("Logic error at Make_Action ( 1 ) \n");
						getchar();
					}

					if (action_table[next_state->id][symbol.no].Kind == 'e') 	{// �ʱ�ȭ�� ��� �ִ� ����. shift action �� �ִ´�.
							action_table[next_state->id][symbol.no].Kind = 's';
							action_table[next_state->id][symbol.no].num = to_state_id;
					}	else { // ���� �̹� ��� ����.
						if (action_table[next_state->id][symbol.no].Kind == 's'
							&& action_table[next_state->id][symbol.no].num == to_state_id)	{ // �̹� ���� ������ ������ ��� ����
							// �� ���� �ٽ� ���� �ʰ� �׳� �Ѿ� ��. ������ �ƴ�.
						}
						else 	{// ���� ���� �ٸ� ������ �̹� ����. "LR0 �� �ƴϴ�"��� ������.
							printf("action table �� ���� 2 �� �̻��� ���� ä������ ��Ȳ�� ���� �߻�1.\n");
							getchar();
						}
					}
				} // if (!symbol.kind) ...
			}
			else {  // dot �� ���� �������� ��ġ�Ͽ� dot ������ �ɺ��� ���� �����.
				if (curr_item->RuleNum == 0)	{ // Augmented rule �� S' -> S. �� �� state �� �ִ� �����.
					if (action_table[next_state->id][Max_terminal - 1].Kind == 'e') { // ��� ������ accept �׼� �־���.
						action_table[next_state->id][Max_terminal - 1].Kind = 'a'; // Action[state][$] �� 'a' (accept) �� ����.
					}	else {
						printf("Logic error: action table�� a �� ���� ���� 2�� �ְ� �Ǵ� ��Ȳ �߻�. \n");
						getchar();
					}
				}	else	{// ��Ʈ�� �������� �������̹Ƿ� �̿� ���Ͽ� reduction �۾��� ������.
					for (i_0 = 0; i_0 < Max_terminal; i_0++){
						if (Follow_table[rules[curr_item->RuleNum].leftside.no][i_0]) 	{// ���� ���� ��ܸ���ȣ�� ��� follow �ɺ�����.
							if (action_table[next_state->id][i_0].Kind == 'e') {// ���� ��� ������, reduction �׼� �ִ´�.
								action_table[next_state->id][i_0].Kind = 'r';
								action_table[next_state->id][i_0].num = curr_item->RuleNum;
							}
							else 	{// �̹� �� ���� ä���� ������,
								if (action_table[next_state->id][i_0].Kind == 'r'
									&& action_table[next_state->id][i_0].num == curr_item->RuleNum) 	{
									// ���� ���� ������ ������ �����Ƿ� ������ �ƴϰ� �׳� �����ϰ� ������.
								}
								else 	{
									printf("action table �� ���� 2 �� �̻��� ���� ä������ ��Ȳ�� ���� �߻�2.\n");
									getchar();
								}
							}
						} // if : Follow_table[rules[curr_item->RuleNum].leftside.no][i_0]
					} // for : i_0
				} //else.
			} // else  ( rules[curr_item->RuleNum].rleng ==....

			curr_item = curr_item->link;
		} // while : curr_item

		// ���� state (node) �� �̵��Ѵ�.
		next_state = next_state->next;
	} // while : next_state

} // make_action_table ( )

// state node �ϳ��� �Ҵ� �޾Ƽ� �̿� ���� �����͸� ��ȯ��.
ty_ptr_state_node	makeStateNode(void){
	ty_ptr_state_node cur;

	cur = (ty_ptr_state_node)malloc(sizeof(ty_state_node));

	cur->id         = -1;
	cur->item_cnt   = 0;
	cur->first_item = NULL;
	cur->next  = NULL;

	return cur;
} // makeStateNode ( )

// arc node �ϳ��� �Ҵ� �޾Ƽ� �̿� ���� �����͸� ��ȯ��.
ty_ptr_arc_node	makeArcNode(void){
	ty_ptr_arc_node cur;

	cur = (ty_ptr_arc_node)malloc(sizeof(ty_arc_node));

	cur->from_state = -1;
	cur->to_state  = -1;
	cur->symbol.kind = -1;
	cur->symbol.no   = -1;
	cur->link = NULL;

	return cur;
} // makeArcNode ( )

// Find the number of items in item list IS
int	length_of_item_list(ty_ptr_item_node IS){
	int					cnt    = 0;
	ty_ptr_item_node	cursor = IS;

	while(cursor){
		cnt++;
		cursor = cursor->link;
	} // while : cursor

	return cnt;
} // length_of_item_list ( )

// �� ������ ����Ʈ�� ������ �׽�Ʈ�Ѵ� (����: �����۵��� ���ĵ��� �ʾҴٰ� ��).
// �����ϸ� 1, �ٸ��� 0 �� ��ȯ.
int same_test_of_two_item_lists ( ty_ptr_item_node list1,  ty_ptr_item_node list2 ) {
	int l1, l2 , p1_exists_in_list2;
	ty_ptr_item_node p1 = list1;
	ty_ptr_item_node p2;

	l1 = length_of_item_list(list1); l2 = length_of_item_list(list2);	// ����Ʈ ���� ������ ���� �˾� ��.
	if (l1 != l2)
		return 0 ;	// ���̰� �ٸ��� ������ �� ����.

	while( p1 ){
		// check if p1 exists in list2.
		p2 = list2 ; // take one item in list2.
		p1_exists_in_list2 = 0 ; // initially, assume that it does not exist.

		while(p2) { //try to find p1 in list2 by scanning list2 using p2.
			if(p1->RuleNum == p2->RuleNum && p1->DotNum == p2->DotNum){
				p1_exists_in_list2 = 1 ; // p1 is found in list2.
				break;
			}

			p2 = p2->link;
		} // while.

		if ( ! p1_exists_in_list2 )
			return 0 ; // the two lists are different since an item in list1 does not exist in list2.
		else
			p1 = p1->link;	// ���� ���������� ����.
	} // while.

	return 1;	// ��� test �� �����.  1 �� ��ȯ.
} //same_test_of_two_item_lists

//  To_item_list ��  �̹� �����ϴ� ��� state �� item list�� ���Ѵ�.
//  ���� ���� �߰ߵǸ�, �� �߰ߵ� state �� ��� �����͸� ��ȯ;
//  �߰��� �ȵǸ� To_item_list �� ������ ���ο� state �� ����� ���(state node �� ���� ����� �� �ڿ� ����)�ϰ�
//  �� �� state node �� ���� �����͸� ��ȯ.
ty_ptr_state_node	add_a_state_node_if_it_does_not_exist
		(ty_ptr_state_node State_Node_List_Header, ty_ptr_item_node To_item_list){
	int Number_Of_Items	= 0;	// ���ο� item list�� item ������ �����ϴ� ����
	ty_ptr_state_node		curr_state			= State_Node_List_Header;
	ty_ptr_state_node		prev_state		= NULL;
	ty_ptr_state_node		new_state_node	= NULL;

	Number_Of_Items = length_of_item_list(To_item_list);

	// To_item_list �� ��� state �� item list �� �޶�� ���ο� state �� �߰��Ѵ�.
	while(curr_state){
		// item ������ ���� ������ Ȯ����.
		if(curr_state->item_cnt != Number_Of_Items){  // cursor �� ����Ű�� �����̵��� ������ ����Ʈ�� ������ ���� �ٸ��Ƿ� ���� ������Ʈ�� ��.
			prev_state = curr_state;
			curr_state		= curr_state->next;
			continue;
		} // if : state node�� item list�� �����ϰ� �ִ� item�� ������ ��ġ ���� ������ Ȯ������ �ʾƵ� �������ϴ�.

		int is_same = same_test_of_two_item_lists ( curr_state->first_item, To_item_list) ; // �����ϸ� 1, �ƴϸ� 0 �� ����.
		if ( is_same ) { // curr_state �� �����۸���Ʈ�� To_item_list �� ������ ��Ȳ��.
			delete_and_free_items_in_item_list (To_item_list) ; // free all items in To_list.
			return curr_state ; // ������ ������ �Ǹ��� �� ������Ʈ ����� �����͸� ��ȯ�� ��.
		}

		// �� ������ ����Ʈ�� �ٸ� ������ �Ǹ��Ǿ����Ƿ� ���� ������Ʈ ���� Ŀ���� �ű�.
		prev_state = curr_state;
		curr_state = curr_state->next;
	} // while

	// state node list ��ü�� ���ص� ���� �����۸���Ʈ�� ���� �����̵� ��尡 �߰ߵ��� �ʾ���.
	// ���� ���ο� �����̵� ��带 ����� To_item_list �� ���̰� �� ��带 ������Ʈ ��� ����Ʈ�� �� �ڿ� �߰���.
	new_state_node = makeStateNode();	// ���ο� state node �� �Ҵ�.

	// state  node list �� ���� ���ο� state node �� �����մϴ�.
	new_state_node->item_cnt   = Number_Of_Items;
	new_state_node->first_item = To_item_list;
	new_state_node->id	= prev_state-> id + 1;	// state ��ȣ�� �ο�.
	new_state_node->next = NULL;
	prev_state->next	= new_state_node;	// �Ŵܴ�.

	// �������� total_num_states �� ũ�⸦ ���������ݴϴ�.
	total_num_states++;

	return new_state_node;	// To_item_list �� ���� state node �� ��ȯ.
} // add_a_state_node_if_it_does_not_exist ( )

// add an arc (from_num, to_num, Symbol) only if it is not in the arc list.
void	add_arc_if_it_does_not_exist(ty_ptr_arc_node *Arc_List_Header, int from_num, int to_num, sym Symbol ){
	ty_ptr_arc_node	Arc_curr, Arc_last = NULL ;
	ty_ptr_arc_node	Arc_new = NULL;
	int same_arc_exists = 0;

	Arc_new = makeArcNode();

	Arc_new->from_state = from_num;
	Arc_new->to_state  = to_num;
	Arc_new->symbol = Symbol;
	Arc_new->link = NULL;

	if (*Arc_List_Header == NULL) {	// ��ũ ����Ʈ�� ��� �ִ�.
		*Arc_List_Header = Arc_new; // ù ��ũ�� �� �޾� �ش�.
		total_num_arcs++;
		return;
	}

	Arc_curr = *Arc_List_Header ;
	while ( Arc_curr ) 		{
		if ( Arc_curr->from_state == from_num && Arc_curr->to_state == to_num
			&& Arc_curr->symbol.kind == Symbol.kind && Arc_curr->symbol.no == Symbol.no)	{
			same_arc_exists = 1;	// ������ ��ũ�� �̹� ������.
			break;
		}
		else {
			Arc_last = Arc_curr ;
			Arc_curr = Arc_curr->link ;
		}
	} // while

	if (!same_arc_exists) {
		Arc_last->link = Arc_new;	// ���ο� ��ũ�� �Ŵ޾� �ش�.
		total_num_arcs++;
	}
	else
		free(Arc_new);
} // add_arc_if_it_does_not_exist ( )

// Make a goto graph.
// �Է����� ���� 0 �� ������Ʈ�� �޴´�.
// �۾� ����� state node list �� arc node list �̴�.
// �Լ��� �� ������ �κп��� �̵鿡 ���� �����͸� �������� Header_goto_graph �� �ִ´�.
void	make_goto_graph(ty_ptr_item_node IS_0){
	// �������� ����
	int	i_0, i_1, i_max;
	sym sym_temp;
	ty_ptr_state_node		State_Node_List_Header	 = NULL;	// ������Ʈ ����Ʈ�� ���
	ty_ptr_arc_node			Arc_List_Header = NULL;	// ��ũ ����Ʈ�� ���
	ty_ptr_state_node		next_state	= NULL;		// ���� �۾����� �����̵� ��忡 ���� ������.
	ty_ptr_state_node		to_state	= NULL;	    // goto �� ���� ���� �� state ��忡 ���� ������.
	ty_ptr_item_node		To_item_list = NULL;	// goto �� ���� �˾� �� ���ο� item list.

	// ������Ʈ 0 �� �� state node list �� ù ��忡 �ֵ��� ��.
	State_Node_List_Header = makeStateNode();
	State_Node_List_Header->id = 0 ;
	State_Node_List_Header->item_cnt = length_of_item_list(IS_0) ;
	State_Node_List_Header->first_item = IS_0; // item list �� ù ��带 ����Ű�� ��.
	State_Node_List_Header->next = NULL ;
	total_num_states = 1 ;
	next_state = State_Node_List_Header ;

	while(next_state){
		// �ܸ���ȣ, ��ܸ���ȣ
		for(i_0 = 0 ; i_0 < 2; i_0++) { // i_0 = 0 �̸� �ܸ���ȣ�� ���� ó���̰�, 1 �̸� ��ܸ���ȣ�� ���� ó����.
			// i_max �� �� ���(�ܸ�/��ܸ�) �� ���� �� ��ȣ�� ���� ������ ��.
			i_max = i_0 ? Max_nonterminal : Max_terminal-1; // Max_terminal-1 �� ������ $ �ܸ���ȣ�� �����ϱ� ����.
			for(i_1 = 0; i_1 < i_max; i_1++){ // i_1 �� �ܸ�/��ܸ� �� ��쿡�� ��ȣ�� ��ȣ�� ��Ÿ��.
				// ���� �ɺ� �ϳ��� ����.
				sym_temp.kind = i_0;	// �ܸ�, ��ܸ� ����
				sym_temp.no   = i_1;	// symbol �� ������ȣ
				// ���� ������Ʈ���� �����ɺ� sym_temp �� ���� goto �� ������ ����Ʈ�� �˾� ��.
				To_item_list = goto_function(next_state->first_item, sym_temp);  // goto �Լ��� goto ����� �˾� ��.

				if(To_item_list){ // To_item_list �� empty �� �ƴϸ�, �̸� �� state �� �߰�.
					//  To_item_list �� ���ο� state �� ����Ѵ� (��, ������ ���� �������� �ʴ� ��쿡��).
					//  ���� �Լ��� �� �۾��� �ϰ�, To_item_list �� ������ state node �� �����͸� ��ȯ�Ѵ�.
					to_state = add_a_state_node_if_it_does_not_exist(State_Node_List_Header, To_item_list);

					// [next_state, to_state, sym_temp] ��ũ�� �̹� �������� ������ ���ο� ��ũ�� �߰���.
					add_arc_if_it_does_not_exist (&Arc_List_Header, next_state->id, to_state->id, sym_temp);
				} // if : To_item_list
			} // for : i_1 : ��� ��ȣ�� ���Ͽ� goto �� ����
		} // for : i_0 : �ܸ���ȣ �� ��ܸ���ȣ
		next_state = next_state->next;
	} // while : next_state

	// state node list �� arc list �� ����� ����. �̵��� ����ü�� �����ϰ� �� ����ü�� �����͸�
	// �������� Header_goto_graph �� �����Ѵ�.
	Header_goto_graph = (ty_ptr_goto_graph)malloc(sizeof(ty_goto_graph));
	Header_goto_graph->first_state = State_Node_List_Header;
	Header_goto_graph->first_arc = Arc_List_Header;
} // : make_goto_graph ( )

// Delete all items in the item_list.
int	delete_and_free_items_in_item_list(ty_ptr_item_node item_list){
	ty_ptr_item_node item_next   = NULL;
	ty_ptr_item_node curr_item = item_list->link;

	while(curr_item){
		item_next = curr_item->link;
		free(curr_item);
		curr_item = item_next;
	} // while : curr_item
	free(item_list);
	return 0;
} // delete_and_free_items_in_item_list ( )

// goto-graph �� ���Ͽ� ����Ѵ�. ���ϸ�: goto_graph.txt
void	printGotoGraph(ty_ptr_goto_graph gsp){
	ty_ptr_state_node		State_Node_List_Header = gsp->first_state;
	ty_ptr_arc_node		item_list  = gsp->first_arc;
	char str[10];
	FILE *fpw;
	fpw = fopen("goto_graph.txt", "w");

	while(State_Node_List_Header){
		fprintf(fpw, "ID[%2d] (%3d) : ", State_Node_List_Header->id, State_Node_List_Header->item_cnt);
		fitemListPrint(State_Node_List_Header->first_item, fpw);
		State_Node_List_Header = State_Node_List_Header->next;
	} // while : State_Node_List_Header
	printf("\nTotal number of states = %d\n", total_num_states);
	fprintf(fpw, "Total number of states = %d\n", total_num_states);

	fprintf(fpw, "\nGoto arcs:\nFrom   To  Symbol\n");
	while(item_list){
		fprintf(fpw, "%4d %4d    ", item_list->from_state, item_list->to_state);
		if (item_list->symbol.kind == 0)
			strcpy(str, Terminals_list[item_list->symbol.no].str );
		else
			strcpy(str, Nonterminals_list[item_list->symbol.no].str );
		fprintf(fpw, "%s \n", str);

		item_list = item_list->link;
	} // while : item_list
	printf("Total number of arcs = %d\n", total_num_arcs);
	fprintf(fpw, "Total number of arcs = %d\n", total_num_arcs);
	fclose(fpw);
} // printGotoGraph

// �Է�:  arc_list,  from state ��ȣ,  grammar symbol
// ��ũ ����Ʈ���� �־��� from_state, grammar symbol �� ��ũ�� ã�Ƽ� ������ to_state ��ȣ�� ��ȯ(������ -1).
int		find_arc_in_arc_list ( ty_ptr_arc_node arc_list, int from_id, sym asym) {
	ty_ptr_arc_node	curr = arc_list;

	while (curr) {
		if(curr->from_state == from_id && curr->symbol.kind == asym.kind && curr->symbol.no == asym.no )
			return curr->to_state;
		curr = curr->link ;
	} // while

	return -1;
} // find_arc_in_arc_list ( )

int lookUp_nonterminal( char *str) {
	int i ;
	for (i = 0; i < Max_nonterminal; i++ )
		if (strcmp(str, Nonterminals_list[i].str) == 0)
			return i;
	return -1;
}

int lookUp_terminal(char *str) {
	int i ;
	for (i = 0; i < Max_terminal ; i++)
		if (strcmp(str, Terminals_list[i].str) == 0)
			return i;
	return -1;
}

void read_grammar(char fileName[50]) {
	FILE *fps;
	char line[500]; // line buffer
	char symstr[10];
	char *ret;
	int i, k, n_sym, n_rule, i_leftSymbol, i_rightSymbol, i_right, synkind;
	fps = fopen(fileName, "r" );
	if (!fps) {
		printf("File open error of grammar file.\n");
		getchar(); // make program pause here.
	}

	ret = fgets(line, 500, fps); // ignore line 1
	ret = fgets(line, 500, fps); // ignore line 2
	ret = fgets(line, 500, fps); // read nonterminals line.
	i = 0; n_sym = 0;
	do { // read nonterminals.
		while (line[i] == ' ' || line[i] == '\t') i++; // skip spaces.
		if (line[i] == '\n') break;
		k = 0;
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
		{ symstr[k] = line[i]; i++; k++; }
		symstr[k] = '\0'; // a nonterminal string is finished.
		strcpy(Nonterminals_list[n_sym].str, symstr);
		Nonterminals_list[n_sym].kind = 1; // nonterminal.
		Nonterminals_list[n_sym].no = n_sym;
		n_sym++;
	} while (1);
	Max_nonterminal = n_sym;

	i = 0; n_sym = 0;
	ret = fgets(line, 500, fps); // read terminals line.
	do { // read terminals.
		while (line[i] == ' ' || line[i] == '\t') i++; // skip spaces.
		if (line[i] == '\n') break;
		k = 0;
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
		{ symstr[k] = line[i]; i++; k++; }
		symstr[k] = '\0'; // a terminal string is finished.
		strcpy(Terminals_list[n_sym].str, symstr);
		Terminals_list[n_sym].kind = 0; // terminal.
		Terminals_list[n_sym].no = n_sym;
		n_sym++;
	} while (1);
	Max_terminal = n_sym;
	printf("Number of terminals and nonterminals : %d,  %d\n", Max_terminal, Max_nonterminal);

	ret = fgets(line, 500, fps); // ignore one line.
	n_rule = 0;
	do { // reading rules.
		ret = fgets(line, 500, fps);
		if (!ret)
			break; // no characters were read. So reading rules is terminated.

		// if the line inputed has only white spaces, we should skip this line.
		// this is determined as follows: length==0; or first char is not an alphabet.
		i = 0;
		if (strlen(line) < 1)
			continue;
		else {
			while (line[i] == ' ' || line[i] == '\t') i++; // skip spaces.
			if ( ! isalpha(line[i]) )
				continue;
		}

		// take off left symbol of a rule.
		while (line[i] == ' ' || line[i] == '\t') i++; // skip spaces.
		k = 0;
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
		{ symstr[k] = line[i]; i++; k++; }
		symstr[k] = '\0'; // a nonterminal string is finished.
		i_leftSymbol = lookUp_nonterminal(symstr);
		if (i_leftSymbol < 0) {
			printf("Wrong left symbol of a rule.\n");
			getchar();
		}

		// left symbol is stored of the rule.
		rules[n_rule].leftside.kind = 1; rules[n_rule].leftside.no = i_leftSymbol;
		strcpy(rules[n_rule].leftside.str , symstr);

		// By three lines below, we move to first char of first sym of RHS.
		while (line[i] != '>') i++;
		i++;
		while (line[i] == ' ' || line[i] == '\t') i++;

		// Collect the symbols of the RHS of the rule.
		i_right = 0;
		do { // reading symbols of RHS
			k = 0;
			while ( i < strlen(line) && (line[i] != ' '
				&& line[i] != '\t' && line[i] != '\n') )
			{ symstr[k] = line[i]; i++; k++; }
			symstr[k] = '\0';
			if (strcmp(symstr, "epsilon") == 0) { // this is epsilon rule.
				rules[n_rule].rleng = 0; // declare that this rule is an epsilon rule.
				break;
			}

			if (isupper(symstr[0])) { // this is nonterminal
				synkind = 1;
				i_rightSymbol = lookUp_nonterminal(symstr);
			}
			else { // this is terminal
				synkind = 0;
				i_rightSymbol = lookUp_terminal(symstr);
			}

			if (i_rightSymbol < -1) {
				printf("Wrong right symbol of a rule.\n");
				getchar();
			}

			rules[n_rule].rightside[i_right].kind = synkind;
			rules[n_rule].rightside[i_right].no = i_rightSymbol;
			strcpy(rules[n_rule].rightside[i_right].str, symstr);

			i_right++;

			while (line[i] == ' ' || line[i] == '\t') i++;
			// ���� ���忡��: i >= strlen(line) is needed in case of eof just after the last line.
			if (i >= strlen(line) || line[i] == '\n')
				break; // finish reading  righthand symbols.
		} while (1); // loop of reading symbols of RHS.

		rules[n_rule].rleng = i_right;
		n_rule++;
	} while (1); // loop of reading rules.

	Max_rules = n_rule;
	printf("Total number of rules = %d\n", Max_rules);
} // read_grammar.

// return 1 if an item (2nd parameter) is in the item list (1st parameter). Otherwise, return 0.
int is_item_in_itemlist ( ty_ptr_item_node item_list, ty_ptr_item_node item ) {
	ty_ptr_item_node curr = item_list;

	while(curr){
		if(curr->RuleNum == item->RuleNum && curr->DotNum == item->DotNum)
			return 1;
		else
			curr = curr->link;
	} // while : cursor

	return 0;
} // is_item_in_itemlist ()

// return the pointer to the last item node of an item list.
ty_ptr_item_node	getLastItem ( ty_ptr_item_node it_list ) {
	ty_ptr_item_node curr = it_list;

	while(curr){
		if(!curr->link)
			return curr;
		else
			curr = curr->link;
	} // while

	return NULL;
} // getLastItem ()

void	itemListPrint(ty_ptr_item_node IS){
	int i_0;
	int r_num, d_num;

	ty_ptr_item_node cursor = IS;

	while(cursor){
		r_num = cursor->RuleNum;
		d_num = cursor->DotNum;

		printf("%s -> ", Nonterminals_list[rules[r_num].leftside.no].str );
		for(i_0 = 0; i_0 < rules[r_num].rleng; i_0++){
			if(i_0 == d_num)
				printf(". ");
			printf("%s ", rules[r_num].rightside[i_0].kind ? Nonterminals_list[rules[r_num].rightside[i_0].no].str :
				Terminals_list[rules[r_num].rightside[i_0].no].str );
		} // for : i_0
		if(i_0 == d_num)
			printf(".");
		if(!rules[r_num].rleng)
			printf("��");
		printf("     ");
		cursor = cursor->link;
	} // while : cursor
	printf("\n");
} // itemListPrint ( )

// print items of a item list into a file.
void	fitemListPrint(ty_ptr_item_node IS, FILE *fpw ){
	int i_0;
	int r_num, d_num;

	ty_ptr_item_node cursor = IS;

	while (cursor){
		r_num = cursor->RuleNum;
		d_num = cursor->DotNum;

		fprintf(fpw, "%s -> ", Nonterminals_list[rules[r_num].leftside.no].str);
		for (i_0 = 0; i_0 < rules[r_num].rleng; i_0++){
			if (i_0 == d_num)
				fprintf(fpw, ". ");
			fprintf(fpw, "%s ", rules[r_num].rightside[i_0].kind ? Nonterminals_list[rules[r_num].rightside[i_0].no].str :
				Terminals_list[rules[r_num].rightside[i_0].no].str);
		} // for : i_0
		if (i_0 == d_num)
			fprintf(fpw, ".");
		if (!rules[r_num].rleng)
			fprintf(fpw, "��");
		fprintf(fpw, "     ");
		cursor = cursor->link;
	} // while : cursor
	fprintf(fpw, "\n");
} // fitemListPrint ( )

// IS �� ����Ű�� item list �� closure �� ���Ͽ� IS �� Ȯ���Ѵ�.
// ����:  ��������� IS �� ����� �� �ִ�.
ty_ptr_item_node closure(ty_ptr_item_node IS) {
	ty_ptr_item_node	new_item = NULL;
	ty_ptr_item_node	curr = NULL;
	ty_ptr_item_node	last_item = NULL;

	sym		SymAfterDot;

	int	r_num, d_num;
	int	i_0;

	// ������ curr �� IS ��  ù ��带 ����Ű�� �Ѵ�.
	curr = IS;

	// Core Routine
	while (curr){
		// curr ����� rule ��ȣ
		r_num = curr->RuleNum;
		d_num = curr->DotNum;

		// SymAfterDot : type.sym { int kind / int no }
		if (d_num >= rules[r_num].rleng) {	// dot ������ �ƹ� �ɺ��� ���� ���.
			curr = curr->link;	// ���� item ���� �̵�.
			continue;
		} else
			SymAfterDot = rules[r_num].rightside[d_num];	// dot ������ �ɺ�

		if (!SymAfterDot.kind){		// �ܸ���ȣ�̸� �����ϰ� �Ѿ��.
			curr = curr->link;
			continue;
		}

		// SymAfterDot �� ��ܸ���ȣ�̴�. SymAfterDot �� �����ɺ��� ������ �긶�� �������� �߰��Ѵ�.
		for (i_0 = 0; i_0 < Max_rules; i_0++) {
			// �� i_0 �� ���� �ɺ��� SymAfterDot �� �ƴϸ� �� ���� �����Ѵ�.
			if (rules[i_0].leftside.no != SymAfterDot.no)
				continue;

			// item node �ϳ��� �����.
			new_item = (ty_ptr_item_node)malloc(sizeof(ty_item_node));

			// rule ��ȣ r, dot number = 0 �� ���⿡ �ִ´�.
			new_item->RuleNum = i_0;
			new_item->DotNum = 0;
			new_item->link = NULL;

			// new_item �� ���ݱ����� closure ����� �̹� �����ϸ� �߰����� �ʴ´�.
			if (is_item_in_itemlist(IS, new_item)){
				free(new_item);
				continue;
			} // if : is_item_in_itemlist ()

			//NewItemNodePtr �� IS �� �� �ڿ� ���δ�.
			last_item = getLastItem(IS);
			last_item->link = new_item;

		} // for : i_0
		curr = curr->link;
	} // while : curr

	return IS;
} // closure.

// Goto function. Compute the item list which can be reached from IS  by  sym_val.
ty_ptr_item_node	goto_function( ty_ptr_item_node IS, sym sym_val ){
	int r_num, d_num ;
	sym	SymAfterDot ;
	ty_ptr_item_node curr_item;
	ty_ptr_item_node New_Item;
	ty_ptr_item_node Go_To_Result_List  = NULL;
	ty_ptr_item_node i_cursor  = NULL;
	ty_ptr_item_node temp_item = NULL;

	curr_item = IS;
	while (curr_item) {		// �μ��� ���� item list IS ���� ��� item ���� ó���� �ش�.
		r_num = curr_item->RuleNum ;
		d_num = curr_item->DotNum ;

		if ( d_num >= rules[r_num].rleng ) 		{	// dot �� �� ���� ����. �� �������� ����.
			curr_item = curr_item->link;
			continue ;
		}

		SymAfterDot = rules[r_num].rightside[d_num];  // dot �ٷ� �� �ɺ�.

		// �� ������ �ɺ��� goto�� �ɺ��� �ٸ��� ���� ���������� �Ѿ� ��.
		if (!(SymAfterDot.kind == sym_val.kind && SymAfterDot.no == sym_val.no) ) 	{
			curr_item = curr_item->link;
			continue;
		}

		// curr_item �� A->apha . X beta �̰�, X == sym_val �̹Ƿ�,
		// A->apha X . beta �� �߰��ؾ� ��.

		New_Item = (ty_ptr_item_node)malloc(sizeof(ty_item_node)) ;	// �� item �� �����
		New_Item->RuleNum = r_num ;
		New_Item->DotNum  = d_num + 1 ;
		New_Item->link    = NULL ;

		// New_Item �� �̹� �����ϸ�, ���� �ʰ� �� ������ curr_item ó���� �����Ѵ�.
		if ( is_item_in_itemlist ( Go_To_Result_List, New_Item ) ){
			free(New_Item);
			curr_item = curr_item->link;
			continue;
		} // if : is_item_in_itemlist ()

		//NewItemNodePtr �� Go_To_Result_List �� �� �ڿ� ���δ�.
		if (Go_To_Result_List == NULL) {
			Go_To_Result_List = New_Item;
		} else {
			temp_item = getLastItem( Go_To_Result_List ) ;
			temp_item->link = New_Item;		// ������ item �� �ǵ��� ���δ�.
		} // if

		curr_item = curr_item->link;
	} // while

	if (Go_To_Result_List)
		return closure( Go_To_Result_List ) ;
	else
		return NULL;
} // goto_function ( )

// ��� ��ܸ���ȣ�� first �� ���Ѵ�. (������ ó���� �����Ͽ� case-2���� ó����.)
void first_all() {
	int i, j, r, m, A, k, n, Xno;
	sym X, Y;

	// ���� ��� ��ܸ���ȣ���� first �� ���Ͽ� First_table �� ����Ѵ�.
	for (i = 0; i < Max_nonterminal; i++) {
		X = Nonterminals_list[i];
		if (done_first[i] == 0) { // ���� �� ��ܸ���ȣ�� ó������ ����.
			top_ch_stack = -1; // ������ ��� ��. (�� ���� ����� ���ص� ��.)
			first(X);
		}
		if (top_ch_stack != -1) {
			printf("Logic error. stack top should be -1.\n");
			getchar();  // �ϴ� ���߰� ��.
		}
	} // for

	// ������ ó��
	int change_occurred;
	type_recompute recom;

	//printf("\nNumber of recomputation points = %d\n", num_recompute);
	while (1) {
		// ���� �ѹ� �� ��, ��� ���������� ó���Ѵ�.
		//printf("\nLoop of processing recompuation list starts.\n");
		change_occurred = 0;
		for (m = 0; m < num_recompute; m++) {
			recom = recompute_list[m];
			r = recom.r; Xno = recom.X; i = recom.i; A = recom.Yi;
			k = rules[r].rleng;
			//printf("\nrecomputation point to process: [%d, %s, %d, %s]\n", r, Nonterminals_list[Xno].str, i, Nonterminals_list[A].str);
			for (j = i; j < k; j++) {
				Y = rules[r].rightside[j];
				if (Y.kind == 0) {  // �ܸ���ȣ�̸�,
					if (First_table[Xno][Y.no] == 0) {
						change_occurred = 1;
						//printf("%s is added to first of %s in recomputing\n", Y.str, Nonterminals_list[Xno].str);
					}
					First_table[Xno][Y.no] = 1;
					break;  // ��Ģ r �� ó�� �����ϰ�, ���� ���������� ����.
				}
				// ���� ���� Y�� ��ܸ���ȣ��. Y �� first �� ��� X �� first�� ����Ѵ�(�ԽǷ� ����).
				for (n = 0; n < Max_terminal; n++) {
					if (First_table[Y.no][n] == 1) {
						if (First_table[Xno][n] == 0) {
							change_occurred = 1;
							//printf("%s in first of %s is added to first of %s in recomputing\n", Terminals_list[n].str, Y.str, Nonterminals_list[Xno].str);
						}
						First_table[Xno][n] = 1;
					}  // if
				}  // for(n=0
				if (First_table[Y.no][Max_terminal] == 0)	// Y �� first �� �ԽǷ��� ���ٸ�,
					break;  // Y ������ ����.
			} // for (j=i
			if (j == k) {  // �̰��� ���̸�, ���� ������ ��� �ɺ��� �ԽǷ��� ����.
				if (First_table[Xno][Max_terminal] == 0) {
					change_occurred = 1;
					//printf("epsilon is added to first of %s in recomputing by rule %d\n", Nonterminals_list[Xno].str, r);
				}
				First_table[Xno][Max_terminal] = 1;  // �ԽǷ��� �־� ��.
			} // if
		} // for (m=0
		if (change_occurred == 0)
			break;	// ������ ó������ ���� ��ȭ�� ������.
	} // while
	//printf("\nComputing first for all nonterminal symbols ends.\n");
} // end of first_all

// Is nonterminal Y in recompute list?
int is_nonterminal_in_recompute_list(int Y) {
	int i;
	for (i = 0; i < num_recompute; i++) {
		if (recompute_list[i].X == Y)
			return 1;
	}
	return 0;
}  // end of is_nonterminal_in_recompute_list

// is a nonterminal in ch_stack?
int nonterminal_is_in_stack(int Y) {
	int i;
	if (top_ch_stack >= 0) {
		for (i = 0; i <= top_ch_stack; i++)
			if (ch_stack[i] == Y)
				return 1;
	}
	return 0;
} // end of nonterminal_is_in_stack

//  first computation of one nonterminal with capability of dealing with case-2.
void first(sym X) {              // assume X is a nonterminal.
	int i, r, rleng, ie;
	sym Yi;

	top_ch_stack++;
	ch_stack[top_ch_stack] = X.no;  // push to the stack.
	//printf("first(%s) starts. \n", X.str);

	for (r = 0; r < Max_rules; r++) {
		if (rules[r].leftside.no != X.no)
			continue; // skip this rule since left side is not X.
		rleng = rules[r].rleng;
		if (rleng == 0) {
			First_table[X.no][Max_terminal] = 1; // �ԽǷ��� first ��  �߰�.
			//printf("epsilon is added to first of %s.\n", X.str);
			continue;  // ���� ��� ����.
		}

		//printf("Begin to use rule %d: %s -> ", r, rules[r].leftside.str);
		//for (ie = 0; ie < rleng; ie++) printf(" %s", rules[r].rightside[ie].str);
		//printf("\n");

		for (i = 0; i < rleng; i++) {
			//printf("Use the rule\'s right side symbol: %s\n", rules[r].rightside[i].str);
			Yi = rules[r].rightside[i];	// Yi �� ���� �������� ��ġ i �� �ɺ�
			if (Yi.kind == 0) {	// Yi is terminal
				First_table[X.no][Yi.no] = 1;	// Yi�� X�� first �� �߰�.
				//printf("%s is added to first of %s.\n", Yi.str, X.str);
				break;	// exit this loop to go to next rule.
			}
			// Now, Yi is nonterminal.
			if (X.no == Yi.no) {	// case-1 �߻���.
				//printf("Case 1  has occurred: rule: %d, at RHS position %d, Left symbol: %s\n", r, i, X.str);
				if (First_table[X.no][Max_terminal] == 1) {
					//printf("first of %s has epsilon. So symbols after %s in RHS is processed.\n", X.str, X.str);
					continue;  // epsilon �� first of X �� �����Ƿ� Yi �������� ó���ϰ� ��.
				}
				else
					//printf("first of %s does not have epsilon. So move to next rule.\n", X.str)
					;
				break; // epsilon �� first of X �� �����Ƿ� Yi�� ������ �����ϰ� ���� ��� ����.
			}

			if (done_first[Yi.no] == 1) {	// Yi �� first ����� �̹� ������.
				if (is_nonterminal_in_recompute_list(Yi.no)) {	// Yi �� �½ɺ��� �������� �����ϸ�,
					a_recompute.r = r; a_recompute.X = X.no; a_recompute.i = i; a_recompute.Yi = Yi.no;
					recompute_list[num_recompute] = a_recompute;	// ������ [r,X,i,Yi] �� �ִ´�.
					num_recompute++;
					//printf("������ �߰�:[%d,%s,%d,%s]\n\n", r, Nonterminals_list[X.no].str, i, Nonterminals_list[Yi.no].str);
				}
			}
			else {	// done of Yi == 0�̴�. �� Yi �� first ����� ���� ������.
				if (nonterminal_is_in_stack(Yi.no)) {	// Yi �� ch_stack �� �ִٸ�
					//printf("Case_3 occurrs: calling first(%s) cannot be done since it is in stack.\n", Yi.str);
					a_recompute.r = r; a_recompute.X = X.no; a_recompute.i = i; a_recompute.Yi = Yi.no;
					recompute_list[num_recompute] = a_recompute;	// ������ [r,X,i,Yi] �� �ִ´�.
					num_recompute++;
					//printf("������ �߰�:[%d,%s,%d,%s]\n\n", r, Nonterminals_list[X.no].str, i, Nonterminals_list[Yi.no].str);
				}
				else {	// Yi �� ch_stack �� ����.
					//printf("\nCall first(%s) in first(%s).\n", Yi.str, X.str);
					first(Yi);	// Yi �� first �� ����Ͽ� First_table �� �����.
					//printf("Return from first(%s) to first(%s). \n\n", Yi.str, X.str);
					if (is_nonterminal_in_recompute_list(Yi.no)) {	// Yi�� �½ɺ��� �������� �ִٸ�,
						a_recompute.r = r; a_recompute.X = X.no; a_recompute.i = i; a_recompute.Yi = Yi.no;
						recompute_list[num_recompute] = a_recompute;	// ������ [r,X,i,Yi] �� �ִ´�.
						num_recompute++;
						//printf("������ �߰�:[%d,%s,%d,%s]\n\n", r, Nonterminals_list[X.no].str, i, Nonterminals_list[Yi.no].str);
					}
				}  // else
			} // else
			// Yi �� first �� ���� �߿� epsilon �� �ƴ� �͵��� first_X �� �־��ش�.
			int n;
			for (n = 0; n < Max_terminal; n++) {
				if (First_table[Yi.no][n] == 1) {

					if (First_table[X.no][n] == 0) {
						First_table[X.no][n] = 1;
						//printf("%s in first of %s is added to first of %s.\n", Terminals_list[n].str, Yi.str, X.str);
					}
				}
			}
			// epsilon�� Yi �� first �� ���ٸ� ���� ��� ����. �ִٸ� Yi �� ���� �� �ɺ��� ����.
			if (First_table[Yi.no][Max_terminal] == 0) {
				//printf("Move to next rule since first of %s does not have epsilon.\n", Yi.str);
				break;
			}
		} // for (i=0
		// break �� �������� ����� �´ٸ� i != rleng �̴�. �׷��� �ʴٸ� i==rleng �̰�,
		// �� ���� r ��Ģ�� �������� ��� �ɺ��� first �� epsilon�� ������. ���� X �� ������ �Ѵ�.
		if (i == rleng) {
			First_table[X.no][Max_terminal] = 1;  // X �� first �� epsilon�� ������ �Ѵ�.
			//printf("epsilon is added to first of %s.\n", X.str);
		}
	} // for (r=0

	done_first[X.no] = 1;  // X �� done (first ���Ϸ�)�� 1 �� �Ѵ�.

	// pop stack.
	ch_stack[top_ch_stack] = -1;  // dummy ���� �ִ´�. ��� �� ���� ���ص� ��!
	top_ch_stack--;  // �� ���� ������ pop �� �ϴ� ����.
	//printf("first(%s) ends. \n", X.str);
} // end of first

// alpha is an arbitrary string of terminals or nonterminals.
// A dummy symbol with kind = -1 must be placed at the end as the end marker.
// length: number of symbols in alpha.
void first_of_string(sym alpha[], int length, int first_result[]) {
	int i, k;
	sym Yi;
	for (i = 0; i < Max_terminal + 1; i++)
		first_result[i] = 0; // initialize the first result of alpha

	// Let alpha be Y0 Y1 ... Y(length-1)

	for (i = 0; i < length; i++) {
		Yi = alpha[i];
		if (Yi.kind == 0) {  // Yi is terminal
			first_result[Yi.no] = 1;
			break;
		}
		else {  // ���� ���� Yi �� ��ܸ���ȣ�̴�.
			for (k = 0; k < Max_terminal; k++)	 // copy first of Yi to first of alpha
				if (First_table[Yi.no][k] == 1) first_result[k] = 1;
			if (First_table[Yi.no][Max_terminal] == 0)
				break; // first of Yi does not have epsilon. So forget remaining part.
		}
	} // for
	if (i == length)
		first_result[Max_terminal] = 1;  // epsilon �� �ִ´�.
} // end of function first_of_string


// This function computes the follow of a nonterminal with index X.
int follow_nonterminal(int X) {
	int i, j, k, m;
	int first_result[Max_symbols]; // one row of First table.
	int leftsym;
	sym SymString[Max_string_of_RHS];

	for (i = 0; i < Max_terminal + 1; i++)
		first_result[i] = 0; // initialization.

	for (i = 0; i < Max_rules; i++) {
		leftsym = rules[i].leftside.no;
		for (j = 0; j < rules[i].rleng; j++)
		{    //  the symbol of index j of the RHS of rule i is to be processed in this iteration
			if (rules[i].rightside[j].kind == 0 || rules[i].rightside[j].no != X)
				continue; // not X. so skip this symbol j.

			// Now, position j has a nonterminal which is equal to X.
			if (j < rules[i].rleng - 1) {  // there are symbols after position j in RHS of rule i.
				m = 0;
				for (k = j + 1; k < rules[i].rleng; k++, m++) SymString[m] = rules[i].rightside[k];
				SymString[m].kind = -1;  // end of string marker.
				first_of_string(SymString, m, first_result); // Compute the first of the string after position j of rule i.
																		   // the process result is received via first_result.
				for (k = 0; k < Max_terminal; k++) // Copy the first symbols of the remaining string(except eps) to the Follow of X.
					if (first_result[k] == 1)
						Follow_table[X][k] = 1;
			} // if

			if (j == rules[i].rleng - 1 || first_result[Max_terminal] == 1) // j is last symbol or first result has epsilon
			{
				if (rules[i].leftside.no == X)
					continue; // left symbol of this rule == X. So no need of addition.
				if (done_follow[leftsym] == 0) // the follow set of the left sym of rule i was not computed.
					follow_nonterminal(leftsym);		// compute it.
				for (k = 0; k < Max_terminal; k++) // add follow terminals of left side symbol to follow of X (without epsil).
					if (Follow_table[leftsym][k] == 1)
						Follow_table[X][k] = 1;
			}
		} // end of for j=0.
	} // end of for i
	done_follow[X] = 1;  // put the completion mark for this nonterminal.
	return 1;
} // end of function follow_nonterminal.

// �ļ��� ���� �Ľ�Ʈ���� ȭ�鿡 �׷� ����.
//  nptr: ����� subtree �� ��Ʈ���.
//  px, py:  ����� ����� �θ����� ��ǥ,
//  first_child_flag:  nptr ��尡 �θ��� ù �ڽ����� ����
//  last_child_flag: nptr ��尡 �θ��� ������ �ڽ����� ����
//  root_flag: nptr �� ��ü �Ľ�Ʈ���� ��Ʈ���� ����

void display_tree(ty_ptr_tree_node nptr, int px, int py, int first_child_flag, int last_child_flag, int root_flag) {
	int i, j, x, y;	// ���� ��� (nptr)�� ��µ� ��ġ
	int x_vert, y_start, y_end;

	//  �����ɺ��� ��µ� ��ǥ�� ���Ѵ�: (x, y)
	if (root_flag == 1) {	// root ����� ��쿡�� �θ� ��ġ�� �����ɺ��� ����Ѵ�.
		x = px; y = py;
	}
	else {	// root �� �ƴ� ���
		if (first_child_flag == 1) {	// �� ���� �θ��� ù �ڽ��̴�.
			x = px + 7;
			y = py;
		}
		else {
			x = px + 7;
			y = 2 + last_y;
		}
	}
	gotoxy(x, y);	// Ŀ���� �����.
	printf("%2s", nptr->nodesym.str);	// �����ɺ��� ����Ѵ�.
	last_y = y; // ���� �ֱٿ� ����� �ɺ��� y ��ǥ�� �����Ѵ�.

	if (root_flag == 0) { // ��Ʈ��尡 �ƴϸ� ����, ������ �׷��� �Ѵ�.
		// ���� �׸���
		if (first_child_flag == 1) {
			// �ڽ��� ������ - 5���� �׸���
			for (j = 1; j <= 5; j++) {
				gotoxy(x-j, y);
				printf("-");
			}
		}
		else {  // first �ڽ��� �ƴϸ� �ڽ��� ������ +-- �� �׸���
			for (j = 1; j <= 2; j++) {
				gotoxy(x - j, y);
				printf("-");
			}
			gotoxy(x - 3, y);
			printf("+");

		}
		// ù �ڽ��� �ƴ� ������ �ڽ��̸� ������ �׸���
		if (first_child_flag == 0 && last_child_flag == 1) {
			x_vert = x - 3;
			y_start = py + 1; y_end = y;
			for (j = y_start; j < y_end; j++) {
				gotoxy(x_vert, j);
				printf("|");
			}
			gotoxy(x_vert, y_end);
			printf("+");
		}

	}  // ����, ������ �׸���.

	// �ڽĵ��� �׸���
	int rno, rleng, first_flag, last_flag;
	rno = nptr->rule_no_used;
	rleng = rules[rno].rleng;
	if (nptr->nodesym.kind == 1) {	// �ܸ���ȣ ���� �ڽ��� �����Ƿ� �ڽ��� �׸��� �ʴ´�.
		if (rleng == 0) {  // �ڽ��� ���� ��ܸ� ����̴�. �� epsilon ���� �̿��� ����̴�.
			gotoxy(x + 2, y);
			printf("-----eps");
		}
		else {	// �ڽĵ��� ������ �̵��� ���� ����ϰ� �Ѵ�.
			for (j = 0; j < nptr->child_cnt; j++) {
				if (j == 0) first_flag = 1; else first_flag = 0;
				if (j == nptr->child_cnt - 1) last_flag = 1; else last_flag = 0;
				display_tree(nptr->children[j], x, y, first_flag, last_flag, 0);
			}
		}
	}
	return;
}	// display_tree

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	int i, j, base_x, base_y;
	sym a_nonterminal = {1, 0 };
	char grammar_file_name[50], input_file_name[50];

	//read_grammar("G_arith_no_LR.txt");
	//read_grammar("G_arith_with_LR.txt");
	//strcpy(grammar_file_name, "Grammar_data.txt");
	strcpy(grammar_file_name, "G_arith_with_LR.txt");
	//strcpy(grammar_file_name, "G_arith_no_LR.txt");
	//strcpy(grammar_file_name, "G_case1.txt");
	//strcpy(grammar_file_name, "G_arbi_1.txt");

	printf("\n������ ���Ϸκ��� �о� ���Դϴ�: ���ϸ�: %s \n", grammar_file_name);
	read_grammar(grammar_file_name);
	strcpy(Terminals_list[Max_terminal].str, "eps");  // �͹̳� epsilon �ɺ��� �̸��� �ο���(��¿� �̿��).

	// initialze First table.
	for (i = 0; i < Max_nonterminal; i++) {
		for (j = 0; j < (Max_terminal + 1); j++) {   // 0 ~ Max_terminal including epsilon.
			First_table[i][j] = 0;
		}
		done_first[i] = 0;
	}

	// initialze Follow table.
	for (i = 0; i < Max_nonterminal; i++) {
		for (j = 0; j < Max_terminal; j++) {  // 0 ~ (Maxterminal-1)
			Follow_table[i][j] = 0;
		}
		done_follow[i] = 0;
	}


	// Compute first of all nonterminals
	num_recompute = 0;  // ������ ����Ʈ�� ��� ���´�.
	first_all();

	// Compute follow of all nonterminals
	Follow_table[0][Max_terminal - 1] = 1; // make $ to be a follow symbol of the starting nonterminal.
										   // Index of $ is the last index which is (Max_terminal-1) !!

	for (i = 0; i < Max_nonterminal; i++) {
		if (done_follow[i] == 0) // if follow computation of nonterminal i was not done_first, compute it.
			follow_nonterminal(i);
	}

	// Print first of all nonterminals.
	for (i = 0; i < Max_nonterminal; i++) {
		printf("First(%s): ", Nonterminals_list[i].str);
		for (j = 0; j < Max_terminal; j++) {
			if (First_table[i][j] == 1)
				printf("%s  ", Terminals_list[j].str);
		}
		if (First_table[i][Max_terminal] == 1)
			printf(" eps");
		printf("\n");
	}

	// Print follow of all nonterminals.
	printf("\n");
	for (i = 0; i < Max_nonterminal; i++) {
		printf("Follow(%s): ", Nonterminals_list[i].str);
		for (j = 0; j < Max_terminal; j++) {
			if (Follow_table[i][j] == 1)
				printf("%s  ", Terminals_list[j].str);
		}
		printf("\n");
	}

	printf("\nFist, Follow ��� �۾��� ��Ĩ�ϴ�.\n\n");

	printf("Goto-graph �� ����ϴ�.\n\n");

	ty_ptr_item_node	ItemSet0, tptr;

	tptr = (ty_ptr_item_node)malloc(sizeof(ty_item_node));
	tptr->RuleNum = 0;
	tptr->DotNum = 0;
	tptr->link = NULL;

	ItemSet0 = closure(tptr);  // This is state 0.

	make_goto_graph(ItemSet0);
	printGotoGraph(Header_goto_graph);

	printf("goto-graph ����⸦ �����մϴ�(������ ���� goto_graph.txt  ���� Ȯ�� ������).\n");


	// �Ľ� ���̺� �����.
	make_action_table();  // action ���̺� �����.
	print_Action_Table();
	make_goto_table();  // goto ���̺� �����.
	print_Goto_Table();

	printf("�Ľ����̺� ����⸦ �����մϴ�:  action and goto tables.\n");


	//"source_arith_1.txt",  "source_arith.txt",  "source_G_case_1_1.txt",  "source_arbi_1.txt
	strcpy(input_file_name, "source_arith_2.txt");
	printf("�Ľ��� �Է¹����� ���� ���Ͽ��� �����ɴϴ�. ���ϸ�: %s\n", input_file_name);

	fps = fopen(input_file_name, "r");  // �Է¹����� ���� ������ fopen �Ѵ�.
	if (!fps) {
		printf("�Է������� ���Ⱑ �����Ͽ����ϴ�.\n");
		getchar();
	}

	// ���� ���� �Է¹��忡 ���Ͽ� LR Parsing �� �����Ѵ�.
	Root_parse_tree = parsing_a_sentence(fps);
	fclose(fps);

	printf("LR �Ľ��� ����Ǿ����ϴ�.\n");

	// �Ľ�Ʈ���� ȭ�鿡 �׸���.

	printf("������ �Ľ�Ʈ���� ����Դϴ�.\n\n");		// �� �� ���� ��� ������ �Ľ�Ʈ���� �׸���.

	// ���� Ŀ���� ��ǥ�� �˾� ����  (base_x, base_y) �� �����Ѵ�.
	CONSOLE_SCREEN_BUFFER_INFO presentCur;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &presentCur);
	base_x = presentCur.dwCursorPosition.X; // ���̽� ��ǥ�� x.
	base_y = presentCur.dwCursorPosition.Y; // ���̽� ��ǥ�� y.
	last_y = base_y;

	// ��ü �Ľ�Ʈ���� �׸���.
	display_tree(Root_parse_tree, base_x, base_y, 0, 0, 1);
	printf("\n\n");

	printf("���α׷��� �����մϴ�.\n");
} // main.