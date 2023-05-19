#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint;

typedef struct{
    int x;
    int y;
}Coord;

typedef enum{G_PLAY, G_LOST, G_WIN, G_NUM}GameState;
const char *GameStateStr[G_NUM] = {"G_PLAY", "G_LOST", "G_WIN"};
typedef enum{TS_HIDE, TS_REVL, TS_FLAG, TS_QEST, TS_INVL, TS_NUM}TileState;
const char *TileStateStr[TS_NUM] = {"TS_HIDE", "TS_REVL", "TS_FLAG", "TS_QEST", "TS_INVL"};

typedef struct{
    TileState type;
    Coord pos;
}Action;

typedef struct{
    int num;    // -1 if tile is bomb
    TileState state;
}Tile;

typedef struct{
    GameState state;
    Coord len;
    int numBombs;
    Tile **tile;
}Board;

#endif /* end of include guard: TYPES_H */
