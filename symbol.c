#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "vector.h"

Symbol *SymbolCreate(const char *name, bool terminal)
{
    Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
    sym->Name = strdup(name);
    sym->Terminal = terminal;
    sym->First = VectorCreate();
    return sym;
}

void SymbolDelete(Symbol *sym)
{
    if(sym->Name) free(sym->Name);
    if(sym->First) VectorDelete(sym->First);
    free(sym);
}
