#include <stdio.h>
#include <windows.h>
#include "defs.h"
#include "init.c"
#include "bitboard.c"
#include "board.c"
#include "attack.c"
#include "io.c"
#include "move_gen.c"
#include "evaluate.c"
#include "perft.c"
#include "search.c"
#include "misc.c"
#include "uci.c"
#include "xboard.c"

#define WAC "r7/5R2/1ppk1pp1/7p/2P1P3/PK5P/P5P1/8 b - - 2 37 "

int main(){
    all_init();

    Board board[1];
    Search_info info[1];
	info->quit = False;
    init_hash_table(board->hash_table);

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
	
	printf("Welcome to Vice! Type 'vice' for console mode...\n");
	
	char line[256];
	while (TRUE) {
		memset(&line[0], 0, sizeof(line));

		fflush(stdout);
		if (!fgets(line, 256, stdin))
			continue;
		if (line[0] == '\n')
			continue;
		if (!strncmp(line, "uci",3)) {			
			uci_loop(board, info);
			if(info->quit == TRUE) break;
			continue;
		} 
        else if (!strncmp(line, "xboard",6))	{
			xboard_loop(board, info);
			if(info->quit == TRUE) break;
			continue;
		} 
        else if (!strncmp(line, "vice",4))	{
			console_loop(board, info);
			if(info->quit == TRUE) break;
			continue;
		} 
        else if(!strncmp(line, "quit",4))	{
			break;
		}
	}
	
	// free(board->PvTable->pTable);
	return 0;
}