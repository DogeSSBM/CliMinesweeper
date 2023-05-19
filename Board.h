#ifndef BOARD_H
#define BOARD_H

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
    if(strlen(boardArgs[0]) > 2 || strlen(boardArgs[1]) > 2 || strlen(boardArgs[2]) > 2)
        usage();
    Board board = {0};
    int* bi[3] = {&board.len.x, &board.len.y, &board.numBombs};
    for(uint i = 0; i < 3; i++){
        char *pos = boardArgs[i];
        if(!parseLe2(&pos, bi[i]) || *pos != '\0')
            usage();
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
                if((x == 0 && y == 0) || !inBound(adjpos, board.len) || board.tile[adjpos.x][adjpos.y].num == -1)
                    continue;
                board.tile[adjpos.x][adjpos.y].num++;
            }
        }
    }
    board.state = G_PLAY;
    return floodAt(board, fst);
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

#endif /* end of include guard: BOARD_H */
