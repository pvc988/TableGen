#pragma once

#include <stdbool.h>
#include <stdio.h>

typedef struct TokenDef TokenDef;

typedef unsigned (*TokenEvaluator)(TokenDef *tokenDef, char *text);

typedef struct TokenDef
{
    int Id;
    const char *Name;
    TokenEvaluator Evaluator;
} TokenDef;

typedef struct Token
{
    TokenDef *Def;
    char Text[32];
} Token;

extern TokenDef LexerEndOfInput;
extern TokenDef LexerError;

void LexerSetStream(FILE *stream);
void LexerSetTokenDefId(const char *name, int id);
bool LexerTokenGet(Token *token);
void LexerCleanup(void);
