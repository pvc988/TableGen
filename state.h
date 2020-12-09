#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct State State;
typedef struct Symbol Symbol;
typedef struct Transition Transition;
typedef struct Vector Vector;

typedef struct State
{
    size_t Index;
    Vector *Items;
    Vector *Transitions;
} State;

State *StateCreate(void);
void StateDelete(State *state);
bool StateSimilar(State *a, State *b);
bool StateEquivalent(State *a, State *b);
Transition *StateGetTransition(State *state, Symbol *symbol);
