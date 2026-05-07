#include "moves.h"
#include <string.h>

static int inBounds(int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

static void addMove(Move *out, int *cnt, int fr, int fc, int tr, int tc, int flags) {
    out[*cnt].fromRow = fr; out[*cnt].fromCol = fc;
    out[*cnt].toRow   = tr; out[*cnt].toCol   = tc;
    out[*cnt].flags   = flags;
    (*cnt)++;
}

static void addPawnMoves(Board *b, int r, int c, Move *out, int *cnt) {
    int dir   = (b->turn == WHITE) ? -1 :  1;
    int start = (b->turn == WHITE) ?  6 :  1;
    int promo = (b->turn == WHITE) ?  0 :  7;

    int nr = r + dir;
    if (inBounds(nr, c) && isEmpty(b->squares[nr][c])) {
        if (nr == promo) {
            addMove(out, cnt, r, c, nr, c, FLAG_PROMO_Q);
            addMove(out, cnt, r, c, nr, c, FLAG_PROMO_R);
            addMove(out, cnt, r, c, nr, c, FLAG_PROMO_B);
            addMove(out, cnt, r, c, nr, c, FLAG_PROMO_N);
        } else {
            addMove(out, cnt, r, c, nr, c, FLAG_NONE);
            if (r == start && isEmpty(b->squares[nr + dir][c]))
                addMove(out, cnt, r, c, nr + dir, c, FLAG_NONE);
        }
    }

    int caps[2] = {c - 1, c + 1};
    for (int i = 0; i < 2; i++) {
        int nc = caps[i];
        if (!inBounds(nr, nc)) continue;
        int isEnPassant = (b->enPassantCol == nc && r == (b->turn == WHITE ? 3 : 4));
        Piece target = b->squares[nr][nc];
        if (isEnPassant) {
            addMove(out, cnt, r, c, nr, nc, FLAG_EN_PASSANT);
        } else if (!isEmpty(target) && colorOf(target) != b->turn) {
            if (nr == promo) {
                addMove(out, cnt, r, c, nr, nc, FLAG_PROMO_Q);
                addMove(out, cnt, r, c, nr, nc, FLAG_PROMO_R);
                addMove(out, cnt, r, c, nr, nc, FLAG_PROMO_B);
                addMove(out, cnt, r, c, nr, nc, FLAG_PROMO_N);
            } else {
                addMove(out, cnt, r, c, nr, nc, FLAG_NONE);
            }
        }
    }
}

static void addSliding(Board *b, int r, int c, int dr, int dc, Move *out, int *cnt) {
    for (int i = 1; i < 8; i++) {
        int nr = r + dr * i, nc = c + dc * i;
        if (!inBounds(nr, nc)) break;
        Piece target = b->squares[nr][nc];
        if (isEmpty(target)) {
            addMove(out, cnt, r, c, nr, nc, FLAG_NONE);
        } else {
            if (colorOf(target) != b->turn)
                addMove(out, cnt, r, c, nr, nc, FLAG_NONE);
            break;
        }
    }
}

static void addKnight(Board *b, int r, int c, Move *out, int *cnt) {
    int deltas[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (int i = 0; i < 8; i++) {
        int nr = r + deltas[i][0], nc = c + deltas[i][1];
        if (!inBounds(nr, nc)) continue;
        Piece t = b->squares[nr][nc];
        if (isEmpty(t) || colorOf(t) != b->turn)
            addMove(out, cnt, r, c, nr, nc, FLAG_NONE);
    }
}

static void addKing(Board *b, int r, int c, Move *out, int *cnt) {
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = r + dr, nc = c + dc;
            if (!inBounds(nr, nc)) continue;
            Piece t = b->squares[nr][nc];
            if (isEmpty(t) || colorOf(t) != b->turn)
                addMove(out, cnt, r, c, nr, nc, FLAG_NONE);
        }
    }
    if (b->turn == WHITE && r == 7 && c == 4) {
        if ((b->castleRights & CASTLE_WK) &&
            isEmpty(b->squares[7][5]) && isEmpty(b->squares[7][6]))
            addMove(out, cnt, r, c, 7, 6, FLAG_CASTLE_K);
        if ((b->castleRights & CASTLE_WQ) &&
            isEmpty(b->squares[7][3]) && isEmpty(b->squares[7][2]) && isEmpty(b->squares[7][1]))
            addMove(out, cnt, r, c, 7, 2, FLAG_CASTLE_Q);
    }
    if (b->turn == BLACK && r == 0 && c == 4) {
        if ((b->castleRights & CASTLE_BK) &&
            isEmpty(b->squares[0][5]) && isEmpty(b->squares[0][6]))
            addMove(out, cnt, r, c, 0, 6, FLAG_CASTLE_K);
        if ((b->castleRights & CASTLE_BQ) &&
            isEmpty(b->squares[0][3]) && isEmpty(b->squares[0][2]) && isEmpty(b->squares[0][1]))
            addMove(out, cnt, r, c, 0, 2, FLAG_CASTLE_Q);
    }
}

int generatePseudoMoves(Board *b, Move *out) {
    int cnt = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece p = b->squares[r][c];
            if (isEmpty(p) || colorOf(p) != b->turn) continue;
            switch (p) {
                case W_PAWN: case B_PAWN:
                    addPawnMoves(b, r, c, out, &cnt); break;
                case W_KNIGHT: case B_KNIGHT:
                    addKnight(b, r, c, out, &cnt); break;
                case W_BISHOP: case B_BISHOP:
                    addSliding(b, r, c, -1, -1, out, &cnt);
                    addSliding(b, r, c, -1,  1, out, &cnt);
                    addSliding(b, r, c,  1, -1, out, &cnt);
                    addSliding(b, r, c,  1,  1, out, &cnt); break;
                case W_ROOK: case B_ROOK:
                    addSliding(b, r, c, -1,  0, out, &cnt);
                    addSliding(b, r, c,  1,  0, out, &cnt);
                    addSliding(b, r, c,  0, -1, out, &cnt);
                    addSliding(b, r, c,  0,  1, out, &cnt); break;
                case W_QUEEN: case B_QUEEN:
                    addSliding(b, r, c, -1, -1, out, &cnt);
                    addSliding(b, r, c, -1,  1, out, &cnt);
                    addSliding(b, r, c,  1, -1, out, &cnt);
                    addSliding(b, r, c,  1,  1, out, &cnt);
                    addSliding(b, r, c, -1,  0, out, &cnt);
                    addSliding(b, r, c,  1,  0, out, &cnt);
                    addSliding(b, r, c,  0, -1, out, &cnt);
                    addSliding(b, r, c,  0,  1, out, &cnt); break;
                case W_KING: case B_KING:
                    addKing(b, r, c, out, &cnt); break;
                default: break;
            }
        }
    }
    return cnt;
}

int isInCheck(Board *b, int color) {
    int kr = -1, kc = -1;
    Piece king = (color == WHITE) ? W_KING : B_KING;
    for (int r = 0; r < 8 && kr < 0; r++)
        for (int c = 0; c < 8 && kr < 0; c++)
            if (b->squares[r][c] == king) { kr = r; kc = c; }
    if (kr < 0) return 1;

    int saved = b->turn;
    b->turn = color ^ 1;
    Move tmp[256];
    int cnt = generatePseudoMoves(b, tmp);
    b->turn = saved;

    for (int i = 0; i < cnt; i++)
        if (tmp[i].toRow == kr && tmp[i].toCol == kc) return 1;
    return 0;
}

static int squareAttacked(Board *b, int row, int col, int byColor) {
    int saved = b->turn;
    b->turn = byColor;
    Move tmp[256];
    int cnt = generatePseudoMoves(b, tmp);
    b->turn = saved;
    for (int i = 0; i < cnt; i++)
        if (tmp[i].toRow == row && tmp[i].toCol == col) return 1;
    return 0;
}

static int castlePathSafe(Board *b, int color, int kside) {
    int r = (color == WHITE) ? 7 : 0;
    int cols[3] = {4, kside ? 5 : 3, kside ? 6 : 2};
    for (int i = 0; i < 3; i++)
        if (squareAttacked(b, r, cols[i], color ^ 1)) return 0;
    return 1;
}

int generateLegalMoves(Board *b, Move *out) {
    Move pseudo[256];
    int pcount = generatePseudoMoves(b, pseudo);
    int cnt = 0;
    for (int i = 0; i < pcount; i++) {
        Move m = pseudo[i];
        if ((m.flags & FLAG_CASTLE_K) && !castlePathSafe(b, b->turn, 1)) continue;
        if ((m.flags & FLAG_CASTLE_Q) && !castlePathSafe(b, b->turn, 0)) continue;
        UndoInfo u = applyMove(b, m);
        if (!isInCheck(b, b->turn ^ 1)) out[cnt++] = m;
        undoMove(b, m, u);
    }
    return cnt;
}

int movesForSquare(Board *b, int row, int col, Move *out) {
    Move all[256];
    int total = generateLegalMoves(b, all);
    int cnt = 0;
    for (int i = 0; i < total; i++)
        if (all[i].fromRow == row && all[i].fromCol == col)
            out[cnt++] = all[i];
    return cnt;
}

int isCheckmate(Board *b) {
    Move moves[256];
    return isInCheck(b, b->turn) && generateLegalMoves(b, moves) == 0;
}

int isStalemate(Board *b) {
    Move moves[256];
    return !isInCheck(b, b->turn) && generateLegalMoves(b, moves) == 0;
}

int isDraw50(Board *b) {
    return b->halfMoveClock >= 100;
}
