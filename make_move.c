#include "defs.h"

#define HASH_PIECE(piece,sq) (board->pos_key ^= piece_keys[piece][sq])
#define HASH_CA (board->pos_key ^= castle_keys[board->castle_perm])
#define HASH_TURN (board->pos_key ^= turn_keys)
#define HASH_EP (board->pos_key ^= piece_keys[EMPTY][board->en_passant])

const int castle_perm[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void clear_piece(const int sq, Board *board) {

	// ASSERT(SqOnBoard(sq));
	
    int piece = board->board[sq];
	
    // ASSERT(PieceValid(piece));
	
	int color = pieces_color[piece];
	int index = 0;
	int t_piece_num = -1;
	
    HASH_PIECE(piece,sq);
	
	board->board[sq] = EMPTY;
    board->material[color] -= pieces_val[piece];
	
	if(pieces_big[piece]) {
			board->big_piece[color]--;
		if(pieces_major[piece]) {
			board->major_piece[color]--;
		} 
        else {
			board->minor_piece[color]--;
		}
	} 
    else {
		board->pawns[color] ^= (1ULL << SQ120_TO_SQ64[sq]);
		board->pawns[BOTH] ^= (1ULL << SQ120_TO_SQ64[sq]);
	}
	
	for(index = 0; index < board->piece_num[piece]; ++index) {
		if(board->piece_list[piece][index] == sq) {
			t_piece_num = index;
			break;
		}
	}
	
	// ASSERT(t_piece_num != -1);
	board->piece_num[piece]--;
	board->piece_list[piece][t_piece_num] = board->piece_list[piece][board->piece_num[piece]];
}

static void add_piece(const int sq, Board *board, const int piece) {

    // ASSERT(PieceValid(piece));
    // ASSERT(SqOnBoard(sq));
	
	int color = pieces_color[piece];

    HASH_PIECE(piece, sq);
	
	board->board[sq] = piece;

    if(pieces_big[piece]) {
			board->big_piece[color]++;
		if(pieces_major[piece]) {
			board->major_piece[color]++;
		} else {
			board->minor_piece[color]++;
		}
	} else {
		board->pawns[color] ^= (1ULL << SQ120_TO_SQ64[sq]);
		board->pawns[BOTH] ^= (1ULL << SQ120_TO_SQ64[sq]);
	}
	
	board->material[color] += pieces_val[piece];

	board->piece_list[piece][board->piece_num[piece]++] = sq;
}

static void move_piece(const int from, const int to, Board *board) {

    // ASSERT(SqOnBoard(from));
    // ASSERT(SqOnBoard(to));
	
	int index = 0;
	int piece = board->board[from];	
	int color = pieces_color[piece];
	
// #ifdef DEBUG
// 	int t_PieceNum = FALSE;
// #endif

	HASH_PIECE(piece,from);
	board->board[from] = EMPTY;
	
	HASH_PIECE(piece,to);
	board->board[to] = piece;
	
	if(!pieces_big[piece]) {
		board->pawns[color] ^= (1ULL << SQ120_TO_SQ64[from]);
		board->pawns[BOTH] ^= (1ULL << SQ120_TO_SQ64[from]);
		board->pawns[color] ^= (1ULL << SQ120_TO_SQ64[to]);
		board->pawns[BOTH] ^= (1ULL << SQ120_TO_SQ64[to]);	
	}    
	
	for(index = 0; index < board->piece_num[piece]; ++index) {
		if(board->piece_list[piece][index] == from) {
			board->piece_list[piece][index] = to;

// #ifdef DEBUG
// 			t_PieceNum = TRUE;
// #endif
			break;
		}
	}

	//ASSERT(t_PieceNum);
}

int make_move(Board *board, int move) {

	// ASSERT(CheckBoard(board));
	
	int from = FROM_SQ(move);
    int to = TO_SQ(move);
    int turn = board->turn;
	
	// ASSERT(SqOnBoard(from));
    // ASSERT(SqOnBoard(to));
    // ASSERT(SideValid(turn));
    // ASSERT(PieceValid(board->pieces[from]));
	
	board->history[board->his_ply].pos_key = board->pos_key;
	
	if(move & EP_FLAG) {
        if(turn == WHITE) {
            clear_piece(to-10, board);
        } else {
            clear_piece(to+10, board);
        }
    } 
	else if (move & CA_FLAG) {
        switch(to) {
            case C1:
                move_piece(A1, D1, board);
			break;
            case C8:
                move_piece(A8, D8, board);
			break;
            case G1:
                move_piece(H1, F1, board);
			break;
            case G8:
                move_piece(H8, F8, board);
			break;
            default: ASSERT(False); break;
        }
    }	
	
	if(board->en_passant != NO_SQ) HASH_EP;
    HASH_CA;
	
	board->history[board->his_ply].move = move;
    board->history[board->his_ply].fifty_move = board->fifty_move;
    board->history[board->his_ply].en_passant = board->en_passant;
    board->history[board->his_ply].castle_perm = board->castle_perm;

    board->castle_perm &= castle_perm[from];
    board->castle_perm &= castle_perm[to];
    board->en_passant = NO_SQ;

	HASH_CA;
	
	int captured = CAPTURED(move);
    board->fifty_move++;
	
	if(captured != EMPTY) {
        // ASSERT(PieceValid(captured));
        clear_piece(to, board);
        board->fifty_move = 0;
    }
	
	board->his_ply++;
	board->ply++;
	
	if(PiecePawn[board->board[from]]) {
        board->fifty_move = 0;
        if(move & PS_FLAG) {
            if(turn == WHITE) {
                board->en_passant = from+10;
                // ASSERT(RanksBrd[board->en_passant]==RANK_3);
            } else {
                board->en_passant = from-10;
                // ASSERT(RanksBrd[board->en_passant]==RANK_6);
            }
            HASH_EP;
        }
    }
	
	move_piece(from, to, board);
	
	int pr_piece = PROMOTED(move);
    if(pr_piece != EMPTY)   {
        // ASSERT(PieceValid(pr_piece) && !PiecePawn[pr_piece]);
        clear_piece(to, board);
        add_piece(to, board, pr_piece);
    }
	
	if(piece_king[board->board[to]]) {
        board->kings[board->turn] = to;
    }
	
	board->turn ^= 1;
    HASH_TURN;

    // ASSERT(CheckBoard(board));
		
	if(sq_attacked(board->kings[turn], board->turn, board))  {
        take_move(board);
        return False;
    }
	return True;
}

void take_move(Board *board) {
	
	// ASSERT(CheckBoard(board));
	
	board->his_ply--;
    board->ply--;
	
    int move = board->history[board->his_ply].move;
    int from = FROM_SQ(move);
    int to = TO_SQ(move);	
	
	// ASSERT(SqOnBoard(from));
    // ASSERT(SqOnBoard(to));
	
	if(board->en_passant != NO_SQ) HASH_EP;
    HASH_CA;

    board->castle_perm = board->history[board->his_ply].castle_perm;
    board->fifty_move = board->history[board->his_ply].fifty_move;
    board->en_passant = board->history[board->his_ply].en_passant;

    if(board->en_passant != NO_SQ) HASH_EP;
    HASH_CA;

    board->turn ^= 1;
    HASH_TURN;
	
	if(EP_FLAG & move) {
        if(board->turn == WHITE) {
            add_piece(to-10, board, bP);
        } else {
            add_piece(to+10, board, wP);
        }
    } else if(CA_FLAG & move) {
        switch(to) {
            case C1: move_piece(D1, A1, board); break;
            case C8: move_piece(D8, A8, board); break;
            case G1: move_piece(F1, H1, board); break;
            case G8: move_piece(F8, H8, board); break;
            default: ASSERT(False); break;
        }
    }
	
	move_piece(to, from, board);
	
	if(piece_king[board->board[from]]) {
        board->kings[board->turn] = from;
    }
	
	int captured = CAPTURED(move);
    if(captured != EMPTY) {
        // ASSERT(PieceValid(captured));
        add_piece(to, board, captured);
    }
	
	if(PROMOTED(move) != EMPTY)   {
        // ASSERT(PieceValid(PROMOTED(move)) && !PiecePawn[PROMOTED(move)]);
        clear_piece(from, board);
        add_piece(from, board, (pieces_color[PROMOTED(move)] == WHITE ? wP : bP));
    }
	
    // ASSERT(CheckBoard(board));
}

void make_null_move(Board *board) {

    // ASSERT(CheckBoard(board));
    // ASSERT(!SqAttacked(board->KingSq[board->side],board->side^1,board));

    board->ply++;
    board->history[board->his_ply].pos_key = board->pos_key;

    if(board->en_passant != NO_SQ) HASH_EP;

    board->history[board->his_ply].move = NO_MOVE;
    board->history[board->his_ply].fifty_move = board->fifty_move;
    board->history[board->his_ply].en_passant = board->en_passant;
    board->history[board->his_ply].castle_perm = board->castle_perm;
    board->en_passant = NO_SQ;

    board->turn ^= 1;
    board->his_ply++;
    HASH_TURN;
   
    // ASSERT(CheckBoard(board));

    return;
} // MakeNullMove

void take_null_move(Board *board) {
    // ASSERT(CheckBoard(board));

    board->his_ply--;
    board->ply--;

    if(board->en_passant != NO_SQ) HASH_EP;

    board->castle_perm = board->history[board->his_ply].castle_perm;
    board->fifty_move = board->history[board->his_ply].fifty_move;
    board->en_passant = board->history[board->his_ply].en_passant;

    if(board->en_passant != NO_SQ) HASH_EP;
    board->turn ^= 1;
    HASH_TURN;
  
    // ASSERT(CheckBoard(board));
}