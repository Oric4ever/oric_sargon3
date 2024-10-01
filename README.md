# Unofficial port of Sargon III for the Oric

More than 40 years later, I finally dived into a port of Sargon III for the Oric...

It started with a [reverse engineering of the Apple II version](https://github.com/Oric4ever/sargon3_disassembly),
and then many patches to adapt to the different hardware and memory map...

So here is the first public version, with the following main differences with the Apple II version :

- redefined graphics : a 2D board is always on screen along with a moves list on the left.
- no-disk version : the Oric version is provided as a tape version

## How to use

Run the provided [sargon3.tap](https://github.com/Oric4ever/oric_sargon3/blob/main/sargon3.tap) in an Oric emulator.

Moves are entered in algebraic notation (eg. E2-E4). Read [the manual](https://archive.org/details/apple2_hayden_sargon_iii_manual_1983) to learn all the functions ! 

NB: You will know that Sargon III is waiting for you when you see the block cursor in the left.


## How to build

You will first need to extract the files from the [Apple II disk](https://archive.org/download/Sargon_III_Hayden_Book_Company-1983/Sargon_III_Hayden_Book_Company-1983.dsk).

A simple Makefile is provided with a few quick & dirty tools...

Basically the `patcher` tool will take the AppleII binary and an Openings Book file, apply patches and add Oric graphics in order to build `sargon3.tap`.

## Work In Progress

- for now the Oric version starts with a preloaded Openings Book file, it does not start with the root of the Openings book, and it won't load any Openings Book file afterwards.
So I will try to reclaim some space in the memory map in order to have a larger Openings Library, and an option to load specific Openings (for training).
- currently trying to replace the approximate timings with precise timings (the Apple II has no hardware timer, the Oric has a VIA)...
- will try to add a Level 0 so that Sargon is more accessible for chess beginners (currently even level 1E might be challenging)...
- will provide a compact version of the window view on Sargon's computation, I have planned a moves column for that on the right...
