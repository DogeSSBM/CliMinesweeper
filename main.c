#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
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

void printX(const int x)
{
    fputs("   |", stdout);
    for(int i = 0; i < x; i++)
        printf("%2u |", i);
    fputs("\n", stdout);
}

void printLine(const int x)
{
    for(int i = 0; i <= x; i++){
        fputs("---+", stdout);
    }
    fputs("---\n", stdout);
}

void boardPrint(const Board board, const bool reveal)
{
    printX(board.len.x);
    printLine(board.len.x);
    for(int y = 0; y < board.len.y; y++){
        printf("%2u |", y);
        for(int x = 0; x < board.len.x; x++){
            switch(board.tile[x][y].state){
                case TS_HIDE:
                    if(reveal)
                        if(board.tile[x][y].num == -1)
                            fputs("[B]|", stdout);
                        else
                            printf("[%c]|", board.tile[x][y].num == 0 ? ' ' : '0'+board.tile[x][y].num);
                    else
                        fputs("[ ]|", stdout);
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
        printLine(board.len.x);
    }
    printX(board.len.x);
    fputs("\n", stdout);
}

bool parseLe2(char **strptr, int *num)
{
    const uint slen  = strlen(*strptr);
    if(slen < 1 || !isdigit(**strptr))
        return false;
    *num = **strptr - '0';
    (*strptr)++;
    if(slen == 1)
        return true;
    if(isspace(**strptr))
        return true;
    if(!isdigit(**strptr))
        return false;
    *num *= 10;
    *num += **strptr - '0';
    (*strptr)++;
    return true;
}

Board boardParse(char** boardArgs)
{
    if(strlen(boardArgs[0]) > 2 || strlen(boardArgs[1]) > 2 || strlen(boardArgs[2]) > 2){
        usage();
    }
    Board board = {0};
    int* bi[3] = {&board.len.x, &board.len.y, &board.numBombs};
    for(uint i = 0; i < 3; i++){
        char *pos = boardArgs[i];
        if(!parseLe2(&pos, bi[i]) || *pos != '\0'){
            printf("itr:%i\n", i);
            usage();
        }
    }

    if(board.len.x < 4 || board.len.y < 4 || board.len.x >= 99 || board.len.y >= 99 ||
    ((int)board.numBombs >= (board.len.x * board.len.y)-8)){
        fprintf(stderr, "board.numBombs: %u must be < %u\n(board.len.x(%u) * board.len.y(%u))\nboard.board.len.x(%u) and board.len.y(%u) must be >= 4 and < 99\n",
            board.numBombs, board.len.x * board.len.y, board.len.x, board.len.y,
            board.len.x, board.len.y
        );
        exit(EXIT_FAILURE);
    }
    return board;
}

Board boardFree(Board board)
{
    if(board.tile){
        for(int x = 0; x < board.len.x; x++){
            if(board.tile[x])
                free(board.tile[x]);
        }
        free(board.tile);
    }
    return board;
}

Board boardAlloc(Board board)
{
    board.tile = calloc(board.len.x, sizeof(Tile*));
    for(int x = 0; x < board.len.x; x++)
        board.tile[x] = calloc(board.len.y, sizeof(Tile));
    return board;
}

bool inBound(const Coord pos, const Coord len)
{
    return pos.x >= 0 && pos.y >= 0 && pos.x < len.x && pos.y < len.y;
}

Board floodAt(Board board, const Coord pos)
{
    board.tile[pos.x][pos.y].state = TS_REVL;
    const int num = board.tile[pos.x][pos.y].num;
    if(num == -1)
        board.state = G_LOST;
    if(num == -1 || num != 0)
        return board;

    for(int x = -1; x <= 1; x++){
        for(int y = -1; y <= 1; y++){
            const Coord adjpos = (const Coord){.x = pos.x+x, .y = pos.y+y};
            if((x == 0 && y == 0) || !inBound(adjpos, board.len))
                continue;
            const int adjnum = board.tile[adjpos.x][adjpos.y].num;
            const TileState adjstate = board.tile[adjpos.x][adjpos.y].state;
            if(adjnum == 0 && adjstate == TS_HIDE)
                board = floodAt(board, adjpos);
            else if(adjnum > 0  && adjstate == TS_HIDE)
                board.tile[adjpos.x][adjpos.y].state = TS_REVL;
        }
    }
    return board;
}

uint boardTilesLeft(const Board board)
{
    uint left = 0;
    for(int y = 0; y < board.len.y; y++)
        for(int x = 0; x < board.len.x; x++)
            left += board.tile[x][y].num >= 0 && board.tile[x][y].state != TS_REVL;

    return left;
}

Board boardPlaceBombs(Board board, const Coord fst)
{
    for(int i = 0; i < board.numBombs; i++){
        Coord pos = {0};
        do{
            pos.x = rand() % board.len.x;
            pos.y = rand() % board.len.y;
        }while(
            (pos.x >= fst.x-1 && pos.y >= fst.y-1 && pos.x <= fst.x+1 && pos.y <= fst.y+1) ||
            board.tile[pos.x][pos.y].num == -1
        );
        board.tile[pos.x][pos.y].num = -1;
        for(int x = -1; x <= 1; x++){
            for(int y = -1; y <= 1; y++){
                const Coord adjpos = {.x = pos.x + x, .y = pos.y +y};
                if((x == 0 && y == 0) || !inBound(adjpos, board.len))
                    continue;
                board.tile[pos.x+x][pos.y+y].num++;
            }
        }
    }
    board.state = G_PLAY;
    return floodAt(board, fst);
}

void actionHelp(void)
{
    fputs("Action help -\n", stdout);
    fputs("\tTo reveal a tile:\n\t\tr <x position> <y positon>\n", stdout);
    fputs("\tTo flag a tile:\n\t\tf <x position> <y positon>\n", stdout);
    fputs("\tTo question mark a tile:\n\t\tq <x position> <y positon>\n", stdout);
    fputs("\tTo unmark a tile:\n\t\tu <x position> <y positon>\n", stdout);
}

TileState actionChar(const char c)
{
    switch(c){
        case 't':
            return TS_HIDE;
            break;
        case 'r':
            return TS_REVL;
            break;
        case 'f':
            return TS_FLAG;
            break;
        case 'q':
            return TS_QEST;
            break;
        default:
            break;
    }
    return TS_INVL;
}

bool actionValidate(const Board board, char *str, Action *act)
{
    const uint slen = strlen(str);
    if(str[slen-1] != '\n'){
        int c = ' ';
        while((c = fgetc(stdin)) != '\n' && c != '\0');
        return false;
    }
    // "r 10 10"
    // "r 1 1"
    if((act->type = actionChar(*str)) == TS_INVL || str[1] != ' ')
        return false;
    str+=2;
    if(!parseLe2(&str, &(act->pos.x)) || *str != ' ')
        return false;
    str++;
    if(!parseLe2(&str, &(act->pos.y)) || *str != '\n')
        return false;
    if(!inBound(act->pos, board.len))
        return false;
    return true;
}

void actionPrint(const Action act)
{
    fputs("Action -\n", stdout);
    printf("\tact.type: %s\n", TileStateStr[act.type]);
    printf("\tact.pos: (%u,%u)\n\n", act.pos.x, act.pos.y);
}

Action actionParse(const Board board)
{
    Action act = {0};
    while(1){
        char buff[16] = {0};
        buff[0] = '\0';
        fputs("Enter an action or ? for help.\n", stdout);
        char* buf = fgets(buff, 16, stdin);
        printf("buf[%zu]:\"%s\"\n", strlen(buf), buf);
        if(!buf || buf[0] == '?' || !actionValidate(board, buf, &act))
            actionHelp();
        else
            break;
    }
    actionPrint(act);
    return act;
}

Board boardArgs(int argc, char** argv)
{
    Board board = {0};
    if(argc == 1){
        board.len.x = 12;
        board.len.y = 8;
        board.numBombs = 32;
    }else if(argc == 4){
        board = boardParse(&argv[1]);
    }else{
        usage();
    }
    return board;
}

int main(int argc, char** argv)
{
    srand(time(0));
    Board board = boardAlloc(boardArgs(argc, argv));
    boardPrintInfo(board);
    boardPrint(board, false);
    Action act = {0};
    do{
        act = actionParse(board);
    }while(act.type != TS_REVL);
    board = boardPlaceBombs(board, act.pos);

    while(board.state == G_PLAY){
        boardPrint(board, false);
        act = actionParse(board);
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
