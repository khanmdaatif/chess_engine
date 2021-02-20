#include "defs.h"

static void check_up(Search_info *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == True && GetTickCount() > info->stoptime) {
		info->stopped = True;
	}
	read_input(info);
}

static void pick_next_move(int move_num, Move_list *list) {

	Move temp;
	int index = 0;
	int best_score = 0; 
	int best_num = move_num;
	
	for (index = move_num; index < list->count; ++index) {
		if (list->move_list[index].score > best_score) {
			best_score = list->move_list[index].score;
			best_num = index;
		}
	}
	temp = list->move_list[move_num];
	list->move_list[move_num] = list->move_list[best_num];
	list->move_list[best_num] = temp;
}

int is_repeatition(const Board *board){
    int index = 0;

    for (index = board->his_ply - board->fifty_move; index < board->his_ply-1; index++){
        if (board->pos_key == board->history[index].pos_key){
            return True;
        }
    }
    return False;
}

static void clear_for_search(Board *board, Search_info *info) {
	int index = 0;
	int index2 = 0;
	
	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < BOARD_SQ_NUM; ++index2) {
			board->search_history[index][index2] = 0;
		}
	}
	
	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < MAX_DEPTH; ++index2) {
			board->search_killers[index][index2] = 0;
		}
	}	
	
	board->ply = 0;
	board->hash_table->overwrite=0;
	board->hash_table->hit=0;	
	board->hash_table->cut=0;	
	
	info->starttime = GetTickCount();
	info->stopped = 0;
	info->nodes = 0;
    info->fh = 0;
	info->fhf = 0;
}

static int quiescence(int alpha, int beta, Board *board, Search_info *info) {
	// ASSERT(CheckBoard(board));
	info->nodes++;

	if(( info->nodes & 2047 ) == 0) {
		check_up(info);
	}
	
	if(is_repeatition(board) || board->fifty_move >= 100) {
		return 0;
	}
	
	if(board->ply > MAX_DEPTH - 1) {
		return eval_position(board);
	}
	
	int score = eval_position(board);
	
	if(score >= beta) {
		return beta;
	}
	
	if(score > alpha) {
		alpha = score;
	}
	
	Move_list list[1];
    generate_all_captures(board,list);
      
    int move_num = 0;
	int legal = 0;
	int old_alpha = alpha;
	int best_move = NO_MOVE;
	score = -INF;
	
	for(move_num = 0; move_num < list->count; ++move_num) {	
		pick_next_move(move_num, list);	
		
        if ( make_move(board, list->move_list[move_num].move) == False)  {
            continue;
        }
        
		legal++;
		score = - quiescence( -beta, -alpha, board, info);		
        take_move(board);

		if (info->stopped == True){
			return 0;
		}
		
		if(score > alpha) {
			if(score >= beta) {
				if(legal==1) {
					info->fhf++;
				}
				info->fh++;	
				return beta;
			}
			alpha = score;
			best_move = list->move_list[move_num].move;
		}		
    }

	return alpha;
}

static int alphabeta(int alpha, int beta, int depth, Board *board, Search_info *info, int do_null) {
    // ASSERT(CheckBoard(board)); 
	
	if(depth == 0) {
		info->nodes++;
		return quiescence(alpha, beta, board, info);
	}
	
	info->nodes++;

	if(( info->nodes & 2047 ) == 0) {
		check_up(info);
	}

	if ((is_repeatition(board) == True || board->fifty_move >= 100) && board->ply) {
		return 0;
	}
	
	if(board->ply > MAX_DEPTH - 1) {
		return quiescence(alpha, beta, board, info);
	}

	int in_check = sq_attacked(board->kings[board->turn], board->turn^1, board);
	
	if(in_check == True) { depth++; }
	
	int score = -INF;

	int pv_move = NO_MOVE;
	if( probe_hash_entry(board, &pv_move, &score, alpha, beta, depth) == True ) {
		board->hash_table->cut++;
		return score;
	}

	if( do_null && !in_check && board->ply && (board->big_piece[board->turn] > 0) && depth >= 4) {
		make_null_move(board);
		score = -alphabeta( -beta, -beta + 1, depth-4, board, info, False);
		take_null_move(board);
		if(info->stopped == True) {
			return 0;
		}
		if (score >= beta && abs(score) < IS_MATE) {		 
		  return beta;
		}	
	}
	
	Move_list list[1];
    generate_all_moves(board, list);
    // print_move_list(list);

    int move_num = 0;
	int legal = 0;
	int old_alpha = alpha;
	int best_move = NO_MOVE;
	score = -INF;

	if (pv_move != NO_MOVE){
		for(move_num = 0; move_num < list->count; move_num++){
			if (list->move_list[move_num].move == pv_move){
				list->move_list[move_num].score = 2000000;
				break;
			}
		}
	}
	
	for(move_num = 0; move_num < list->count; move_num++) {	
		pick_next_move(move_num, list);
       
        if ( make_move(board, list->move_list[move_num].move) == False)  {
            continue;
        }
        
		legal++;
		score = -alphabeta( -beta, -alpha, depth-1, board, info, True);		
        take_move(board);

		if (info->stopped == True){
			return 0;
		}
		
		if(score > alpha) {
			if(score >= beta) {
				if(legal==1) {
					info->fhf++;
				}
				info->fh++;	

				if (!(list->move_list[move_num].move & CAP_FLAG)){
					board->search_killers[1][board->ply] = board->search_killers[0][board->ply];
					board->search_killers[0][board->ply] = list->move_list[move_num].move;
				}

				store_hash_entry(board, best_move, beta, HFBETA, depth);
				return beta;
			}

			alpha = score;
			best_move = list->move_list[move_num].move;

			if (!(list->move_list[move_num].move & CAP_FLAG)){
				board->search_history[board->board[FROM_SQ(best_move)]][TO_SQ(best_move)] += depth;
			}
		}		
    }
	
	if(legal == 0) {
		if(sq_attacked(board->kings[board->turn], board->turn^1, board) == True) {
			return -INF + board->ply;
		} else {
			return 0;
		}
	}
	
	if(alpha != old_alpha) {
		store_hash_entry(board, best_move, alpha, HFEXACT, depth);
	}
	else{
		store_hash_entry(board, best_move, alpha, HFALPHA, depth);
	}
	
	return alpha;
} 

void search_position(Board *board, Search_info *info) {
	// .. iterative deepening, search init
    int best_move = NO_MOVE;
	int best_score = -INF;
	int current_depth = 0;
	int pv_moves = 0;
	int pv_num = 0;
	
	clear_for_search(board,info);
	
	// iterative deepening
	for( current_depth = 1; current_depth <= info->depth; ++current_depth ) {
		best_score = alphabeta(-INF, INF, current_depth, board, info, True);
		
		// out of time?
		if (info->stopped == True){
			break;
		}
		
		pv_moves = get_pv_line(current_depth, board);
		best_move = board->pv_array[0];
		if(info->GAME_MODE == UCIMODE) {
			printf("info score cp %d depth %d nodes %ld time %d ",
				best_score, current_depth, info->nodes, GetTickCount()-info->starttime);
		} 
		else if(info->GAME_MODE == XBOARDMODE && info->POST_THINKING == True) {
			printf("%d %d %d %ld ",
				current_depth, best_score, (GetTickCount()-info->starttime)/10, info->nodes);
		} 
		else if(info->POST_THINKING == True) {
			printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
				best_score, current_depth, info->nodes, GetTickCount()-info->starttime);
		}
		if(info->GAME_MODE == UCIMODE || info->POST_THINKING == True) {
			pv_moves = get_pv_line(current_depth, board);	
			printf("pv");		
			for(pv_num = 0; pv_num < pv_moves; ++pv_num) {
				printf(" %s", print_move(board->pv_array[pv_num]));
			}
			printf("\n");
		}
		// printf("Ordering:%.2f\n",(info->fhf/info->fh));
	}

	if(info->GAME_MODE == UCIMODE) {
		printf("bestmove %s\n", print_move(best_move));
	} 
	else if(info->GAME_MODE == XBOARDMODE) {		
		printf("move %s\n", print_move(best_move));
		make_move(board, best_move);
	} 
	else {	
		printf("\n\n***!! Vice makes move %s !!***\n\n",print_move(best_move));
		make_move(board, best_move);
		print_board(board);
	}
}
