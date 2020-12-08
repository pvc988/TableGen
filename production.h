#pragma once

typedef struct Symbol Symbol;
typedef struct Vector Vector;

typedef struct Production
{
    char *Id;
    Symbol *Left;
    Vector *Right;
} Production;

Production *ProductionCreate(char *id, Symbol *left, Vector *right);
void ProductionDelete(Production *prod);
