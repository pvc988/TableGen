#include <stdlib.h>

#include "state.h"
#include "vector.h"

State *StateCreate(void)
{
    State *state = (State *)malloc(sizeof(State));
    state->Items = VectorCreate();
    state->Transitions = VectorCreate();
    return state;
}

void StateDelete(State *state)
{
    if(state->Items) VectorDelete(state->Items);
    if(state->Transitions) VectorDelete(state->Transitions);
    free(state);
}
