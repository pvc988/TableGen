#include <stdlib.h>
#include <string.h>

#include "production.h"
#include "vector.h"

Production *ProductionCreate(char *id, Symbol *left, Vector *right)
{
    Production *prod = (Production *)malloc(sizeof(Production));
    prod->Id = id ? strdup(id) : 0;
    prod->Left = left;
    prod->Right = right;
    return prod;
}

void ProductionDelete(Production *prod)
{
    if(prod->Id) free(prod->Id);
    if(prod->Right) VectorDelete(prod->Right);
    free(prod);
}
