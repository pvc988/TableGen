#include <stdbool.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[])
{
    if(!ParserCreate("test.lrpt"))
        return -1;

    FILE *input = stdin;
    if(argc >= 2)
    {
        input = fopen(argv[1], "rb");
        if(!input)
        {
            fprintf(stderr, "Couldn't open input file %s\n", argv[1]);
            return -1;
        }
    }

    int result = 0;
    LexerSetStream(input);
    bool ok = ParserParse(&result);
    LexerCleanup();
    if(ok) printf("result = %d\n", result);

    if(input != stdin)
        fclose(input);

    ParserDelete();
    return 0;
}
