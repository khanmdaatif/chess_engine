#include "defs.h"
#include <stdio.h>
// #include "move_gen.c"
// #include "make_move.c"

long leaf_nodes;

void perft(int depth, Board *board) {

    ASSERT(check_board(board));  

	if(depth == 0) {
        leaf_nodes++;
        return;
    }	

    Move_list list[1];
    generate_all_moves(board, list);
      
    int move_num = 0;
	for(move_num = 0; move_num < list->count; ++move_num) {	
       
        if ( !make_move(board, list->move_list[move_num].move))  {
            continue;
        }

        perft(depth - 1, board);
        take_move(board);
    }
    return;
}

void perft_test(int depth, Board *board) {

    ASSERT(check_board(board));
	// PrintBoard(board);

	printf("\nStarting Test To Depth:%d\n",depth);	
	leaf_nodes = 0;
	
    Move_list list[1];
    generate_all_moves(board,list);	
    
    int move;	    
    int move_num = 0;
	for(move_num = 0; move_num < list->count; ++move_num) {
        move = list->move_list[move_num].move;
        if ( !make_move(board,move))  {
            continue;
        }

        long cumnodes = leaf_nodes;
        perft(depth - 1, board);
        take_move(board);

        long oldnodes = leaf_nodes - cumnodes;
        printf("move %d : %s : %ld\n",move_num+1,print_move(move),oldnodes);
    }
	
	printf("\nTest Complete : %ld nodes visited\n",leaf_nodes);
    return;
}
