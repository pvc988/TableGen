#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "fsm.h"
#include "grammar.h"
#include "parsetable.h"

static void usageInfo(void);

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Missing grammar filename\n");
        usageInfo();
        return -1;
    }
    if(argc < 3)
    {
        fprintf(stderr, "Missing table filename\n");
        usageInfo();
        return -1;
    }
    if(argc < 4)
    {
        fprintf(stderr, "Missing algorithm name\n");
        usageInfo();
        return -1;
    }

    if(strcasecmp("LR1", argv[3]) && strcasecmp("LALR1", argv[3]))
    {
        fprintf(stderr, "Invalid algorithm\n");
        usageInfo();
        return -1;
    }

    bool lalr = argc > 3 && !strcmp(argv[3], "LALR1");

    char *grammarFileName = argv[1];
    char *outputFileName = argv[2];

    Grammar *grammar = GrammarFromFile(grammarFileName);
    if(!grammar) return -1;
    GrammarBuildFirstSets(grammar);

    FSM *fsm = FSMCreate(grammar);
    if(lalr) FSMBuildLALR1States(fsm);
    else FSMBuildLR1States(fsm);

    ParseTable *pt = ParseTableCreate(fsm);
    ParseTableToFile(pt, outputFileName);

    // delete objects after use
    ParseTableDelete(pt);
    FSMDelete(fsm);
    GrammarDelete(grammar);

    return 0;
}

void usageInfo(void)
{
    fprintf(stderr, "usage: tablegen <grammar> <output> <algorithm>\n");
    fprintf(stderr, "   grammar - grammar file to be used for table generation\n");
    fprintf(stderr, "   output - name of file to be generated\n");
    fprintf(stderr, "   algorithm - LR1 or LALR1 algorithm can be used\n");
}
