// uci.c

#include <stdio.h>
#include "defs.h"
#include <string.h>

#define INPUTBUFFER 400 * 6

void parse_go(char* line, Search_info *info, Board *board) {
    
	int depth = -1, movestogo = 30,movetime = -1;
	int time = -1, inc = 0;
    char *ptr = NULL;
	info->timeset = False;
	
	if ((ptr = strstr(line,"infinite"))) {
		;
	} 
	
	if ((ptr = strstr(line,"binc")) && board->turn == BLACK) {
		inc = atoi(ptr + 5);
	}
	
	if ((ptr = strstr(line,"winc")) && board->turn == WHITE) {
		inc = atoi(ptr + 5);
	} 
	
	if ((ptr = strstr(line,"wtime")) && board->turn == WHITE) {
		time = atoi(ptr + 6);
	} 
	  
	if ((ptr = strstr(line,"btime")) && board->turn == BLACK) {
		time = atoi(ptr + 6);
	} 
	  
	if ((ptr = strstr(line,"movestogo"))) {
		movestogo = atoi(ptr + 10);
	} 
	  
	if ((ptr = strstr(line,"movetime"))) {
		movetime = atoi(ptr + 9);
	}
	  
	if ((ptr = strstr(line,"depth"))) {
		depth = atoi(ptr + 6);
	} 
	
	if(movetime != -1) {
		time = movetime;
		movestogo = 1;
	}
	
	info->starttime = GetTickCount();
	info->depth = depth;
	
	if(time != -1) {
		info->timeset = True;
		time /= movestogo;
		time -= 50;		
		info->stoptime = info->starttime + time + inc;
	} 
	
	if(depth == -1) {
		info->depth = MAX_DEPTH;
	}
	
	printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
		time, info->starttime, info->stoptime, info->depth, info->timeset);

	search_position(board, info);
}

// position fen fenstr
// position startpos
// ... moves e2e4 e7e5 b7b8q
void parse_position(char* line_in, Board *board) {
	
	line_in += 9;
    char *ptr_char = line_in;
	
    if(strncmp(line_in, "startpos", 8) == 0){
        parse_fen(START_FEN, board);
        update_material(board);
    } 
    else {
        ptr_char = strstr(line_in, "fen");
        if(ptr_char == NULL) {
            parse_fen(START_FEN, board);
            update_material(board);
        } 
        else {
            ptr_char+=4;
            parse_fen(ptr_char, board);
            update_material(board);
        }
    }
	
	ptr_char = strstr(line_in, "moves");
	int move;
	
	if(ptr_char != NULL) {
        ptr_char += 6;
        while(*ptr_char) {
              move = parse_move(ptr_char,board);
              printf("# %s %s", ptr_char, print_move(move));
			  if(move == NO_MOVE) break;
			  make_move(board, move);
              board->ply=0;
              while(*ptr_char && *ptr_char!= ' ') ptr_char++;
              ptr_char++;
        }
    }

	print_board(board);	
}

void uci_loop(Board *board, Search_info *info) {
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	
	char line[INPUTBUFFER];

    printf("id name %s\n", NAME);
    printf("id author MD AATIF KHAN\n");
    printf("uciok\n");
	   
	while (True) {
		memset(&line[0], 0, sizeof(line));
        fflush(stdout);

        if (!fgets(line, INPUTBUFFER, stdin))
        continue;

        if (line[0] == '\n')
        continue;

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } 
        else if (!strncmp(line, "position", 8)) {
            parse_position(line, board);
        } 
        else if (!strncmp(line, "ucinewgame", 10)) {
            parse_position("position startpos\n", board);
        } 
        else if (!strncmp(line, "go", 2)) {
            parse_go(line, info, board);
        } 
        else if (!strncmp(line, "quit", 4)) {
            info->quit = True;
            break;
        } 
        else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n",NAME);
            printf("id author Bluefever\n");
            printf("uciok\n");
        }
		if(info->quit) break;
    }
}