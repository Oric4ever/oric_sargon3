#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define NB(int_array) (sizeof(int_array)/sizeof(int))
#define STOP -1

#define START 0x0C00
#define END 0xBFDF

uint8_t mem[64*1024];
uint8_t header[] = { 0x16, 0x16, 0x16, 0x24,
    0, 0,
    0x80, // memory block
    0xC7, // AUTO start
    END >> 8, END & 0xFF, 
    START  >> 8, START  & 0xFF,
    0 };
char    name[] = { '\0' };


void wpatch(int addresses[], int nb_addresses, uint8_t patch[], int patch_size){}
void byte (int address, int value)    { mem[address] = value & 0xff; }
void inverse(int address)             { mem[address] ^= 0x80; }
void word (int address, int value)    { mem[address] = value & 0xff; mem[address+1] = (value >> 8) & 0xff; }
void bytes(int address, int values[]) { for (int i=0; values[i] != STOP; i++) byte(address+i, values[i]); }
void string(int address, char *str)   { while (*str) mem[address++] = *str++; }
void move(int src, int dst, int nb)   { for (int i=0; i<nb; i++) mem[dst+i]=mem[src+i]; }

#define WNDLFT  0x20
#define WNDWDTH 0x21
#define WNDTOP  0x22
#define WNDBTM  0x23
#define CH      0x24
#define CV      0x25
#define BASL    0x28
#define BASH    0x29
#define BAS2L   0x2A
#define BAS2H   0x2B

#define COUT    0x0D00
#define PRINTOUT 0x0D03
#define SETVID  0x0D13
#define BASCALC 0x0D1C
#define COUT1   0x0D40
#define VIDOUT  0x0D4A
#define VTAB    0x0D81
#define VTABZ   0x0D83
#define HOME    0x0D9D
#define CLEOLZ  0x0DE3

int init[] = {
    0xa9, 0xb8,         // lda #$b8    ; keep cursor out of screen
    0x85, 0x13,         // sta TXTLINE+1
    0xa9, 0x00,         // lda #0
    0x8D, 0x6A, 0x02,   // sta VDUFLAGS_ORIC
    0x85, WNDTOP,       // sta WNDTOP
    0x85, WNDLFT,       // sta WNDLFT
    0xa9, 8,            // lda #8
    0x85, WNDWDTH,      // sta WNDWDTH
    0xa9, 25,           // lda #28
    0x85, WNDBTM,       // sta WNDBTM
//    0x20, HOME & 0xff, HOME >> 8, // jsr HOME
    0x4c, 0x38, 0x0E,   // jmp $0E38
};

int monitor[] = {
// COUT
    0x6c, 0x36, 0x00,   // jmp (CSWL)
// PRINTOUT
    0x29, 0x7F,         // and #$7F
    0x48,               // pha
    0x20, 0x3e, 0x02,   // jsr $023E
    0x68,               // pla
    0xc9, 0x0d,         // cmp #$0D
    0xd0, 0x0d,         // bne RTS1
    0xa9, 0x0a,         // lda #$0a
    0x4c, 0x3e, 0x02,   // jmp $023E
// SETVID
    0xa9, COUT1 & 0xFF, // lda #<COUT1
    0x85, 0x36,         // sta CSWL
    0xa9, COUT1 >> 8,   // lda #>COUT1
    0x85, 0x37,         // sta CSWH
    0x60,               // rts
// BASCALC
    0x85, BASL,         // sta BASL
    0x0A,               // asl a
    0x0A,               // asl a
    0x65, BASL,         // adc BASL
    0x85, BASL,         // sta BASL
    0xA9, 0x00,         // lda #0
    0x06, BASL,         // asl BASL
    0x2a,               // rol a
    0x06, BASL,         // asl BASL
    0x2a,               // rol a
    0x06, BASL,         // asl BASL
    0x2a,               // rol a
    0x85, BASH,         // sta BASH
    0xA5, BASL,         // lda BASL
    0x69, 0x80,         // adc #$80
    0x85, BASL,         // sta BASL
    0xa5, BASH,         // lda BASH
    0x69, 0xbb,         // adc #$BB
    0x85, BASH,         // sta BASH
    0xa5, BASL,         // lda BASL
    0x60,               // rts
// COUT1 normally at FDF0, here at 0D40
    0x84, 0x35,         // sty YSAV1
    0x48,               // pha
    0x20, VIDOUT & 0xff, VIDOUT >> 8,   // jsr VIDOUT
//0xea, 0xea, 0xea, // temporarily disable COUT
    0x68,               // pla
    0xa4, 0x35,         // ldy YSAV1
    0x60,               // rts
// VIDOUT normally at FBFD, here at 0D4A
    0xc9, 0x20,         // cmp #$20
    0xb0, 0x02,         // bcs *+4
    0x69, 0x40,         // adc #$40     00-1F: letters in inverse mode
    0x49, 0x80,         // eor #$80     inverse bit inversed
    0xc9, 0x20,         // cmp #$20
    0x90, 0x0d,         // bcc ctrlchars
    0xa4, CH,           // ldy CH
    0x91, BASL,         // sta (BASL),y
    0xe6, CH,           // inc CH
    0xa5, CH,           // lda CH
    0xc5, WNDWDTH,      // cmp WNDWDTH
    0xb0, 0x45,         // bcs CR
    0x60,               // rts
// ctrlchars at 0d63
    0xc9, 0x0d,         // cmp #$0d
    0xf0, 0x40,         // beq CR
    0xc9, 0x0a,         // cmp #$0a
    0xf0, 0x40,         // beq LF
    0xc9, 0x08,         // cmp #$08
    0xd0, 0x1B,         // bne RTS
// BS at 0D6F
    0xc6, CH,           // dec CH
    0x10, 0x17,         // bpl RTS4
    0xa5, 0x21,         // lda WNDWDTH
    0x85, CH,           // sta CH
    0xc6, CH,           // dec CH
// UP at 0D79
    0xa5, WNDTOP,       // lda WNDTOP
    0xc5, CV,           // cmp CV
    0xb0, 0x0b,         // bcs RTS4
    0xc6, CV,           // dec CV
// VTAB normally at FC22, here at 0D81
    0xa5, CV,           // lda CV
// VTABZ at 0D83
    0x20, BASCALC & 0xff, BASCALC >> 8,   // jsr BASCALC
    0x65, WNDLFT,       // adc WNDLFT
    0x85, BASL,         // sta BASL
    0x60,               // rts
// CLEOP1 at 0D8B
    0x48,               // pha
    0x20, VTABZ  & 0xff, VTABZ  >> 8,   // jsr VTABZ
    0x20, CLEOLZ & 0xff, CLEOLZ >> 8,   // jsr CLEOLZ
    0xa0, 0x00,         // ldy #$00
    0x68,               // pla
    0x69, 0x00,         // adc #$00
    0xc5, WNDBTM,       // cmp WNDBTM
    0x90, 0xf0,         // bcc CLEOP1
    0xb0, 0xe4,         // bcs VTAB
// HOME at 0D9D
    0xa5, WNDTOP,       // lda WNDTOP
    0x85, CV,           // sta CV
    0xa0, 0x00,         // ldy #0
    0x84, CH,           // sty CH
    0xf0, 0xe4,         // beq CLEOP1
// CR at 0DA7
    0xa9, 0x00,         // lda #0
    0x85, CH,           // sta CH
// LF at 0DAB
    0xe6, CV,           // inc CV
    0xa5, CV,           // lda CV
    0xc5, WNDBTM,       // cmp WNDBTM
    0x90, 0xd0,         // bcc VTABZ
    0xc6, CV,           // dec CV
// SCROLL
    0xa5, WNDTOP,       // lda WNDTOP
    0x48,               // pha
    0x20, VTABZ  & 0xff, VTABZ  >> 8,   // jsr VTABZ
// SCRL1
    0xa5, BASL,         // lda BASL
    0x85, BAS2L,        // sta BAS2L
    0xa5, BASH,         // lda BASH
    0x85, BAS2H,        // sta BAS2H
    0xa4, WNDWDTH,      // ldy WNDWDTH
    0x88,               // dey
    0x68,               // pla
    0x69, 0x01,         // adc #1
    0xc5, WNDBTM,       // cmp WNDBTM
    0xb0, 0x0d,         // bcs SCLR3
    0x48,               // pha
    0x20, VTABZ  & 0xff, VTABZ  >> 8,   // jsr VTABZ
// SCRL2
    0xb1, BASL,         // lda (BASL),y
    0x91, BAS2L,        // sta (BAS2L),y
    0x88,               // dey
    0x10, 0xf9,         // bpl SCRL2
    0x30, 0xe1,         // bmi SCRL1
// SCRL3
    0xa0, 0x00,         // ldy #0
    0x20, CLEOLZ & 0xff, CLEOLZ >> 8,   // jsr CLEOLZ
    0xb0, 0xa0,         // bcs VTAB
    0xa4, CH,           // ldy CH
// CLEOLZ normally at FC9E, here at 0DE3
    0xa9, 0x20,         // lda #$20
// CLEOL2
    0x91, BASL,         // sta (BASL),y
    0xc8,               // iny
    0xc4, WNDWDTH,      // cpy WNDWDTH
    0x90, 0xf9,         // bcc CLEOL2
    0x60,               // rts
// 
    STOP
};

int nops[] = { 0xEA, 0xEA, 0xEA, STOP };

void time_patches() {
    // disable approximate timer incrementation
//    word(0x64EF, 0x01a9);   // lda #1 to clear Z flag

    // TODO: increment $DA/$DB once per second, with the hardware timer
}

void patch_levels()
{
    // add level 0 key
    int level0[] = {
        0xEA,       // nop
        0xA2, 0x00, // ldx #0
        0xC9, 0xA9, // cmp #$A9 ; ')'
        0xF0, 0x0C, // beq L9BBA 
        STOP
    };
    bytes(0x9BA7, level0);

    // move level tables to add level 0
    move(0x6148, 0x8E3D, 8); byte(0x8E3C, 120); // 120 secs for level 0
    move(0x6158, 0x8E34, 8); byte(0x8E33, 0);   // msb
    move(0x6256, 0x8E2B, 8); byte(0x8E2A, 60);  // 60 moves
    // 6255 -> 8E2A
    word(0xA361, 0x8E2A);
    word(0xA371, 0x8E2A);
    // 6157 -> 8E33
    word(0xA358, 0x8E33);
    word(0xA36C, 0x8E33);
    // 6147 -> 8E3C
    word(0xA34E, 0x8E3C);
    word(0xA367, 0x8E3C);
}

void all_patches() {
// Install init routine in page 0C, will be overwritten
    bytes(START, init);

// Install "reduced monitor" in page 0D, seems safe
    bytes(0x0D00, monitor);

// page 0E will be overwritten

// page 0F is not moved to page 3, so page 3 calls remain in page 0F
    int calls[] = { 0xB0B8, 0xB0BB, 0xB0E9, 0xB0EC, 0xB0FB, 0xB157 };
    for (int i=0; i<NB(calls); i++) byte(calls[i], 0x0F);

    // replace read of C000 (KEYBOARD) by read of 02DF
    int addresses[] = { 0x6310, 0x6329, 0x9261, 0x9308, 0x9463, 0x98F6, 0x9B29, 0x9E73, 0xA63C, 0xAE05, 0xAE36, 0xAE4E};
    for (int i=0; i<NB(addresses); i++) word(addresses[i], 0x02DF);

    // replace BIT STROBE by LSR 02DF
    int addr[] = { 0x6314, 0x930C, 0x9467, 0x98FA, 0x9B2D, 0x9E77, 0xA643, 0xAE09, 0xAE3A, 0xAE52 };
    int patch[] = { 0x4E, 0xDF, 0x02, STOP };
    for (int i=0; i<NB(addr); i++) bytes(addr[i], patch);

    time_patches();
    patch_levels();

    // fix small bug that transfers an extra space
    byte(0x9AC6, 39);

    // fix bug that forgets subpromotion
    byte(0x6CF7, 0x88);

    // replace direct writes to screen address 0680
    word(0x9AA8, 0xBF90);
    word(0x9ACA, 0xBF90);

    // replace direct writes to screen address 05A5 (level)
    word(0x8843, 0xBBCD); 
    word(0x933D, 0xBBCD); 
    word(0x93C5, 0xBBCD); 
    word(0x9DB0, 0xBBCD); 
    // replace direct writes to screen address 05A6 (Easy mode)
    word(0x885B, 0xBBCE); 
    word(0x9ECB, 0xBBCE); 
    word(0x9ED3, 0xBBCE); 
    // replace direct writes to screen address 05A7 (Verify mode)
    word(0x884F, 0xBBCF); 
    word(0x9EF0, 0xBBCF); 
    // replace direct access to screen address 0527 (blinking '*')
    word(0x6279, 0xBB80);
    word(0x627E, 0xBB80);

    // replace graphics with alternate chars
    int L88D9[] = {
    // EMPTY SQUARE
         0x20, 0x20, 0x20 ,
         0x20, 0x20, 0x20 ,
         0x20, 0x20, 0x20 ,

         0x20, 0x20, 0x20 ,
         0x20, 0x20, 0x20 ,
         0x20, 0x20, 0x20 ,
    // PAWN
         0x20, 0x20, 0x20 ,
         0x21, 0x22, 0x23 ,
         0x24, 0x25, 0x26 ,

         0xa0, 0xa0, 0xa0 ,
         0xa0, 0x27, 0xa0 ,
         0x28, 0x29, 0x2a ,
    // KNIGHT
         0x5e, 0x5f, 0x60 ,
         0x61, 0x62, 0x63 ,
         0x30, 0x31, 0x64 ,

         0x65, 0x66, 0x67 ,
         0x68, 0x69, 0x6a ,
         0x38, 0x39, 0x6b ,
    // KING
         0x46, 0x47, 0x48 ,
         0x49, 0x4a, 0x4b ,
         0x4c, 0x31, 0x4d ,

         0xa0, 0x4e, 0xa0 ,
         0x4f, 0x4a, 0x50 ,
         0x51, 0x39, 0x52 ,
    // QUEEN
         0x53, 0x54, 0x55 ,
         0x56, 0x57, 0x58 ,
         0x4c, 0x31, 0x4d ,

         0x59, 0x5a, 0x5b ,
         0x5c, 0x57, 0x5d ,
         0x51, 0x39, 0x52 ,
    // ROOK
         0x2b, 0x2c, 0x2d ,
         0x2e, 0x20, 0x2f ,
         0x30, 0x31, 0x32 ,

         0x33, 0x34, 0x35 ,
         0x36, 0x20, 0x37 ,
         0x38, 0x39, 0x3a ,
    // BISHOP
         0x3b, 0x3c, 0x3d ,
         0x3e, 0x3f, 0x40 ,
         0x30, 0x31, 0x32 ,

         0x41, 0x42, 0x43 ,
         0x44, 0x3f, 0x45 ,
         0x38, 0x39, 0x3a ,
    // Digits
         0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    // Letters
         0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
         STOP
    };
    bytes(0x88D9, L88D9);

    // patch graphics indirection table
    for (int i=0; i<14; i++)
        word(0x8e45 + i*2, 0x88D9 + 9*i);
    for (int i=14; i<14+16; i++)
        word(0x8e45 + i*2, 0x88D9 + 14*9 + (i-14));

    // patch routine 8EDA to calculate line addr
    int L8EDA[] = {
        0x85, 0x5e, //      sta screen_ptr
        0xa9, 0x00, //      lda #0
        0x85, 0x5f, //      sta screen_ptr+1

        0xa5, 0x5e, //      lda screen_ptr
        0x0a,       //      asl a
        0x0a,       //      asl a
        0x65, 0x5e, //      adc screen_ptr      ; x5 (140)
        0x0a,       //      asl a
        0x26, 0x5f, //      rol screen_ptrh     ; x10
        0x0a,       //      asl a
        0x26, 0x5f, //      rol screen_ptrh     ; x20
        0x0a,       //      asl a
        0x26, 0x5f, //      rol screen_ptrh     ; x40

        0x69, 0x80, //      adc #$80
        0x85, 0x5e, //      sta screen_ptr
        0xa5, 0x5f, //      lda screen_ptrh
        0x69, 0xbb, //      adc #$BB
        0x85, 0x5f, //      sta screen_ptrh
        0x60,       //      rts
        STOP
    };
    bytes(0x8EDA, L8EDA);

    // patch routine 8EFC to calculate screen x and y :
    // multiply both x and y by 3, and shift origin to (10, 2)
    int L8EFC[] = {
        0xad, 0xc5, 0x13,   //      lda board_y
        0x0a,               //      asl a
        0x6d, 0xc5, 0x13,   //      adc board_y
        0x69, 0x02,         //      adc #2
        0x8d, 0x8a, 0x13,   //      sta screen_y

        0xad, 0xc4, 0x13,   //      lda board_x
        0x0a,               //      asl a
        0x6d, 0xc4, 0x13,   //      adc board_x
        0x69, 0x0a,         //      adc #10
        0x8d, 0x8b, 0x13,   //      sta screen_x
        0x60,               //      rts
        STOP
    };
    bytes(0x8EFC, L8EFC);

    // 3 chars per line instead of 4
    byte(0x8F34, 3);

    // patch routine 8F36 to transfer graphics chars
    int L8F36[] = {
        0x20, 0xfc, 0x8e,   // jsr L8EFC
        0x20, 0x1d, 0x8f,   // jsr L8F1D       ; exits with x = 3
        0x8e, 0x8a, 0x13,   // stx screen_y    ; 3 lines of chars
                            // display_loop:
        0xb1, 0x5c,         // lda ($5C),y
        0x4d, 0x89, 0x13,   // eor $1389       ; eventually inverse the char
        0x49, 0x80,         // eor #$80        ; inverse all from Apple to Oric
        0x91, 0x5e,         // sta ($5E),y
        0xc8,               // iny
        0xca,               // dex
        0xd0, 0xf3,         // bne display_loop
        0x18,               // clc
        0xa5, 0x5e,         // lda $5E
        0x69, 0x25,         // adc #37
        0x85, 0x5e,         // sta $5E
        0xa5, 0x5f,         // lda $5F
        0x69, 0x00,         // adc #0
        0x85, 0x5f,         // sta $5F
        0xa2, 0x03,         // ldx #3          ; 3 chars per line
        0xce, 0x8a, 0x13,   // dec screen_y
        0xd0, 0xdf,         // bne display_loop
        0x60,               // rts
        STOP
    };
    bytes(0x8F36, L8F36);

    // patch routine 8FC3 to display a board coordinate
    int L8FC3[] = {
        0x20, 0x1d, 0x8f,   // jsr L8F1D
        0xb1, 0x5c,         // lda ($5C),y
        0x91, 0x5e,         // sta ($5E),y
        0x60,               // rts
        STOP
    };
    bytes(0x8FC3, L8FC3);

    // 3 lines of chars instead of 22 lines of graphics
    byte(0x902e, 3);

    // invert with $80 instead of $ff
    byte(0x8eb3, 0x80);
    byte(0x9038, 0x80);
    byte(0x907c, 0x80);
    byte(0x9091, 0x80);

    // patch 8F61 routine to show board coordinates
    byte(0x8F82, 1); // 1 line more instead of 8 graphic lines
    byte(0x8F87, 8); // column 8 instead of column 0
    byte(0x8FB7,27); // at line 27 instead of graphic line 184


/* TODO: find a way to display both screens simultaneously,
 *  because they are used even when not shown...
 *  Idea: group the normal list to the left, and group the
 *  window on search to the right (with a column of attributes
 *  to hide/show it)
    // replace line addresses to second screen
    int lsb[] = { 0x70, 0x98, 0xC0, 0xE8, 0x10, 0x38, 0x60, 0x88,
                  0xB0, 0xD8, 0x00, 0x28, 0x50, 0x78, 0xA0, 0xC8, STOP };
    int msb[] = { 0xBC, 0xBC, 0xBC, 0xBC, 0xBD, 0xBD, 0xBD, 0xBD,
                  0xBD, 0xBD, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, 0xBE, STOP };
    bytes(0xA85D, lsb);
    bytes(0xA86D, msb); 
*/

    // replace COUT
    int couts[] = { 0x8826, 0x889B, 0x945D, 0x949A, 0x9544, 0x955A, 0x955F, 0x956A, 0x9572, 0x9581, 0x9590, 0x959B, 0x95CD, 0x95D6, 0x95D9, 0x95E9,
        0x95F4, 0x95F9, 0x962D, 0x9A09, 0x9A14, 0x9A1D, 0x9A6C, 0x9A73, 0x9A8B, 0x9D1A, 0xADF2, 0xB594, 0xB6D6, 0xB6E0 };
    for (int i=0; i<NB(couts); i++) word(couts[i], COUT);

    // toggle video inverse bit
    inverse(0x883A);
    inverse(0x8841);
    inverse(0x884D);
    inverse(0x8859);
    inverse(0x88A2);
    inverse(0x88BA);
    inverse(0x933B);
    inverse(0x93C3);
    inverse(0x9AA6);
    inverse(0x9AC8);
    word(0x9AD7, 0xEAEA);
    byte(0x9ADA, 0x80);
    word(0x9AEA, 0x7F29);
    word(0x9AF7, 0x7F29);
    inverse(0x9B06);
    word(0x9B18, 0x7F29);
    word(0x9B1A, 0x2891);
    inverse(0x9DAE);
    inverse(0x9EC9);
    inverse(0x9ED1);
    inverse(0x9EEA);
    inverse(0x9EEE);
    inverse(0xA817);
    inverse(0xA827);
    inverse(0xA84B);
    inverse(0xA858);
    inverse(0xA895);
    inverse(0xA89B);
    inverse(0xA8B7);
    inverse(0xA9C9);
    for (int i=0; i<40; i++) {
        inverse(0x8504 + i);
        inverse(0x852C + i);
        inverse(0x8554 + i);
    } 

    // Sargon title with graphics chars
    string(0x8590, "             `abcdefghijk  yyy     LEVEL");
    string(0x85B8, "             lmnopqrstuvw  zzz          ");

    // remove initial message
    bytes(0x9123, nops);

    byte(0x880e, 0x02); // change window top

    // direct write the 2 first lines and skip move list preparation
    int initscreen[] = {
        0xa0, 79,           // ldy #79
        0xb9, 0x90, 0x85,   // lda 8590,y
        0x99, 0x80, 0xbb,   // sta BB80,y
        0x88,               // dey
        0x10, 0xf7,         // bpl *-9
        0x4c, 0x31, 0x88,   // jmp 8831
        STOP
    };
    bytes(0x8819, initscreen);

    byte(0x9D22, 0x4c); word(0x9D23, 0x9D32);

    // replace CTRL-H by DELETE
    byte(0x9AE1, 0xFF);
    // replace cursor movements
    byte(0x991b, 0x09);
    byte(0x9923, 0x0b);
    byte(0x992B, 0x0a);

    // replace CLRSCRN
    int clrscrns[] = { 0x8812, 0x8832, 0x9DDD, 0xAB70, 0xABED, 0xB6D1 };
    for (int i=0; i<NB(clrscrns); i++) word(clrscrns[i], HOME);

    // replace VTABZ
    int vtabz[] = { 0x8817, 0x882F, 0x8871, 0x9D30 };
    for (int i=0; i<NB(vtabz); i++) word(vtabz[i], VTABZ);

    //replace one SPEAKER soundloop by keyboard click
    int click[] = { 0x4C, 0x2A, 0xFB, STOP };
    bytes(0x91EB, click); // period 14 x128

    // and the other one by JMP PING
    int ping[] = { 0x4C, 0x9F, 0xFA, STOP };
    bytes(0x98C8, ping); // period 48 x48

    // replace VTAB
    word(0x9A7B, VTAB);

    // replace GETLN by ?
    // word(0xACDD, );

    // replace SETVID
    word(0x0E39, SETVID);
    word(0xB643, SETVID);
    word(0xBFC9, SETVID);

    // hook COUT to PRINTOUT
    byte(0x9534, PRINTOUT & 0xFF);
    byte(0x9538, PRINTOUT >> 8);

    // hook COUT to COUT1
    byte(0x9547, COUT1 & 0xFF);
    byte(0x954B, COUT1 >> 8);

    // remove printer tabulation for now
    byte(0x9557, 0x60);

    // change location of cursor on screen
    byte(0x9a54, 0); // cursor column for ?
    byte(0x9a60, 0); // cursor column for white player
    byte(0x9a64, 0); // cursor column for black player
    byte(0xb58e, 0); // cursor column for entry
/*
    // replace printer's CR with LF
    byte(0x8731, 0x0A);
    byte(0x8738, 0x0A);
    byte(0x8740, 0x0A);
    byte(0x875B, 0x0A);
    byte(0x8776, 0x0A);
    byte(0x8794, 0x0A);

    byte(0x9568, 0x0A);
    byte(0x9570, 0x0A);
    byte(0x957F, 0x0A);
    byte(0x958E, 0x0A);
    byte(0x9A6A, 0x0A);
*/
    // replace first SETKBD with clrscreen
    word(0x0E3C, HOME);

    // ignore other SETKBD
    bytes(0xB640, nops);
    bytes(0xB744, nops);

    // don't print the move number
    bytes(0x8864, nops);
    bytes(0x95A4, nops);
    bytes(0x99A5, nops);

    // clear end of line by writing 4 spaces (instead of 35)
    byte(0x9B57, 4);

    // go to new-line after each move
    byte(0x999E, 0x03);

    // print a lowercase x instead of an X for a capture
    byte(0x99DB, 0x78);
    byte(0x8657, 0x78);

    // ignore JSR B186 for now: opening book preloaded
    bytes(0x912C, nops);
    bytes(0x92C9, nops);

    /*
    // pretend read of library succeeded
    bytes(0xB1AF, nops);
    byte (0x3DFD, 0);
    */


    // move references of pages B5-BF to pages 20-2B
    int refB5[] = { 
        0x9122, 0x9A3F, 0xB302, 0xB311, 0xB316, 0xB31C, 0xB326,
        0xB329, 0xB32C, 0xB330, 0xB334, 0xB337, 0xB33A, 0xB340,
        0xB343, 0xB346, 0xB349, 0xB34C, 0xB34F, 0xB352, 0xB355,
        0xB35A, 0xB35D, 0xB381, 0xB389, 0xB66D, 0xB678, 0xB67D,
        0xB688, 0xB6A3, 0xB7C4, 0xB7CF };
    for (int i=0; i<NB(refB5); i++) {
        assert(mem[refB5[i]] == 0xB5);
        byte(refB5[i], 0x20);
    }

    int refB6[] = {
        0xB668, 0xB673, 0xB682, 0xB6B0, 0xB774, 0xB785 };
    for (int i=0; i<NB(refB6); i++) {
        assert(mem[refB6[i]] == 0xB6);
        byte(refB6[i], 0x21);
    }

    int refB7[] = {
        0x0FE4,
        0xB702, 0xB705, 0xB70A, 0xB70D, 0xB710, 0xB713, 0xB718, 0xB71D,
        0xB720, 0xB724, 0xB729, 0xB740, 0xB74C, 0xB750, 0xB753, 0xB756,
        0xB759, 0xB75C, 0xB761, 0xB766, 0xB76B, 0xB771, 0xB77A, 0xB77F,
        0xB788, 0xB795, 0xB798, 0xB79E, 0xB7A8, 0xB7AB, 0xB7AE, 0xB7B1,
        0xB7C7, 0xB7CC, 0xB7D4, 0xB73A, 0xB76E, 0xB78B, 0xB79B, 0xBFD8};
    for (int i=0; i<NB(refB7); i++) {
        assert(mem[refB7[i]] == 0xB7);
        byte(refB7[i], 0x22);
    }

    int refB8[] = {
        0xB84E, 0xB856, 0xB85B, 0xB860, 0xB89C, 0xB8A1, 0xB8A6, 0xB8AB,
        0xB8B0, 0xBDBB, 0xBE37, 0xBE42, 0xBE53, 0xBEF6, 0xBF1E, 0xBF69,
        0xBF7E};
    for (int i=0; i<NB(refB8); i++) {
        assert(mem[refB8[i]] == 0xB8);
        byte(refB8[i], 0x23);
    }

    int refB9[] = {
        0xB9D3, 0xB9DF, 0xBDC6, 0xBE8D, 0xBEED, 0xBF4B, 0xBF64, 0xBF73};
    for (int i=0; i<NB(refB9); i++) {
        assert(mem[refB9[i]] == 0xB9);
        byte(refB9[i], 0x24);
    }

    int refBA[] = {
        0xB86F, 0xB884, 0xB897, 0xB9D6, 0xB9E2,
        0xB90B, 0xB91C, 0xB92C, 0xB9D9, 0xB9E5, 0xB9EC, 0xBD89 };
    for (int i=0; i<NB(refBA); i++) {
        assert(mem[refBA[i]] == 0xBA);
        byte(refBA[i], 0x25);
    }

    int refBB[] = {
        0xB811, 0xB86B, 0xB880, 0xB890, 0xB8CB, 0xB921, 0xBEC1, 0xBEC7};
    for (int i=0; i<NB(refBB); i++) {
        assert(mem[refBB[i]] == 0xBB);
        byte(refBB[i], 0x26);
    }

    int refBC[] = {
        0xB80A, 0xB80E, 0xB820, 0xB825, 0xB83A, 0xB868, 0xB8CE, 0xB8D2,
        0xB910, 
        0xBC6B, 0xBC6E, 0xBC7D, 0xBC82, 0xBC87, 0xBC8C, 0xBC91, 0xBC96,
        0xBCAC, 0xBCB1, 0xBCB6, 0xBCBB, 0xBF19};
    for (int i=0; i<NB(refBC); i++) {
        assert(mem[refBC[i]] == 0xBC);
        byte(refBC[i], 0x27);
    }

    int refBD[] = {
        0x0FDB, 0xB7B9, 0xBDEC };
    for (int i=0; i<NB(refBD); i++) {
        assert(mem[refBD[i]] == 0xBD);
        byte(refBD[i], 0x28);
    }

    int refBE[] = {
        0xBD96, 0xBDD6, 0xBDE5, 0xBDE9, 0xBDFB, 0xBE0A, 0xBE0F,
        0xBE66, 0xBE6F, 0xBE9F, 0xBECF, 0xBED8};
    for (int i=0; i<NB(refBE); i++) {
        assert(mem[refBE[i]] == 0xBE);
        byte(refBE[i], 0x29);
    }

    int refBF[] = {
        0xBE2D, 0xBF34, 0xBF8C, 0xBF93, 
        0xB379, 0xB743, 0xBEDB, 0xBF3C, 0xBF3F, 0xBF42 };
    for (int i=0; i<NB(refBF); i++) {
        assert(mem[refBF[i]] == 0xBF);
        byte(refBF[i], 0x2A);
    }
}

void plot(int y, int x, char *str) {
    for ( ; *str; x++,str++)
        mem[0xbb80+y*40+x] = *str;
}

void build_screen() {
    for (int i=0xbb80; i<0xbfe0; i++) mem[i]=0x20;
    for (int i=0; i<24; i++) mem[0xbb80+(i+2)*40+ 9] = 9; // alt charset
    for (int i=0; i<24; i++) mem[0xbb80+(i+2)*40+34] = 8; // std charset
    plot(27, 10, " A  B  C  D  E  F  G  H");

    for (int row=0; row<8; row++)
        mem[0xbb80 + (3*row+3)*40 + 8] = '8'-row;
}

int main(void) {
    FILE *in = fopen("sargoniii#0x0e00.BIN", "r");
    fread(mem+0x0e00, 1,  256, in);             // transfer routines
    fread(mem+0x0f00, 1,  256, in);             // normally sent to page 3
    fread(mem+0x4000, 1, 8192, in);             // Sargon III graphics screen 2
    fread(mem+0x6000, 1, 0xC000-0x6000, in);    // main code and data
    fclose(in);

//    in = fopen("b000#0x1000.BIN", "r");         // root of opening library
    in = fopen("bc80#0x1000.BIN", "r");
//    in = fopen("book.extract", "r");
    fread(mem+0x3004, 1, 4096, in);
    fclose(in);

    all_patches();
    memcpy(mem+0x2000, mem+0xB500, 0xC000-0xB500); // move pages B5-BF to 20-2A

    in = fopen("oric_patterns.bin", "r");
    fseek(in, 256L, SEEK_SET);
    fread(mem+0xb500, 0xbb80-0xb500, 1, in);
    fclose(in);

    build_screen();

    FILE *out = fopen("sargon3.tap","w");
    fwrite(header, 1, sizeof(header), out);
    fwrite(name  , 1, sizeof(name)  , out);
    fwrite(mem+START, 1, END+1-START, out);
    fclose(out);
    return 0;
}
