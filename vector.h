#pragma once

#include <stdbool.h>
#include "stddef.h"

typedef bool (*VectorItemEqualityComparer)(void *a, void *b);

typedef struct Vector
{
    void **Items;
    size_t ItemCount;
    size_t AllocatedItems;
} Vector;

Vector *VectorCreate(void);
void VectorDelete(Vector *vec);
void VectorInsertItem(Vector *vec, size_t idx, void *data);
void VectorPrependItem(Vector *vec, void *data);
void VectorAppendItem(Vector *vec, void *data);
void VectorDeleteItem(Vector *vec, size_t idx);
bool VectorContainsItem(Vector *vec, void *data, VectorItemEqualityComparer comparer);
bool VectorAppendIfUnique(Vector *vec, void *data, VectorItemEqualityComparer comparer);
size_t VectorMergeItems(Vector *dst, Vector *src, VectorItemEqualityComparer comparer);
