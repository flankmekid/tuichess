#ifndef GAME_H
#define GAME_H

#include "board.h"

#define MODE_HVH  0
#define MODE_HVAI 1

#define MENU_MODE  0
#define MENU_COLOR 1
#define MENU_DIFF  2
#define MENU_ELO   3

#define GAMEOVER_NONE      0
#define GAMEOVER_CHECKMATE 1
#define GAMEOVER_STALEMATE 2
#define GAMEOVER_DRAW_50   3

struct GameState {
    Board    board;
    Move     history[512];
    UndoInfo undoHistory[512];
    int      historyLen;
    Piece    captured[2][16];
    int      capturedLen[2];
    int      mode;
    int      playerColor;
    int      menuSection;
    int      difficulty;
    int      customElo;
    int      cursorRow, cursorCol;
    int      selectedRow, selectedCol;
    int      pieceSelected;
    Move     legalMoves[256];
    int      legalMoveCount;
    int      gameOver;
    int      quitting;
};

typedef struct GameState GameState;

void initGame(GameState *gs);
void runMenu(GameState *gs);
void runGame(GameState *gs);
void applyPlayerMove(GameState *gs, Move m);
void runAiTurn(GameState *gs);
void checkGameEnd(GameState *gs);

#endif
