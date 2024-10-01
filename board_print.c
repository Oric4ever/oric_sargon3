#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

uint8_t patterns[2][128][8];

enum Piece { PAWN, ROOK, BISHOP, KING, QUEEN, KNIGHT };
#define EMPTY -1

int8_t board[8][8] = { // from line 8 to line 1 or line 1 to line 8
    { ROOK, KNIGHT, BISHOP, QUEEN,  KING, BISHOP, KNIGHT, ROOK },
    { PAWN,  PAWN ,  PAWN ,  PAWN,  PAWN,  PAWN ,  PAWN , PAWN },
    { EMPTY, EMPTY, EMPTY , EMPTY, EMPTY, EMPTY , EMPTY , EMPTY},
    { EMPTY, EMPTY, EMPTY , EMPTY, EMPTY, EMPTY , EMPTY , EMPTY},
    { EMPTY, EMPTY, EMPTY , EMPTY, EMPTY, EMPTY , EMPTY , EMPTY},
    { EMPTY, EMPTY, EMPTY , EMPTY, EMPTY, EMPTY , EMPTY , EMPTY},
    { PAWN,  PAWN ,  PAWN ,  PAWN,  PAWN,  PAWN ,  PAWN , PAWN },
    { ROOK, KNIGHT, BISHOP, QUEEN,  KING, BISHOP, KNIGHT, ROOK }
};

typedef uint8_t Graphic[3][3];

// graphics of Black pieces
Graphic graphics[6][2] = {
    // PAWN
    { { { 0x20, 0x20, 0x20 }, 
		{ 0x21, 0x22, 0x23 }, 
		{ 0x24, 0x25, 0x26 }}, 
	
	   {{ 0xa0, 0xa0, 0xa0 }, 
		{ 0xa0, 0x27, 0xa0 }, 
		{ 0x28, 0x29, 0x2a }}},
	// ROOK
    { { { 0x2b, 0x2c, 0x2d }, 
		{ 0x2e, 0x20, 0x2f }, 
		{ 0x30, 0x31, 0x32 }},
	
	  {	{ 0x33, 0x34, 0x35 }, 
		{ 0x36, 0x20, 0x37 }, 
		{ 0x38, 0x39, 0x3a }}},
	// BISHOP
	{ { { 0x3b, 0x3c, 0x3d }, 
		{ 0x3e, 0x3f, 0x40 }, 
		{ 0x30, 0x31, 0x32 }},
	
      { { 0x41, 0x42, 0x43 }, 
		{ 0x44, 0x3f, 0x45 }, 
		{ 0x38, 0x39, 0x3a }}},
	// KING
    { { { 0x46, 0x47, 0x48 }, 
		{ 0x49, 0x4a, 0x4b }, 
		{ 0x4c, 0x31, 0x4d }},
	
      { { 0xa0, 0x4e, 0xa0 }, 
		{ 0x4f, 0x4a, 0x50 }, 
		{ 0x51, 0x39, 0x52 }}},
	// QUEEN
    { {	{ 0x53, 0x54, 0x55 }, 
		{ 0x56, 0x57, 0x58 }, 
		{ 0x4c, 0x31, 0x4d }},
	
	  {	{ 0x59, 0x5a, 0x5b }, 
		{ 0x5c, 0x57, 0x5d }, 
		{ 0x51, 0x39, 0x52 }}},
	// KNIGHT
	{ { { 0x5e, 0x5f, 0x60 }, 
		{ 0x61, 0x62, 0x63 }, 
		{ 0x30, 0x31, 0x64 }},
	
      { { 0x65, 0x66, 0x67 }, 
		{ 0x68, 0x69, 0x6a }, 
		{ 0x38, 0x39, 0x6b }}}
};


uint8_t screen[28][40];


void plot(int line, int col, char *str) {
    while (*str)
        screen[line][col++] = *str++;
}

void build_screen() {
    for (int line=0; line<28; line++)
        for (int column=0; column<40; column++)
            screen[line][column] = ' ';

    for (int line=2; line<2+24; line++)
        screen[line][15] = 9;   // ALT CHAR SET
 
    plot(0, 0, "LEVEL 1E");
    plot(0, 19, "`abcdefghijk  yyy");
    plot(1, 19, "lmnopqrstuvw  zzz");
    plot(3, 0, "  1. E2-E4=Q+");
    plot(4, 0, "  .. E7-E5");
    plot(5, 0, "  2. G1-F3");
    plot(6, 0, "  .. E7xF8=Q+");

    for (int row=0; row<8; row++) {
        screen[3+3*row][14] = '8'-row;
        for (int col=0; col<8; col++) {
            int piece = board[row][col];
            int background = row%2 == col%2;
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    screen[2+3*row+i][16+3*col+j] = piece < 0 ? ' '+128*background
                                                  :  row  < 4 ? graphics[piece][ background ][i][j]
                                                  :             graphics[piece][1-background][i][j] + 128;
        }
    }
    plot(27, 16, " A  B  C  D  E  F  G  H");
}

void print_screen() {
    printf("P1\n");
    printf("240 224\n");
    for (int y=0; y<28; y++) {
        for (int line=0; line < 8; line++) {
            int charset = 0;
            for (int x=0; x<40; x++) {
                int  code    = screen[y][x] & 0x7f;
                bool inverse = screen[y][x] & 0x80;
                int  pattern = code < 32 ? 0 : patterns[charset][code][line];
                if (code >= 8 && code <= 15) charset = code & 1;
                if (inverse) pattern ^= 0b111111;

                for (int bit=5; bit>=0; bit--)
                    printf("%c", pattern & (1 << bit) ? '0' : '1');
            }
            printf("\n");
        }
    }
}

int main(void) {
    FILE *in = fopen("oric_patterns.bin", "r");
    fread(patterns, 256, 8, in);
    fclose(in);

    build_screen();
    FILE *out = fopen("oric_screen.bin", "w");
    fwrite(screen,28,40,out);
    fclose(out);

    print_screen();
}

