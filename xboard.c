// xboard.c

#include "stdio.h"
#include "defs.h"
#include "string.h"

int three_fold_rep(const Board *board) {
	int i = 0, r = 0;
	for (i = 0; i < board->his_ply; ++i)	{
	    if (board->history[i].pos_key == board->pos_key) {
		    r++;
		}
	}
	return r;
}

int draw_material(const Board *board) {

    if (board->piece_num[wP] || board->piece_num[bP]) return False;
    if (board->piece_num[wQ] || board->piece_num[bQ] || board->piece_num[wR] || board->piece_num[bR]) return False;
    if (board->piece_num[wB] > 1 || board->piece_num[bB] > 1) {return False;}
    if (board->piece_num[wN] > 1 || board->piece_num[bN] > 1) {return False;}
    if (board->piece_num[wN] && board->piece_num[wB]) {return False;}
    if (board->piece_num[bN] && board->piece_num[bB]) {return False;}
	
    return True;
}

int check_result(Board *board) {

    if (board->fifty_move > 100) {
     printf("1/2-1/2 {fifty move rule (claimed by Vice)}\n"); return True;
    }

    if (three_fold_rep(board) >= 2) {
     printf("1/2-1/2 {3-fold repetition (claimed by Vice)}\n"); return True;
    }
	
	if (draw_material(board) == True) {
     printf("1/2-1/2 {insufficient material (claimed by Vice)}\n"); return True;
    }
	
	Move_list list[1];
    generate_all_moves(board,list);
      
    int move_num= 0;
	int found = 0;
	for(move_num= 0; move_num < list->count; ++move_num) {	
       
        if ( !make_move(board,list->move_list[move_num].move))  {
            continue;
        }
        found++;
		take_move(board);
		break;
    }
	
	if(found != 0) return False;

	int in_check = sq_attacked(board->kings[board->turn], board->turn^1, board);
	
	if(in_check == True)	{
	    if(board->turn == WHITE) {
	      printf("0-1 {black mates (claimed by Vice)}\n"); return True;
        } 
        else {
	      printf("0-1 {white mates (claimed by Vice)}\n"); return True;
        }
    } 
    else {
      printf("\n1/2-1/2 {stalemate (claimed by Vice)}\n"); return True;
    }	
	return False;	
}

void print_options() {
	printf("feature ping=1 setboard=1 colors=0 usermove=1\n");      
	printf("feature done=1\n");
}

void xboard_loop(Board *board, Search_info *info) {
	info->GAME_MODE = XBOARDMODE;
	info->POST_THINKING = True;

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	
    print_options(); // HACK

	int depth = -1, movestogo[2] = {30, 30}, movetime = -1;
	int time = -1, inc = 0;                             
	int engine_side = BOTH;                    
	int time_left;  
    int sec;                         
	int mps;    
	int move = NO_MOVE;	
	int i, score;
	char in_buf[80], command[80];
	
	while (True) { 

		fflush(stdout);

		if(board->turn == engine_side && check_result(board) == False) {  
			info->starttime = GetTickCount();
			info->depth = depth;
			
			if(time != -1) {
				info->timeset = True;
				time /= movestogo[board->turn];
				time -= 50;		
				info->stoptime = info->starttime + time + inc;
			} 
	
			if(depth == -1 || depth > MAX_DEPTH) {
				info->depth = MAX_DEPTH;
			}
		
			printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
				time, info->starttime, info->stoptime, info->depth, info->timeset, movestogo[board->turn], mps);
				search_position(board, info);
			
			if(mps != 0) {
				movestogo[board->turn^1]--;
				if(movestogo[board->turn^1] < 1) {
					movestogo[board->turn^1] = mps;
				}
			}
		}

		fflush(stdout); 
	
		memset(&in_buf[0], 0, sizeof(in_buf));
		fflush(stdout);
		if (!fgets(in_buf, 80, stdin))
		continue;
    
		sscanf(in_buf, "%s", command);
    
		if(!strcmp(command, "quit")) { 
			break; 
		}
		
		if(!strcmp(command, "force")) { 
			engine_side = BOTH; 
			continue; 
		} 
		
		if(!strcmp(command, "protover")){
		  printf("feature ping=1 setboard=1 colors=0 usermove=1\n");      
		  printf("feature done=1\n");
		  continue;
		}
		
		if(!strcmp(command, "sd")) {
			sscanf(in_buf, "sd %d", &depth); 
			continue; 
		}
		
		if(!strcmp(command, "st")) {
			sscanf(in_buf, "st %d", &movetime); 
			continue; 
		}    
		
        if(!strcmp(command, "level")) {
			sec = 0;
			movetime = -1;
			if( sscanf(in_buf, "level %d %d %d", &mps, &time_left, &inc) != 3) {
			  sscanf(in_buf, "level %d %d:%d %d", &mps, &time_left, &sec, &inc);
		      printf("DEBUG level with :\n");
			}	
            else {
		      printf("DEBUG level without :\n");
			}			
			time_left *= 60000;
			time_left += sec * 1000;
			movestogo[0] = movestogo[1] = 30;
			if(mps != 0) {
				movestogo[0] = movestogo[1] = mps;
			}
			time = -1;
		    printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps%d\n", time_left, movestogo[0], inc,mps);
			continue; 
		}  

		if(!strcmp(command, "ping")) { 
			printf("pong%s\n", in_buf+4); 
			continue; 
		}
		
		if(!strcmp(command, "new")) { 
			engine_side = BLACK; 
			parse_fen(START_FEN, board);
            update_material(board);

			depth = -1; 
			continue; 
		}
		
		if(!strcmp(command, "setboard")){
			engine_side = BOTH;  
			parse_fen(in_buf+9, board); 
            update_material(board);
			continue; 
		}   		
		
		if(!strcmp(command, "go")) { 
			engine_side = board->turn;  
			continue; 
		}		
		  
		if(!strcmp(command, "usermove")){
            movestogo[board->turn]--;
			move = parse_move(in_buf+9, board);	
			if(move == NO_MOVE) continue;
			make_move(board, move);
            board->ply=0;
		}    
    }	
}

void console_loop(Board *board, Search_info *info) {

	printf("Welcome to Vice In Console Mode!\n");
	printf("Type help for commands\n\n");

	info->GAME_MODE = CONSOLEMODE;
	info->POST_THINKING = True;
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	
	int depth = MAX_DEPTH, movetime = 3000;            
	int engine_side = BOTH;    
	int move = NO_MOVE;		
	char in_buf[80], command[80];	
	
	engine_side = BLACK; 
	parse_fen(START_FEN, board);
    update_material(board);	
	
	while(True) { 

		fflush(stdout);

		if(board->turn == engine_side && check_result(board) == False) {  
			info->starttime = GetTickCount();
			info->depth = depth;
			
			if(movetime != 0) {
				info->timeset = True;
				info->stoptime = info->starttime + movetime;
			} 	
			
			search_position(board, info);
		}	
		
		printf("\nVice > ");

		fflush(stdout); 
	
		memset(&in_buf[0], 0, sizeof(in_buf));
		fflush(stdout);
		if (!fgets(in_buf, 80, stdin))
		continue;
    
		sscanf(in_buf, "%s", command);
		
		if(!strcmp(command, "help")) { 
			printf("Commands:\n");
			printf("quit - quit game\n");
			printf("force - computer will not think\n");
			printf("print - show board\n");
			printf("post - show thinking\n");
			printf("nopost - do not show thinking\n");
			printf("new - start new game\n");
			printf("go - set computer thinking\n");
			printf("depth x - set depth to x\n");
			printf("time x - set thinking time to x seconds (depth still applies if set)\n");
			printf("view - show current depth and movetime settings\n");
			printf("** note ** - to reset time and depth, set to 0\n");
			printf("enter moves using b7b8q notation\n\n\n");
			continue;
		}
    
		if(!strcmp(command, "quit")) { 
			info->quit = True;
			break; 
		}
		
		if(!strcmp(command, "post")) { 
			info->POST_THINKING = True;
			continue; 
		}
		
		if(!strcmp(command, "print")) { 
			print_board(board);
			continue; 
		}
		
		if(!strcmp(command, "nopost")) { 
			info->POST_THINKING = False;
			continue; 
		}
		
		if(!strcmp(command, "force")) { 
			engine_side = BOTH; 
			continue; 
		} 
		
		if(!strcmp(command, "view")) {
			if(depth == MAX_DEPTH) printf("depth not set ");
			else printf("depth %d",depth);
			
			if(movetime != 0) printf(" movetime %ds\n",movetime/1000);
			else printf(" movetime not set\n");
			
			continue; 
		}
		
		if(!strcmp(command, "depth")) {
			sscanf(in_buf, "depth %d", &depth); 
		    if(depth==0) depth = MAX_DEPTH;
			continue; 
		}
		
		if(!strcmp(command, "time")) {
			sscanf(in_buf, "time %d", &movetime); 
			movetime *= 1000;
			continue; 
		} 
		
		if(!strcmp(command, "new")) { 
			engine_side = BLACK; 
			parse_fen(START_FEN, board);
            update_material(board);
			continue; 
		}
		
		if(!strcmp(command, "go")) { 
			engine_side = board->turn;  
			continue; 
		}	
		
		move = parse_move(in_buf, board);
		if(move == NO_MOVE) {
			printf("Command unknown:%s\n",in_buf);
			continue;
		}
		make_move(board, move);
		board->ply=0;
    }	
}