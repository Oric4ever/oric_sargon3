#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define STD 0
#define ALT 1
int pattern_index=32;
uint8_t patterns[2][128][8];

bool same_pattern(int n, int m) {
    bool same = true;
    for (int row=0; row<8; row++)
        if (patterns[ALT][n][row] != patterns[ALT][m][row])
            same = false;
    return same;
}

bool inversed_pattern(int n, int m) {
    bool same = true;
    for (int row=0; row<8; row++)
        if ((patterns[ALT][n][row]^0b111111) != patterns[ALT][m][row])
            same = false;
    return same;
}

int existing_pattern()
{
    for (int i=32; i<pattern_index; i++) {
        if (same_pattern(pattern_index, i)) return i+32;
        if (inversed_pattern(pattern_index, i)) return i+32+128;
    }
    return 0;
}


int main(void) {
    FILE *in = fopen("oric_charset.bin", "r");
    fread(&patterns[0][32], 96, 8, in);
    fclose(in);

    in = fopen("oric_graphics.pbm", "r");
    char buf[8][80];
    fgets(buf[0], 80, in); // P1
    fgets(buf[0], 80, in); // dimensions

    for (int i=0; i < 12; i++) {
        fgets(buf[0], 80, in); assert(buf[0][0] == '\n'); // empty line

        for (int row=0; row<3; row++) {
	        for (int line=0; line<8; line++)
	            fgets(buf[line], 80, in);
	
	        for (int offset=0; offset<=12; offset+=6) {
	            for (int line=0; line<8; line++) {
	                uint8_t pattern = 0;
	                for (int bit=5; bit>=0; bit--) {
	                    bool set = buf[line][offset+(5-bit)] == '1';
	                    if (set) pattern |= (1 << bit);
	                }
	                patterns[ALT][pattern_index][line] = pattern;
	            }
                int old_pattern = existing_pattern();
                printf("%02x ", old_pattern ? old_pattern : pattern_index);
    	        if (!old_pattern) pattern_index++;
	        }
            printf("\n");
	    }
        printf("\n");
    }
    fclose(in);

    in = fopen("title.pbm", "r");
    fgets(buf[0], 80, in); // P1
    fgets(buf[0], 80, in); // dimensions
    pattern_index = 96; // replace lowercase letters
    for (int row=0; row<2; row++) {
        for (int line=0; line<8; line++)
            fgets(buf[line], 80, in);
	
        for (int offset=0; offset<72; offset+=6, pattern_index++) {
            for (int line=0; line<8; line++) {
                uint8_t pattern = 0;
                for (int bit=5; bit>=0; bit--) {
                    bool set = buf[line][offset+(5-bit)] == '0';
                    if (set) pattern |= (1 << bit);
                }
                patterns[STD][pattern_index][line] = pattern;
            }
        }
    }
    patterns[STD]['y'][0] = 0b000000;
    patterns[STD]['y'][1] = 0b011111;
    patterns[STD]['y'][2] = 0b111110;
    patterns[STD]['y'][3] = 0b011100;
    patterns[STD]['y'][4] = 0b011100;
    patterns[STD]['y'][5] = 0b011100;
    patterns[STD]['y'][6] = 0b011100;
    patterns[STD]['y'][7] = 0b011100;
    patterns[STD]['z'][0] = 0b011100;
    patterns[STD]['z'][1] = 0b011100;
    patterns[STD]['z'][2] = 0b011100;
    patterns[STD]['z'][3] = 0b011100;
    patterns[STD]['z'][4] = 0b111110;
    patterns[STD]['z'][5] = 0b000000;
    patterns[STD]['z'][6] = 0b000000;
    patterns[STD]['z'][7] = 0b000000;



    FILE *out = fopen("oric_patterns.bin", "w");
    fwrite(patterns, 32+96+32+80, 8, out);
    fclose(out);

    return 0;
}
