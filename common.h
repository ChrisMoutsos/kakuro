#ifndef COMMON_H
#define COMMON_H

enum CellType { NONCLUE = 0, CLUE };

enum Direction { UP = 0, RIGHT, DOWN, LEFT };

enum Color_t { CLUECOLOR = 0, CLUETEXTCOLOR, NONCLUECOLOR,
               NONCLUETEXTCOLOR, NOTECOLOR, SELECTCOLOR,
               BORDERCOLOR };

struct CellInfo {
    //Nonclue or clue
    CellType type;
    //Value, or downClue and RightClue
    int valueOrClues[2];
    bool notes[10];
    bool fixed;
};

struct CellPos {
    int row;
    int col;
};

#endif

