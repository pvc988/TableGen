#include "stdlib.h"

#include "transition.h"

Transition *TransitionCreate(Symbol *sym, State *state)
{
    Transition *trans = (Transition *)malloc(sizeof(Transition));
    trans->Symbol = sym;
    trans->State = state;
    return trans;
}

void TransitionDelete(Transition *trans)
{
    free(trans);
}
