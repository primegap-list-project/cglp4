/* conio3.c              Thomas R. Nicely          2016.04.01.0400
 *
 * Freeware copyright (C) 2016 Thomas R. Nicely <http://www.trnicely.net>.
 * Released into the public domain by the author, who disclaims any legal
 * liability arising from its use.
 *
 * These functions provide part of the functionality of the conio
 * functions of DJGPP and Borland/Inprise C to the other platforms
 * supported, in full or in part, by this library: Digital Mars,
 * MinGW, CygWin, and SUSE Linux 10.x. Note that the support is
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
 * The macro __CYGWIN__ is used to detect CygWin.
 * The macro __MINGW__ is used to detect MinGW.
 * The macro __BORLANDC__ is used to detect Borland/Inprise C.
 * The macro __MSVC__ is used to detect Microsoft Visual C++.
 * The macro __WIN32__ is used to detect the Win32 API, which
 *   includes CygWin, MinGW, Digital Mars in its default mode,
 *   Borland C in its default mode, and MSVC.
 */

#ifndef _CONIO3_H_
  #include "conio3.h"
#endif

/* Globals */

int __CSW=80;        /* Console screen width */
int __CSH=50;        /* Console screen height */
int __BACKGROUND = BLACK;
int __FOREGROUND = WHITE;
int __INIT_ATTR = WHITE_ON_BLACK;
int __WINLEFT = 0;  /* Global x coordinate of window UL corner */
int __WINTOP = 0;   /* Global y coordinate of window UL corner */

static struct __text_info tiGlobal =
  {
  1, 1,
  80, 50,
  WHITE_ON_BLACK,
  WHITE_ON_BLACK,
  C4350,
  80, 50,
  1,1
  };

#define __CALC_POS(i, j) (i * size.X + j)

/**********************************************************************/
void __clearline3(void)
{
/* Clear the entire current line and leave the cursor at the
   left margin. */

#if defined(__DJGPP__) || defined (__BORLANDC__)

cputs("\r");
clreol();

#elif defined(__DMC__)

cputs("\r");
disp_open();
disp_eeol();
disp_close();

#elif defined(__MINGW__)

/* MinGW console emulator has problems. */

fprintf(stderr, "\r                                        "
                "                                       \r");
fflush(stderr);

#elif defined(__CYGWIN__)

fprintf(stderr, "\r");
__clreol(); fflush(stderr);

#elif defined(__LINUX__)

char sz[256];

__gettextinfo(&tiGlobal);
memset(sz, ' ', __CSW-1);
sz[__CSW-1]=0;
fprintf(stderr, "\r%s\r", sz); fflush(stderr);

#endif

return;
}
/**********************************************************************/
void __clreol(void)
{
/* Erase from the current cursor position to the end of the line. Do
   not move the cursor. */

#ifdef __DMC__

disp_open();
disp_eeol();
disp_close();

#elif defined(__WIN32__)

COORD coord;
DWORD written;

__gettextinfo(&tiGlobal);
coord.X = __WINLEFT + tiGlobal.curx - 1;
coord.Y = __WINTOP  + tiGlobal.cury - 1;

FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE),
  ' ', tiGlobal.screenwidth - tiGlobal.curx + 1, coord, &written);
__gotoxy(tiGlobal.curx, tiGlobal.cury);
fflush(NULL);

#elif defined(__LINUX__)

system("tput el"); fflush(NULL);

#endif

return;
}
/**********************************************************************/
void __clrscr(void)
{
#ifdef __DMC__

disp_open();
disp_move(0, 0);
disp_eeop();
disp_move(0,0);
disp_close();

#elif defined(__WIN32__)

DWORD written;
int i;
COORD xy;

__gettextinfo(&tiGlobal);
xy.X=__WINLEFT;
xy.Y=__WINTOP;
FillConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
  __FOREGROUND + (__BACKGROUND << 4), __CSW*__CSH, xy, &written);
FillConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), ' ',
  __CSW*__CSH, xy, &written);
__gotoxy(1, 1);

#elif defined(__LINUX__)

system("clear");
fflush(NULL);

#endif

return;
}
/**********************************************************************/
void __cputsxy(int x, int y, char *sz)
{
__gotoxy(x, y);

#if defined(__DJGPP__) || defined (__BORLANDC__) || defined(__MINGW__)
  cputs(sz);
#elif defined(__DMC__)
  disp_open();
  disp_puts(sz);
  disp_close();
#elif defined(__CYGWIN__) || defined(__LINUX__)
  puts(sz); fflush(NULL);
#endif

return;
}
/**********************************************************************/
void __delline(void)
{
/* Delete current line, move those below up. */

#ifdef __WIN32__

COORD coord;
SMALL_RECT rect;
CHAR_INFO fillchar;

__gettextinfo(&tiGlobal);
coord.X = __WINLEFT;
coord.Y = __WINTOP + tiGlobal.cury - 1;
rect.Left = __WINLEFT;
rect.Top = __WINTOP + tiGlobal.cury;
rect.Right = __WINLEFT + tiGlobal.screenwidth - 1;
rect.Bottom = __WINTOP + tiGlobal.screenheight - 1;
fillchar.Attributes = __FOREGROUND + (__BACKGROUND << 4);
fillchar.Char.AsciiChar = ' ';
ScrollConsoleScreenBufferA(GetStdHandle(STD_OUTPUT_HANDLE),
  &rect, NULL, coord, &fillchar);
__gotoxy(tiGlobal.curx, tiGlobal.cury);

#elif defined(__LINUX__)

system("tput dl1");

#endif

fflush(NULL);
return;
}
/**********************************************************************/
char *__getpass(const char *szPrompt, char *szPW)
{
/* Read password from console. This function behaves like cgets.
   CygWin and Linux return null; CygWin lacks getch() while
   Linux lacks both getch() and the return of cursor coordinates
   x and y. */

#if !defined(__CYGWIN__) && !defined(__LINUX__)

int maxlength = szPW[0];
int length = 0;
int ch = 0;
int x, y;

cputs((char *)szPrompt);
__gettextinfo(&tiGlobal);
x = tiGlobal.curx;
y = tiGlobal.cury;

while(ch != '\r')
  {
  ch = getch();
  switch(ch)
    {
    case '\r' : break;  /* ENTER key */
    case '\b' :         /* backspace key */
      {
      if(length > 0)__putchxy(x + --length, y, ' ');
      __gotoxy(x + length, y);
      break;
      }
    default:
      {
      if(length < maxlength)
        {
        __putchxy(x + length, y, '*');
        szPW[2 + length++] = ch;
        }
      }
    }
  }
    
szPW[1] = length;
szPW[2 + length] = '\0';

#else

szPW[1]=szPW[2]=0;

#endif

return(&szPW[2]);
}
/**********************************************************************/
void __gettext(int left, int top, int right, int bottom,
  struct __char_info *buf)
{
/* Copies text from a screen rectangle to buf. The source rectangle
   has corners at (left, top) and (right, bottom). */

#ifdef __WIN32__

int i, j, n;
SMALL_RECT r;
CHAR_INFO *buffer;
COORD size, xy;

__gettextinfo(&tiGlobal);
r.Left = __WINLEFT + left - 1;
r.Top =  __WINTOP + top - 1;
r.Right = __WINLEFT + right - 1;
r.Bottom = __WINTOP + bottom - 1;
size.X = right - left + 1;
size.Y = bottom - top + 1;
buffer = (CHAR_INFO *)malloc(size.X * size.Y * sizeof(CHAR_INFO));

xy.X=0;
xy.Y=0;
ReadConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),
  (PCHAR_INFO) buffer, size, xy, &r);

for(i=n=0; i < size.Y; i++)
  {
  for(j=0; j < size.X; j++)
    {
    buf[n].letter = buffer[__CALC_POS(i, j)].Char.AsciiChar;
    buf[n].attr = buffer[__CALC_POS(i, j)].Attributes;
    n++;
    }
  }
free(buffer);

#elif defined(__LINUX__)  /* No implementation yet for Linux */

#endif

return;
}
/**********************************************************************/
void __gettextinfo(struct __text_info *pti)
{
/* Stores current console parameters in *pti. Assigns values
   to the global conio parameters _INIT_ATTR (the initial
   screen attribute, or "normal" attribute), __CSW (screen
   width), __CSH (screen height), and the global coordinates
   (__WINLEFT, __WINTOP) of the upper left corner of the
   console box (Win32 only). */

static int iFirst=1;

#if defined(__DMC__)

disp_open();

pti->winleft=0;
pti->wintop=0;
pti->winright=0;
pti->winbottom=0;
pti->attribute=disp_getattr();
if(iFirst)  /* "Normal" screen attribute stored on first call only */
  {
  pti->normattr=pti->attribute;
  __INIT_ATTR=pti->normattr;
  iFirst=0;
  }
pti->currmode=disp_getmode();
pti->screenheight=disp_numrows;
pti->screenwidth=disp_numcols;
__CSW=pti->screenwidth;
__CSH=pti->screenheight;
pti->curx=disp_cursorcol - 1;
pti->cury=disp_cursorrow - 1;

disp_close();

#elif defined(__WIN32__)

CONSOLE_SCREEN_BUFFER_INFO csbi;

GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
__WINLEFT = csbi.srWindow.Left;
__WINTOP = csbi.srWindow.Top;
pti->curx = csbi.dwCursorPosition.X - __WINLEFT + 1;
pti->cury = csbi.dwCursorPosition.Y - __WINTOP  + 1;
pti->attribute = csbi.wAttributes;
if(iFirst)
  {
  pti->normattr=pti->attribute;
  __INIT_ATTR=pti->normattr;
  iFirst=0;
  }
pti->screenwidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
pti->screenheight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
__CSW=pti->screenwidth;
__CSH=pti->screenheight;

#elif defined(__LINUX__)  /* only partial information returned */

struct winsize ws;

pti->winleft=0;
pti->wintop=0;
pti->winright=0;
pti->winbottom=0;
pti->attribute=WHITE_ON_BLACK;
if(iFirst)
  {
  pti->normattr=pti->attribute;
  __INIT_ATTR=pti->normattr;
  iFirst=0;
  }
pti->currmode=0;
pti->screenheight=50;
pti->screenwidth=80;
pti->curx=0;
pti->cury=0;
if(!ioctl(STDIN_FILENO, TIOCGWINSZ, &ws))
  {
  pti->screenwidth=ws.ws_col;
  pti->screenheight=ws.ws_row;
  }
__CSW=pti->screenwidth;
__CSH=pti->screenheight;

#endif

return;
}
/**********************************************************************/
void __gotoxy(int iCol, int iRow)
{
/* Home is (1,1) for this call, DJGPP, and Borland C, but home is (0,0)
   for Digital Mars, Win32, and Linux. */

#ifdef __WIN32__
  COORD c;
#elif defined(__LINUX__)
  char sz[18];
#endif

__gettextinfo(&tiGlobal);

if(iRow < 1)iRow=1;
if(iCol < 1)iCol=1;
if(iRow > __CSW)iRow=__CSW;
if(iCol > __CSH)iCol=__CSH;
iRow--;
iCol--;

#ifdef __DMC__

disp_open();
disp_move(iRow, iCol);  /* DMC expects row first */
disp_close();

#elif defined(__WIN32__)

c.X = __WINLEFT + iCol;
c.Y = __WINTOP  + iRow;
SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
fflush(NULL);

#elif defined(__LINUX__)

sprintf(sz, "tput cup %d %d", iRow, iCol);  /* tput expects row first */
system(sz);
fflush(NULL);

#endif

return;
}
/**********************************************************************/
void __highvideo(void)
{
#ifdef __DMC__
  int iAttr;
  disp_open();
  iAttr=disp_getattr();
  disp_setattr(iAttr | 8);
  disp_close();
#elif defined(__WIN32__)
  __gettextinfo(&tiGlobal);
  __textcolor(__FOREGROUND | 8);
#elif defined(__LINUX__)
  system("tput bold");
#endif

return;
}
/**********************************************************************/
void __insline(void)
{
/*
 * Insert blank line at the cursor position. Original content of the
 * line and content of lines below moves one line down. The last line
 * is deleted.
 */

#ifdef __WIN32__

COORD coord;
SMALL_RECT rect;
CHAR_INFO fillchar;

__gettextinfo(&tiGlobal);
coord.X = __WINLEFT;
coord.Y = __WINTOP + tiGlobal.cury;
rect.Left = __WINLEFT;
rect.Top = __WINTOP + tiGlobal.cury - 1;
rect.Right = __WINLEFT + tiGlobal.screenwidth - 1;
rect.Bottom = __WINTOP + tiGlobal.screenheight - 2;
fillchar.Attributes = __FOREGROUND + (__BACKGROUND << 4);
fillchar.Char.AsciiChar = ' ';
ScrollConsoleScreenBufferA(GetStdHandle(STD_OUTPUT_HANDLE),
  &rect, NULL, coord, &fillchar);
__gotoxy(tiGlobal.curx, tiGlobal.cury);

#elif defined(__LINUX__)

system("tput il1");

#endif

fflush(NULL);
return;
}
/**********************************************************************/
int __kbhit(void)
{
/* No implementation yet for CygWin or Linux. Return 1 (keystroke
   detected) to avoid infinite loops in while(!kbhit()) constructs. */

return(1);
}
/**********************************************************************/
void __lowvideo(void)
{
#ifdef __DMC__
  int iAttr;
  disp_open();
  iAttr=disp_getattr();
  disp_setattr(iAttr ^ 8);
  disp_close();
#elif defined(__WIN32__)
  __gettextinfo(&tiGlobal);
  __textcolor(__FOREGROUND ^ 8);
#elif defined(__LINUX__)
  system("tput dim");
#endif

return;
}
/**********************************************************************/
void __movetext(int left, int top, int right, int bottom,
  int destleft, int desttop)
{
/* Copies a rectangle of text extending from (left, top) to
   (right, bottom) to a rectangle whose upper left corner is at
   (destleft, desttop). The source rectangle is not erased. */

#ifdef __WIN32__

struct __char_info *buffer;

buffer = (struct __char_info *)malloc(
  (right - left + 1) * (bottom - top + 1) * sizeof(struct __char_info));
__gettext(left, top, right, bottom, buffer);
__puttext(destleft, desttop, destleft + right - left,
    desttop + bottom - top, buffer);
free(buffer);
fflush(NULL);

#endif

return;
}
/**********************************************************************/
void __nocursor(void)
{
#if defined(__DJGPP__) || defined(__BORLANDC__)
  _setcursortype(_NOCURSOR);
#elif defined(__DMC__)
  disp_open();
  disp_hidecursor();
  disp_close();
#elif defined(__WIN32__)
  __setcursortype(0);
#elif defined(__LINUX__)
  system("tput civis");
#endif

return;
}
/**********************************************************************/
void __normalcursor(void)
{
#if defined(__DJGPP__) || defined(__BORLANDC__)
  _setcursortype(_NORMALCURSOR);
#elif defined(__DMC__)
  disp_open();
  disp_showcursor();
  disp_close();
#elif defined(__WIN32__)
  __setcursortype(0);
#elif defined(__LINUX__)
  system("tput cnorm");
#endif

return;
}
/**********************************************************************/
void __normvideo(void)
{
#ifdef __DMC__
  disp_open();
  disp_setattr(__INIT_ATTR);
  disp_close();
#elif defined(__WIN32__)
  __textattr(__INIT_ATTR);
#elif defined(__LINUX__)
  system("tput sgr0");
#endif

return;
}
/**********************************************************************/
void __putchxy(int x, int y, char ch)
{
__gotoxy(x, y);
#if defined(__DJGPP__) || defined (__BORLANDC__) || defined(__MINGW__)
  putch(ch);
#elif defined(__DMC__)
  disp_open();
  disp_putc(ch);
  disp_close();
#elif defined(__CYGWIN__) || defined(__LINUX__)
  putchar(ch); fflush(NULL);
#endif

return;
}
/**********************************************************************/
void __puttext(int left, int top, int right, int bottom,
  struct __char_info *buf)
{
/* Copies text from buf to a screen rectangle. The target rectangle
   has corners at (left, top) and (right, bottom). */

#ifdef __WIN32__

int i, j, n;
SMALL_RECT r;
CHAR_INFO* buffer;
COORD size, xy;

__gettextinfo(&tiGlobal);
r.Left = __WINLEFT + left - 1;
r.Top = __WINTOP + top - 1;
r.Right = __WINLEFT + right - 1;
r.Bottom = __WINTOP + bottom - 1;
size.X = right - left + 1;
size.Y = bottom - top + 1;
buffer = (CHAR_INFO *)malloc(size.X * size.Y * sizeof(CHAR_INFO));
for(i=n=0; i < size.Y; i++)
  {
  for(j=0; j < size.X; j++)
    {
    buffer[__CALC_POS(i, j)].Char.AsciiChar = buf[n].letter;
    buffer[__CALC_POS(i, j)].Attributes = buf[n].attr;
    n++;
    }
  }

xy.X=0;
xy.Y=0;
WriteConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE),
  buffer, size, xy, &r);
free(buffer);
fflush(NULL);

#elif defined(__LINUX__)  /* No implementation yet for Linux */

#endif

return;
}
/**********************************************************************/
void __setcursortype(unsigned long dwSize)
{
/* DJGPP and Borland already have _setcursortype. Linux uses an
   incompatible method. This algorithm is thus for Digital Mars
   and Win32. */

#ifdef __DMC__

disp_open();
disp_setcursortype(dwSize);
disp_close();

#elif defined(__WIN32__)

CONSOLE_CURSOR_INFO cci;

if(dwSize==0)
  {
  cci.bVisible = FALSE;
  }
else
  {
  cci.dwSize = dwSize;
  cci.bVisible = TRUE;
  }
SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);

#endif

return;
}
/**********************************************************************/
void __textattr(int iAttr)
{
#ifdef __DMC__

disp_open();
disp_setattr(iAttr);
disp_close();

#elif defined(__WIN32__)

WORD wAttr;

wAttr=(WORD)(iAttr & 0x0000007F);  /* discard all bits except 0-6 */
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wAttr);

#endif

__gettextinfo(&tiGlobal);
__FOREGROUND = tiGlobal.attribute & 0x0000000F;
__BACKGROUND = (tiGlobal.attribute & 0x00000070) >> 4;

/* No implementation yet for Linux. */

return;
}
/**********************************************************************/
void __textbackground(int iNewColor)
{
int iOldAttr, iNewAttr;

#ifdef __DMC__

disp_open();
iOldAttr=disp_getattr();
iNewAttr=(iOldAttr & 0x0000000F) | (iNewColor << 4);
disp_setattr(iNewAttr);
disp_close();

#elif defined(__WIN32__)

__gettextinfo(&tiGlobal);
__BACKGROUND = iNewColor;
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
  (WORD)(__FOREGROUND | (iNewColor << 4)));

#endif

__gettextinfo(&tiGlobal);
__FOREGROUND = tiGlobal.attribute & 0x0000000F;
__BACKGROUND = (tiGlobal.attribute & 0x00000070) >> 4;

/* No implementation yet for Linux. */

return;
}
/**********************************************************************/
void __textcolor(int iNewColor)
{
/* No implementation yet for Linux. */

int iOldAttr, iNewAttr;

#ifdef __DMC__

disp_open();
iOldAttr=disp_getattr();
iNewAttr=(iOldAttr & 0xFFFFFF00) | iNewColor;
disp_setattr(iNewAttr);
disp_close();

#elif defined(__WIN32__)

__gettextinfo(&tiGlobal);
__FOREGROUND = iNewColor;
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
  (WORD)(iNewColor | (__BACKGROUND << 4)));

#endif

__gettextinfo(&tiGlobal);
__FOREGROUND = tiGlobal.attribute & 0x0000000F;
__BACKGROUND = (tiGlobal.attribute & 0x00000070) >> 4;

/* No implementation yet for Linux. */

return;
}
/**********************************************************************/
int __wherex(void)
{
#ifndef __LINUX__
  __gettextinfo(&tiGlobal);
  return(tiGlobal.curx);
#else
  return(-1);  /* No implementation yet for Linux */
#endif
}
/**********************************************************************/
int __wherey(void)
{
#ifndef __LINUX__
  __gettextinfo(&tiGlobal);
  return(tiGlobal.cury);
#else
  return(-1);  /* No implementation yet for Linux */
#endif
}
/**********************************************************************/
/************************** END CONIO3.C ******************************/
/**********************************************************************/
