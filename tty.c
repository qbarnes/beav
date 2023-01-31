/*
*    Wang PC terminal display        TTY.C
*
*/
#include        "def.h"

void ttinit ();
void tttidy ();
void ttmove ();
void tteeol ();
void tteeop ();
void ttbeep ();
void asciiparm ();
void ttnowindow ();		/* stub */
void ttcolor ();
extern void tcapopen ();
extern void tcapclose ();	/* DR */
extern void tcapmove ();
extern void tcapeeop ();
extern void tcapeeol ();
extern void tcaprev ();

#ifdef MSDOS
#include    "dos.h"
extern bool ibm_pc, mem_map;
#endif
#define BEL     0x07		/* BEL character.               */
#define ESC     0x1B		/* ESC character.               */

extern int ttrow;
extern int ttcol;
extern int tttop;
extern int ttbot;
extern int tthue;

int tceeol = 3;			/* Costs.                       */

/*
* Initialize the terminal when the editor
* gets started up.
*/
void
ttinit ()
{
#ifdef MSDOS
    ttraw ();
#endif
#ifdef OS2
    ttraw ();
#endif
#ifdef UNIX
    tcapopen ();
    tcapmove (0, 0);
    tcapeeop ();
#endif
}

/*
* Clean up the terminal, in anticipation of
* a return to the command interpreter.
*/
void
tttidy ()
{
#ifdef MSDOS
    ttcooked ();
#endif
#ifdef OS2
    ttcooked ();
#endif
#ifdef UNIX
    tcapclose ();
#endif
}

/*
* Move the cursor to the specified
* origin 0 row and column position. Try to
* optimize out extra moves; redisplay may
* have left the cursor in the right
* location last time!
*/
void
ttmove (row, col)
    int row, col;
{
#ifdef MSDOS
    union REGS regs;

    /* Move in both axes */
    if (ibm_pc)
    {
	regs.h.ah = 2;
	regs.h.dh = (char) row;
	regs.h.dl = (char) col;
	regs.h.bh = 0;
	int86 (0x10, &regs, &regs);	/* set cursor position */
    }
    else
#endif
#ifdef UNIX
	tcapmove (row, col);
#endif
#ifdef ANSI
    {
	ttputc (ESC);
	ttputc ('[');
	asciiparm (row + 1);
	ttputc (';');
	asciiparm (col + 1);
	ttputc ('H');
    }
#endif
    ttrow = row;
    ttcol = col;
}

/*
* Erase to end of line.
*/
void
tteeol ()
{
#ifdef MSDOS
    char col, row, i;
    union REGS regs;

    if (ibm_pc)
    {
	regs.h.ah = 3;
	regs.h.bh = 0;
	int86 (0x10, &regs, &regs);	/* get cursor position */
	col = regs.h.dl;
	row = regs.h.dh;
	for (i = col; i < (NCOL - 1); i++)
	{
	    regs.h.ah = 0x0e;
	    regs.h.bl = 0;
	    regs.h.bh = 0;
	    regs.h.al = ' ';
	    int86 (0x10, &regs, &regs);	/* set cursor position */
	}
	/* put cursor back to original position */
	regs.h.ah = 2;
	regs.h.bh = 0;
	regs.h.dl = col;
	regs.h.dh = row;
	int86 (0x10, &regs, &regs);	/* get cursor position */
    }
    else
#endif
#ifdef ANSI
    {
	ttputc (ESC);
	ttputc ('[');
#ifdef MSDOS
	if (ibm_pc)
	    ttputc ('0');	/* this is necessary in IBM PC's */
#endif
	ttputc ('K');
    }
#endif
#ifdef UNIX
    tcapeeol ();
#endif
}

/*
* Erase to end of page.
* only ever used when cursor is at 0,0, so IBM screen erase
* is same as eop
*/
void
tteeop ()
{
#ifdef MSDOS
    union REGS regs;
    char i, j;

    if (ibm_pc)
    {
	regs.h.ah = 6;
	regs.h.al = 0;
	regs.x.cx = 0;
	regs.x.dx = (nrow << 8) | (NCOL - 1);
	int86 (0x10, &regs, &regs);	/* scroll to clear screen */
    }
    else
#endif
#ifdef	ANSI
    {
	ttcolor (CTEXT);
	ttputc (ESC);
	ttputc ('[');
#ifdef MSDOS
	if (ibm_pc)
	    ttputc ('0');
	else
#endif
	    ttputc ('2');
	ttputc ('J');
    }
#endif
#ifdef UNIX
    tcapeeop ();
#endif
}

/*
* Make a noise.
*/
void
ttbeep ()
{
    ttputc (BEL);
    ttflush ();
}

/*
* Convert a number to decimal
* ascii, and write it out. Used to
* deal with numeric arguments.
*/
void
asciiparm (n)
    register int n;
{
    register int q;

    q = n / 10;
    if (q != 0)
	asciiparm (q);
    ttputc ((n % 10) + '0');
}

/*
* Switch to full screen scroll. This is
* used by "spawn.c" just before is suspends the
* editor, and by "display.c" when it is getting ready
* to exit.  This is a no-op.
*/
void
ttnowindow ()
{
}

/*
* Set the current writing color to the
* specified color. Watch for color changes that are
* not going to do anything (the color is already right)
* and don't send anything to the display.
*/
void
ttcolor (color)
    register int color;
{
#ifdef MSDOS
    if (mem_map)
    {
	tthue = color;		/* Save the color.      */
	return;
    }
#endif
#ifdef UNIX
    if (color == CTEXT)
	tcaprev (FALSE);
    else
	tcaprev (TRUE);
    tthue = color;		/* Save the color.      */
#endif
#ifdef ANSI
    if (color != tthue)
    {
	if (color == CTEXT)
	{			/* Normal video.        */
	    ttputc (ESC);
	    ttputc ('[');
	    ttputc ('0');
	    ttputc ('m');
	}
	else if (color == CMODE)
	{			/* Reverse video.       */
	    ttputc (ESC);
	    ttputc ('[');
	    ttputc ('7');
	    ttputc ('m');
	}
	tthue = color;		/* Save the color.      */
    }
#endif
}
