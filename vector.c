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

void *VectorGetItem(Vector *vec, size_t idx)
{
    if(idx >= vec->ItemCount)
        return 0;
    return vec->Items[idx];
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

bool VectorContainsItem(Vector *vec, void *data, VectorItemEqualityComparer comparer)
{
    if(!comparer)
    {
        for(size_t i = 0; i < vec->ItemCount; ++i)
        {
            if(vec->Items[i] == data)
                return true;
        }
    }
    else
    {
        for(size_t i = 0; i < vec->ItemCount; ++i)
        {
            if(comparer(vec->Items[i], data))
                return true;
        }
    }
    return false;
}

bool VectorAppendIfUnique(Vector *vec, void *data, VectorItemEqualityComparer comparer)
{
    if(VectorContainsItem(vec, data, comparer))
        return false;
    VectorAppendItem(vec, data);
    return true;
}

size_t VectorMergeItems(Vector *dst, Vector *src, VectorItemEqualityComparer comparer)
{
    size_t added = 0;
    for(size_t i = 0; i < src->ItemCount; ++i)
    {
        void *item = src->Items[i];
        if(VectorAppendIfUnique(dst, item, comparer))
            ++added;
    }
    return added;
}

size_t VectorIndexOf(Vector *vec, void *data)
{
    for(size_t i = 0; i < vec->ItemCount; ++i)
    {
        if(vec->Items[i] == data)
            return i;
    }
    return (size_t)-1;
}

void VectorAppendItems(Vector *dst, Vector *src)
{
    for(size_t i = 0; i < src->ItemCount; ++i)
        VectorAppendItem(dst, src->Items[i]);
}
