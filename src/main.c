#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "tokenize.h"

void listHistory(TokenList, History *);
void cd(TokenList);
void getpath(TokenList);
void setpath(TokenList);
void runExternalCommand(char **argv);

int main(void) {
  char line_buffer[512];
  History history = {0};

  char *original_path = getenv("PATH");
  char *home_path = getenv("HOME");
  if (chdir(home_path) < 0) {
    fprintf(stderr, "Failed to change to home directory: %s\n",
            strerror(errno));
  }

  while (1) {
    printf("$ ");
    char *input = fgets(line_buffer, sizeof(line_buffer), stdin);

    // Exit on EOF or read error.
    if (input == NULL) {
      break;
    }

    TokenList tokens = tokenize(input);
    if (tokens.length == -1) {
      fprintf(stderr, "Error: line too long.\n");
      continue;
    }

    // Uncomment to display tokens:
    // for (int i = 0; i < tokens.length; i++) {
    //   printf("token %d: [%s]\n", i, tokens.tokens[i]);
    // }

    if (tokens.length == 0) {
      // Prompt again on empty line.
      continue;
    }

    appendHistory(&history, tokens);

    char *command = tokens.tokens[0];

    if (strcmp(command, "exit") == 0) {
      break;
    }

    if (strcmp(command, "history") == 0) {
      listHistory(tokens, &history);
    } else if (strcmp(command, "cd") == 0) {
      cd(tokens);
    } else if (strcmp(command, "getpath") == 0) {
      getpath(tokens);
    } else if (strcmp(command, "setpath") == 0) {
      setpath(tokens);
    } else {
      runExternalCommand(tokens.tokens);
    }
  }

  freeHistory(history);

  fprintf(stderr, "\nResetting path to original: %s\n", original_path);
  setenv("PATH", original_path, 1);
}

void listHistory(TokenList tokens, History *history) {
  if (tokens.length != 1) {
    fprintf(stderr, "Error: history expects no arguments: 'history'.\n");
    return;
  }

  for (int i = 0; i < history->length; i++) {
    printf("%d: ", i + 1);
    TokenList line = getHistory(history, i);
    printf("%s", line.tokens[0]); // Empty lines aren't stored in history.
    for (int i = 1; i < line.length; i++) {
      printf(" %s", line.tokens[i]);
    }
    printf("\n");
  }
}

void cd(TokenList tokens) {
  char *path;
  switch (tokens.length) {
  case 1:
    path = getenv("HOME");
    break;
  case 2:
    path = tokens.tokens[1];
    break;
  default:
    fprintf(stderr, "Error: cd expects 0 or 1 arguments: 'cd [path]'.\n");
    return;
  }

  if (chdir(path) < 0) {
    fprintf(stderr, "Failed to chdir to '%s': %s\n", path, strerror(errno));
  }
}

void getpath(TokenList tokens) {
  if (tokens.length != 1) {
    fprintf(stderr, "Error: getpath expects no arguments: 'getpath'.\n");
    return;
  }

  printf("%s\n", getenv("PATH"));
}

void setpath(TokenList tokens) {
  if (tokens.length != 2) {
    fprintf(stderr, "Error: setpath expects one argument: 'setpath <PATH>'.\n");
    return;
  }
  setenv("PATH", tokens.tokens[1], 1);
}

void runExternalCommand(char **argv) {
  int pid = fork();
  int stat;

  switch (pid) {
  case -1:
    fprintf(stderr, "Failed to fork to run '%s': %s\n", argv[0],
            strerror(errno));
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
