#include "history.h"

void appendHistory(History *history, TokenList line) {
    history->items[(history->start + history->length) % HISTORY_CAPACITY] = line;
    if (history->length == HISTORY_CAPACITY) {
        history->start = (history->start + 1) % HISTORY_CAPACITY;
    } else {
        history->length++;
    }
}

TokenList getHistory(History *history, int index) {
    return history->items[(history->start + index) % HISTORY_CAPACITY];
}

void freeHistory(History history) {
    for (int i = 0; i < HISTORY_CAPACITY; i++) {
        freeTokens(history.items[i]);
    }
}
