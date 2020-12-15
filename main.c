#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsm.h"
#include "grammar.h"
#include "parsetable.h"

unsigned debug;

static void usageInfo(void);

enum
{
    ARG_GRAMMAR = 0,
    ARG_OUTPUT,
    ARG_ALGO,
    ARG_DEBUG
};

int main(int argc, char *argv[])
{
    // parse command line options
    char *grammarFileName = 0;
    char *outputFileName = 0;
    bool lalr = false;
    bool compact = false;
    unsigned nextArg = ARG_GRAMMAR;
    for(int i = 1; i < argc; ++i)
    {
        char *arg = argv[i];
        if(arg[0] == '-')
        {   // option
            nextArg = ARG_GRAMMAR;
            switch(arg[1])
            {
            case 'o':
                nextArg = ARG_OUTPUT;
                break;
            case 'a':
                nextArg = ARG_ALGO;
                break;
            case 'd':
                nextArg = ARG_DEBUG;
                break;
            case 'c':
                compact = true;
                break;
            default:
                fprintf(stderr, "Unknown option '%s'\n", arg);
                usageInfo();
                return -1;
            }
        }
        else
        {   // value
            switch(nextArg)
            {
            case ARG_GRAMMAR:
                if(grammarFileName)
                {
                    fprintf(stderr, "Grammar file already specified\n");
                    return -1;
                }
                grammarFileName = arg;
                break;
            case ARG_OUTPUT:
                if(outputFileName)
                {
                    fprintf(stderr, "Output file already specified\n");
                    return -1;
                }
                outputFileName = arg;
                break;
            case ARG_ALGO:
                if(!strcmp(arg, "LR1")) lalr = false;
                else if(!strcmp(arg, "LALR1")) lalr = true;
                else
                {
                    fprintf(stderr, "Unknown parsing algorithm '%s'\n", arg);
                    return -1;
                }
                break;
            case ARG_DEBUG:
                debug = strtoul(arg, 0, 0);
                break;
            default:
                fprintf(stderr, "Couldn't parse command line options\n");
                usageInfo();
                return -1;
            }
            nextArg = ARG_GRAMMAR;
        }
    }

    if(!grammarFileName)
    {
        fprintf(stderr, "Missing grammar filename\n");
        usageInfo();
        return -1;
    }
    if(!outputFileName)
    {
        fprintf(stderr, "Missing output filename\n");
        usageInfo();
        return -1;
    }

    Grammar *grammar = GrammarFromFile(grammarFileName);
    if(!grammar) return -1;
    GrammarBuildFirstSets(grammar);

    FSM *fsm = FSMCreate(grammar);
    if(lalr) FSMBuildLALR1States(fsm);
    else FSMBuildLR1States(fsm);

    ParseTable *pt = ParseTableCreate(fsm);
    if(pt)
    {
        ParseTableToFile(pt, outputFileName, compact);
        ParseTableDelete(pt);
    }
    FSMDelete(fsm);
    GrammarDelete(grammar);

    return 0;
}

void usageInfo(void)
{
    fprintf(stderr, "usage: tablegen [options] <grammar> -o <filename>\n");
    fprintf(stderr, "   grammar - grammar file to be used for table generation\n");
    fprintf(stderr, "   -o <filename> - output file path\n");
    fprintf(stderr, "   -a <algorithm> - LR1 or LALR1 algorithm can be used (default: LR1)\n");
    fprintf(stderr, "   -d <value> - numeric value specifying debug message level (default: 0)\n");
    fprintf(stderr, "   -c - generate output file in compact form\n");
}
