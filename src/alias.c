#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alias.h"

void init_aliases(AliasList *alias_list) {
    alias_list->count = 0; // set count to 0
    for (int i = 0; i < MAX_ALIASES; i++) {
        alias_list->alias[i][0] = NULL; // initialise all name variables to null
        alias_list->alias[i][1] = NULL; // otherwise we have random data in memory
    }
}


void add_alias(AliasList *alias_list, char *name, char *command) {
    for (int i = 0; i < alias_list->count; i++) { 
        if (strcmp(alias_list->alias[i][0], name) == 0) { // check if alias with same name already exists
            free(alias_list->alias[i][1]); // free old command memory
            alias_list->alias[i][1] = strdup(command); // update command
            printf("Alias '%s' was already set and has been overwritten.\n", name);
            return;
        }
    }
    if (alias_list->count >= MAX_ALIASES) { // check if we have space for new alias
        fprintf(stderr, "Error! Maximum number of aliases (%d) reached.\n", MAX_ALIASES);
        return;
    }

    alias_list->alias[alias_list->count][0] = strdup(name);     // duplicate name
    alias_list->alias[alias_list->count][1] = strdup(command);  // duplicate command
    alias_list->count++;
}

void remove_alias(AliasList *alias_list, char *name) {
    int index = -1;
    for (int i = 0; i < alias_list->count; i++) { // find index of alias to remove
        if (strcmp(alias_list->alias[i][0], name) == 0) { // compare names
            index = i; // found the alias to remove
            break;;
        }
    }
    if (index == -1) {
        fprintf(stderr, "Error! Alias '%s' not found\n", name);
        return;
    }
    free(alias_list->alias[index][0]); // free memory for name and command of the alias being removed
    free(alias_list->alias[index][1]);

    // Shift all aliases after the removed one to the left
    for (int i = index; i < alias_list->count - 1; i++) {
        alias_list->alias[i][0] = alias_list->alias[i + 1][0];
        alias_list->alias[i][1] = alias_list->alias[i + 1][1];
    }

    alias_list->alias[alias_list->count - 1][0] = NULL; // set last alias to null after shifting
    alias_list->alias[alias_list->count - 1][1] = NULL;

    alias_list->count--; //reduce count of aliases
}

void print_aliases(AliasList *alias_list) {
    if(alias_list->count == 0) {
        printf("No aliases have been set yet\n");
        return;
    }
    for (int i = 0; i < alias_list->count; i++) { // loop through aliases and print them in format ' name = command'
        printf("%s = %s\n", alias_list->alias[i][0], alias_list->alias[i][1]);
    }
}