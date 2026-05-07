#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"

#define DIFF_EASY      0
#define DIFF_NORMAL    1
#define DIFF_HARD      2
#define DIFF_VERY_HARD 3
#define DIFF_CUSTOM    4

void eloToDepthAndN(int elo, int *depth, int *n);
Move getBestMove(Board *b, int depth, int topN);

#endif
