/* conio3.h              Thomas R. Nicely          2016.04.01.0400
 *
 * Freeware copyright (C) 2016 Thomas R. Nicely <http://www.trnicely.net>.
 * Released into the public domain by the author, who disclaims any legal
 * liability arising from its use.
 *
 * This is the header for the conio simulation functions for which
 * the C source code appears in the accompanying file conio3.h.
 *
 * These functions provide part of the functionality of the conio
 * functions of DJGPP and Borland/Inprise C to the other platforms
 * supported, in full or in part, by this library: Digital Mars,
 * MinGW, Cygwin, and SUSE Linux 10.x. Note that the support is
 * neither complete nor flawless. Note also that conio3 does *not*
 * provide support for Unicode/wchar_t. Development is ongoing;
 * the package is presently at an "early beta" stage. Any MSVC and
 * Cygwin support is at alpha level.
 *
 * MinGW code is targeted to run in an ordinary Windows DOS box,
 * not within the MSYS environment, where it exhibits different
 * behavior.
 *
 * Portions of this code, especially the Win32 sections, are adapted
 * from the package devpak CONIO 2.0 (CONIO2), written and released
 * to the public domain by Hongli Lai, tkorrovi, Andrew Westcott,
 * and Michal Molhanec, and targeted at the Win32 MinGW/Dev-C++
 * platform. The original CONIO 2.0 is available at
 * <http://sourceforge.net/project/showfiles.php?group_id=115967>;
 * thanks to David Hoke for this pointer, and for his own
 * adaptation of CONIO 2.0.
 *
 * The macro __DJGPP__ is used to detect DJGPP (v203 and v204).
 * The macro __DMC__ is used to detect 32-bit Digital Mars.
 * The macro __LINUX__ is used to detect SUSE GNU/Linux 10.0.
 * The macro __CYGWIN__ is used to detect Cygwin.
 * The macro __MINGW__ is used to detect MinGW.
 * The macro __BORLANDC__ is used to detect Borland/Inprise C.
 * The macro __MSVC__ is used to detect Microsoft Visual C++.
 * The macro __WIN32__ is used to detect the Win32 API, which
 *   includes Cygwin, MinGW, Digital Mars in its default mode,
 *   Borland C in its default mode, and MSVC.
 */

#ifndef _CONIO3_H_

#define _CONIO3_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _TRN_H_  /* avoid duplication */

/* Crudely commented out to enable trial compilation on Linux.
#if defined(__linux__) && !defined(__i386__)
  #error ...Intel/AMD 386 compatible processor (with FPU) required...
#endif
*/

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE 1
#endif
#ifndef __USE_GNU
  #define __USE_GNU 1
#endif
#ifndef _ISOC99_SOURCE
  #define _ISOC99_SOURCE 1
#endif

#undef __DJ203__
#undef __DJ204__
#if defined(__DJGPP__) && __DJGPP__==2 && defined(__DJGPP_MINOR__)
  #if __DJGPP_MINOR__ == 3
    #define __DJ203__ 203
  #endif
#else
  #if __DJGPP_MINOR__ == 4
    #define __DJ204__ 204
  #endif
#endif

#undef __LINUX__
#ifdef __linux__
  #define __LINUX__ 1
#endif

#undef __MINGW__
#ifdef __MINGW32__
  #define __MINGW__ 1
#endif

#undef __MSVC__
#ifdef _MSC_VER
  #define __MSVC__ 1
#endif

#if !defined(__LINUX__) && !defined(__CYGWIN__)
  #include <conio.h>
#endif
#ifdef __DMC__
  #include <disp.h>
#endif
#ifdef __LINUX__
  #include <unistd.h>     /* for STDIN_FILENO */
  #include <sys/ioctl.h>
#endif

#if defined(__WIN32__) || defined(_WIN32) || defined(__CYGWIN__) \
    || defined(__MINGW__) || defined(__MSVC__)
  #undef __WIN32__
  #define __WIN32__ 1
  #include <windows.h>
  #include <winbase.h>
#endif

#endif  /* not _TRN_H_ */

/**********************************************************************/

/* The following declaration attempts to reconcile C++ compilers with
   standard C code, specifically function prototypes.  No
   "#include <system_header.h>" directives should fall within the scope
   of this declaration. It is terminated by a right bracket near the
   end of the file. */

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************/

/* Structure containing console parameters. Not all the fields of
   __text_info are valid at this time. */

struct __text_info {
    unsigned char winleft;        /* left window coordinate */
    unsigned char wintop;         /* top window coordinate */
    unsigned char winright;       /* right window coordinate */
    unsigned char winbottom;      /* bottom window coordinate */
    unsigned char attribute;      /* text attribute */
    unsigned char normattr;       /* text attribute at progr invoc */
    unsigned char currmode;       /* BW40, BW80, C40, C80, C4350 */
    unsigned char screenheight;   /* lines per screen */
    unsigned char screenwidth;    /* characters per line */
    unsigned char curx;           /* cursor x-coord in current window */
    unsigned char cury;           /* cursor y-coord in current window */
};

/* For DJGPP/Borland compatibility */

#if !defined(__DJGPP__) && !defined(__BORLANDC__)

enum text_modes {LASTMODE=-1, BW40=0, C40, BW80, C80, MONO=7, C4350=64};

enum COLORS  /* 0 through 15, as in original IBM PC CGA */
  {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  MAGENTA,
  BROWN,
  LIGHTGRAY,
  DARKGRAY,
  LIGHTBLUE,
  LIGHTGREEN,
  LIGHTCYAN,
  LIGHTRED,
  LIGHTMAGENTA,
  YELLOW,
  WHITE
  };

#endif  /* not DJGPP and not BORLANDC */

#define WHITE_ON_BLACK WHITE+(BLACK<<4)

/* Obsolete identifiers. */

#define __iSW __CSW
#define __iSH __CSH

/**********************************************************************/

/* The following structure is used by __gettext and __puttext. It is
   a non-Unicode version of the Windows CHAR_INFO structure. */

struct __char_info
  {
  char letter;           /* character value */
  unsigned short attr;   /* attribute value */
  };

#if !defined(__DJGPP__) && !defined(__BORLANDC__)

/* Predefined cursor types. */

#define _NOCURSOR 0
#define _SOLIDCURSOR 100
#define _NORMALCURSOR 20  /* cursor filling 20 percent of cell height */

#endif  /* not DJGPP and not BORLANDC */

/**********************************************************************/

/* Internal conio functions, available to all environments. Some of
   these functions are not currently implemented, and are only
   included for potential future development; see conio3.c. */

void __clearline3(void);
void __clreol(void);
void __clrscr(void);
void __cputsxy(int x, int y, char *sz);
void __delline(void);
char *__getpass(const char *szPrompt, char *szPW);
void __gettext(int left, int top, int right, int bottom,
       struct __char_info *buf);
void __gettextinfo(struct __text_info *pti);
void __gotoxy(int iCol, int iRow);
void __highvideo(void);
void __insline(void);
int  __kbhit(void);
void __lowvideo(void);
void __movetext(int left, int top, int right, int bottom,
       int destleft, int desttop);
void __nocursor(void);
void __normalcursor(void);
void __normvideo(void);
void __putchxy(int x, int y, char ch);
void __puttext(int left, int top, int right, int bottom,
       struct __char_info *buf);
void __setcursortype(unsigned long dwSize);
void __textattr(int iAttr);
void __textbackground(int iNewColor);
void __textcolor(int iNewColor);
int  __wherex(void);
int  __wherey(void);

/* Equate internal function names to standard DJGPP/Borland conio names
   for other environments. */

#ifndef _TRN_H_  /* Avoid duplication */

#undef __OBSL__  /* Output blank screen line (usually at program exit) */
#if defined(__LINUX__)
  #define __OBSL__ {printf("\n"); fprintf(stderr, "\n");}
#else
  #define __OBSL__ {printf("\n");}
#endif

#if defined(__DJGPP__) || defined(__BORLANDC__)
  #define __clearline()  {cputs("\r"); clreol();}
#elif defined(__MINGW__)
  #define __clearline() \
  cputs("\r                                                                               \r")
#elif defined(__DMC__)
  #define __clearline()  \
    {cputs("\r"); disp_open(); disp_eeol(); disp_close(); fflush(stderr);}
#else
#define __clearline()  \
  { \
  char *pch; int iColumns, i; \
  pch=getenv("COLUMNS"); \
  if(pch)iColumns=atof(pch); else iColumns=80; \
  fflush(stderr); \
  putc('\r', stderr); \
  for(i=1; i < iColumns; i++)putc(' ', stderr); \
  putc('\r', stderr); \
  fflush(stderr); \
  }
#endif  /* __clearline */

#define __CLEAR_LINE__ __clearline()

/* clrscr in Cygwin must be kludged; use the conio3 version instead */

#if defined(__MINGW__) || defined(__LINUX__)
  #undef  clrscr
  #define clrscr() {system("clear");}
#elif defined(__DMC__)
  #undef  clrscr
  #define clrscr() {system("cls");}
#elif !defined(__DJGPP__) && !defined(__BORLANDC__)
  #undef  clrscr
  #define clrscr                 __clrscr
#endif  /* clrscr */

#ifdef __MINGW__

/*
 * These defines enable the use all MinGW conio.h functions without
 * the underscore. However, _getch, _getche, and _kbhit are unreliable.
 */

#undef cgets
#undef cprintf
#undef cputs
#undef cscanf
#undef putch
#undef ungetch
#undef getch
#undef getche
#undef kbhit

#define cgets   _cgets
#define cprintf _cprintf
#define cputs   _cputs
#define cscanf  _cscanf
#define putch   _putch
#define ungetch _ungetch
#define getch   getchar
#define getche  getchar
#define kbhit   getchar

#endif  /* MINGW */

#if defined(__CYGWIN__) || defined(__LINUX__)

/* No valid implementation of these functions is currently available
   for Cygwin or Linux. The cputs function is provided by custom
   code in trn.c. */

int __cputs(const char *sz);
#undef  cputs
#define cputs    __cputs
#define getch    getchar
#define putch    putchar
#define cprintf  printf
#define cscanf   scanf
#define kbhit()  getchar

#endif  /* CYGWIN or LINUX */

#if defined(__BORLANDC__) && defined(__WIN32__)
    /* special version of cputs for speed */

int __cputs(const char *sz);
#undef  cputs
#define cputs __cputs

#endif  /* BORLANDC with WIN32 */

#endif  /* not _TRN_H_ */

#if !defined(__DJGPP__) && !defined(__BORLANDC__)

#define clreol                 __clreol
#define delline                __delline
#if !defined(__LINUX__) && !defined(__CYGWIN__)
  #define getpass              __getpass
#endif
#define gettext                __gettext
#define gettextinfo            __gettextinfo
#define gotoxy                 __gotoxy
#define highvideo              __highvideo
#define insline                __insline
#define lowvideo               __lowvideo
#define movetext               __movetext
#define normvideo              __normvideo
#define puttext                __puttext
#define ScreenClear            __clrscr
#define setcursortype          __setcursortype
#define textattr               __textattr
#define textbackground         __textbackground
#define textcolor              __textcolor
#define text_info              __text_info
#define wherex                 __wherex
#define wherey                 __wherey

#endif  /* not DJGPP and not BORLANDC */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

/**********************************************************************/
/**************************** END CONIO3.H ****************************/
/**********************************************************************/
/**********************************************************************/

#endif  /* not _CONIO3_H_ */
