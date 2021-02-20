#include <stdio.h>
#include "defs.h"
#include "make_move.c"

const int hash_size = 0x100000 * 16;

int get_pv_line(const int depth, Board *board) {
	//ASSERT(depth < MAXDEPTH);

	int move = probe_pv_move(board);
	int count = 0;
	
	while(move != NO_MOVE && count < depth) {
	
		// ASSERT(count < MAXDEPTH);
	
		if( move_exists(board, move) ) {
			make_move(board, move);
			board->pv_array[count++] = move;
		} else {
			break;
		}		
		move = probe_pv_move(board);	
	}
	
	while(board->ply > 0) {
		take_move(board);
	}
	return count;
}

int probe_pv_move(const Board *board) {

	int index = board->pos_key % board->hash_table->num_entries;
	//ASSERT(index >= 0 && index <= board->HashTable->numEntries - 1);
	
	if( board->hash_table->hash_table[index].pos_key == board->pos_key ) {
		return board->hash_table->hash_table[index].move;
	}
	return NO_MOVE;
}

void clear_hash_table(Hash_table *table) {

  Hash_entry *table_entry;
  
  for (table_entry = table->hash_table; table_entry < table->hash_table + table->num_entries; table_entry++) {
    table_entry->pos_key = 0ULL;
    table_entry->move = NO_MOVE;
    table_entry->depth = 0;
    table_entry->score = 0;
    table_entry->flags = 0;
  }
  table->newwrite=0;
}

void init_hash_table(Hash_table *table) {  
  
    table->num_entries = hash_size / sizeof(Hash_entry);
    table->num_entries -= 2;
    if(table->hash_table != NULL) free(table->hash_table);
    table->hash_table = (Hash_entry *) malloc(table->num_entries * sizeof(Hash_entry));
    clear_hash_table(table);
    printf("hash_table init complete with %d entries\n", table->num_entries);
}

int probe_hash_entry(Board *board, int *move, int *score, int alpha, int beta, int depth) {

	int index = board->pos_key % board->hash_table->num_entries;
	
	// ASSERT(index >= 0 && index <= board->hash_table->numEntries - 1);
    // ASSERT(depth>=1&&depth<MAXDEPTH);
    // ASSERT(alpha<beta);
    // ASSERT(alpha>=-INFINITE&&alpha<=INFINITE);
    // ASSERT(beta>=-INFINITE&&beta<=INFINITE);
    // ASSERT(board->ply>=0&&board->ply<MAXDEPTH);
	
	if( board->hash_table->hash_table[index].pos_key == board->pos_key ) {
		*move = board->hash_table->hash_table[index].move;
		if(board->hash_table->hash_table[index].depth >= depth){
			board->hash_table->hit++;
			
			// ASSERT(board->hash_table->hash_table[index].depth>=1&&board->hash_table->hash_table[index].depth<MAXDEPTH);
            // ASSERT(board->hash_table->hash_table[index].flags>=HALPHA&&board->hash_table->hash_table[index].flags<=HFEXACT);
			
			*score = board->hash_table->hash_table[index].score;
			if(*score > IS_MATE) *score -= board->ply;
            else if(*score < -IS_MATE) *score += board->ply;
			
			switch(board->hash_table->hash_table[index].flags) {
				
                // ASSERT(*score>=-INFINITE&&*score<=INFINITE);

                case HFALPHA: if(*score<=alpha) {
                    *score=alpha;
                    return True;
                    }
                    break;
                case HFBETA: if(*score>=beta) {
                    *score=beta;
                    return True;
                    }
                    break;
                case HFEXACT:
                    return True;
                    break;
                default: ASSERT(False); break;
            }
		}
	}
	
	return False;
}

void store_hash_entry(Board *board, const int move, int score, const int flags, const int depth) {

	int index = board->pos_key % board->hash_table->num_entries;
	
	// ASSERT(index >= 0 && index <= board->hash_table->numEntries - 1);
	// ASSERT(depth>=1&&depth<MAXDEPTH);
    // ASSERT(flags>=HFUPPER&&flags<=HFEXACT);
    // ASSERT(score>=-INFINITE&&score<=INFINITE);
    // ASSERT(board->ply>=0&&board->ply<MAXDEPTH);
	
	if( board->hash_table->hash_table[index].pos_key == 0) {
		board->hash_table->newwrite++;
	} else {
		board->hash_table->overwrite++;
	}
	
	if(score > IS_MATE) score += board->ply;
    else if(score < -IS_MATE) score -= board->ply;
	
	board->hash_table->hash_table[index].move = move;
    board->hash_table->hash_table[index].pos_key = board->pos_key;
	board->hash_table->hash_table[index].flags = flags;
	board->hash_table->hash_table[index].score = score;
	board->hash_table->hash_table[index].depth = depth;
}