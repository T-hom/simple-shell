#include <stdlib.h>
#include <string.h>

#include "tokenize.h"

const char *token_delimiters = " \t\n|><&;";

#define MAX_TOKENS 128

// Allocates a list of tokens from the input line. Should later be freed with freeTokens.
TokenList tokenize(char *line) {
  TokenList list = {0};
  list.tokens = calloc(MAX_TOKENS, sizeof(char*));

  char *token = strtok(line, token_delimiters);
  while (token) {
    if (list.length + 1 == MAX_TOKENS) { // +1 to reserve space for NULL ending
        free(list.tokens);
        return (TokenList) {NULL, -1};
    }

    list.tokens[list.length++] = strdup(token);

    token = strtok(NULL, token_delimiters);
  }

  // To use the list as the `argv` argument to `execv`, there needs to be a NULL terminator.
  list.tokens[list.length] = NULL;

  return list;
}

void freeTokens(TokenList list) {
    for (int i = 0; i < list.length; ++i) {
        free(list.tokens[i]);
    }
    free(list.tokens);
}
