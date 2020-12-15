#pragma once

#include "stdbool.h"
#include "stddef.h"

typedef struct FSM FSM;
typedef struct Production Production;
typedef struct State State;
typedef struct Symbol Symbol;
typedef struct Vector Vector;

typedef enum ActionType
{
    AT_ERROR = 0,
    AT_ACCEPT,
    AT_SHIFT,
    AT_REDUCE,
    AT_GOTO
} ActionType;

typedef struct Action
{
    ActionType Type;
    union
    {
        State *State;
        Production *Production;
    };
} Action;

typedef struct ParseTable
{
    FSM *FSM;
    size_t ColumnCount;
    size_t RowCount;
    Symbol **Header;
    Action *Actions;
} ParseTable;

ParseTable *ParseTableCreate(FSM *FSM);
void ParseTableDelete(ParseTable *pt);
bool ParseTableToFile(ParseTable *pt, const char *filename, bool compact);
