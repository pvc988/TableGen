#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "production.h"
#include "symbol.h"

static const char *endOfInputSymbolName = "$";
static const char *emptySymbolName = "~";
static const char *errorSymbolName = "!";

static void msgMalformed(void)
{
    fprintf(stderr, "Malformed rule\n");
}

Grammar *GrammarFromFile(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if(!f)
    {
        fprintf(stderr, "Couldn't open grammar file '%s'\n", filename);
        return 0;
    }
    Grammar *grammar = GrammarCreate();

    size_t ruleBufAllocated = 256;
    char *ruleBuf = malloc(ruleBufAllocated);
    for(;;)
    {
        size_t chrIdx = 0;
        bool skipSpace = true;
        bool comment = false;

        // read in and sanitize input
        for(char c = getc(f), lc = ' '; c != EOF; lc = c, c = getc(f))
        {
            if(c == '#') comment = true;
            if(c == '\n') comment = false;
            if(comment || c == '\n' || c == '\t') c = ' ';
            if(c == ';') break;

            if(skipSpace && c != ' ')
                skipSpace = false;

            if(c == ' ')
            {
                if(skipSpace || lc == ' ' || lc == '{')
                    continue;
            }
            else if(c == '}' && lc == ' ')
            {
                ruleBuf[chrIdx - 1] = '}';
                continue;
            }

            if(ruleBufAllocated - chrIdx <= 1)
            {
                ruleBufAllocated += 64;
                ruleBuf = realloc(ruleBuf, ruleBufAllocated);
            }
            ruleBuf[chrIdx++] = c;
        }
        // get rid of any trailing space (multiples were already dealt with)
        if(ruleBuf[chrIdx - 1] == ' ')
            ruleBuf[--chrIdx] = 0;

        // append terminating character
        ruleBuf[chrIdx++] = 0;

        // reached EOF; no more rules to process
        if(feof(f))
            break;

        fprintf(stderr, "Found rule: '%s'\n", ruleBuf);

        // split left and right sides
        char *arrowHead = strchr(ruleBuf, '>');
        if(!arrowHead || (arrowHead - ruleBuf < 2) || arrowHead[-1] != '-')
        {
            free(ruleBuf);
            GrammarDelete(grammar);
            msgMalformed();
            return 0;
        }

        char *leftSide = ruleBuf;
        char *rightSide = arrowHead + 1;
        arrowHead[-1] = 0;

        // get rid of any left side trailing space
        if(arrowHead[-2] == ' ') arrowHead[-2] = 0;

        // get rid of any right side leading space
        if(rightSide[0] == ' ') ++rightSide;

        fprintf(stderr, "  Left side: '%s'\n", leftSide);
        fprintf(stderr, "  Right side: '%s'\n", rightSide);

        // create left side symbol
        Symbol *leftSym = (Symbol *)DictionaryGetValue(grammar->Symbols, leftSide);
        if(!leftSym)
        {
            fprintf(stderr, "    New symbol: '%s'\n", leftSide);
            leftSym = SymbolCreate(leftSide, false);
            DictionaryAddItem(grammar->Symbols, leftSym->Name, leftSym);
        }

        // split right side using '|' as production separator
        char *symbolStart = rightSide;
        Vector *rightSyms = VectorCreate();
        char *idStart = 0;
        for(char *right = rightSide; ; ++right)
        {
            char c = *right;

            if(c == ' ' || !c)
            {
                *right = 0;
                if(symbolStart[0] == ' ') ++symbolStart;
                if(symbolStart[0] != '{' && right[-1] != '}')
                {
                    Symbol *sym = (Symbol *)DictionaryGetValue(grammar->Symbols, symbolStart);
                    if(!sym)
                    {
                        fprintf(stderr, "    New symbol: '%s'\n", symbolStart);
                        sym = SymbolCreate(symbolStart, true);
                        DictionaryAddItem(grammar->Symbols, symbolStart, sym);
                    }
                    VectorAppendItem(rightSyms, sym);
                    if(c) symbolStart = ++right;
                    c = *right;
                }
                else
                {
                    right[-1] = 0;
                    idStart = symbolStart + 1;
                }
            }

            if(c == '|' || !c)
            {                
                if(idStart && idStart[0] == ' ') ++idStart;
                if(idStart) fprintf(stderr, "    New production: '%s'\n", idStart);
                else fprintf(stderr, "    New anonymous production\n");
                Production *newProduction = ProductionCreate(idStart, leftSym, rightSyms);
                VectorAppendItem(grammar->Productions, newProduction);
                idStart = 0;
                if(!c) break;
                if(right[1] == ' ') ++right;
                symbolStart = right;
                rightSyms = VectorCreate();
            }
        }
    }
    free(ruleBuf);
    fclose(f);

    if(!grammar->Symbols->ItemCount)
    {
        GrammarDelete(grammar);
        fprintf(stderr, "Grammar has no symbols\n");
        return 0;
    }

    if(!grammar->Productions->ItemCount)
    {
        GrammarDelete(grammar);
        fprintf(stderr, "Grammar has no productions\n");
        return 0;
    }

    // resolve terminal/non-terminal and print all symbols
    fprintf(stderr, "\nSymbols:\n");
    for(size_t i = 0; i < grammar->Symbols->ItemCount; ++i)
    {
        Symbol *sym = (Symbol *)grammar->Symbols->Items[i].Data;
        for(size_t i = 0; i < grammar->Productions->ItemCount; ++i)
        {
            Production *prod = (Production *)grammar->Productions->Items[i];
            if(sym == prod->Left)
            {
                sym->Terminal = false;
                break;
            }
        }
        fprintf(stderr, "'%s': %s\n", sym->Name, sym->Terminal ? "terminal" : "non-terminal");
    }

    // TODO: Add malformed rule checks here

    // add end of input symbol to the end of the first production
    // (if its not there already)
    Production *prod0 = (Production *)grammar->Productions->Items[0];
    size_t prod0Len = prod0->Right->ItemCount;
    if(prod0->Right->Items[prod0Len - 1] != grammar->EndOfInput)
        VectorAppendItem(prod0->Right, grammar->EndOfInput);

    // print productions
    fprintf(stderr, "\nProductions:\n");
    for(size_t i = 0 ; i < grammar->Productions->ItemCount; ++i)
    {
        Production *prod = (Production *)grammar->Productions->Items[i];
        fprintf(stderr, "%s ->", prod->Left->Name);

        for(size_t i = 0; i < prod->Right->ItemCount; ++i)
        {
            Symbol *sym = (Symbol *)prod->Right->Items[i];
            fprintf(stderr, " %s", sym->Name);
        }

        if(prod->Id) fprintf(stderr, " {%s}", prod->Id);
        fprintf(stderr, "\n");
    }

    return grammar;
}

Grammar *GrammarCreate(void)
{
    Grammar *grammar = (Grammar *)malloc(sizeof(Grammar));
    grammar->Symbols = DictionaryCreate();
    grammar->Productions = VectorCreate();

    // add special symbols
    grammar->EndOfInput = SymbolCreate(endOfInputSymbolName, true);
    grammar->EmptySymbol = SymbolCreate(emptySymbolName, true);
    grammar->ErrorSymbol = SymbolCreate(errorSymbolName, true);
    grammar->EmptySymbol->Nullable = true;
    DictionaryAddItem(grammar->Symbols, grammar->EndOfInput->Name, grammar->EndOfInput);
    DictionaryAddItem(grammar->Symbols, grammar->EmptySymbol->Name, grammar->EmptySymbol);
    DictionaryAddItem(grammar->Symbols, grammar->ErrorSymbol->Name, grammar->ErrorSymbol);

    return grammar;
}

void GrammarDelete(Grammar *grammar)
{
    if(grammar->Productions) VectorDelete(grammar->Productions);
    if(grammar->Symbols) DictionaryDelete(grammar->Symbols);
    free(grammar);
}

void GrammarBuildFirstSets(Grammar *grammar)
{
    // calculate first sets for all terminals
    for(size_t i = 0; i < grammar->Symbols->ItemCount; ++i)
    {
        Symbol *sym = (Symbol *)grammar->Symbols->Items[i].Data;
        if(!sym->Terminal) continue;
        VectorAppendItem(sym->First, sym);
    }

    // now time for the rest
    for(bool updated = true; updated;)
    {
        updated = false;
        for(size_t i = 0; i < grammar->Productions->ItemCount; ++i)
        {
            Production *prod = (Production *)grammar->Productions->Items[i];
            Symbol *left = prod->Left;
            size_t symIdx;
            for(symIdx = 0; symIdx < prod->Right->ItemCount; ++symIdx)
            {
                Symbol *sym = (Symbol *)prod->Right->Items[symIdx];
                updated |= VectorMergeItems(left->First, sym->First, 0);
                if(!sym->Nullable)
                    break;
            }
            if(symIdx == prod->Right->ItemCount)
            {
                updated |= !left->Nullable;
                left->Nullable = true;
            }
        }
    }

    // print first sets
    fprintf(stderr, "\nFirst sets:\n");
    for(size_t i = 0; i < grammar->Symbols->ItemCount; ++i)
    {
        Symbol *sym = (Symbol *)grammar->Symbols->Items[i].Data;
        fprintf(stderr, "'%s': ", sym->Name);
        for(size_t i = 0; i < sym->First->ItemCount; ++i)
        {
            Symbol *s = (Symbol *)sym->First->Items[i];
            fprintf(stderr, " %s", s->Name);
        }
        fprintf(stderr,  "\n");
    }
}
