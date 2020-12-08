#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct DictionaryItem
{
    char *Key;
    void *Data;
} DictionaryItem;

typedef struct Dictionary
{
    DictionaryItem *Items;
    size_t ItemCount;
    size_t AllocatedItems;
} Dictionary;

Dictionary *DictionaryCreate(void);
void DictionaryDelete(Dictionary *dict);
bool DictionaryAddItem(Dictionary *dict, const char *key, void *data);
bool DictionaryRemoveItem(Dictionary *dict, const char *key);
void *DictionaryGetValue(Dictionary *dict, const char *key);
