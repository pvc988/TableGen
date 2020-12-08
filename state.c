#include <stdlib.h>

#include "item.h"
#include "state.h"
#include "transition.h"
#include "vector.h"

State *StateCreate(void)
{
    State *state = (State *)malloc(sizeof(State));
    state->Index = (size_t)-1;
    state->Items = VectorCreate();
    state->Transitions = VectorCreate();
    return state;
}

void StateDelete(State *state)
{
    if(state->Items)
    {
        for(size_t i = 0; i < state->Items->ItemCount; ++i)
        {
            Item *it = (Item *)state->Items->Items[i];
            ItemDelete(it);
        }
        VectorDelete(state->Items);
    }
    if(state->Transitions)
    {
        for(size_t i = 0; i < state->Transitions->ItemCount; ++i)
        {
            Transition *t = (Transition *)state->Transitions->Items[i];
            TransitionDelete(t);
        }
        VectorDelete(state->Transitions);
    }
    free(state);
}

bool StateSimilar(State *a, State *b)
{
    if(a->Items->ItemCount != b->Items->ItemCount)
        return false;

    for(size_t i = 0; i < a->Items->ItemCount; ++i)
    {
        size_t eqCount = 0;
        Item *u = (Item *)a->Items->Items[i];
        for(size_t i = 0; i < b->Items->ItemCount; ++i)
        {
            Item *v = (Item *)b->Items->Items[i];
            if(ItemSimilar(u, v))
            {
                if(eqCount) return false;
                ++eqCount;
            }
        }
        if(eqCount != 1)
            return false;
    }

    return true;
}

bool StateEquivalent(State *a, State *b)
{
    if(a->Items->ItemCount != b->Items->ItemCount)
        return false;

    for(size_t i = 0; i < a->Items->ItemCount; ++i)
    {
        size_t eqCount = 0;
        Item *u = (Item *)a->Items->Items[i];
        for(size_t i = 0; i < b->Items->ItemCount; ++i)
        {
            Item *v = (Item *)b->Items->Items[i];
            if(ItemEquivalent(u, v))
            {
                if(eqCount) return false;
                ++eqCount;
            }
        }
        if(eqCount != 1)
            return false;
    }

    return true;
}
