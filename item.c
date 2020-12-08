#include <stdlib.h>

#include "item.h"
#include "vector.h"

typedef struct Symbol Symbol;

static bool lookaheadsEquivalent(Item *a, Item *b)
{
    if(a->Lookaheads->ItemCount != b->Lookaheads->ItemCount)
        return false;

    for(size_t i = 0; i < a->Lookaheads->ItemCount; ++i)
    {
        size_t eqCount = 0;
        Symbol *u = (Symbol *)a->Lookaheads->Items[i];
        for(size_t i = 0; i < b->Lookaheads->ItemCount; ++i)
        {
            Symbol *v = (Symbol *)b->Lookaheads->Items[i];
            if(u == v)
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

Item *ItemCreate(bool core, Production *prod, size_t pos)
{
    Item *item = (Item *)malloc(sizeof(Item));
    item->Core = core;
    item->Production = prod;
    item->Position = pos;
    item->Lookaheads = VectorCreate();
    return item;
}

Item *ItemCreateLA(bool core, Production *prod, size_t pos, Vector *lookaheads)
{
    Item *item = (Item *)malloc(sizeof(Item));
    item->Core = core;
    item->Production = prod;
    item->Position = pos;
    item->Lookaheads = lookaheads;
    return item;
}

void ItemDelete(Item *item)
{
    if(item->Lookaheads) VectorDelete(item->Lookaheads);
    free(item);
}

bool ItemSimilar(Item *a, Item *b)
{
    if(a == b) return true;
    if(a->Production != b->Production) return false;
    if(a->Position != b->Position) return false;
    return true;
}

bool ItemEquivalent(Item *a, Item *b)
{
    return ItemSimilar(a, b) && lookaheadsEquivalent(a, b);
}
