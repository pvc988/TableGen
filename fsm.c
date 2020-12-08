#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "fsm.h"
#include "grammar.h"
#include "item.h"
#include "production.h"
#include "state.h"
#include "symbol.h"
#include "transition.h"
#include "vector.h"

static void printItem(Item *item)
{
    fprintf(stderr, "%s%s -> [", item->Core ? "" : "+", item->Production->Left->Name);
    for(size_t i = 0; i < item->Production->Right->ItemCount; ++i)
    {
        Symbol *sym = (Symbol *)item->Production->Right->Items[i];
        fprintf(stderr, i == item->Position ? ".%s" : " %s", sym->Name);
    }
    fprintf(stderr, item->Position >= item->Production->Right->ItemCount ? ".;" : " ;");
    for(size_t i = 0; i < item->Lookaheads->ItemCount; ++i)
    {
        Symbol *sym = (Symbol *)item->Lookaheads->Items[i];
        fprintf(stderr, " %s", sym->Name);
    }
    fprintf(stderr, "]");
}

static void printTransition(State *accept, Transition *trans)
{
    if(trans->State == accept) fprintf(stderr, "%s -> ACCEPT", trans->Symbol->Name);
    else fprintf(stderr, "%s -> %zu", trans->Symbol->Name, trans->State->Index);
}

static void printState(State *accept, State *state)
{
    for(size_t i = 0; i < state->Items->ItemCount; ++i)
    {
        Item *item = (Item *)state->Items->Items[i];
        fprintf(stderr, "  ");
        printItem(item);
        fprintf(stderr, "\n");
    }
    if(!state->Transitions->ItemCount)
        return;
    fprintf(stderr, " Transitions:\n");
    for(size_t i = 0; i < state->Transitions->ItemCount; ++i)
    {
        Transition *trans = (Transition *)state->Transitions->Items[i];
        fprintf(stderr, "  ");
        printTransition(accept, trans);
        fprintf(stderr, "\n");
    }
}

static Vector *calcFirstSet(Vector *base)
{
    Vector *first = VectorCreate();
    for(size_t i = 0; i < base->ItemCount; ++i)
    {
        Symbol *symbol = (Symbol *)base->Items[i];
        VectorMergeItems(first, symbol->First, 0);
        if(!symbol->Nullable) break;
    }
    return first;
}

static void closeLR1(Grammar *grammar, Vector *itemSet)
{
    for(bool changed = true; changed;)
    {
        changed = false;
        for(size_t i = 0; i < itemSet->ItemCount; ++i)
        {
            Item *item = (Item *)itemSet->Items[i];
            Vector *right = item->Production->Right;
            Symbol *currSymbol = VectorGetItem(right, item->Position);
            if(!currSymbol || currSymbol->Terminal)
                continue;

            for(size_t i = 0; i < item->Lookaheads->ItemCount; ++i)
            {
                Symbol *lookahead = (Symbol *)item->Lookaheads->Items[i];
                for(size_t i = 0; i < grammar->Productions->ItemCount; ++i)
                {
                    Production *prod = grammar->Productions->Items[i];
                    if(prod->Left != currSymbol)
                        continue;

                    // calculate FIRST for current production tail
                    // TODO: newItem should be created only if is going to be used for sure
                    Vector *firstBase = VectorCreate();
                    for(size_t i = item->Position + 1; i < item->Production->Right->ItemCount; ++i)
                        VectorAppendItem(firstBase, item->Production->Right->Items[i]); // tail
                    VectorAppendItem(firstBase, lookahead);
                    Vector *first = calcFirstSet(firstBase);

                    Item *newItem = ItemCreateLA(false, prod, 0, first);
                    bool merged = false;
                    for(size_t i = 0; i < itemSet->ItemCount; ++i)
                    {
                        Item *it = (Item *)itemSet->Items[i];
                        if(ItemSimilar(it, newItem))
                        {
                            changed |= VectorMergeItems(it->Lookaheads, first, 0);
                            merged = true;
                            break;
                        }
                    }
                    if(!merged)
                    {
                        bool added = VectorAppendIfUnique(itemSet, newItem,
                            (VectorItemEqualityComparer)ItemSimilar);
                        if(!added) ItemDelete(newItem);
                        changed |= added;
                    }
                    else ItemDelete(newItem);
                }
            }
        }
    }
}

static State *gotoLR1(Grammar *grammar, State *srcState, Symbol *symbol)
{
    if(symbol == grammar->EndOfInput)
        return 0;   // accept

    State *newState = StateCreate();
    for(size_t i = 0; i < srcState->Items->ItemCount; ++i)
    {
        Item *item = (Item *)srcState->Items->Items[i];
        Symbol *currSym = (Symbol *)VectorGetItem(item->Production->Right, item->Position);
        if(!currSym || currSym != symbol)
            continue;

        Item *newItem = ItemCreate(true, item->Production, item->Position + 1);
        VectorAppendItems(newItem->Lookaheads, item->Lookaheads);
        VectorAppendItem(newState->Items, newItem);
    }
    closeLR1(grammar, newState->Items);
    return newState;
}

static Vector *getCurrentSymbols(State *state)
{
    Vector *syms = VectorCreate();
    for(size_t i = 0; i < state->Items->ItemCount; ++i)
    {
        Item *item = (Item *)state->Items->Items[i];
        Symbol *currSym = (Symbol *)VectorGetItem(item->Production->Right, item->Position);
        if(!currSym) continue;
        VectorAppendIfUnique(syms, currSym, 0);
    }
    return syms;
}

static bool transitionSymsEqual(Transition *a, Symbol *b)
{
    return a->Symbol == b;
}

void buildLR1orLALR1States(FSM *fsm, bool lalr)
{
    State *initialState = StateCreate();
    Item *startItem = ItemCreate(true, fsm->Grammar->Productions->Items[0], 0);
    VectorAppendItem(startItem->Lookaheads, fsm->Grammar->EndOfInput);
    VectorAppendItem(initialState->Items, startItem);
    closeLR1(fsm->Grammar, initialState->Items);
    VectorAppendItem(fsm->States, initialState);

    for(bool changed = true; changed;)
    {
        changed = false;
        for(size_t i = 0; i < fsm->States->ItemCount; ++i)
        {
            State *state = (State *)fsm->States->Items[i];
            Vector *currentSyms = getCurrentSymbols(state);

            // create new states
            for(size_t i = 0; i < currentSyms->ItemCount; ++i)
            {
                Symbol *sym = (Symbol *)currentSyms->Items[i];
                State *newState = gotoLR1(fsm->Grammar, state, sym);
                State *transitionDest = fsm->Accept;
                if(newState)
                {
                    transitionDest = newState;
                    bool unique = true;
                    for(size_t i = 0; i < fsm->States->ItemCount; ++i)
                    {
                        State *s = (State *)fsm->States->Items[i];
                        if(lalr)
                        {   // building LALR(1) FSM
                            if(StateSimilar(newState, s))
                            {
                                // merge corresponding lookaheads
                                for(size_t i = 0; i < s->Items->ItemCount; ++i)
                                {
                                    Item *u = (Item *)s->Items->Items[i];
                                    for(size_t i = 0; i < newState->Items->ItemCount; ++i)
                                    {
                                        Item *v = (Item *)newState->Items->Items[i];
                                        if(u != v && ItemSimilar(u, v))
                                            changed |= VectorMergeItems(u->Lookaheads, v->Lookaheads, 0);
                                    }
                                }
                                transitionDest = s;
                                unique = false;
                                break;
                            }
                        }
                        else
                        {   // building LR(1) FSM
                            if(StateEquivalent(newState, s))
                            {
                                transitionDest = s;
                                unique = false;
                                break;
                            }
                        }
                    }

                    if(unique)
                    {
                        VectorAppendItem(fsm->States, newState);
                        changed = true;
                    }
                    else StateDelete(newState);
                }

                if(!VectorContainsItem(state->Transitions, sym, (VectorItemEqualityComparer)transitionSymsEqual))
                {
                    Transition *newTransition = TransitionCreate(sym, transitionDest);
                    VectorAppendItem(state->Transitions, newTransition);
                    changed = true;
                }
            }
        }
    }

    // states are built; index them
    for(size_t i = 0; i < fsm->States->ItemCount; ++i)
    {
        State *state = (State *)fsm->States->Items[i];
        state->Index = i;
    }

    fprintf(stderr, "\nFSM states:\n");
    for(size_t i = 0; i < fsm->States->ItemCount; ++i)
    {
        State *state = (State *)fsm->States->Items[i];
        fprintf(stderr, "\nState %zu\n", state->Index);
        printState(fsm->Accept, state);
    }
}

FSM *FSMCreate(Grammar *grammar)
{
    FSM *fsm = (FSM *)malloc(sizeof(FSM));
    fsm->Grammar = grammar;
    fsm->States = VectorCreate();
    fsm->Accept = StateCreate();
    return fsm;
}

void FSMDelete(FSM *fsm)
{
    if(fsm->States) VectorDelete(fsm->States);
    if(fsm->Accept) StateDelete(fsm->Accept);
    free(fsm);
}

void FSMBuildLR1States(FSM *fsm)
{
    buildLR1orLALR1States(fsm, false);
}

void FSMBuildLALR1States(FSM *fsm)
{
    buildLR1orLALR1States(fsm, true);
}
