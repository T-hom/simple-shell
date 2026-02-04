#define MAX_ALIASES 10 // max of 10 like pdf says

typedef struct {
    char *alias[MAX_ALIASES][2]; // 2D array alais[i][name][command]
    int count;                // current number of aliases set
} AliasList;

void add_alias(AliasList *alias_list, char *name, char *command);
void remove_alias(AliasList *alias_list, char *name);
void init_aliases(AliasList *alias_list); //  neede because otherwise it has random data

void print_aliases(AliasList *alias_list);
