#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct Production Production;
typedef struct Vector Vector;

typedef struct Item
{
    bool Core;
    Production *Production;
    size_t Position;
    Vector *Lookaheads;
} Item;

Item *ItemCreate(bool core, Production *prod, size_t pos);
Item *ItemCreateLA(bool core, Production *prod, size_t pos, Vector *lookaheads);
void ItemDelete(Item *item);
bool ItemSimilar(Item *a, Item *b);
bool ItemEquivalent(Item *a, Item *b);
