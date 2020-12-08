#pragma once

typedef struct Grammar Grammar;
typedef struct Vector Vector;

typedef struct FSM
{
    Grammar *Grammar;
    Vector *States;
} FSM;

FSM *FSMCreate(Grammar *grammar);
void FSMDelete(FSM *fsm);
void FSMBuildStates(FSM *fsm);
