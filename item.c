#include <stdlib.h>

#include "item.h"
#include "vector.h"

Item *ItemCreate(bool core, Production *prod, size_t pos)
{
    Item *item = (Item *)malloc(sizeof(Item));
    item->Core = core;
    item->Production = prod;
    item->Position = pos;
    item->Lookaheads = VectorCreate();
    return item;
}

void ItemDelete(Item *item)
{
    if(item->Lookaheads) VectorDelete(item->Lookaheads);
    free(item);
}
