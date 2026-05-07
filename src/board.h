#ifndef BOARD_H
#define BOARD_H

typedef enum {
    EMPTY = 0,
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
} Piece;

#define WHITE 0
#define BLACK 1

#define CASTLE_WK 1
#define CASTLE_WQ 2
#define CASTLE_BK 4
#define CASTLE_BQ 8

#define FLAG_NONE       0
#define FLAG_CASTLE_K   1
#define FLAG_CASTLE_Q   2
#define FLAG_EN_PASSANT 4
#define FLAG_PROMO_Q    8
#define FLAG_PROMO_R    16
#define FLAG_PROMO_B    32
#define FLAG_PROMO_N    64

typedef struct {
    Piece squares[8][8];
    int turn;
    int castleRights;
    int enPassantCol;
    int halfMoveClock;
    int fullMoveNumber;
} Board;

typedef struct {
    int fromRow, fromCol;
    int toRow, toCol;
    int flags;
} Move;

typedef struct {
    Piece capturedPiece;
    int castleRights;
    int enPassantCol;
    int halfMoveClock;
    Piece promotedFrom;
} UndoInfo;

void     initBoard(Board *b);
Piece    getPiece(Board *b, int row, int col);
void     setPiece(Board *b, int row, int col, Piece p);
int      isWhite(Piece p);
int      isBlack(Piece p);
int      isEmpty(Piece p);
int      colorOf(Piece p);
UndoInfo applyMove(Board *b, Move m);
void     undoMove(Board *b, Move m, UndoInfo u);

#endif
