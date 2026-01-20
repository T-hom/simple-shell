#include <stdlib.h>
#include <string.h>

#include "tokenize.h"

const char *token_delimiters = " \t\n|><&;";

// Allocates a list of tokens from the input line. Should later be freed with freeTokens.
TokenList tokenize(char *line) {
  TokenList list;
  int capacity = 8;
  list.tokens = calloc(capacity, sizeof(char*));

  char *token = strtok(line, token_delimiters);
  while (token) {
    if (list.length + 1 == capacity) { // +1 to reserve space for NULL ending
      // Grow the capacity of the list.
      capacity *= 2;
      char **newlist = calloc(capacity, sizeof(char*));
      memcpy(newlist, list.tokens, sizeof(char *) * (list.length - 1));
      free(list.tokens);
      list.tokens = newlist;
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
