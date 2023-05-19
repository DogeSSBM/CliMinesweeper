#ifndef ACTION_H
#define ACTION_H

TileState actionChar(const char c)
{
    switch(c){
        case 't': return TS_HIDE;
        case 'r': return TS_REVL;
        case 'f': return TS_FLAG;
        case 'q': return TS_QEST;
        case 'u': return TS_HIDE;
        default: break;
    }
    return TS_INVL;
}

bool actionParse(const Board board, char *str, Action *act)
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

Action actionRead(const Board board)
{
    Action act = {0};
    while(1){
        char buff[16] = {0};
        buff[0] = '\0';
        fputs("Enter an action or ? for help.\n", stdout);
        char* buf = fgets(buff, 16, stdin);
        if(!buf || buf[0] == '?' || !actionParse(board, buf, &act))
            actionHelp();
        else
            break;
    }
    return act;
}

#endif /* end of include guard: ACTION_H */
