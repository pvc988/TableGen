#pragma once

#include <stdbool.h>

typedef struct Vector Vector;

typedef struct Symbol
{
    char *Name;
    bool Terminal;
    bool Nullable;
    Vector *First;
} Symbol;

Symbol *SymbolCreate(const char *name, bool terminal);
void SymbolDelete(Symbol *sym);
