#include "platform.h"
#include "input.h"
#include "moves.h"
#include "render.h"

static void moveCursor(GameState *gs, int dr, int dc) {
    int nr = gs->cursorRow + dr;
    int nc = gs->cursorCol + dc;
    if (nr >= 0 && nr < 8) gs->cursorRow = nr;
    if (nc >= 0 && nc < 8) gs->cursorCol = nc;
}

static void selectSquare(GameState *gs) {
    int r = gs->cursorRow;
    int c = gs->cursorCol;

    if (gs->pieceSelected) {
        for (int i = 0; i < gs->legalMoveCount; i++) {
            if (gs->legalMoves[i].toRow == r && gs->legalMoves[i].toCol == c) {
                applyPlayerMove(gs, gs->legalMoves[i]);
                gs->pieceSelected  = 0;
                gs->legalMoveCount = 0;
                return;
            }
        }
        gs->pieceSelected  = 0;
        gs->legalMoveCount = 0;
    }

    Piece p = getPiece(&gs->board, r, c);
    if (!isEmpty(p) && colorOf(p) == gs->board.turn) {
        gs->selectedRow    = r;
        gs->selectedCol    = c;
        gs->pieceSelected  = 1;
        gs->legalMoveCount = movesForSquare(&gs->board, r, c, gs->legalMoves);
    }
}

void handleInput(GameState *gs, int ch) {
    switch (ch) {
        case KEY_UP:    case 'k': moveCursor(gs, -1,  0); break;
        case KEY_DOWN:  case 'j': moveCursor(gs,  1,  0); break;
        case KEY_LEFT:  case 'h': moveCursor(gs,  0, -1); break;
        case KEY_RIGHT: case 'l': moveCursor(gs,  0,  1); break;

        case '\n': case '\r':
            selectSquare(gs);
            break;

        case 27:
            gs->pieceSelected  = 0;
            gs->legalMoveCount = 0;
            break;

        case 'u':
            if (gs->historyLen > 0) {
                int steps = (gs->mode == MODE_HVAI && gs->historyLen >= 2) ? 2 : 1;
                for (int i = 0; i < steps && gs->historyLen > 0; i++) {
                    gs->historyLen--;
                    Move m     = gs->history[gs->historyLen];
                    UndoInfo u = gs->undoHistory[gs->historyLen];
                    if (!isEmpty(u.capturedPiece)) {
                        int owner = colorOf(u.capturedPiece);
                        if (gs->capturedLen[owner] > 0)
                            gs->capturedLen[owner]--;
                    }
                    undoMove(&gs->board, m, u);
                }
                gs->pieceSelected  = 0;
                gs->legalMoveCount = 0;
                gs->gameOver       = GAMEOVER_NONE;
            }
            break;

#ifdef KEY_RESIZE
        case KEY_RESIZE:
            clear();
            break;
#endif

        default: break;
    }
}
