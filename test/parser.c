#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"

#define AT_SPECIAL          0
#define AT_SHIFT            1
#define AT_REDUCE           2
#define AT_GOTO             3

#define PARSER_FILE_MAGIC   0x5450524C
#define PARSER_STACK_SIZE   256

typedef struct StackItem
{
    unsigned SymbolId;
    unsigned State;
    union
    {
        int Integer;
        Token Token;
    } Value;
} StackItem;


typedef int (*ProductionCallback)(void);

typedef struct Production
{
    char *Name;
    unsigned LeftSymbol;
    unsigned SymCount;
    ProductionCallback Callback;
} Production;

static StackItem parserStack[PARSER_STACK_SIZE];
static unsigned parserStackPtr = 0;

static unsigned productionCount;
static Production *productions;
static char **symbols;
static unsigned tableColumnCount;
static unsigned tableRowCount;
static uint32_t *table;

typedef struct ConstSymbol
{
    const char *Name;
    int Value;
} ConstSymbol;

static ConstSymbol constSyms[] =
{
    { "zero", 0 },
    { "one", 1 },
    { "two", 2 },
    { "three", 3 },
    { "four", 4 },
    { "five", 5 },
    { "six", 6 },
    { "seven", 7 },
    { "eight", 8 },
    { "nine", 9 },
    { "ten", 10 },
    { "hundred", 100 },
    { 0, 0 }
};

static void stackPush(StackItem *item)
{
    if(parserStackPtr >= PARSER_STACK_SIZE)
    {
        fprintf(stderr, "Parser stack overflow\n");
        return;
    }
    parserStack[parserStackPtr++] = *item;
}

static void stackPop(StackItem *item)
{
    if(!parserStackPtr)
    {
        fprintf(stderr, "Parser stack underflow\n");
        return;
    }
    *item = parserStack[--parserStackPtr];
}

static void stackPeek(StackItem *item, unsigned n)
{
    *item = parserStack[parserStackPtr - 1 - n];
}

static void stackPeekBack(StackItem *item, unsigned n)
{
    return stackPeek(item, -n - 1);
}

static void decodeAction(uint32_t action, unsigned *type, unsigned *arg)
{
    *arg = action & 0x0FFFFFFFu;
    *type = action >> 28;
}

static Production *getProduction(const char *name)
{
    for(unsigned i = 0; i < productionCount; ++i)
    {
        if(!productions[i].Name) continue;
        if(!strcmp(name, productions[i].Name))
            return productions + i;
    }
    return 0;
}

static bool setProductionCallback(const char *name, ProductionCallback callback)
{
    Production *prod = getProduction(name);
    if(!prod) return false;
    prod->Callback = callback;
    return true;
}

static int lookupSymbol(const char *name)
{
    for(ConstSymbol *cs = constSyms; cs->Name; ++cs)
    {
        if(!strcmp(name, cs->Name))
            return cs->Value;
    }
    return 0;
}

static int addCallback(void)
{
    StackItem a, b;
    stackPeekBack(&a, 0);
    stackPeekBack(&b, 2);
    return a.Value.Integer + b.Value.Integer;
}

static int subCallback(void)
{
    StackItem a, b;
    stackPeekBack(&a, 0);
    stackPeekBack(&b, 2);
    return a.Value.Integer - b.Value.Integer;
}

static int mulCallback(void)
{
    StackItem a, b;
    stackPeekBack(&a, 0);
    stackPeekBack(&b, 2);
    return a.Value.Integer * b.Value.Integer;
}

static int divCallback(void)
{
    StackItem a, b;
    stackPeekBack(&a, 0);
    stackPeekBack(&b, 2);
    return a.Value.Integer / b.Value.Integer;
}

static int identCallback(void)
{
    StackItem a;
    stackPeekBack(&a, 0);
    return lookupSymbol(a.Value.Token.Text);
}

static int numberCallback(void)
{
    StackItem a;
    stackPeekBack(&a, 0);
    return atoi(a.Value.Token.Text);
}

static int parenCallback(void)
{
    StackItem a;
    stackPeekBack(&a, 1);
    return a.Value.Integer;
}

bool ParserCreate(const char *filename)
{
    // some paranoid level error checking but this is a test after all

    FILE *f = fopen(filename, "rb");
    if(!f)
    {
        fprintf(stderr, "Couldn't open table file\n");
        return false;
    }

    uint32_t magic;
    if(fread(&magic, 4, 1, f) != 1 || magic != PARSER_FILE_MAGIC)
    {
        fprintf(stderr, "Invalid table file magic value\n");
        fclose(f);
        return false;
    }

    uint32_t prodCount;
    if(fread(&prodCount, 4, 1, f) != 1 || !prodCount)
    {
        fprintf(stderr, "Invalid production count\n");
        fclose(f);
        return false;
    }
    productionCount = prodCount;

    // read productions
    productions = (Production *)calloc(prodCount, sizeof(Production));
    for(unsigned i = 0; i < productionCount; ++i)
    {
        if(fread(&productions[i].LeftSymbol, 4, 1, f) != 1)
        {
            fprintf(stderr, "Couldn't read production %u left symbol index\n", i);
            ParserDelete();
            fclose(f);
            return false;
        }
        if(fread(&productions[i].SymCount, 4, 1, f) != 1)
        {
            fprintf(stderr, "Couldn't read production %u symbol count\n", i);
            ParserDelete();
            fclose(f);
            return false;
        }
    }

    uint32_t namedProdCount;
    if(fread(&namedProdCount, 4, 1, f) != 1)
    {
        fprintf(stderr, "Couldn't read named production count\n");
        ParserDelete();
        fclose(f);
        return false;
    }

    // read named productions
    for(uint32_t i = 0; i < namedProdCount; ++i)
    {
        uint32_t prodIdx;
        if(fread(&prodIdx, 4, 1, f) != 1)
        {
            fprintf(stderr, "Couldn't read named production index\n");
            ParserDelete();
            fclose(f);
            return false;
        }
        if(prodIdx >= productionCount)
        {
            fprintf(stderr, "Couldn't read named production index\n");
            ParserDelete();
            fclose(f);
            return false;
        }
        uint32_t nameLen;
        if(fread(&nameLen, 4, 1, f) != 1)
        {
            fprintf(stderr, "Couldn't read named production name length\n");
            ParserDelete();
            fclose(f);
            return false;
        }
        productions[prodIdx].Name = malloc(nameLen + 1);
        if(fread(productions[prodIdx].Name, 1, nameLen, f) != nameLen)
        {
            fprintf(stderr, "Couldn't read named production name length\n");
            ParserDelete();
            fclose(f);
            return false;
        }
        productions[prodIdx].Name[nameLen] = 0;
    }

    uint32_t colCount;
    if(fread(&colCount, 4, 1, f) != 1 || !colCount)
    {
        fprintf(stderr, "Couldn't read table column count\n");
        ParserDelete();
        fclose(f);
        return false;
    }
    tableColumnCount = colCount;

    // read symbols
    symbols = (char **)calloc(colCount, sizeof(char *));
    for(unsigned i = 0; i < tableColumnCount; ++i)
    {
        uint32_t nameLen;
        if(fread(&nameLen, 4, 1, f) != 1)
        {
            fprintf(stderr, "Couldn't read symbol name length\n");
            ParserDelete();
            fclose(f);
            return false;
        }
        symbols[i] = malloc(nameLen + 1);
        if(fread(symbols[i], 1, nameLen, f) != nameLen)
        {
            fprintf(stderr, "Couldn't read symbol name\n");
            ParserDelete();
            fclose(f);
            return false;
        }
        symbols[i][nameLen] = 0;
    }

    uint32_t rowCount;
    if(fread(&rowCount, 4, 1, f) != 1 || !rowCount)
    {
        fprintf(stderr, "Couldn't read table row count\n");
        ParserDelete();
        fclose(f);
        return false;
    }
    tableRowCount = rowCount;

    // read table data
    table = calloc(tableColumnCount * tableRowCount, sizeof(uint32_t));
    for(unsigned row = 0; row < tableRowCount; ++row)
    {
        if(fread(table + row * tableColumnCount, 4, tableColumnCount, f) != tableColumnCount)
        {
            fprintf(stderr, "Couldn't read row %u data\n", row);
            ParserDelete();
            fclose(f);
            return false;
        }
    }
    fclose(f);

    // setup lexer token ids
    for(unsigned i = 0; i < tableColumnCount; ++i)
        LexerSetTokenDefId(symbols[i], i);

    // setup production callbacks
    setProductionCallback("add", addCallback);
    setProductionCallback("sub", subCallback);
    setProductionCallback("mul", mulCallback);
    setProductionCallback("div", divCallback);
    setProductionCallback("ident", identCallback);
    setProductionCallback("number", numberCallback);
    setProductionCallback("paren", parenCallback);

    return true;
}

void ParserDelete(void)
{
    if(productions)
    {
        for(unsigned i = 0; i < productionCount; ++i)
        {
            if(productions[i].Name)
                free(productions[i].Name);
        }
        free(productions);
    }
    if(table) free(table);
    productions = 0;
    table = 0;
}

bool ParserParse(int *result)
{
    parserStackPtr = 0; // clear stack
    unsigned state = 0; // initial state
    StackItem si;       // temporary stack item
    si.SymbolId = 0;
    si.State = state;
    si.Value.Integer = 0;
    stackPush(&si);
    Token token;
    LexerTokenGet(&token);
    for(;;)
    {   // main parser loop
        if(token.Def == &LexerError)
        {
            fprintf(stderr, "Unknown token\n");
            return false;
        }
        else if(!token.Def)
        {
            fprintf(stderr, "Token is null. Something went really wrong\n");
            return false;
        }

        unsigned tokenId = token.Def->Id;
        if(tokenId >= tableColumnCount)
        {
            fprintf(stderr, "Token index >= table column count. Something went really wrong\n");
            return false;
        }

        unsigned actionArg;
        unsigned actionType;
        decodeAction(table[state * tableColumnCount + tokenId], &actionType, &actionArg);

        switch(actionType)
        {
        case AT_SPECIAL:
            if(actionArg == 1)
            {
                stackPop(&si);
                *result = si.Value.Integer;
                return true;
            }
            fprintf(stderr, "Syntax error\n");
            return false;

        case AT_SHIFT:
            state = actionArg;
            si.SymbolId = tokenId;
            si.State = state;
            si.Value.Token = token;
            stackPush(&si);
            LexerTokenGet(&token);
            break;

        case AT_REDUCE:
        {
            Production *prod = productions + actionArg;
            StackItem newSI;

            for(unsigned i = 0; i < prod->SymCount; ++i) stackPop(&newSI);
            if(prod->Callback) newSI.Value.Integer = prod->Callback();

            stackPeek(&si, 0);
            decodeAction(table[si.State * tableColumnCount + prod->LeftSymbol],
                    &actionType, &actionArg);
            if(actionType != AT_GOTO)
            {
                fprintf(stderr, "Action type != GoTo for non-terminal\n");
                return false;
            }
            state = actionArg;
            newSI.SymbolId = prod->LeftSymbol;
            newSI.State = state;
            stackPush(&newSI);
            break;
        }

        default:
            fprintf(stderr, "Wrong action type %u\n", actionType);
            return false;
        }
    }

    return true;
}
