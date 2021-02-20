#include "defs.h"

void init_sq120_to_sq64(){
    int index = 0;
    int rank = RANK_1;
    int file = FILE_A;
    int sq120 = 0;
    int sq64 = 0;

    for (index = 0; index < BOARD_SQ_NUM; index++){
        SQ120_TO_SQ64[index] = OFF_BOARD;
    }

    for (rank = RANK_1; rank <= RANK_8; rank++){
        for (file = FILE_A; file <= FILE_H; file++){
            sq120 = fr2sq(file, rank);
            SQ64_TO_SQ120[sq64] = sq120;
            SQ120_TO_SQ64[sq120] = sq64;
            sq64++;
        }
    }
}

void init_bit_mask(){
    for (int index = 0; index < 64; index++){
        set_mask[index] = (1ULL << index);
        clear_mask[index] = ~set_mask[index];
    }
}

void init_hash_keys() {
	
	int i = 0;
	int j = 0;
	for(i = 0; i < 13; i++) {
		for(j = 0; j < 120; j++) {
			piece_keys[i][j] = rand64();
		}
	}

	turn_keys = rand64();

	for(i = 0; i < 16; i++) {
		castle_keys[i] = rand64();
	}
}

void init_file_rank_board(){
    int index = 0;
    for (index = 0; index < BOARD_SQ_NUM; index++){
        rank_board[index] = OFF_BOARD;
        file_board[index] = OFF_BOARD;
    }

    int rank = RANK_1;
    int file = FILE_A;
    int sq120 = 0;
    for (rank = RANK_1; rank <= RANK_8; rank++){
        for (file = FILE_A; file <= FILE_H; file++){
            sq120 = fr2sq(file, rank);
            rank_board[sq120] = rank;
            file_board[sq120] = file;
        }
    }
}

void init_eval_mask() {

	int sq, tsq, r, f;
	
	for(sq = 0; sq < 8; ++sq) {		
        file_mask[sq] = 0ULL; 
		rank_mask[sq] = 0ULL; 
	}
	
	for(r = RANK_8; r >= RANK_1; r--) {
        for (f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            file_mask[f] |= (1ULL << sq);
            rank_mask[r] |= (1ULL << sq);
        }
	}
	
	for(sq = 0; sq < 64; ++sq) {
		isolated_mask[sq] = 0ULL; 
		white_passed_mask[sq] = 0ULL; 
		black_passed_mask[sq] = 0ULL;
    }

	for(sq = 0; sq < 64; ++sq) {
		tsq = sq + 8;
		
        while(tsq < 64) {
            white_passed_mask[sq] |= (1ULL << tsq);
            tsq += 8;
        }

        tsq = sq - 8;
        while(tsq >= 0) {
            black_passed_mask[sq] |= (1ULL << tsq);
            tsq -= 8;
        }

        if(file_board[SQ64_TO_SQ120[sq]] > FILE_A) {
            isolated_mask[sq] |= file_mask[file_board[SQ64_TO_SQ120[sq]] - 1];

            tsq = sq + 7;
            while(tsq < 64) {
                white_passed_mask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 9;
            while(tsq >= 0) {
                black_passed_mask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
		
        if(file_board[SQ64_TO_SQ120[sq]] < FILE_H) {
            isolated_mask[sq] |= file_mask[file_board[SQ64_TO_SQ120[sq]] + 1];

            tsq = sq + 9;
            while(tsq < 64) {
                white_passed_mask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 7;
            while(tsq >= 0) {
                black_passed_mask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
	}	
	
	// for(sq = 0; sq < 64; ++sq) {
	// 	print_bitboard(isolated_mask[sq]);
	// }
}

void all_init(){
    init_sq120_to_sq64();
    init_bit_mask();
    init_hash_keys();
    init_file_rank_board();
    init_eval_mask();
    init_mvv_lva();
}
