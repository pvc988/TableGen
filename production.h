#pragma once

#include <stddef.h>

typedef struct Symbol Symbol;
typedef struct Vector Vector;

typedef struct Production
{
    size_t Index;
    char *Id;
    Symbol *Left;
    Vector *Right;
} Production;

Production *ProductionCreate(char *id, Symbol *left, Vector *right);
void ProductionDelete(Production *prod);
