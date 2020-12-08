#pragma once

#include "dictionary.h"
#include "vector.h"

typedef struct Symbol
{
    char *Name;
    bool Terminal;
    bool Nullable;
    Vector *First;
} Symbol;

Symbol *SymbolCreate(const char *name, bool terminal);
void SymbolDelete(Symbol *sym);

typedef struct Production
{
    char *Id;
    Symbol *Left;
    Vector *Right;
} Production;

Production *ProductionCreate(char *id, Symbol *left, Vector *right);
void ProductionDelete(Production *prod);

typedef struct Grammar
{
    Dictionary *Symbols;
    Vector *Productions;
    Symbol *EndOfInput;
    Symbol *EmptySymbol;
    Symbol *ErrorSymbol;
} Grammar;

Grammar *GrammarFromFile(const char *filename);
Grammar *GrammarCreate(void);
void GrammarDelete(Grammar *grammar);
