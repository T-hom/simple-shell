#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "tokenize.h"
#include "alias.h"

int invokeHistory(TokenList *, History *);
int substituteHistory(TokenList *, History *, long);
void listHistory(TokenList, History *);
void cd(TokenList);
void getpath(TokenList);
void setpath(TokenList);
void runExternalCommand(char **argv);
void alias(TokenList tokens, AliasList *alias_list);
void unalias(TokenList tokens, AliasList *alias_list);

int main(void) {
  char line_buffer[512];
  History history = {0};
  AliasList alias_list;

  init_aliases(&alias_list);

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

    char *command = tokens.tokens[0];

    if (command[0] == '!') { // History invocation.
      if (invokeHistory(&tokens, &history) < 0) {
        continue;
      }
      command = tokens.tokens[0]; // Previous command may have been freed and replaced.
    } else {
      appendHistory(&history, tokens);
    }


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
    } else if (strcmp(command, "alias") == 0) {
      alias(tokens, &alias_list);
    } else if (strcmp(command, "unalias") == 0) {
      unalias(tokens, &alias_list);
    } else {
      runExternalCommand(tokens.tokens);
    }
  }

  freeHistory(history);

  fprintf(stderr, "\nResetting path to original: %s\n", original_path);
  setenv("PATH", original_path, 1);
}

int invokeHistory(TokenList *tokens, History *history) {
  if (tokens->length != 1) {
    fprintf(stderr, "Error: history invocation expects no arguments.\n");
    return -1;
  }

  char *command = tokens->tokens[0];

  if (strcmp(command, "!!") == 0) {
    return substituteHistory(tokens, history, -1);
  }

  if (command[1] == '\0') {
    fprintf(stderr, "Unrecognized command: '!'.\n");
    return -1;
  }

  char *number_end;
  long offset = strtol(&command[1], &number_end, 10);

  if (*number_end != '\0') {
    // The remainder of the command was not a valid number.
    fprintf(stderr, "Unrecognized command: '%s'\n", command);
    return -1;
  }

  return substituteHistory(tokens, history, offset);
}

int substituteHistory(TokenList *tokens, History *history, long offset) {
  long index;
  if (offset > 0) {
    index = offset - 1;
  } else if (offset < 0) {
    index = history->length + offset;
  } else {
    fprintf(stderr, "Error: history offset must be positive or negative.\n");
    return -1;
  }

  if (index < 0 || index >= history->length) {
    fprintf(stderr, "Error: entry %ld does not exist in history.\n",
            offset);
    return -1;
  }
  
  freeTokens(*tokens);
  *tokens = getHistory(history, index);

  return 0;
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
  pid_t pid = fork();

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
    while (wait(NULL) == -1) {
      // wait returns -1 if interrupted, so keep calling until it succeeds.
    }
    break;
  }
}

void alias(TokenList tokens, AliasList *alias_list) {
  if (tokens.length == 1) { // if nothing comes after alias command, print all aliases
    print_aliases(alias_list);
    return;
  }

  if (tokens.length == 2) { // check if there is a command
    fprintf(stderr, "Error: alias expects a command: 'alias <name> <command>'\n");
    return;
  }

  
  char *alias_name = tokens.tokens[1]; // get alias name

  // build the command from the rest of the tokens
  char alias_command[512] = "";
  for (int i = 2; i < tokens.length; i++) {
    if (i > 2) strcat(alias_command, " "); // add space between words
    strcat(alias_command, tokens.tokens[i]);
  }

  add_alias(alias_list, alias_name, alias_command);
}

void unalias(TokenList tokens, AliasList *alias_list) {
  if (tokens.length != 2) {
    fprintf(stderr, "Error! unalias only expects one argument: 'unalias <name>'\n");
    return;
  }
  remove_alias(alias_list, tokens.tokens[1]);
}