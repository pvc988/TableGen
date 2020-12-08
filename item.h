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
void ItemDelete(Item *item);
