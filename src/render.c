#include "platform.h"
#include "render.h"
#include "moves.h"
#include "engine.h"
#include <string.h>
#include <stdio.h>
#include <wchar.h>

#define CP_NORMAL   1
#define CP_CURSOR   2
#define CP_SELECTED 3
#define CP_LEGAL    4
#define CP_HEADER   5
#define CP_STATUS   6
#define CP_W_PIECE  7
#define CP_B_PIECE  8
#define CP_TITLE    9

void initRender(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(CP_NORMAL,   COLOR_WHITE,  -1);
    init_pair(CP_CURSOR,   COLOR_BLACK,  COLOR_CYAN);
    init_pair(CP_SELECTED, COLOR_BLACK,  COLOR_YELLOW);
    init_pair(CP_LEGAL,    COLOR_BLACK,  COLOR_GREEN);
    init_pair(CP_HEADER,   COLOR_CYAN,   -1);
    init_pair(CP_STATUS,   COLOR_WHITE,  -1);
    init_pair(CP_W_PIECE,  COLOR_WHITE,  -1);
    init_pair(CP_B_PIECE,  COLOR_RED,    -1);
    init_pair(CP_TITLE,    COLOR_YELLOW, -1);
}

void cleanupRender(void) {
    endwin();
}

static void printGlyph(int y, int x, Piece p) {
    wchar_t wcs[2];
    switch (p) {
        case W_KING:   wcs[0] = 0x265A; break;
        case W_QUEEN:  wcs[0] = 0x265B; break;
        case W_ROOK:   wcs[0] = 0x265C; break;
        case W_BISHOP: wcs[0] = 0x265D; break;
        case W_KNIGHT: wcs[0] = 0x265E; break;
        case W_PAWN:   wcs[0] = 0x265F; break;
        case B_KING:   wcs[0] = 0x2654; break;
        case B_QUEEN:  wcs[0] = 0x2655; break;
        case B_ROOK:   wcs[0] = 0x2656; break;
        case B_BISHOP: wcs[0] = 0x2657; break;
        case B_KNIGHT: wcs[0] = 0x2658; break;
        case B_PAWN:   wcs[0] = 0x2659; break;
        default:       wcs[0] = 0x00B7; break;
    }
    wcs[1] = L'\0';
    mvaddwstr(y, x, wcs);
}

static void moveToStr(GameState *gs, int idx, char *buf, int buflen) {
    Move m = gs->history[idx];
    char files[] = "abcdefgh";
    if (m.flags & FLAG_CASTLE_K) {
        snprintf(buf, buflen, "O-O   ");
    } else if (m.flags & FLAG_CASTLE_Q) {
        snprintf(buf, buflen, "O-O-O ");
    } else {
        int fromRank = 8 - m.fromRow;
        int toRank   = 8 - m.toRow;
        char promo[3] = "";
        if      (m.flags & FLAG_PROMO_Q) snprintf(promo, 3, "=Q");
        else if (m.flags & FLAG_PROMO_R) snprintf(promo, 3, "=R");
        else if (m.flags & FLAG_PROMO_B) snprintf(promo, 3, "=B");
        else if (m.flags & FLAG_PROMO_N) snprintf(promo, 3, "=N");
        snprintf(buf, buflen, "%c%d%c%d%s",
            files[m.fromCol], fromRank,
            files[m.toCol],   toRank, promo);
    }
}

void renderHistory(GameState *gs) {
    int maxw, maxh;
    getmaxyx(stdscr, maxh, maxw);
    (void)maxh;

    attron(COLOR_PAIR(CP_HEADER));
    mvprintw(1, 2, "MOVES: ");
    attroff(COLOR_PAIR(CP_HEADER));

    int pairs = gs->historyLen / 2;
    int x = 10;
    int startPair = pairs - (maxw - 12) / 18;
    if (startPair < 0) startPair = 0;

    for (int i = startPair; i <= pairs && x < maxw - 20; i++) {
        char wbuf[16] = "......";
        char bbuf[16] = "......";
        if (i * 2 < gs->historyLen)     moveToStr(gs, i * 2,     wbuf, 16);
        if (i * 2 + 1 < gs->historyLen) moveToStr(gs, i * 2 + 1, bbuf, 16);
        mvprintw(1, x, "%d.%s %s  ", i + 1, wbuf, bbuf);
        x += 18;
    }

    mvhline(2, 2, ACS_HLINE, maxw - 4);
}

static int isLegalTarget(GameState *gs, int row, int col) {
    for (int i = 0; i < gs->legalMoveCount; i++)
        if (gs->legalMoves[i].toRow == row && gs->legalMoves[i].toCol == col)
            return 1;
    return 0;
}

void renderBoard(GameState *gs) {
    int maxw, maxh;
    getmaxyx(stdscr, maxh, maxw);
    (void)maxh;

    int boardTop  = 4;
    int cellW     = 3;
    int boardW    = 8 * cellW + 3;
    int boardLeft = (maxw - boardW) / 2;

    attron(COLOR_PAIR(CP_STATUS));
    mvprintw(boardTop, boardLeft + 3, " a  b  c  d  e  f  g  h ");
    attroff(COLOR_PAIR(CP_STATUS));
    boardTop++;
    mvhline(boardTop, boardLeft + 2, ACS_HLINE, 8 * cellW + 1);
    boardTop++;

    for (int r = 0; r < 8; r++) {
        int rank = 8 - r;
        attron(COLOR_PAIR(CP_STATUS));
        mvprintw(boardTop + r, boardLeft, "%d", rank);
        attroff(COLOR_PAIR(CP_STATUS));
        mvaddch(boardTop + r, boardLeft + 1, ACS_VLINE);

        for (int c = 0; c < 8; c++) {
            int x = boardLeft + 2 + c * cellW;
            int y = boardTop + r;

            int isCursor   = (r == gs->cursorRow   && c == gs->cursorCol);
            int isSelected = (gs->pieceSelected && r == gs->selectedRow && c == gs->selectedCol);
            int isLegal    = (gs->pieceSelected && isLegalTarget(gs, r, c));

            Piece p = getPiece(&gs->board, r, c);

            int cp = CP_NORMAL;
            if (isSelected)    cp = CP_SELECTED;
            else if (isCursor) cp = CP_CURSOR;
            else if (isLegal)  cp = CP_LEGAL;

            attron(COLOR_PAIR(cp));
            mvprintw(y, x, " ");
            if (!isEmpty(p)) {
                if (cp == CP_NORMAL)
                    attron(COLOR_PAIR(isWhite(p) ? CP_W_PIECE : CP_B_PIECE));
                printGlyph(y, x + 1, p);
                if (cp == CP_NORMAL)
                    attroff(COLOR_PAIR(isWhite(p) ? CP_W_PIECE : CP_B_PIECE));
            } else {
                printGlyph(y, x + 1, EMPTY);
            }
            attron(COLOR_PAIR(cp));
            mvprintw(y, x + 2, " ");
            attroff(COLOR_PAIR(cp));
        }
    }

    int bottomLine = boardTop + 8;
    mvhline(bottomLine, boardLeft + 2, ACS_HLINE, 8 * cellW + 1);
}

void renderStatus(GameState *gs) {
    int maxw, maxh;
    getmaxyx(stdscr, maxh, maxw);

    int row = maxh - 2;
    mvhline(row, 0, ACS_HLINE, maxw);
    row++;

    const char *turnStr = (gs->board.turn == WHITE) ? "White" : "Black";
    int inCheck = isInCheck(&gs->board, gs->board.turn);

    attron(COLOR_PAIR(CP_STATUS));
    if (inCheck)
        mvprintw(row, 2, "%s to move  CHECK  |  [hjkl/arrows] cursor  [enter] select  [u] undo  [q] menu", turnStr);
    else
        mvprintw(row, 2, "%s to move         |  [hjkl/arrows] cursor  [enter] select  [u] undo  [q] menu", turnStr);
    attroff(COLOR_PAIR(CP_STATUS));
}

void renderAll(GameState *gs) {
    erase();
    renderHistory(gs);
    renderBoard(gs);
    renderStatus(gs);
    refresh();
}

void renderMenu(GameState *gs) {
    int maxw, maxh;
    getmaxyx(stdscr, maxh, maxw);
    int mid = maxw / 2;

    erase();
    attron(COLOR_PAIR(CP_TITLE) | A_BOLD);
    mvprintw(maxh / 5, mid - 4, "tuichess");
    attroff(COLOR_PAIR(CP_TITLE) | A_BOLD);

    int row = maxh / 5 + 3;
    int sec = gs->menuSection;

#define SECTION_HDR(label, active) do { \
    int cp = (active) ? CP_SELECTED : CP_HEADER; \
    attron(COLOR_PAIR(cp) | ((active) ? A_BOLD : 0)); \
    mvprintw(row, mid - 9, "  %-13s", label); \
    attroff(COLOR_PAIR(cp) | ((active) ? A_BOLD : 0)); \
    row += 2; \
} while(0)

#define MENU_ITEM(label, chosen, active) do { \
    if (chosen) { \
        int cp = (active) ? CP_CURSOR : CP_SELECTED; \
        attron(COLOR_PAIR(cp) | A_BOLD); \
        mvprintw(row, mid - 9, "> %-13s", label); \
        attroff(COLOR_PAIR(cp) | A_BOLD); \
    } else { \
        attron(COLOR_PAIR(CP_NORMAL)); \
        mvprintw(row, mid - 9, "  %-13s", label); \
        attroff(COLOR_PAIR(CP_NORMAL)); \
    } \
    row++; \
} while(0)

    SECTION_HDR("Mode", sec == MENU_MODE);
    MENU_ITEM("Human vs Human", gs->mode == MODE_HVH,  sec == MENU_MODE);
    MENU_ITEM("Human vs AI",    gs->mode == MODE_HVAI, sec == MENU_MODE);

    if (gs->mode == MODE_HVAI) {
        row++;
        SECTION_HDR("Play as", sec == MENU_COLOR);
        MENU_ITEM("White", gs->playerColor == WHITE, sec == MENU_COLOR);
        MENU_ITEM("Black", gs->playerColor == BLACK, sec == MENU_COLOR);

        row++;
        SECTION_HDR("Difficulty", sec == MENU_DIFF || sec == MENU_ELO);
        const char *diffLabels[5] = {"Easy", "Normal", "Hard", "Very Hard", "Custom"};
        for (int i = 0; i < 5; i++) {
            int active = (sec == MENU_DIFF || sec == MENU_ELO);
            MENU_ITEM(diffLabels[i], gs->difficulty == i, active);
        }

        if (gs->difficulty == DIFF_CUSTOM) {
            row++;
            int eloCp = (sec == MENU_ELO) ? CP_CURSOR : CP_NORMAL;
            attron(COLOR_PAIR(eloCp));
            mvprintw(row, mid - 9, "  ELO: %d  ", gs->customElo);
            attroff(COLOR_PAIR(eloCp));
        }
    }

#undef SECTION_HDR
#undef MENU_ITEM

    row += 2;
    attron(COLOR_PAIR(CP_STATUS));
    mvprintw(row, mid - 22, "[h/l] section  [j/k] select  [enter] start  [q] quit");
    attroff(COLOR_PAIR(CP_STATUS));

    refresh();
}

void renderPromoMenu(GameState *gs, int *choice) {
    (void)gs;
    int maxw, maxh;
    getmaxyx(stdscr, maxh, maxw);
    int mid = maxw / 2;
    int row = maxh / 2 - 4;

    const char *opts[4] = {"Queen", "Rook", "Bishop", "Knight"};
    int sel = 0;

    while (1) {
        attron(COLOR_PAIR(CP_HEADER));
        mvprintw(row, mid - 8, "  Promote pawn to:  ");
        attroff(COLOR_PAIR(CP_HEADER));
        for (int i = 0; i < 4; i++) {
            if (i == sel) {
                attron(COLOR_PAIR(CP_SELECTED) | A_BOLD);
                mvprintw(row + 2 + i, mid - 8, "> %-8s", opts[i]);
                attroff(COLOR_PAIR(CP_SELECTED) | A_BOLD);
            } else {
                attron(COLOR_PAIR(CP_NORMAL));
                mvprintw(row + 2 + i, mid - 8, "  %-8s", opts[i]);
                attroff(COLOR_PAIR(CP_NORMAL));
            }
        }
        attron(COLOR_PAIR(CP_STATUS));
        mvprintw(row + 7, mid - 14, "[up/dn j/k] navigate  [enter] confirm");
        attroff(COLOR_PAIR(CP_STATUS));
        refresh();

        int ch = getch();
        if (ch == KEY_UP   || ch == 'k') { if (sel > 0) sel--; }
        if (ch == KEY_DOWN || ch == 'j') { if (sel < 3) sel++; }
        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) break;
    }
    *choice = sel;
}

void renderGameOver(GameState *gs) {
    int maxw, maxh;
    getmaxyx(stdscr, maxh, maxw);
    int mid   = maxw / 2;
    int row   = maxh / 2 - 2;

    const char *msg;
    if (gs->gameOver == GAMEOVER_CHECKMATE)
        msg = (gs->board.turn == WHITE) ? "Black wins by checkmate!" : "White wins by checkmate!";
    else if (gs->gameOver == GAMEOVER_STALEMATE)
        msg = "Draw — stalemate.";
    else
        msg = "Draw — 50-move rule.";

    attron(COLOR_PAIR(CP_TITLE) | A_BOLD);
    mvprintw(row, mid - (int)strlen(msg) / 2, "%s", msg);
    attroff(COLOR_PAIR(CP_TITLE) | A_BOLD);

    attron(COLOR_PAIR(CP_STATUS));
    mvprintw(row + 2, mid - 16, "[enter] Return to menu   [q] Quit");
    attroff(COLOR_PAIR(CP_STATUS));

    refresh();
}
