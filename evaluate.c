// evaluate.c

#include <stdio.h>
#include "defs.h"

#define ENDGAME_MAT (1 * pieces_val[wR] + 2 * pieces_val[wN] + 2 * pieces_val[wP] + pieces_val[wK])

const int pawn_isolated = -10;
const int pawn_passed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 }; 
const int rook_open_file = 5;
const int rook_semi_open_file = 5;
const int queen_open_file = 5;
const int queen_semi_open_file = 3;
const int bishop_pair = 30;

const int pawn_table[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int knight_table[64] = {
0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0		
};

const int bishop_table[64] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int rook_table[64] = {
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0		
};

const int king_e[64] = {	
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	0	,	10	,	15	,	15	,	15	,	15	,	10	,	0	,
	0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
	0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
	0	,	10	,	15	,	15	,	15	,	15	,	10	,	0	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	
};

const int king_o[64] = {	
	0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,	-10	,
	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70		
};

int material_draw(const Board *board) {
    if (!board->piece_num[wR] && !board->piece_num[bR] && !board->piece_num[wQ] && !board->piece_num[bQ]) {
	  if (!board->piece_num[bB] && !board->piece_num[wB]) {
	      if (board->piece_num[wN] < 3 && board->piece_num[bN] < 3) {  return True; }
	  } 
	  else if (!board->piece_num[wN] && !board->piece_num[bN]) {
	     if (abs(board->piece_num[wB] - board->piece_num[bB]) < 2) { return True; }
	  } 
	  else if ((board->piece_num[wN] < 3 && !board->piece_num[wB]) || (board->piece_num[wB] == 1 && !board->piece_num[wN])) {
	    if ((board->piece_num[bN] < 3 && !board->piece_num[bB]) || (board->piece_num[bB] == 1 && !board->piece_num[bN]))  { return True; }
	  }
	} 
	
	else if (!board->piece_num[wQ] && !board->piece_num[bQ]) {
        if (board->piece_num[wR] == 1 && board->piece_num[bR] == 1) {
            if ((board->piece_num[wN] + board->piece_num[wB]) < 2 && (board->piece_num[bN] + board->piece_num[bB]) < 2)	{ return True; }
        } 
		else if (board->piece_num[wR] == 1 && !board->piece_num[bR]) {
            if ((board->piece_num[wN] + board->piece_num[wB] == 0) && (((board->piece_num[bN] + board->piece_num[bB]) == 1) || ((board->piece_num[bN] + board->piece_num[bB]) == 2))) { return True; }
        } 
		else if (board->piece_num[bR] == 1 && !board->piece_num[wR]) {
            if ((board->piece_num[bN] + board->piece_num[bB] == 0) && (((board->piece_num[wN] + board->piece_num[wB]) == 1) || ((board->piece_num[wN] + board->piece_num[wB]) == 2))) { return True; }
        }
    }
  return False;
}

int eval_position(const Board *board) {

	int piece;
	int piece_num;
	int sq;
	int score = board->material[WHITE] - board->material[BLACK];

	if(!board->piece_num[wP] && !board->piece_num[bP] && material_draw(board) == True) {
		return 0;
	}
	
	piece = wP;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score += pawn_table[SQ120_TO_SQ64[sq]];

		if( (isolated_mask[SQ120_TO_SQ64[sq]] & board->pawns[WHITE]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score += pawn_isolated;
		}
		
		if( (white_passed_mask[SQ120_TO_SQ64[sq]] & board->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			score += pawn_passed[rank_board[sq]];
		}
	}	

	piece = bP;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score -= pawn_table[mirror64[SQ120_TO_SQ64[sq]]];

		if( (isolated_mask[SQ120_TO_SQ64[sq]] & board->pawns[BLACK]) == 0) {
			//printf("bP Iso:%s\n",PrSq(sq));
			score -= pawn_isolated;
		}
		
		if( (black_passed_mask[SQ120_TO_SQ64[sq]] & board->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			score -= pawn_passed[7 - rank_board[sq]];
		}
	}	
	
	piece = wN;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score += knight_table[SQ120_TO_SQ64[sq]];
	}	

	piece = bN;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score -= knight_table[mirror64[SQ120_TO_SQ64[sq]]];
	}			
	
	piece = wB;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score += bishop_table[SQ120_TO_SQ64[sq]];
	}	

	piece = bB;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score -= bishop_table[mirror64[SQ120_TO_SQ64[sq]]];
	}	

	piece = wR;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score += rook_table[SQ120_TO_SQ64[sq]];
		if(!(board->pawns[BOTH] & file_mask[file_board[sq]])) {
			score += rook_open_file;
		} else if(!(board->pawns[WHITE] & file_mask[file_board[sq]])) {
			score += rook_semi_open_file;
		}
	}	

	piece = bR;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		score -= rook_table[mirror64[SQ120_TO_SQ64[sq]]];
		if(!(board->pawns[BOTH] & file_mask[file_board[sq]])) {
			score -= rook_open_file;
		} else if(!(board->pawns[BLACK] & file_mask[file_board[sq]])) {
			score -= rook_semi_open_file;
		}
	}

	piece = wQ;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		if(!(board->pawns[BOTH] & file_mask[file_board[sq]])) {
			score += queen_open_file;
		} else if(!(board->pawns[WHITE] & file_mask[file_board[sq]])) {
			score += queen_semi_open_file;
		}
	}	

	piece = bQ;	
	for(piece_num = 0; piece_num < board->piece_num[piece]; ++piece_num) {
		sq = board->piece_list[piece][piece_num];
		// ASSERT(SqOnBoard(sq));
		if(!(board->pawns[BOTH] & file_mask[file_board[sq]])) {
			score -= queen_open_file;
		} else if(!(board->pawns[BLACK] & file_mask[file_board[sq]])) {
			score -= queen_semi_open_file;
		}
	}
	
	piece = wK;
	sq = board->piece_list[piece][0];
	
	if(board->material[BLACK] <= ENDGAME_MAT) {
		score += king_e[SQ120_TO_SQ64[sq]];
	} 
	else {
		score += king_o[SQ120_TO_SQ64[sq]];
	}
	
	piece = bK;
	sq = board->piece_list[piece][0];
	
	if (board->material[WHITE] <= ENDGAME_MAT){
		score -= king_e[mirror64[SQ120_TO_SQ64[sq]]];
	} 
	else {
		score -= king_o[mirror64[SQ120_TO_SQ64[sq]]];
	}

	if(board->piece_num[wB] >= 2) score += bishop_pair;
	if(board->piece_num[bB] >= 2) score -= bishop_pair;

	if(board->turn == WHITE) {
		return score;
	} else {
		return -score;
	}	
}
