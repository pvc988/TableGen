#pragma once

#include "stddef.h"

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
