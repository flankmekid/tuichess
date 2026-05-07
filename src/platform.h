#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <ncursesw/ncurses.h>
#elif defined(_WIN32)
#include <pdcurses/curses.h>
#else
#include <ncursesw/ncurses.h>
#endif

#endif
