#ifndef DEFS
#define DEFS

#include <stdlib.h>

#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long int U64;
#define NAME "Simple Chess Engine"
#define BOARD_SQ_NUM 120
#define MAX_MOVE 2048
#define MAX_POS_MOVE 256
#define MAX_DEPTH 24
#define NO_MOVE 0

#define INF 30000
#define IS_MATE (INF - MAX_DEPTH)

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

enum {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};

enum {WHITE, BLACK, BOTH};

enum {A1 = 21, B1, C1, D1, E1, F1, G1, H1,
      A8 = 91, B8, C8, D8, E8, F8, G8, H8,
      NO_SQ = 99, OFF_BOARD = 100};

enum {WKCA  = 1, WQCA = 2, BKCA = 4, BQCA = 8};

enum {False, True};

enum {UCIMODE, XBOARDMODE, CONSOLEMODE};

char piece_chars[] = ".PNBRQKpnbrqk";
char turn_chars[] = "wb-";
char file_chars[] = "ABCDEFGH";
char rank_chars[] = "12345678";

typedef struct{
  int move;
  int score;
}Move;

typedef struct{
  Move move_list[MAX_POS_MOVE];
  int count;
}Move_list;

enum {  HFNONE, HFALPHA, HFBETA, HFEXACT}; 

typedef struct {
	U64 pos_key;
	int move;
	int score;
	int depth;
	int flags;
} Hash_entry;

typedef struct {
	Hash_entry *hash_table;
	int num_entries;
	int newwrite;
	int overwrite;
	int hit;
	int cut;
} Hash_table;

typedef struct{
    int move;
    int castle_perm;
    int en_passant;
    int fifty_move;
    int pos_key;
    
} Move_history;

typedef struct {
    int board[BOARD_SQ_NUM];
    U64 pawns[3];
    int kings[2];

    int turn;
    int en_passant;
    int fifty_move;
    int castle_perm;

    U64 pos_key;

    int piece_num[13];
    int big_piece[2];
    int major_piece[2];
    int minor_piece[2];
    int material[2];
    

    int ply;
    int his_ply;
    Move_history  history[MAX_MOVE];

    int piece_list[13][10];

    Hash_table hash_table[1];
    int pv_array[MAX_DEPTH];

    int search_history[13][BOARD_SQ_NUM];
	  int search_killers[2][MAX_DEPTH];
} Board;

typedef struct {

	int starttime;
	int stoptime;
	int depth;
	int depthset;
	int timeset;
	int movestogo;
	int infinite;
	
	long nodes;
	
	int quit;
	int stopped;

  float fh;
	float fhf;

  int GAME_MODE;
	int POST_THINKING;
} Search_info;
/* Macros Function */

#define fr2sq(f, r) (10 * r + f + 21)

#define set_bit(bb, sq) (bb |= set_mask[sq])
#define clear_bit(bb, sq) (bb &= clear_mask[sq])

// #define rand64() ((U64) rand() | (U64) rand() << 15 | (U64) rand() << 30 | (U64) rand() << 45 | ((U64) rand() << 60))

#define FROM_SQ(m) (m & 0x7F) 
#define TO_SQ(m) ((m >> 7) & 0x7F) 
#define CAPTURED(m) ((m >> 14) & 0xF) 
#define PROMOTED(m) ((m >> 20) & 0xF) 

#define EP_FLAG 0x40000
#define PS_FLAG 0x80000
#define CA_FLAG 0x1000000

#define CAP_FLAG 0x7C000
#define PROM_FLAG 0xF00000

/* Global Variable */

int SQ64_TO_SQ120[64] = {0};
int SQ120_TO_SQ64[120] = {0};

U64 set_mask[64];
U64 clear_mask[64];

U64 piece_keys[13][120];
U64 turn_keys;
U64 castle_keys[16];

int pieces_big[13] = {False, False, True, True, True, True, True, False, True, True, True, True, True};
int pieces_major[13] = {False, False, False, False, True, True, True, False, False, False, True, True, True};
int pieces_minor[13] = {False, False, True, True, False, False, False, False, True, True, False, False, False};
int pieces_val[13] = {0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000};
int pieces_color[13] = {BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};

int file_board[120];
int rank_board[120];

int PiecePawn[13] = { False, True, False, False, False, False, False, True, False, False, False, False, False };	
int piece_knight[13] = { False, False, True, False, False, False, False, False, True, False, False, False, False };
int piece_king[13] = { False, False, False, False, False, False, True, False, False, False, False, False, True };
int piece_rook_queen[13] = { False, False, False, False, True, True, False, False, False, False, True, True, False };
int piece_bishop_queen[13] = { False, False, False, True, False, True, False, False, False, True, False, True, False };

int slide[13] = {False, False, False, True, True, True, False, False, False, True, True, True, False};

int mirror64[64] = {
56	,	57	,	58	,	59	,	60	,	61	,	62	,	63	,
48	,	49	,	50	,	51	,	52	,	53	,	54	,	55	,
40	,	41	,	42	,	43	,	44	,	45	,	46	,	47	,
32	,	33	,	34	,	35	,	36	,	37	,	38	,	39	,
24	,	25	,	26	,	27	,	28	,	29	,	30	,	31	,
16	,	17	,	18	,	19	,	20	,	21	,	22	,	23	,
8	,	9	,	10	,	11	,	12	,	13	,	14	,	15	,
0	,	1	,	2	,	3	,	4	,	5	,	6	,	7
};

U64 file_mask[8];
U64 rank_mask[8];

U64 black_passed_mask[64];
U64 white_passed_mask[64];
U64 isolated_mask[64];

/* Functions */
U64 rand64() {
  U64 r = 0ULL;
  for (int i=0; i<64; i++) {
    r = r*2 + rand()%2;
  }
  return r;
}

int count_bit(U64 bb);
int pop_bit(U64 *bb);
void print_bitboard(U64 bb);

U64 generate_pos_key(const Board *board);

void reset_board(Board *board);
int parse_fen(char *fen, Board *board);
void print_board(const Board *board);
void update_material(Board *board);
int check_board(const Board *board);

int sq_attacked(const int sq, const int turn, const Board *board);

char *print_sq(const int sq);
char *print_move(const int move);
void print_move_list(const Move_list *list);
int parse_move(char *ptr_char, Board *board);

void generate_all_moves(const Board *board, Move_list *list);
void generate_all_captures(const Board *board, Move_list *list);
int move_exists(Board *board, const int move);
int init_mvv_lva();

int make_move(Board *board, int move);
void take_move(Board *board);
void make_null_move(Board *board);
void take_null_move(Board *board);

void perft(int depth, Board *board);
void perft_test(int depth, Board *board);

int get_pv_line(const int depth, Board *board);
int probe_pv_move(const Board *board);
void clear_hash_table(Hash_table *table);
void init_hash_table(Hash_table *table);
int probe_hash_entry(Board *board, int *move, int *score, int alpha, int beta, int depth);
void store_hash_entry(Board *board, const int move, int score, const int flags, const int depth);

int eval_position(const Board *board);

void uci_loop(Board *board, Search_info *info);

void read_input(Search_info *info); 

void search_position(Board *board, Search_info *info);

void xboard_loop(Board *board, Search_info *info);
void console_loop(Board *board, Search_info *info);

#endif