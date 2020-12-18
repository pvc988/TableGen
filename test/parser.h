#pragma once

#include <stdbool.h>

bool ParserCreate(const char *filename);
void ParserDelete(void);
bool ParserParse(int *result);
