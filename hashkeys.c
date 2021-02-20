#include "defs.h"

U64 generate_pos_key(const Board *board){
    int sq = 0;
    int piece = EMPTY;
    U64 final_key = 0ULL;

    // pieces
    for (sq = 0; sq < BOARD_SQ_NUM; sq++){
        piece = board->board[sq];
        if (piece != OFF_BOARD && piece != EMPTY){
            final_key ^= piece_keys[piece][sq];
        }
    }

    if (board->turn == WHITE){
        final_key ^= turn_keys;
    }

    if (board->en_passant != NO_SQ){
        final_key ^= piece_keys[EMPTY][board->en_passant];
    }

    final_key ^= castle_keys[board->castle_perm];
    
    return final_key;
}