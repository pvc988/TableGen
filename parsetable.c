#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fsm.h"
#include "grammar.h"
#include "item.h"
#include "parsetable.h"
#include "production.h"
#include "state.h"
#include "symbol.h"
#include "transition.h"

static bool isCellFree(Action *a, Item *item)
{
    if(a->Type != AT_ERROR)
    {
        if(item->Production->Id)
        {
            fprintf(stderr, "Conflict in production %zu {%s}\n",
                    item->Production->Index,
                    item->Production->Id);
        }
        else
        {
            fprintf(stderr, "Conflict in production %zu\n",
                    item->Production->Index);
        }
        return false;
    }
    return true;
}

static uint32_t encodeAction(Action *a)
{
    switch(a->Type)
    {
    case AT_ERROR:
        return 0x00000000u;
    case AT_ACCEPT:
        return 0x00000001u;
    case AT_SHIFT:
        return 0x10000000u | a->State->Index;
    case AT_REDUCE:
        return 0x20000000u | a->Production->Index;
    case AT_GOTO:
        return 0x30000000u | a->State->Index;
    }
    return (uint32_t)-1;
}

ParseTable *ParseTableCreate(FSM *FSM)
{
    ParseTable *pt = (ParseTable *)calloc(1, sizeof(ParseTable));

    pt->FSM = FSM;

    pt->ColumnCount = pt->FSM->Grammar->Symbols->ItemCount;
    pt->RowCount = pt->FSM->States->ItemCount;

    // alloc table header
    pt->Header = (Symbol **)malloc(sizeof(Symbol *) * pt->ColumnCount);

    // index symbols and add them to header array
    for(size_t i = 0; i < pt->FSM->Grammar->Symbols->ItemCount; ++i)
    {
        Symbol *sym = (Symbol *)pt->FSM->Grammar->Symbols->Items[i].Data;
        sym->Index = i;
        pt->Header[i] = sym;
    }

    pt->Actions = (Action *)calloc(pt->ColumnCount * pt->RowCount, sizeof(Action));

    for(size_t i = 0; i < FSM->States->ItemCount; ++i)
    {
        State *state = (State *)FSM->States->Items[i];
        uint32_t row = state->Index;
        for(size_t i = 0; i < state->Items->ItemCount; ++i)
        {
            Item *item = (Item *)state->Items->Items[i];
            Symbol *currSymbol = (Symbol *)VectorGetItem(item->Production->Right, item->Position);
            if(!currSymbol)
            {   // reduce
                for(size_t i = 0; i < item->Lookaheads->ItemCount; ++i)
                {
                    Symbol *la = (Symbol *)item->Lookaheads->Items[i];
                    uint32_t col = la->Index;
                    Action *a = pt->Actions + (row * pt->ColumnCount + col);
                    if(!isCellFree(a, item))
                    {
                        ParseTableDelete(pt);
                        return 0;
                    }
                    a->Type = AT_REDUCE;
                    a->Production = item->Production;
                }
            }
            else
            {   // shift/accept or goto
                uint32_t col = currSymbol->Index;
                State *dest = (State *)StateGetTransition(state, currSymbol)->State;
                Action *a = pt->Actions + (row * pt->ColumnCount + col);
                a->State = dest;
                if(currSymbol->Terminal)
                {   // shift or accept
                    if(!isCellFree(a, item))
                    {
                        ParseTableDelete(pt);
                        return 0;
                    }
                    a->Type = currSymbol == pt->FSM->Grammar->EndOfInput ?
                                AT_ACCEPT : AT_SHIFT;
                }
                else a->Type = AT_GOTO; // goto
            }
        }
    }

    return pt;
}

void ParseTableDelete(ParseTable *pt)
{
    if(pt->Header) free(pt->Header);
    if(pt->Actions) free(pt->Actions);
    free(pt);
}

bool ParseTableToFile(ParseTable *pt, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if(!f) return false;

    // write magic value
    fwrite("LRPT", 4, 1, f);

    // write right symbol counts and left symbol for each production
    // (needed for reduce step)
    // and calculate namedProdCount while at it
    uint32_t prodCount = pt->FSM->Grammar->Productions->ItemCount;
    uint32_t namedProdCount = 0;
    fwrite(&prodCount, 4, 1, f);
    for(uint32_t i = 0; i < prodCount; ++i)
    {
        Production *prod = (Production *)pt->FSM->Grammar->Productions->Items[i];
        if(prod->Id) ++namedProdCount;
        uint32_t leftIdx = prod->Left->Index;
        fwrite(&leftIdx, 4, 1, f);
        uint32_t symCount = prod->Right->ItemCount;
        fwrite(&symCount, 4, 1, f);
    }

    // write named production indices and ids
    fwrite(&namedProdCount, 4, 1, f);
    for(uint32_t i = 0; i < prodCount; ++i)
    {
        Production *prod = (Production *)pt->FSM->Grammar->Productions->Items[i];
        if(!prod->Id) continue;
        uint32_t prodIdx = prod->Index;
        fwrite(&prodIdx, 4, 1, f);
        uint32_t idLen = strlen(prod->Id);
        fwrite(&idLen, 4, 1, f);
        fwrite(prod->Id, idLen, 1, f);
    }

    // write table header (symbols)
    uint32_t cols = pt->ColumnCount;
    fwrite(&cols, 4, 1, f);
    for(uint32_t col = 0; col < cols; ++col)
    {
        Symbol *sym = pt->Header[col];
        uint32_t nameLen = strlen(sym->Name);
        fwrite(&nameLen, 4, 1, f);
        fwrite(sym->Name, nameLen, 1, f);
    }

    // write table cells
    uint32_t rows = pt->RowCount;
    fwrite(&rows, 4, 1, f);
    size_t tableCells = pt->ColumnCount * pt->RowCount;
    for(size_t i = 0; i < tableCells; ++i)
    {
        Action *a = pt->Actions + i;
        uint32_t cell = encodeAction(a);
        fwrite(&cell, 4, 1, f);
    }

    fclose(f);
    return true;
}
