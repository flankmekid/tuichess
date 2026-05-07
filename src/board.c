#include "board.h"
#include <string.h>

void initBoard(Board *b) {
    memset(b, 0, sizeof(*b));

    b->squares[0][0] = B_ROOK;   b->squares[0][1] = B_KNIGHT;
    b->squares[0][2] = B_BISHOP; b->squares[0][3] = B_QUEEN;
    b->squares[0][4] = B_KING;   b->squares[0][5] = B_BISHOP;
    b->squares[0][6] = B_KNIGHT; b->squares[0][7] = B_ROOK;
    for (int c = 0; c < 8; c++) b->squares[1][c] = B_PAWN;

    b->squares[7][0] = W_ROOK;   b->squares[7][1] = W_KNIGHT;
    b->squares[7][2] = W_BISHOP; b->squares[7][3] = W_QUEEN;
    b->squares[7][4] = W_KING;   b->squares[7][5] = W_BISHOP;
    b->squares[7][6] = W_KNIGHT; b->squares[7][7] = W_ROOK;
    for (int c = 0; c < 8; c++) b->squares[6][c] = W_PAWN;

    b->turn           = WHITE;
    b->castleRights   = CASTLE_WK | CASTLE_WQ | CASTLE_BK | CASTLE_BQ;
    b->enPassantCol   = -1;
    b->halfMoveClock  = 0;
    b->fullMoveNumber = 1;
}

Piece getPiece(Board *b, int row, int col) {
    return b->squares[row][col];
}

void setPiece(Board *b, int row, int col, Piece p) {
    b->squares[row][col] = p;
}

int isWhite(Piece p) { return p >= W_PAWN && p <= W_KING; }
int isBlack(Piece p) { return p >= B_PAWN && p <= B_KING; }
int isEmpty(Piece p) { return p == EMPTY; }
int colorOf(Piece p) { return isBlack(p) ? BLACK : WHITE; }

UndoInfo applyMove(Board *b, Move m) {
    UndoInfo u;
    u.capturedPiece = b->squares[m.toRow][m.toCol];
    u.castleRights  = b->castleRights;
    u.enPassantCol  = b->enPassantCol;
    u.halfMoveClock = b->halfMoveClock;
    u.promotedFrom  = EMPTY;

    Piece moving = b->squares[m.fromRow][m.fromCol];
    b->enPassantCol = -1;

    if (m.flags & FLAG_CASTLE_K) {
        int r = m.fromRow;
        b->squares[r][6] = moving;
        b->squares[r][4] = EMPTY;
        b->squares[r][5] = (b->turn == WHITE) ? W_ROOK : B_ROOK;
        b->squares[r][7] = EMPTY;
    } else if (m.flags & FLAG_CASTLE_Q) {
        int r = m.fromRow;
        b->squares[r][2] = moving;
        b->squares[r][4] = EMPTY;
        b->squares[r][3] = (b->turn == WHITE) ? W_ROOK : B_ROOK;
        b->squares[r][0] = EMPTY;
    } else if (m.flags & FLAG_EN_PASSANT) {
        b->squares[m.toRow][m.toCol]     = moving;
        b->squares[m.fromRow][m.toCol]   = EMPTY;
        b->squares[m.fromRow][m.fromCol] = EMPTY;
        u.capturedPiece = (b->turn == WHITE) ? B_PAWN : W_PAWN;
    } else if (m.flags & (FLAG_PROMO_Q | FLAG_PROMO_R | FLAG_PROMO_B | FLAG_PROMO_N)) {
        u.promotedFrom = moving;
        Piece promo;
        if      (m.flags & FLAG_PROMO_Q) promo = (b->turn == WHITE) ? W_QUEEN  : B_QUEEN;
        else if (m.flags & FLAG_PROMO_R) promo = (b->turn == WHITE) ? W_ROOK   : B_ROOK;
        else if (m.flags & FLAG_PROMO_B) promo = (b->turn == WHITE) ? W_BISHOP : B_BISHOP;
        else                             promo = (b->turn == WHITE) ? W_KNIGHT : B_KNIGHT;
        b->squares[m.toRow][m.toCol]     = promo;
        b->squares[m.fromRow][m.fromCol] = EMPTY;
    } else {
        b->squares[m.toRow][m.toCol]     = moving;
        b->squares[m.fromRow][m.fromCol] = EMPTY;
    }

    if (moving == W_PAWN && m.fromRow - m.toRow == 2)
        b->enPassantCol = m.fromCol;
    else if (moving == B_PAWN && m.toRow - m.fromRow == 2)
        b->enPassantCol = m.fromCol;

    if (moving == W_KING) b->castleRights &= ~(CASTLE_WK | CASTLE_WQ);
    if (moving == B_KING) b->castleRights &= ~(CASTLE_BK | CASTLE_BQ);
    if (moving == W_ROOK && m.fromRow == 7 && m.fromCol == 7) b->castleRights &= ~CASTLE_WK;
    if (moving == W_ROOK && m.fromRow == 7 && m.fromCol == 0) b->castleRights &= ~CASTLE_WQ;
    if (moving == B_ROOK && m.fromRow == 0 && m.fromCol == 7) b->castleRights &= ~CASTLE_BK;
    if (moving == B_ROOK && m.fromRow == 0 && m.fromCol == 0) b->castleRights &= ~CASTLE_BQ;

    if (!isEmpty(u.capturedPiece) || moving == W_PAWN || moving == B_PAWN)
        b->halfMoveClock = 0;
    else
        b->halfMoveClock++;

    if (b->turn == BLACK) b->fullMoveNumber++;
    b->turn ^= 1;

    return u;
}

void undoMove(Board *b, Move m, UndoInfo u) {
    b->turn ^= 1;
    b->castleRights  = u.castleRights;
    b->enPassantCol  = u.enPassantCol;
    b->halfMoveClock = u.halfMoveClock;
    if (b->turn == BLACK) b->fullMoveNumber--;

    if (m.flags & FLAG_CASTLE_K) {
        int r = m.fromRow;
        Piece king = b->squares[r][6];
        Piece rook = b->squares[r][5];
        b->squares[r][4] = king;
        b->squares[r][7] = rook;
        b->squares[r][6] = EMPTY;
        b->squares[r][5] = EMPTY;
    } else if (m.flags & FLAG_CASTLE_Q) {
        int r = m.fromRow;
        Piece king = b->squares[r][2];
        Piece rook = b->squares[r][3];
        b->squares[r][4] = king;
        b->squares[r][0] = rook;
        b->squares[r][2] = EMPTY;
        b->squares[r][3] = EMPTY;
    } else if (m.flags & FLAG_EN_PASSANT) {
        Piece pawn = b->squares[m.toRow][m.toCol];
        b->squares[m.fromRow][m.fromCol] = pawn;
        b->squares[m.toRow][m.toCol]     = EMPTY;
        b->squares[m.fromRow][m.toCol]   = u.capturedPiece;
    } else if (u.promotedFrom != EMPTY) {
        b->squares[m.fromRow][m.fromCol] = u.promotedFrom;
        b->squares[m.toRow][m.toCol]     = u.capturedPiece;
    } else {
        Piece moved = b->squares[m.toRow][m.toCol];
        b->squares[m.fromRow][m.fromCol] = moved;
        b->squares[m.toRow][m.toCol]     = u.capturedPiece;
    }
}
