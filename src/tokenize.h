#ifndef TOKENIZE_H
#define TOKENIZE_H

typedef struct {
  char **tokens;
  int length;
} TokenList;

TokenList tokenize(char *line);
void freeTokens(TokenList list);

#endif
