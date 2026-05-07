#ifndef RENDER_H
#define RENDER_H

#include "game.h"

void initRender(void);
void cleanupRender(void);
void renderAll(GameState *gs);
void renderBoard(GameState *gs);
void renderHistory(GameState *gs);
void renderStatus(GameState *gs);
void renderMenu(GameState *gs);
void renderPromoMenu(GameState *gs, int *choice);
void renderGameOver(GameState *gs);

#endif
