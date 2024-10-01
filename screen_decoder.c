#include <stdio.h>
#include <stdint.h>

uint8_t mem[64*1024];

int main(void) {
    FILE *f = fopen("sargoniii#0x0e00.BIN", "r");
    fread(mem+0x0e00,65536,1,f);
    fclose(f);

    FILE *screen = fopen("screen.pbm", "w");
    fprintf(screen, "P1\n");
    fprintf(screen, "280 192\n");
    for (int part=0; part < 3; part++) {
        for (int line=0; line < 8; line++) {
            for (int offset=0; offset < 8; offset++) {
                int addr = 0x1000 + part * 40 + line * 128 + offset * 1024;
                for (int column = 0; column < 40; column++) 
                    for (int bit = 1; bit < 128; bit *= 2)
                        fprintf(screen, "%c", mem[addr+column]&bit ? '0' : '1');
                fprintf(screen, "\n");
            }
        }
    }
    fclose(screen);

    FILE *graph = fopen("graphics.pbm", "w");
    fprintf(graph, "P1\n");
    fprintf(graph, "28 286\n");
    for (int block = 0; block < 13; block++) {
        for (int line=0; line < 22; line++) {
            for (int column = 0; column < 4; column++) {
                int addr = 0x4ad9 + 0x0e00 + block*(28/7*22+1) + line * 4 + column;
                for (int bit = 1; bit < 128; bit *= 2)
                    fprintf(graph, "%c", mem[addr]&bit ? '0' : '1');
            }
            fprintf(graph, "\n");
        }
    } 
    fclose(graph);

    FILE *letters = fopen("letters.pbm", "w");
    fprintf(letters, "P1\n");
    fprintf(letters, "14 128\n");
    for (int line=0; line < 128; line++) {
        for (int column = 0; column < 2; column++) {
            int addr = 0x4ad9 + 0x0e00 + 13*(28/7*22+1) + line*2 + column ;
            for (int bit = 1; bit < 128; bit *= 2)
                fprintf(letters, "%c", mem[addr]&bit ? '0' : '1');
        }
        fprintf(letters, "\n");
    }
    fclose(letters);


    return 0;
}

