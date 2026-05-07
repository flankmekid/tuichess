#include "platform.h"
#include "game.h"
#include "render.h"
#include "moves.h"
#include "engine.h"
#include "input.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void initGame(GameState *gs) {
    memset(gs, 0, sizeof(*gs));
    gs->mode        = MODE_HVAI;
    gs->playerColor = WHITE;
    gs->menuSection = MENU_MODE;
    gs->difficulty  = DIFF_NORMAL;
    gs->customElo   = 1200;
    gs->cursorRow  = 7;
    gs->cursorCol  = 0;
    gs->quitting   = 0;
}

void checkGameEnd(GameState *gs) {
    if (isCheckmate(&gs->board))  { gs->gameOver = GAMEOVER_CHECKMATE; return; }
    if (isStalemate(&gs->board))  { gs->gameOver = GAMEOVER_STALEMATE; return; }
    if (isDraw50(&gs->board))     { gs->gameOver = GAMEOVER_DRAW_50;   return; }
}

void applyPlayerMove(GameState *gs, Move m) {
    if (gs->historyLen >= 512) return;

    if (m.flags & (FLAG_PROMO_Q | FLAG_PROMO_R | FLAG_PROMO_B | FLAG_PROMO_N)) {
        int targetFlags;
        if (gs->mode == MODE_HVH) {
            int choice = 0;
            renderPromoMenu(gs, &choice);
            int flags[4] = {FLAG_PROMO_Q, FLAG_PROMO_R, FLAG_PROMO_B, FLAG_PROMO_N};
            targetFlags = flags[choice];
        } else {
            targetFlags = FLAG_PROMO_Q;
        }
        for (int i = 0; i < gs->legalMoveCount; i++) {
            if (gs->legalMoves[i].fromRow == m.fromRow &&
                gs->legalMoves[i].fromCol == m.fromCol &&
                gs->legalMoves[i].toRow   == m.toRow   &&
                gs->legalMoves[i].toCol   == m.toCol   &&
                gs->legalMoves[i].flags   == targetFlags) {
                m = gs->legalMoves[i];
                break;
            }
        }
    }

    UndoInfo u = applyMove(&gs->board, m);
    gs->history[gs->historyLen]     = m;
    gs->undoHistory[gs->historyLen] = u;
    gs->historyLen++;

    if (!isEmpty(u.capturedPiece)) {
        int owner = colorOf(u.capturedPiece);
        if (gs->capturedLen[owner] < 16)
            gs->captured[owner][gs->capturedLen[owner]++] = u.capturedPiece;
    }
    checkGameEnd(gs);
}

void runAiTurn(GameState *gs) {
    if (gs->gameOver) return;

    int depth, topN;
    if (gs->difficulty == DIFF_CUSTOM) {
        eloToDepthAndN(gs->customElo, &depth, &topN);
    } else {
        int depthMap[4] = {2, 3, 4, 5};
        int topNMap[4]  = {5, 3, 2, 1};
        depth = depthMap[gs->difficulty];
        topN  = topNMap[gs->difficulty];
    }

    Move best = getBestMove(&gs->board, depth, topN);
    if (best.fromRow == 0 && best.fromCol == 0 &&
        best.toRow   == 0 && best.toCol   == 0) return;

    if (best.flags & (FLAG_PROMO_Q | FLAG_PROMO_R | FLAG_PROMO_B | FLAG_PROMO_N))
        best.flags = FLAG_PROMO_Q;

    if (gs->historyLen >= 512) return;
    UndoInfo u = applyMove(&gs->board, best);
    gs->history[gs->historyLen]     = best;
    gs->undoHistory[gs->historyLen] = u;
    gs->historyLen++;
    if (!isEmpty(u.capturedPiece)) {
        int owner = colorOf(u.capturedPiece);
        if (gs->capturedLen[owner] < 16)
            gs->captured[owner][gs->capturedLen[owner]++] = u.capturedPiece;
    }
    checkGameEnd(gs);
}

void runGame(GameState *gs) {
    while (!gs->quitting) {
        if (gs->gameOver) {
            renderAll(gs);
            renderGameOver(gs);
            int ch = getch();
            if (ch == 'q' || ch == 'Q') { gs->quitting = 1; return; }
            if (ch == '\n' || ch == '\r') return;
#ifdef KEY_RESIZE
            if (ch == KEY_RESIZE) { clear(); }
#endif
            continue;
        }

        if (gs->mode == MODE_HVAI && gs->board.turn != gs->playerColor) {
            renderAll(gs);
            runAiTurn(gs);
            continue;
        }

        renderAll(gs);
        int ch = getch();
        if (ch == 'q' || ch == 'Q') return;
        handleInput(gs, ch);
    }
}

void runMenu(GameState *gs) {
    initRender();

    char eloStr[8] = "1200";
    int  eloLen    = 4;
    gs->customElo  = 1200;
    gs->menuSection = MENU_MODE;

    while (!gs->quitting) {
        renderMenu(gs);
        int ch = getch();

        if (ch == 'q' || ch == 'Q') { gs->quitting = 1; break; }

#ifdef KEY_RESIZE
        if (ch == KEY_RESIZE) { clear(); continue; }
#endif

        if (ch == '\n' || ch == '\r') {
            if (gs->difficulty == DIFF_CUSTOM) {
                gs->customElo = eloLen ? atoi(eloStr) : 400;
                if (gs->customElo < 400)  gs->customElo = 400;
                if (gs->customElo > 2800) gs->customElo = 2800;
            }
            gs->quitting       = 0;
            gs->menuSection    = MENU_MODE;
            initBoard(&gs->board);
            gs->historyLen     = 0;
            gs->capturedLen[0] = 0;
            gs->capturedLen[1] = 0;
            gs->pieceSelected  = 0;
            gs->legalMoveCount = 0;
            gs->gameOver       = GAMEOVER_NONE;
            gs->cursorRow      = (gs->playerColor == BLACK) ? 0 : 7;
            gs->cursorCol      = 0;
            runGame(gs);
            if (gs->quitting) break;
            gs->menuSection = MENU_MODE;
            continue;
        }

        int sec = gs->menuSection;

        if (ch == KEY_LEFT || ch == 'h') {
            if (sec == MENU_COLOR)                              gs->menuSection = MENU_MODE;
            else if (sec == MENU_DIFF || sec == MENU_ELO)      gs->menuSection = MENU_COLOR;
        }
        if (ch == KEY_RIGHT || ch == 'l') {
            if (sec == MENU_MODE && gs->mode == MODE_HVAI)     gs->menuSection = MENU_COLOR;
            else if (sec == MENU_COLOR)                        gs->menuSection = MENU_DIFF;
            else if (sec == MENU_DIFF && gs->difficulty == DIFF_CUSTOM) gs->menuSection = MENU_ELO;
        }

        if (sec == MENU_MODE) {
            if      (ch == KEY_UP   || ch == 'k') { if (gs->mode > 0)         gs->mode--; }
            else if (ch == KEY_DOWN || ch == 'j') { if (gs->mode < MODE_HVAI) gs->mode++; }
        }
        if (sec == MENU_COLOR) {
            if ((ch == KEY_UP || ch == 'k') && gs->playerColor == BLACK) gs->playerColor = WHITE;
            if ((ch == KEY_DOWN || ch == 'j') && gs->playerColor == WHITE) gs->playerColor = BLACK;
        }
        if (sec == MENU_DIFF) {
            if (ch == KEY_UP || ch == 'k') {
                if (gs->difficulty > 0) gs->difficulty--;
            } else if (ch == KEY_DOWN || ch == 'j') {
                if (gs->difficulty < DIFF_CUSTOM) gs->difficulty++;
                if (gs->difficulty == DIFF_CUSTOM) gs->menuSection = MENU_ELO;
            }
        }
        if (sec == MENU_ELO) {
            if (ch >= '0' && ch <= '9' && eloLen < 4) {
                eloStr[eloLen++] = (char)ch;
                eloStr[eloLen]   = '\0';
                int v = atoi(eloStr);
                if (v > 2800) { eloStr[--eloLen] = '\0'; v = atoi(eloStr); }
                gs->customElo = v ? v : 400;
            } else if ((ch == KEY_BACKSPACE || ch == 127) && eloLen > 0) {
                eloStr[--eloLen] = '\0';
                gs->customElo = eloLen ? atoi(eloStr) : 400;
            }
        }
    }

    cleanupRender();
}
