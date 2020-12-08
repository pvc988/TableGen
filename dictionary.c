#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

#define ALIGN(val, align) (align * ((val + align - 1) / align))

static const size_t AllocIncrement = 64;

static size_t binSearch(Dictionary *dict, const char *value, size_t *lastM)
{
    size_t m = 0;
    if(!dict->ItemCount)
    {
        if(lastM) *lastM = m;
        return (size_t)-1;
    }
    size_t l = 0, r = dict->ItemCount - 1;
    while(l <= r && l < dict->ItemCount && r != (size_t)-1)
    {
        m = (l + r) >> 1;
        int cmp = strcmp(dict->Items[m].Key, value);
        if(!cmp)
        {
            if(lastM) *lastM = m;
            return m;
        }
        else if(cmp < 0) l = m + 1;
        else r = m - 1;
    }
    if(lastM) *lastM = m;
    return (size_t)-1;
}

Dictionary *DictionaryCreate(void)
{
    return (Dictionary *)calloc(1, sizeof(Dictionary));
}

void DictionaryDelete(Dictionary *dict)
{
    if(dict->Items)
    {
        for(size_t i = 0; i < dict->ItemCount; ++i)
            free(dict->Items[i].Key);
        free(dict->Items);
    }
    free(dict);
}

bool DictionaryAddItem(Dictionary *dict, const char *key, void *data)
{
    size_t m = (size_t)-1;
    size_t foundIdx = binSearch(dict, key, &m);
    if(foundIdx != (size_t)-1)
        return false;

    size_t itemsNeeded = ALIGN(dict->ItemCount + 1, AllocIncrement);
    if(itemsNeeded != dict->AllocatedItems)
    {
        dict->Items = (DictionaryItem *)realloc(dict->Items, sizeof(DictionaryItem) * itemsNeeded);
        dict->AllocatedItems = itemsNeeded;
    }

    if(dict->ItemCount && strcmp(dict->Items[m].Key, key) < 0) ++m;

    if(m < dict->ItemCount)
        memmove(dict->Items + m + 1, dict->Items + m, sizeof(DictionaryItem) * (dict->ItemCount - m));

    dict->Items[m].Key = strdup(key);
    dict->Items[m].Data = data;

    ++dict->ItemCount;
    return true;
}

bool DictionaryRemoveItem(Dictionary *dict, const char *key)
{
    size_t foundIdx = binSearch(dict, key, 0);
    if(foundIdx == (size_t)-1)
        return false;
    free(dict->Items[foundIdx].Key);
    --dict->ItemCount;

    if(!dict->Items)
    {
        free(dict->Items);
        dict->Items = 0;
        dict->AllocatedItems = 0;
        return true;
    }

    memmove(dict->Items + foundIdx, dict->Items + foundIdx + 1, sizeof(DictionaryItem) * (dict->ItemCount - foundIdx));

    size_t itemsNeeded = ALIGN(dict->ItemCount, AllocIncrement);
    if(itemsNeeded != dict->AllocatedItems)
    {
        dict->Items = (DictionaryItem *)realloc(dict->Items, sizeof(DictionaryItem) * itemsNeeded);
        dict->AllocatedItems = itemsNeeded;
    }

    return true;
}

void *DictionaryGetValue(Dictionary *dict, const char *key)
{
    size_t foundIdx = binSearch(dict, key, 0);
    if(foundIdx == (size_t)-1)
        return 0;
    return dict->Items[foundIdx].Data;
}
