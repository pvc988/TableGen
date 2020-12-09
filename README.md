# TableGen
## LR(1) and LALR(1) parsing table generator.

This is not yet another parser generator. This program generates LR(1) and LALR(1) parsing tables as binary file from supplied grammar file. Generated tables are language agnostic. Table files can be used to create any kind of parser using any programming language. Generated files contain just enough information to create any parser, ranging from simple educational ones to complete programming languages like C.

### Grammar file syntax

Grammar file syntax is loosely based on YACC with some differences:

* There is only one section in the file.
* Terminal and non-terminal symbols are detected automatically.
* Right arrow symbol (`->`) is used instead of `:` to separate left and right sides of rules.
* Apostrophes are not required for single character tokens. For example, YACC token `'%'` is simply witten as `%`.
* YACC production actions are replaced by identifiers, which can then be referenced by the parser. If no production identifier is specified, then such production can't be referenced in parser code, so it is supposed to take default action.

There are also some reserved characters:

* `|` (pipe symbol) separates alternative productions in the rule. Just as it is done in YACC.
* `;` (semicolon) separates rules from each other. Again, just as in YACC.
* `$` (dollar symbol) is used to denote 'end of input' terminal symbol. Note, that end of input symbol is automatically added after first production in the file (if not specified explicitly).
* `~` (tilde) is used as, so called, 'epsilon' or empty symbol.
* `!` (exclamation mark) is reserved to be used as means of error detection ('error' symbol).
* `#` (number symbol) marks beginning of a comment. Comment spans to the end of the line. There are no multiline comments.

#### Example:
    # simple expression/term/factor grammar example

    S -> E;

    E -> E + T  { add }
       | E - T  { sub }
       | T ;

    T -> T * F  { mul }
       | T / F  { div }
       | F ;

    F -> id     { ident }
       | num    { number }
       | ( E )  { paren }
       ;
#### End of example

Please note that grammar file parser is very primitive, so try to avoid feeding it with illegal input. It can easily crash or misinterpret the input. There is a lot of information outputted to `stderr` during transformation process. So in case of problems, try to examine that.

### Output file format (in pseudocode)

    struct LRPTFile
    {
        u32 Magic; /* 'LRPT' magic number identifying file type */
        u32 ProdCount; /* Total production count described in this file */
        ProdDef ProdDefs[ProdCount]; /* Production definitions.
                                        See: decription of ProdDef */
        u32 NamedProdCount; /* Count of non-anonymous productions */
        NamedProd NamedProds[NamedProdCount]; /* Named productions */
        u32 ColumnCount; /* Number of columns in the table */
        String Header[ColumnCount]; /* Table header. Contains all of the
                                       symbols used in this file */
        u32 RowCount; /* Number of rows of the table */
        u32 Table[RowCount, ColumnCount]; /* Main table data. Contains
                                             actions taken by the parser */
    }

    struct ProdDef
    {
        u32 SymbolIdx; /* Index of symbol of production's left side */
        u32 SymbolCount; /* Count of symbols on the right side of
                            the production. Needed by reduce step
                            of parsing process */
    }

    struct NamedProd
    {
        u32 ProdIdx; /* Index (in ProdDefs) of this named production */
        String Name; /* Name of this production */
    }

    struct String
    {
        u32 Length; /* Length of the string (in bytes) */
        u8 Data[Length]; /* Characters of this string */
    }

Table data entries are encoded as 32 bit numbers in which top 4 bits specify type of the action and lower 28 specify an argument. There are 4 types of actions defined:

* 0 - `special`
* 1 - `shift`
* 2 - `reduce`
* 3 - `goto`

For `shift` and `goto` actions types. Argument specifies next state of the parser. Argument for `reduce` action, tells the parser, by which production to reduce. And finally `special` action type means either error condition (when argument is 0) or accept condition (when argument is 1).

### Building

Building TableGen should be as simple as executing `make` command in top project directory. Apart from standard C library, there are no external library dependencies. Some environment variables can be used to modify default build process:

* `CC` variable allows to change C compiler used (defaults to `gcc`)
* `CFLAGS` allows to change default compiler flags. For exaple `CCFLAGS="-ggdb -O0"` will disable optimizations and add debug information to the executable
* `CCLD` selects linker used (defaults to `CC`)
* `CCLDFLAGS` specifies flags for `CCLD` command
* `LIBS` specifies any extra libraries needed to build the executable
