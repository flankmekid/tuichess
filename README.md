# tuichess

a terminal chess engine written in c using ncurses.

## features

- human vs human and human vs ai game modes
- play as white or black
- unicode chess pieces rendered via ncursesw
- minimax engine with alpha-beta pruning and piece-square table evaluation
- four difficulty presets (easy, normal, hard, very hard) plus custom elo input (400-2800)
- legal move highlighting, undo, pawn promotion overlay, 50-move draw detection
- vim-style cursor movement (hjkl) and arrow keys

## controls

- arrows / hjkl: move cursor
- enter: select / move piece
- u: undo
- q: return to menu

## building

**linux (requires ncursesw):**
```
sudo apt install libncursesw5-dev
make
```

**windows (msys2 mingw64):**
```
pacman -S mingw-w64-x86_64-ncurses
make
```

## running

```
./tuichess
```