#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <ncursesw/ncurses.h>
#elif defined(_WIN32)
#include <pdcurses/curses.h>
#elif __has_include(<ncursesw/ncurses.h>)
#include <ncursesw/ncurses.h>
#else
#include <ncurses.h>
#endif

#endif
