#include "defs.h"
// #include "make_move.c"

#define GET_MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))

const int loop_slide_piece[8] = {wB, wR, wQ, 0, bB, bR, bQ, 0};
const int loop_slide_index[2] = {0, 4};

const int loop_non_slide_piece[6] = {wN, wK, 0, bN, bK, 0};
const int loop_non_slide_index[2] = {0, 3};

const int piece_dir[13][8] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

const int num_dir[13] = {0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};

const int victim_score[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int mvv_lva_scores[13][13];

int init_mvv_lva() {
	int attacker;
	int victim;
	for(attacker = wP; attacker <= bK; ++attacker) {
		for(victim = wP; victim <= bK; ++victim) {
			mvv_lva_scores[victim][attacker] = victim_score[victim] + 6 - ( victim_score[attacker] / 100);
		}
	}	
};

static void add_quiet_move(const Board *board, int move, Move_list *list){
    list->move_list[list->count].move = move;
    list->move_list[list->count].score = 0;
	if (board->search_killers[0][board->ply] == move){
		list->move_list[list->count].score = 90000;
	}
	else if (board->search_killers[1][board->ply] == move){
		list->move_list[list->count].score = 80000;
	}
	else{
		list->move_list[list->count].score = board->search_history[board->board[FROM_SQ(move)]][TO_SQ(move)];
	}

    list->count++;
}

static void add_capture_move(const Board *board, int move, Move_list *list){
    list->move_list[list->count].move = move;
    list->move_list[list->count].score = mvv_lva_scores[CAPTURED(move)][board->board[FROM_SQ(move)]] + 1000000;
    list->count++;
}

static void add_en_passant_move(const Board *board, int move, Move_list *list){
    list->move_list[list->count].move = move;
    list->move_list[list->count].score = 105 + 1000000;
    list->count++;
}

static void add_white_pawn_capture_move( const Board *board, const int from, const int to, const int cap, Move_list *list) {
	if(rank_board[from] == RANK_7) {
		add_capture_move(board, GET_MOVE(from,to,cap,wQ,0), list);
		add_capture_move(board, GET_MOVE(from,to,cap,wR,0), list);
		add_capture_move(board, GET_MOVE(from,to,cap,wB,0), list);
		add_capture_move(board, GET_MOVE(from,to,cap,wN,0), list);
	} else {
		add_capture_move(board, GET_MOVE(from,to,cap,EMPTY,0), list);
	}
}

static void add_white_pawn_move(const Board *board, const int from, const int to, const int cap, Move_list *list) {
	if(rank_board[from] == RANK_7) {
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,wQ,0), list);
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,wR,0), list);
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,wB,0), list);
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,wN,0), list);
	} else {
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

static void add_black_pawn_capture_move( const Board *board, const int from, const int to, const int cap, Move_list *list) {
	if(rank_board[from] == RANK_2) {
		add_capture_move(board, GET_MOVE(from,to,cap,bQ,0), list);
		add_capture_move(board, GET_MOVE(from,to,cap,bR,0), list);
		add_capture_move(board, GET_MOVE(from,to,cap,bB,0), list);
		add_capture_move(board, GET_MOVE(from,to,cap,bN,0), list);
	} else {
		add_capture_move(board, GET_MOVE(from,to,cap,EMPTY,0), list);
	}
}

static void add_black_pawn_move(const Board *board, const int from, const int to, const int cap, Move_list *list) {
	if(rank_board[from] == RANK_2) {
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,bQ,0), list);
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,bR,0), list);
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,bB,0), list);
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,bN,0), list);
	} else {
		add_quiet_move(board, GET_MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

void generate_all_moves(const Board *board, Move_list *list){
    
	list->count = 0;	
	
	int piece = EMPTY;
	int turn = board->turn;
	int sq = 0; int t_sq = 0;
	int piece_num = 0;
	int dir = 0;
	int index = 0;
	int piece_index = 0;
	
	if(turn == WHITE) {
		for(piece_num = 0; piece_num < board->piece_num[wP]; piece_num++) {
			sq = board->piece_list[wP][piece_num];
			
			if(board->board[sq + 10] == EMPTY) {
				add_white_pawn_move(board, sq, sq+10, EMPTY, list);
				if(rank_board[sq] == RANK_2 && board->board[sq + 20] == EMPTY) {
					add_quiet_move(board, GET_MOVE(sq, sq+20, EMPTY, EMPTY, PS_FLAG), list);
				}
			} 
			
			if(board->board[sq + 9] != OFF_BOARD && pieces_color[board->board[sq + 9]] == BLACK) {
				add_white_pawn_capture_move(board, sq, sq+9, board->board[sq + 9], list);
			}  
			if(board->board[sq + 11] != OFF_BOARD && pieces_color[board->board[sq + 11]] == BLACK) {
				add_white_pawn_capture_move(board, sq, sq+11, board->board[sq + 11], list);
			} 

			if (board->en_passant != NO_SQ){
				if(sq + 9 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq + 9, EMPTY, EMPTY, EP_FLAG), list);
				} 
				if(sq + 11 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq + 11, EMPTY, EMPTY, EP_FLAG), list);
				}
			}		
		}

		if(board->castle_perm & WKCA) {
			if(board->board[F1] == EMPTY && board->board[G1] == EMPTY) {
				if(!sq_attacked(E1,BLACK,board) && !sq_attacked(F1,BLACK,board) ) {
					add_quiet_move(board, GET_MOVE(E1, G1, EMPTY, EMPTY, CA_FLAG), list);
				}
			}
		}
		
		if(board->castle_perm & WQCA) {
			if(board->board[D1] == EMPTY && board->board[C1] == EMPTY && board->board[B1] == EMPTY) {
				if(!sq_attacked(E1,BLACK,board) && !sq_attacked(D1,BLACK,board) ) {
					add_quiet_move(board, GET_MOVE(E1, C1, EMPTY, EMPTY, CA_FLAG), list);
				}
			}
		}
	} 

    else {
		for(piece_num = 0; piece_num < board->piece_num[bP]; piece_num++) {
			sq = board->piece_list[bP][piece_num];
			
			if(board->board[sq - 10] == EMPTY) {
				add_black_pawn_move(board, sq, sq-10, EMPTY, list);
				if(rank_board[sq] == RANK_7 && board->board[sq - 20] == EMPTY) {
					add_quiet_move(board, GET_MOVE(sq, sq-20, EMPTY, EMPTY, PS_FLAG), list);
				}
			} 
			
			if(board->board[sq - 9] != OFF_BOARD && pieces_color[board->board[sq - 9]] == WHITE) {
				add_black_pawn_capture_move(board, sq, sq-9, board->board[sq - 9], list);
			}  
			if(board->board[sq- 11] != OFF_BOARD && pieces_color[board->board[sq - 11]] == WHITE) {
				add_black_pawn_capture_move(board, sq, sq-11, board->board[sq - 11], list);
			} 

			if (board->en_passant != NO_SQ){
				if(sq - 9 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq - 9, EMPTY, EMPTY, EP_FLAG), list);
				} 
				if(sq - 11 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq - 11, EMPTY, EMPTY, EP_FLAG), list);
				}
			}		
		}

		if(board->castle_perm & BKCA) {
			if(board->board[F8] == EMPTY && board->board[G8] == EMPTY) {
				if(!sq_attacked(E8, WHITE, board) && !sq_attacked(F8, WHITE, board) ) {
					add_quiet_move(board, GET_MOVE(E8, G8, EMPTY, EMPTY, CA_FLAG), list);
				}
			}
		}
		
		if(board->castle_perm & BQCA) {
			if(board->board[D8] == EMPTY && board->board[C8] == EMPTY && board->board[B8] == EMPTY) {
				if(!sq_attacked(E8, WHITE, board) && !sq_attacked( D8, WHITE, board) ) {
					add_quiet_move(board, GET_MOVE(E8, C8, EMPTY, EMPTY, CA_FLAG), list);
				}
			}
		}
	}

    /* Loop for slide board */
	piece_index = loop_slide_index[turn];
	piece = loop_slide_piece[piece_index++];
	while( piece != 0) {

		for(piece_num = 0; piece_num < board->piece_num[piece]; piece_num++) {
			sq = board->piece_list[piece][piece_num];
			
			for(index = 0; index < num_dir[piece]; ++index) {
				dir = piece_dir[piece][index];
				t_sq = sq + dir;
				
				while(board->board[t_sq] != OFF_BOARD) {				
					// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
					if(board->board[t_sq] != EMPTY) {
						if( pieces_color[board->board[t_sq]] == turn ^ 1) {
							add_capture_move(board, GET_MOVE(sq, t_sq, board->board[t_sq], EMPTY, 0), list);
						}
						break;
					}	
					add_quiet_move(board, GET_MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
			}
		}
		piece = loop_slide_piece[piece_index++];
	}

    piece_index = loop_non_slide_index[turn];
	piece = loop_non_slide_piece[piece_index++];
	while( piece != 0) {		
		
		for(piece_num = 0; piece_num < board->piece_num[piece]; piece_num++) {
			sq = board->piece_list[piece][piece_num];
			
			for(index = 0; index < num_dir[piece]; ++index) {
				dir = piece_dir[piece][index];
				t_sq = sq + dir;

                if (board->board[t_sq] == OFF_BOARD){
                    continue;
                }
				
                if(board->board[t_sq] != EMPTY) {
                    if( pieces_color[board->board[t_sq]] == turn ^ 1) {
                        add_capture_move(board, GET_MOVE(sq, t_sq, board->board[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }	
                add_quiet_move(board, GET_MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
			}
		}
		piece = loop_non_slide_piece[piece_index++];
	}
}

void generate_all_captures(const Board *board, Move_list *list){
    
	list->count = 0;	
	
	int piece = EMPTY;
	int turn = board->turn;
	int sq = 0; int t_sq = 0;
	int piece_num = 0;
	int dir = 0;
	int index = 0;
	int piece_index = 0;
	
	if(turn == WHITE) {
		for(piece_num = 0; piece_num < board->piece_num[wP]; piece_num++) {
			sq = board->piece_list[wP][piece_num];
			
			if(board->board[sq + 9] != OFF_BOARD && pieces_color[board->board[sq + 9]] == BLACK) {
				add_white_pawn_capture_move(board, sq, sq+9, board->board[sq + 9], list);
			}  
			if(board->board[sq + 11] != OFF_BOARD && pieces_color[board->board[sq + 11]] == BLACK) {
				add_white_pawn_capture_move(board, sq, sq+11, board->board[sq + 11], list);
			} 

			if (board->en_passant != NO_SQ){
				if(sq + 9 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq + 9, EMPTY, EMPTY, EP_FLAG), list);
				} 
				if(sq + 11 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq + 11, EMPTY, EMPTY, EP_FLAG), list);
				}
			}		
		}
	} 

    else {
		for(piece_num = 0; piece_num < board->piece_num[bP]; piece_num++) {
			sq = board->piece_list[bP][piece_num];
			
			if(board->board[sq - 9] != OFF_BOARD && pieces_color[board->board[sq - 9]] == WHITE) {
				add_black_pawn_capture_move(board, sq, sq-9, board->board[sq - 9], list);
			}  
			if(board->board[sq- 11] != OFF_BOARD && pieces_color[board->board[sq - 11]] == WHITE) {
				add_black_pawn_capture_move(board, sq, sq-11, board->board[sq - 11], list);
			} 

			if (board->en_passant != NO_SQ){
				if(sq - 9 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq - 9, EMPTY, EMPTY, EP_FLAG), list);
				} 
				if(sq - 11 == board->en_passant) {
					add_en_passant_move(board, GET_MOVE(sq, sq - 11, EMPTY, EMPTY, EP_FLAG), list);
				}
			}		
		}
	}

    /* Loop for slide board */
	piece_index = loop_slide_index[turn];
	piece = loop_slide_piece[piece_index++];
	while( piece != 0) {

		for(piece_num = 0; piece_num < board->piece_num[piece]; piece_num++) {
			sq = board->piece_list[piece][piece_num];
			
			for(index = 0; index < num_dir[piece]; ++index) {
				dir = piece_dir[piece][index];
				t_sq = sq + dir;
				
				while(board->board[t_sq] != OFF_BOARD) {				
					// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
					if(board->board[t_sq] != EMPTY) {
						if( pieces_color[board->board[t_sq]] == turn ^ 1) {
							add_capture_move(board, GET_MOVE(sq, t_sq, board->board[t_sq], EMPTY, 0), list);
						}
						break;
					}	
					t_sq += dir;
				}
			}
		}
		piece = loop_slide_piece[piece_index++];
	}

    piece_index = loop_non_slide_index[turn];
	piece = loop_non_slide_piece[piece_index++];
	while( piece != 0) {		
		
		for(piece_num = 0; piece_num < board->piece_num[piece]; piece_num++) {
			sq = board->piece_list[piece][piece_num];
			
			for(index = 0; index < num_dir[piece]; ++index) {
				dir = piece_dir[piece][index];
				t_sq = sq + dir;

                if (board->board[t_sq] == OFF_BOARD){
                    continue;
                }
				
                if(board->board[t_sq] != EMPTY) {
                    if( pieces_color[board->board[t_sq]] == turn ^ 1) {
                        add_capture_move(board, GET_MOVE(sq, t_sq, board->board[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }	
			}
		}
		piece = loop_non_slide_piece[piece_index++];
	}
}

int move_exists(Board *board, const int move) {
	
	Move_list list[1];
    generate_all_moves(board, list);
      
    int move_num = 0;
	for(move_num = 0; move_num < list->count; ++move_num) {	
       
        if ( !make_move(board, list->move_list[move_num].move))  {
            continue;
        } 
		       
        take_move(board);
		if(list->move_list[move_num].move == move) {
			return True;
		}
    }

	return False;
}