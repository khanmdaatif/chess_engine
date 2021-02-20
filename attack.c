#include "defs.h"

const int knight_dir[8] = { -8, -19,	-21, -12, 8, 19, 21, 12 };
const int rook_dir[4] = { -1, -10,	1, 10 };
const int bishop_dir[4] = { -9, -11, 11, 9 };
const int king_dir[8] = { -1, -10,	1, 10, -9, -11, 11, 9 };

int sq_attacked(const int sq, const int turn, const Board *board) {

	int piece,index,t_sq,dir;
	
	// pawns
	if(turn == WHITE) {
		if(board->board[sq-11] == wP || board->board[sq-9] == wP) {
			return True;
		}
	} 
    else {
		if(board->board[sq+11] == bP || board->board[sq+9] == bP) {
			return True;
		}	
	}

    // knights
    for(index = 0; index < 8; ++index) {		
		piece = board->board[sq + knight_dir[index]];
		if(piece_knight[piece] == True && pieces_color[piece] == turn) {
			return True;
		}
	}
	
	// rooks, queens
	for(index = 0; index < 4; ++index) {		
		dir = rook_dir[index];
		t_sq = sq + dir;
		piece = board->board[t_sq];
		while(piece != OFF_BOARD) {
			if(piece != EMPTY) {
				if(piece_rook_queen[piece] == True && pieces_color[piece] == turn) {
					return True;
				}
				break;
			}
			t_sq += dir;
			piece = board->board[t_sq];
		}
	}
	
	// bishops, queens
	for(index = 0; index < 4; ++index) {		
		dir = bishop_dir[index];
		t_sq = sq + dir;
		piece = board->board[t_sq];
		while(piece != OFF_BOARD) {
			if(piece != EMPTY) {
				if(piece_bishop_queen[piece] && pieces_color[piece] == turn) {
					return True;
				}
				break;
			}
			t_sq += dir;
			piece = board->board[t_sq];
		}
	}
	
	// kings
	for(index = 0; index < 8; ++index) {		
		piece = board->board[sq + king_dir[index]];
		if(piece_king[piece] == True && pieces_color[piece] == turn) {
			return True;
		}
	}
	
    return False;
}

