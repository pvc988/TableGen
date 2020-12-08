#pragma once

typedef struct Grammar Grammar;
typedef struct State State;
typedef struct Vector Vector;

typedef struct FSM
{
    Grammar *Grammar;
    Vector *States;
    State *Accept;
} FSM;

FSM *FSMCreate(Grammar *grammar);
void FSMDelete(FSM *fsm);
void FSMBuildLR1States(FSM *fsm);
void FSMBuildLALR1States(FSM *fsm);
