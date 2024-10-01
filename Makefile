CFLAGS= -fsanitize=bounds-strict -Wall -g

all: book_decoder patcher book_rebuild screen_decoder charset_encoder board_print

patcher: patcher.c
	gcc -Wall -o patcher patcher.c 
	./patcher
	cp sargon3.tap ../../../Oric/TAPES/

book_decoder: book_decoder.c
	gcc $(CFLAGS) -o book_decoder book_decoder.c

book_rebuild: book_rebuild.c
	gcc $(CFLAGS) -o book_rebuild book_rebuild.c

screen_decoder: screen_decoder.c
	gcc $(CFLAGS) -o screen_decoder screen_decoder.c

charset_encoder: charset_encoder.c
	gcc $(CFLAGS) -o charset_encoder charset_encoder.c

board_print: board_print.c
	gcc $(CFLAGS) -o board_print board_print.c
