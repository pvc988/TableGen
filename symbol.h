#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct Vector Vector;

typedef struct Symbol
{
    size_t Index;
    char *Name;
    bool Terminal;
    bool Nullable;
    bool Used;
    Vector *First;
} Symbol;

Symbol *SymbolCreate(const char *name, bool terminal);
void SymbolDelete(Symbol *sym);
