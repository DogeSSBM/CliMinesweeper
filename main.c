#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#include "Types.h"
#include "Print.h"
#include "Board.h"
#include "Action.h"

int main(int argc, char** argv)
{
    srand(time(0));
    Board board = boardAlloc(boardArgs(argc, argv));
    boardPrintInfo(board);
    boardPrint(board, false);
    Action act = {0};
    do{
        act = actionRead(board);
    }while(act.type != TS_REVL);
    board = boardPlaceBombs(board, act.pos);

    while(board.state == G_PLAY){
        boardPrint(board, false);
        act = actionRead(board);
        switch(act.type){
            case TS_HIDE:
                if(board.tile[act.pos.x][act.pos.y].state != TS_REVL)
                    board.tile[act.pos.x][act.pos.y].state = TS_HIDE;
                break;
            case TS_REVL:
                board = floodAt(board, act.pos);
                if(board.state != G_LOST && boardTilesLeft(board) == 0)
                    board.state = G_WIN;
                break;
            case TS_FLAG:
                if(board.tile[act.pos.x][act.pos.y].state != TS_REVL)
                    board.tile[act.pos.x][act.pos.y].state = TS_FLAG;
                break;
            case TS_QEST:
                if(board.tile[act.pos.x][act.pos.y].state != TS_REVL)
                    board.tile[act.pos.x][act.pos.y].state = TS_QEST;
                break;
            default:
                fprintf(stderr, "act.type is invalid(%i)\n", act.type);
                exit(EXIT_FAILURE);
        }
    }
    boardPrint(board, true);
    board = boardFree(board);

    return 0;
}
