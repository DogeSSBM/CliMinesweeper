#ifndef PRINT_H
#define PRINT_H

void usage(void)
{
    fprintf(stderr, "Usage: ./main.out <grid width 0..99> <grid height 0..99> <num bombs 0..99>\n");
    exit(EXIT_FAILURE);
}

void boardPrintInfo(const Board board)
{
    printf("Board -\n");
    printf("\tlen.x: %u\n", board.len.x);
    printf("\tlen.y: %u\n", board.len.y);
    printf("\tnumBombs: %u\n", board.numBombs);
}

void xPrint(const int x)
{
    fputs("   |", stdout);
    for(int i = 0; i < x; i++)
        printf("%2u |", i);
    fputs("\n", stdout);
}

void linePrint(const int x)
{
    for(int i = 0; i <= x; i++)
        fputs("---+", stdout);
    fputs("---\n", stdout);
}

void boardPrint(const Board board, const bool reveal)
{
    xPrint(board.len.x);
    linePrint(board.len.x);
    for(int y = 0; y < board.len.y; y++){
        printf("%2u |", y);
        for(int x = 0; x < board.len.x; x++){
            switch(board.tile[x][y].state){
                case TS_HIDE:
                    if(reveal){
                        if(board.tile[x][y].num == -1)
                            fputs("[B]|", stdout);
                        else
                            printf("[%c]|", board.tile[x][y].num == 0 ? ' ' : '0'+board.tile[x][y].num);
                    }else{
                        fputs("[ ]|", stdout);
                    }
                    break;
                case TS_REVL:
                    if(board.tile[x][y].num == 0)
                        fputs("   |", stdout);
                    else if(board.tile[x][y].num == -1)
                        fputs("}#{|",  stdout);
                    else
                        printf("%2u |", board.tile[x][y].num);
                    break;
                case TS_FLAG:
                    fputs("[F]|", stdout);
                    break;
                case TS_QEST:
                    fputs("[?]|", stdout);
                    break;
                default:
                    fprintf(stderr, "board.tile[%u][%u].state is invalid(%i)\n", x, y, board.tile[x][y].state);
                    exit(EXIT_FAILURE);
            }
        }
        printf("%2u\n", y);
        linePrint(board.len.x);
    }
    xPrint(board.len.x);
    fputs("\n", stdout);
}

void actionPrint(const Action act)
{
    fputs("Action -\n", stdout);
    printf("\tact.type: %s\n", TileStateStr[act.type]);
    printf("\tact.pos: (%u,%u)\n\n", act.pos.x, act.pos.y);
}

void actionHelp(void)
{
    fputs("Action help -\n", stdout);
    fputs("\tTo reveal a tile:\n\t\tr <x position> <y positon>\n", stdout);
    fputs("\tTo flag a tile:\n\t\tf <x position> <y positon>\n", stdout);
    fputs("\tTo question mark a tile:\n\t\tq <x position> <y positon>\n", stdout);
    fputs("\tTo unmark a tile:\n\t\tu <x position> <y positon>\n", stdout);
}

#endif /* end of include guard: PRINT_H */
