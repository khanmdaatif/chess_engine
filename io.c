#include <stdio.h>
#include "defs.h"

char *print_sq(const int sq) {
	
	static char sq_str[3];
	
	int file = file_board[sq];
	int rank = rank_board[sq];
	sprintf(sq_str, "%c%c", ('a'+file), ('1'+rank));
	
	return sq_str;
}

char *print_move(const int move) {

	static char move_str[6];
	
	int ff = file_board[FROM_SQ(move)];
	int rf = rank_board[FROM_SQ(move)];
	int ft = file_board[TO_SQ(move)];
	int rt = rank_board[TO_SQ(move)];
	
	int promoted = PROMOTED(move);
	
	if(promoted) {
		char pchar = 'q';
		if(piece_knight[promoted]) {
			pchar = 'n';
		} else if(piece_rook_queen[promoted] && !piece_bishop_queen[promoted])  {
			pchar = 'r';
		} else if(!piece_rook_queen[promoted] && piece_bishop_queen[promoted])  {
			pchar = 'b';
		}
		sprintf(move_str, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), pchar);
	} 
    else {
		sprintf(move_str, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));
	}
	return move_str;
}

int parse_move(char *ptr_char, Board *board) {

	if(ptr_char[1] > '8' || ptr_char[1] < '1') return NO_MOVE;
    if(ptr_char[3] > '8' || ptr_char[3] < '1') return NO_MOVE;
    if(ptr_char[0] > 'h' || ptr_char[0] < 'a') return NO_MOVE;
    if(ptr_char[2] > 'h' || ptr_char[2] < 'a') return NO_MOVE;

	// 10 * r + f + 21
    int from = (ptr_char[0] - 'a') + 10 * (ptr_char[1] - '1') + 21;
    int to = (ptr_char[2] - 'a') + 10 * (ptr_char[3] - '1') + 21;	
	
	// ASSERT(SqOnBoard(from) && SqOnBoard(to));
	// printf("%c %c %c %c\n", ptr_char[0], ptr_char[1], ptr_char[2], ptr_char[3]);
	// printf("%d %d", from, to);
	
	Move_list list[1];
	generate_all_moves(board,list);      
    int move_num = 0;
	int move = 0;
	int prom_piece = EMPTY;
	
	for(move_num = 0; move_num < list->count; move_num++) {	
		move = list->move_list[move_num].move;
		if(FROM_SQ(move) == from && TO_SQ(move) == to) {
			prom_piece = PROMOTED(move);
			if(prom_piece != EMPTY) {
				if(piece_rook_queen[prom_piece] && !piece_bishop_queen[prom_piece] && ptr_char[4]=='r') {
					return move;
				} 
				else if(!piece_rook_queen[prom_piece] && piece_bishop_queen[prom_piece] && ptr_char[4]=='b') {
					return move;
				} 
				else if (piece_rook_queen[prom_piece] && piece_bishop_queen[prom_piece] && ptr_char[4]=='q') {
					return move;
				} 
				else if (piece_knight[prom_piece] && ptr_char[4]=='n') {
					return move;
				}
				continue;
			}
			return move;
		}
    }
    return NO_MOVE;	
}

void print_move_list(const Move_list *list) {
	int index = 0;
	int score = 0;
	int move = 0;

	printf("MoveList:\n",list->count);
	
	for(index = 0; index < list->count; ++index) {
		move = list->move_list[index].move;
		score = list->move_list[index].score;
		
		printf("move:%d > %s (score:%d)\n",index+1,print_move(move),score);
	}
	printf("MoveList Total %d Moves:\n\n",list->count);
}