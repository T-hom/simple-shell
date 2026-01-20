#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "tokenize.h"

void runExternalCommand(char **argv);

int main(void) {
  char line_buffer[512];
  TokenList tokens = {0};

  char *original_path = getenv("PATH");
  char *home_path = getenv("HOME");
  if (chdir(home_path) < 0) {
    fprintf(stderr, "Failed to change to home directory: %s\n", strerror(errno));
  }

  while (1) {
    printf("$ ");
    char *input = fgets(line_buffer, sizeof(line_buffer), stdin);

    // Exit on EOF or read error.
    if (input == NULL) {
      break;
    }

    freeTokens(tokens); // Free previous tokens (no-op the first time)
    TokenList tokens = tokenize(input);

    // Uncomment to display tokens:
    // for (int i = 0; i < tokens.length; i++) {
    //   printf("token %d: [%s]\n", i, tokens.tokens[i]);
    // }

    if (tokens.length == 0) {
      // Prompt again on empty line.
      continue;
    }

    // Exit if first token is 'exit'.
    if (strcmp(tokens.tokens[0], "exit") == 0) {
      break;
    }

    if (strcmp(tokens.tokens[0], "getpath") == 0) {
      if (tokens.length != 1) {
        fprintf(stderr, "Error: getpath doesn't take arguments, just type 'getpath'.\n");
      } else {
        printf("%s\n", getenv("PATH"));
      }

      continue;
    }

    if (strcmp(tokens.tokens[0], "setpath") == 0) {
      if (tokens.length != 2) {
        fprintf(stderr, "Error: setpath expects one argument: 'setpath <PATH>'.\n");
      }  else {
        setenv("PATH", tokens.tokens[1], 1);
      }

      continue;
    }

    runExternalCommand(tokens.tokens);
  }

  fprintf(stderr, "\nResetting path to original: %s\n", original_path);
  setenv("PATH", original_path, 1);
}

void runExternalCommand(char **argv) {
  int pid = fork();
  int stat;
  
  switch (pid) {
    case -1:
      fprintf(stderr, "Failed to fork to run '%s': %s\n", argv[0], strerror(errno));
      break;

    case 0:
      execvp(argv[0], argv);
      // execv only returns on an error.
      fprintf(stderr, "Failed to exec '%s': %s\n", argv[0], strerror(errno));
      exit(1);

    default:
      while (!wait(&stat)) {
        // wait returns -1 if interrupted, so keep calling until it succeeds.
      }
      break;
  }
}

