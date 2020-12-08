#pragma once

typedef struct Vector Vector;

typedef struct State
{
    Vector *Items;
    Vector *Transitions;
} State;

State *StateCreate(void);
void StateDelete(State *state);
