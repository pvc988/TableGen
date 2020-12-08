#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "grammar.h"

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Missing grammar filename\n");
        return -1;
    }
    if(argc < 3)
    {
        fprintf(stderr, "Missing table filename\n");
        return -1;
    }

    bool lalr = argc > 3 && !strcmp(argv[3], "LALR");

    char *grammarFileName = argv[1];
    char *outputFileName = argv[2];

    Grammar *grammar = GrammarFromFile(grammarFileName);
    if(!grammar) return -1;
    GrammarBuildFirstSets(grammar);

    return 0;
}
