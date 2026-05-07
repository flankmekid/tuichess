#ifndef MOVES_H
#define MOVES_H

#include "board.h"

int  generatePseudoMoves(Board *b, Move *out);
int  generateLegalMoves(Board *b, Move *out);
int  movesForSquare(Board *b, int row, int col, Move *out);
int  isInCheck(Board *b, int color);
int  isCheckmate(Board *b);
int  isStalemate(Board *b);
int  isDraw50(Board *b);

#endif
