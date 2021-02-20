#include <stdio.h>
#include "defs.h"
#include "hashkeys.c"
#include "pv_table.c"

void reset_board(Board *board){
    int index = 0;

    for (index = 0; index < 120; index++){
        board->board[index] = OFF_BOARD;
    }

    for (index = 0; index < 64; index++){
        board->board[SQ64_TO_SQ120[index]] = EMPTY;
    }

    for (index = 0; index < 3; index++){
        board->pawns[index] = 0;
    }

    for (index = 0; index < 2; index++){
        board->big_piece[index] = 0;
        board->minor_piece[index] = 0;
        board->major_piece[index] = 0;
        board->material[index] = 0;
    }

    for (index = 0; index < 13; index++){
        board->piece_num[index] = 0;
    }

    board->kings[0] = NO_SQ; board->kings[1] = NO_SQ; 

    board->turn = BOTH;
    board->en_passant = NO_SQ;
    board->fifty_move = 0;
    board->castle_perm = 0;

    board->ply = 0;
    board->his_ply = 0;

    board->pos_key = 0ULL;
}

void update_material(Board *board){
    int piece, sq, index, color;

    for (index = 0; index < BOARD_SQ_NUM; index++){
        sq = index;
        piece = board->board[index];
        if (piece != OFF_BOARD && piece != EMPTY){
            color = pieces_color[piece];
            if (pieces_big[piece] == True) board->big_piece[color]++;
            if (pieces_major[piece] == True) board->major_piece[color]++;
            if (pieces_minor[piece] == True) board->minor_piece[color]++;

            board->material[color] += pieces_val[piece];
            board->piece_list[piece][board->piece_num[piece]++] = sq;

            if (piece == wK) board->kings[color] = sq;
            if (piece == bK) board->kings[color] = sq;   

            if (piece == wP){
                set_bit(board->pawns[WHITE], SQ120_TO_SQ64[sq]);
                set_bit(board->pawns[BOTH], SQ120_TO_SQ64[sq]);
            }

            if (piece == bP){
                set_bit(board->pawns[BLACK], SQ120_TO_SQ64[sq]);
                set_bit(board->pawns[BOTH], SQ120_TO_SQ64[sq]);
            }
        }
    }
}

int parse_fen(char fen[], Board *board){
    int rank = RANK_8;
    int file = FILE_A;
    int piece = 0;
    int count = 0;
    int i = 0;
    int sq64 = 0;
    int sq120 = 0;

    reset_board(board);

    while (rank >= RANK_1 && *fen){
        count = 1;
        switch (*fen){
            case 'p': piece = bP; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'r': piece = bR; break;
            case 'q': piece = bQ; break;
            case 'k': piece = bK; break;
            case 'P': piece = wP; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'R': piece = wR; break;
            case 'Q': piece = wQ; break;
            case 'K': piece = wK; break;

            case '1': 
            case '2': 
            case '3': 
            case '4': 
            case '5': 
            case '6': 
            case '7': 
            case '8': 
            piece = EMPTY;
            count += *fen - '1';
            break;

            case '/':
            case ' ':
            rank--;
            file = FILE_A;
            fen++;
            continue;

            default:
            printf("FEN Error %d %d", rank, file);
            return -1;
        }

        for (i = 0; i < count; i++){
            sq120 = fr2sq(file, rank);
            if (piece != EMPTY){
                board->board[sq120] = piece;
            }
            file++;
        }

        fen++;
    }

    board->turn = (*fen == 'w') ? WHITE : BLACK;
    fen += 2;

    for (i = 0; i < 4; i++){
        if (*fen == ' ' ){
            break;
        }
        switch (*fen)
        {
        case 'K': board->castle_perm |= WKCA; break;
        case 'Q': board->castle_perm |= WQCA; break;
        case 'k': board->castle_perm |= BKCA; break;
        case 'q': board->castle_perm |= BQCA; break;
        default: break;
        }
        fen++;
    }
    fen++;

    if (*fen != '-'){
        file = fen[0] - 'a';
        rank = fen[1] - '1';

        board->en_passant = fr2sq(file, rank);
    }

    board->pos_key = generate_pos_key(board);
    return 0;
}

void print_board(const Board *board){
    int piece, sq, file, rank;

    printf("Board: \n");

    for (rank = RANK_8; rank >= RANK_1; rank--){
        printf(" %c", rank_chars[rank]);
        for (file = FILE_A; file <= FILE_H; file++){
            sq = fr2sq(file, rank);
            piece = board->board[sq];
            printf(" %c ", piece_chars[piece]);
        }
        printf("\n");
    }

    printf("  ");
    for (file = FILE_A; file <= FILE_H; file++){
        printf(" %c ", file_chars[file]);
    }
    printf("\n");

    printf("Turn: %c\n", turn_chars[board->turn]);

    printf("En passant: %d\n", board->en_passant);

    printf("Castling: %c%c%c%c\n", 
    board->castle_perm & WKCA ? 'K' : ' ',
    board->castle_perm & WQCA ? 'Q' : ' ',
    board->castle_perm & BKCA ? 'k' : ' ',
    board->castle_perm & BQCA ? 'q' : ' ');

    printf("board position key: %x  %x  \n", board->pos_key, board->pos_key>>32);
}

int check_board(const Board *board) {   
 
	int t_piece_num[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_big_piece[2] = { 0, 0};
	int t_major_piece[2] = { 0, 0};
	int t_minor_piece[2] = { 0, 0};
	int t_material[2] = { 0, 0};
	
	int sq64, t_piece, t_pce_num, sq120, color, pcount;
	
	U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};
	
	t_pawns[WHITE] = board->pawns[WHITE];
	t_pawns[BLACK] = board->pawns[BLACK];
	t_pawns[BOTH] = board->pawns[BOTH];
	
	// check piece lists
	for(t_piece = wP; t_piece <= bK; t_piece++) {
		for(t_pce_num = 0; t_pce_num < board->piece_num[t_piece]; ++t_pce_num) {
			sq120 = board->piece_list[t_piece][t_pce_num];
			ASSERT(board->board[sq120]==t_piece);
		}	
	}
	
	// check piece count and other counters	
	for(sq64 = 0; sq64 < 64; sq64++) {
		sq120 = SQ64_TO_SQ120[sq64];
		t_piece = board->board[sq120];
		t_piece_num[t_piece]++;
		color = pieces_color[t_piece];
		if( pieces_big[t_piece] == True)t_big_piece[color]++;
		if( pieces_minor[t_piece] == True)t_minor_piece[color]++;
		if( pieces_major[t_piece] == True) t_major_piece[color]++;
		
		t_material[color] += pieces_val[t_piece];
	}
	
	for(t_piece = wP; t_piece <= bK; t_piece++) {
		ASSERT(t_piece_num[t_piece]==board->piece_num[t_piece]);	
	}
	
	// check bitboards count
	pcount = count_bit(t_pawns[WHITE]);
	ASSERT(pcount == board->piece_num[wP]);
	pcount = count_bit(t_pawns[BLACK]);
	ASSERT(pcount == board->piece_num[bP]);
	pcount = count_bit(t_pawns[BOTH]);
	ASSERT(pcount == (board->piece_num[bP] + board->piece_num[wP]));
	
	// check bitboards squares
	while(t_pawns[WHITE]) {
		sq64 = pop_bit(&t_pawns[WHITE]);
		ASSERT(board->board[SQ64_TO_SQ120[sq64]] == wP);
	}
	
	while(t_pawns[BLACK]) {
		sq64 = pop_bit(&t_pawns[BLACK]);
		ASSERT(board->board[SQ64_TO_SQ120[sq64]] == bP);
	}
	
	while(t_pawns[BOTH]) {
		sq64 = pop_bit(&t_pawns[BOTH]);
		ASSERT( (board->board[SQ64_TO_SQ120[sq64]] == bP) || (board->board[SQ64_TO_SQ120[sq64]] == wP) );
	}
	
	ASSERT(t_material[WHITE] == board->material[WHITE] && t_material[BLACK] == board->material[BLACK]);
	ASSERT(t_minor_piece[WHITE] == board->minor_piece[WHITE] && t_minor_piece[BLACK] == board->minor_piece[BLACK]);
	ASSERT(t_major_piece[WHITE] == board->major_piece[WHITE] && t_major_piece[BLACK] == board->major_piece[BLACK]);
	ASSERT(t_big_piece[WHITE] == board->big_piece[WHITE] && t_big_piece[BLACK] == board->big_piece[BLACK]);	
	
	ASSERT(board->turn == WHITE || board->turn == BLACK);

	ASSERT(generate_pos_key(board) == board->pos_key);
	
	ASSERT(board->en_passant == NO_SQ 
           || ( rank_board[board->en_passant] == RANK_6 && board->turn == WHITE)
		   || ( rank_board[board->en_passant] == RANK_3 && board->turn == BLACK));
	
	ASSERT(board->board[board->kings[WHITE]] == wK);
	ASSERT(board->board[board->kings[BLACK]] == bK);
		 
	return True;	
}