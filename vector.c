#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define ALIGN(val, align) (align * ((val + align - 1) / align))

static const size_t AllocIncrement = 64;

Vector *VectorCreate(void)
{
    return (Vector *)calloc(1, sizeof(Vector));
}

void VectorDelete(Vector *vec)
{
    if(vec->Items)
        free(vec->Items);
    free(vec);
}

void VectorInsertItem(Vector *vec, size_t idx, void *data)
{
    if(idx > vec->ItemCount) return;

    size_t itemsNeeded = ALIGN(vec->ItemCount + 1, AllocIncrement);
    if(itemsNeeded != vec->AllocatedItems)
    {
        vec->Items = (void **)realloc(vec->Items, sizeof(void *) * itemsNeeded);
        vec->AllocatedItems = itemsNeeded;
    }

    if(idx < vec->ItemCount)
        memmove(vec->Items + idx + 1, vec->Items + idx, vec->ItemCount - idx);

    vec->Items[idx] = data;
    ++vec->ItemCount;
}

void VectorPrependItem(Vector *vec, void *data)
{
    VectorInsertItem(vec, 0, data);
}

void VectorAppendItem(Vector *vec, void *data)
{
    VectorInsertItem(vec, vec->ItemCount, data);
}

void VectorDeleteItem(Vector *vec, size_t idx)
{
    if(idx >= vec->ItemCount) return;
    else if(idx != vec->ItemCount - 1)
        memmove(vec->Items + idx, vec->Items + idx + 1, vec->ItemCount - idx - 1);
    --vec->ItemCount;
    size_t itemsNeeded = ALIGN(vec->ItemCount, AllocIncrement);
    if(itemsNeeded != vec->AllocatedItems)
    {
        vec->Items = (void **)realloc(vec->Items, sizeof(void *) * itemsNeeded);
        vec->AllocatedItems = itemsNeeded;
    }
}
