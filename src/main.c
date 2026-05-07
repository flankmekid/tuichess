#include "game.h"
#include <locale.h>

int main(void) {
    setlocale(LC_ALL, "");
    GameState gs;
    initGame(&gs);
    runMenu(&gs);
    return 0;
}
