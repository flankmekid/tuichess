#include "engine.h"
#include "moves.h"
#include <stdlib.h>
#include <time.h>
#include <limits.h>

static const int MATERIAL[13] = {
    0, 100, 320, 330, 500, 900, 20000,
       100, 320, 330, 500, 900, 20000
};

static const int PST_PAWN[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

static const int PST_KNIGHT[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int PST_BISHOP[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

static const int PST_ROOK[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

static const int PST_QUEEN[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

static const int PST_KING[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

static int pstLookup(Piece p, int row, int col) {
    int r = isWhite(p) ? row : (7 - row);
    switch (p) {
        case W_PAWN:   case B_PAWN:   return PST_PAWN[r][col];
        case W_KNIGHT: case B_KNIGHT: return PST_KNIGHT[r][col];
        case W_BISHOP: case B_BISHOP: return PST_BISHOP[r][col];
        case W_ROOK:   case B_ROOK:   return PST_ROOK[r][col];
        case W_QUEEN:  case B_QUEEN:  return PST_QUEEN[r][col];
        case W_KING:   case B_KING:   return PST_KING[r][col];
        default: return 0;
    }
}

static int evaluate(Board *b) {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Piece p = b->squares[r][c];
            if (isEmpty(p)) continue;
            int val = MATERIAL[p] + pstLookup(p, r, c);
            score += isWhite(p) ? val : -val;
        }
    }
    return (b->turn == WHITE) ? score : -score;
}

static void orderMoves(Board *b, Move *moves, int cnt) {
    int head = 0;
    for (int i = 0; i < cnt; i++) {
        if (!isEmpty(b->squares[moves[i].toRow][moves[i].toCol])) {
            Move tmp = moves[head]; moves[head] = moves[i]; moves[i] = tmp;
            head++;
        }
    }
}

static int minimax(Board *b, int depth, int alpha, int beta) {
    if (depth == 0) return evaluate(b);

    Move moves[256];
    int cnt = generateLegalMoves(b, moves);

    if (cnt == 0) {
        if (isInCheck(b, b->turn)) return -20000 - depth;
        return 0;
    }

    orderMoves(b, moves, cnt);

    for (int i = 0; i < cnt; i++) {
        UndoInfo u = applyMove(b, moves[i]);
        int score = -minimax(b, depth - 1, -beta, -alpha);
        undoMove(b, moves[i], u);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

static Move pickFromTopN(Move *moves, int *scores, int cnt, int n) {
    if (n > cnt) n = cnt;
    for (int i = 0; i < n; i++) {
        int best = i;
        for (int j = i + 1; j < cnt; j++)
            if (scores[j] > scores[best]) best = j;
        int ts = scores[i]; scores[i] = scores[best]; scores[best] = ts;
        Move tm = moves[i];  moves[i]  = moves[best];  moves[best]  = tm;
    }
    return moves[rand() % n];
}

void eloToDepthAndN(int elo, int *depth, int *n) {
    if (elo <= 400)  { *depth = 2; *n = 5; return; }
    if (elo >= 2800) { *depth = 5; *n = 1; return; }
    float t = (float)(elo - 400) / 2400.0f;
    *depth  = (int)(2.0f + t * 3.0f + 0.5f);
    *n      = (int)(5.0f - t * 4.0f + 0.5f);
    if (*depth > 5) *depth = 5;
    if (*n < 1)     *n = 1;
}

Move getBestMove(Board *b, int depth, int topN) {
    static int seeded = 0;
    if (!seeded) { srand((unsigned)time(NULL)); seeded = 1; }

    Move moves[256];
    int cnt = generateLegalMoves(b, moves);
    if (cnt == 0) { Move m = {0}; return m; }

    orderMoves(b, moves, cnt);

    int scores[256];
    for (int i = 0; i < cnt; i++) {
        UndoInfo u = applyMove(b, moves[i]);
        scores[i] = -minimax(b, depth - 1, -INT_MAX / 2, INT_MAX / 2);
        undoMove(b, moves[i], u);
    }
    return pickFromTopN(moves, scores, cnt, topN);
}
