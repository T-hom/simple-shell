#ifndef HISTORY_H
#define HISTORY_H

#include "tokenize.h"

#define HISTORY_CAPACITY 20

typedef struct {
  TokenList items[HISTORY_CAPACITY];
  int start;
  int length;
} History;

void appendHistory(History *history, TokenList line);
TokenList getHistory(History *history, int index);
void freeHistory(History history);

#endif
