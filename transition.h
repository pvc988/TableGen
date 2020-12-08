#pragma once

typedef struct State State;
typedef struct Symbol Symbol;

typedef struct Transition
{
    Symbol *Symbol;
    State *State;
} Transition;

Transition *TransitionCreate(Symbol *sym, State *state);
void TransitionDelete(Transition *trans);
