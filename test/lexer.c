#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static unsigned neverEvaluator(TokenDef *tokenDef, char *text);
static unsigned alwaysEvaluator(TokenDef *tokenDef, char *text);
static unsigned charEvaluator(TokenDef *tokenDef, char *text);
static unsigned idEvaluator(TokenDef *tokenDef, char *text);
static unsigned numEvaluator(TokenDef *tokenDef, char *text);

TokenDef LexerEndOfInput = { -1, "$", neverEvaluator };
TokenDef LexerError = { -1, "!", alwaysEvaluator };

static TokenDef tDefAdd = { -1, "+", charEvaluator };
static TokenDef tDefSub = { -1, "-", charEvaluator };
static TokenDef tDefMul = { -1, "*", charEvaluator };
static TokenDef tDefDiv = { -1, "/", charEvaluator };
static TokenDef tDefLParen = { -1, "(", charEvaluator };
static TokenDef tDefRParen = { -1, ")", charEvaluator };
static TokenDef tDefId = { -1, "id", idEvaluator };
static TokenDef tDefNum = { -1, "num", numEvaluator };

static FILE *inputStream = 0;
static TokenDef *tokenDefs[] =
{
    &LexerEndOfInput,
    &tDefAdd,
    &tDefSub,
    &tDefMul,
    &tDefDiv,
    &tDefLParen,
    &tDefRParen,
    &tDefId,
    &tDefNum,
    &LexerError,
    0
};

char *currentLine;
char *currentLineStart;
size_t currentLineLen;

unsigned neverEvaluator(TokenDef *tokenDef, char *text)
{
    (void)tokenDef, (void)text;
    return 0;
}

unsigned alwaysEvaluator(TokenDef *tokenDef, char *text)
{
    (void)tokenDef, (void)text;
    return 1;
}

unsigned charEvaluator(TokenDef *tokenDef, char *text)
{
    return tokenDef->Name[0] == text[0] ? 1 : 0;
}

unsigned idEvaluator(TokenDef *tokenDef, char *text)
{
    (void)tokenDef;
    unsigned i = 0;
    for(i = 0; i < 24 && text[i]; ++i)
    {
        if(!isalpha(text[i]))
            break;
    }
    return i;
}

unsigned numEvaluator(TokenDef *tokenDef, char *text)
{
    (void)tokenDef;
    unsigned i = 0;
    for(i = 0; i < 24 && text[i]; ++i)
    {
        if(!isdigit(text[i]))
            break;
    }
    return i;
}

void LexerSetStream(FILE *stream)
{
    inputStream = stream;
}

void LexerSetTokenDefId(const char *name, int id)
{
    for(TokenDef **def = tokenDefs; *def; ++def)
    {
        if(!strcmp(name, (*def)->Name))
        {
            (*def)->Id = id;
            break;
        }
    }
}

bool LexerTokenGet(Token *token)
{
    token->Def = &LexerEndOfInput;
    token->Text[0] = 0;
    while(!currentLine || !currentLine[0] || currentLine[0] == '\n')
    {
        ssize_t r = getline(&currentLineStart, &currentLineLen, inputStream);
        currentLine = currentLineStart;
        if(r < 0) return false;
    }

    // trim leading whitespaces
    while(currentLine[0] == ' ' || currentLine[0] == '\t' || currentLine[0] == '\n')
        ++currentLine;

    for(TokenDef **def = tokenDefs; *def; ++def)
    {
        TokenDef *td = *def;
        int charsConsumed = td->Evaluator(td, currentLine);
        if(charsConsumed <= 0) continue;

        token->Def = td;
        strncpy(token->Text, currentLine, charsConsumed);
        token->Text[charsConsumed] = 0;
        currentLine += charsConsumed;
        return true;
    }

    return false;
}

void LexerCleanup(void)
{
    if(currentLineStart)
        free(currentLineStart);
}
