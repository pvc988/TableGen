#pragma once

#include "dictionary.h"
#include "vector.h"

typedef struct Symbol Symbol;

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
void GrammarBuildFirstSets(Grammar *grammar);
