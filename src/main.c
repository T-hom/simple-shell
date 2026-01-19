#include <stdio.h>
#include <string.h>

const char *token_delimiters = " \t\n|><&;";

int main(void) {
  char line_buffer[512];

  while (1) {
    printf("$ ");
    char *input = fgets(line_buffer, sizeof(line_buffer), stdin);

    // Exit on EOF or read error.
    if (input == NULL) {
      break;
    }

    char *token = strtok(input, token_delimiters);

    // Exit if first token exists and is 'exit'.
    if (token && strcmp(token, "exit") == 0) {
      break;
    }

    while (token) {
      printf("got token: [%s]\n", token);
      token = strtok(NULL, token_delimiters);
    }
  }
}
