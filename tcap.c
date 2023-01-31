/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
		for beav
*/

#include "def.h"

#ifdef UNIX

#ifdef OS2
#ifndef __EMX__
#define INCL_NOPM
#define INCL_VIO
#include <os2.h>
#endif
#endif

#define	MARGIN	8
#define	SCRSIZ	64
#define	NPAUSE	10		/* # times thru update to pause */
#define BEL     0x07
#define ESC     0x1B

extern char *tgoto ();

#ifdef NOPROTO
extern int ttputc ();
void putpad ();
#endif

#ifdef	COLOR
extern int tcapfcol ();
extern int tcapbcol ();
#endif

#define TCAPSLEN 315
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE, *TI, *TE;	/* DR */

#ifdef BSD
#include <sys/ioctl.h>
struct winsize ttysize;
#endif /* BSD */
#ifdef ULTRIX
struct winsize ttysize;
#endif

void
putpad (str)
    char *str;
{
    tputs (str, 1, ttputc);
}

void
tcapopen ()
{
    char *getenv ();
    char *t, *p, *tgetstr ();
    char tcbuf[1024];
    char *tv_stype;
    char err_str[NCOL];
#ifdef ULTRIX
    struct winsize ttysize;
#endif

    nrow = NROW;

    if ((tv_stype = getenv ("TERM")) == NULL)
#ifdef OS2
	tv_stype = "ansi";
#else
    {
	puts ("Environment variable TERM not defined!\r");
	ttclose ();
	exit (1);
    }
#endif

    if ((tgetent (tcbuf, tv_stype)) != 1)
    {
	sprintf (err_str, "Unknown terminal type %s!\r", tv_stype);
	puts (err_str);
	ttclose ();		/* fix in 1.13 */
	exit (1);
    }


#ifdef BSD
#ifdef ULTRIX
    if (ioctl (0, TIOCGWINSZ, &ttysize) == 0
	&& ttysize.ws_row > 0)
    {
	nrow = ttysize.ws_row;
    }
    else
#else
    if (ioctl (0, TIOCGSIZE, &ttysize) == 0
	&& ttysize.ts_lines > 0)
    {
	nrow = ttysize.ts_lines;
    }
    else
#endif /* ULTRIX */
#endif /* BSD */

#ifndef OS2
    if ((nrow = (short) tgetnum ("li") - 1) == -1)
    {
	puts ("termcap entry incomplete (lines)\r");
	ttclose ();		/* fix in 1.13 */
	exit (1);
    }
    printf ("nrow %d, ncol %d\n", nrow, ncol);

    if ((ncol = (short) tgetnum ("co")) == -1)
    {
	puts ("Termcap entry incomplete (columns)\r");
	ttclose ();		/* fix in 1.13 */
	exit (1);
    }
    /* don't allow to specify a larger number of cols than we can handle 1.13 */
    if (ncol > NCOL)
	ncol = NCOL;
#else
    {
#ifdef __EMX__
	int dst[2];
	_scrsize (dst);
	nrow = dst[1];
	ncol = dst[0];
#else
	VIOMODEINFO viomi;
	viomi.cb = sizeof (viomi);
	VioGetMode (&viomi, 0);
	nrow = viomi.row;
	ncol = viomi.col;
#endif
    }
#endif

    p = tcapbuf;
    t = tgetstr ("pc", &p);
    if (t)
	PC = *t;

    TI = tgetstr ("ti", &p);	/* DR */
    TE = tgetstr ("te", &p);	/* DR */
    CL = tgetstr ("cl", &p);
    CM = tgetstr ("cm", &p);
    CE = tgetstr ("ce", &p);
    UP = tgetstr ("up", &p);
    SO = tgetstr ("so", &p);
    SE = tgetstr ("se", &p);

    if (CL == NULL || CM == NULL || UP == NULL)
    {
	puts ("Incomplete termcap entry\r");
	ttclose ();		/* fix in 1.13 */
	exit (1);
    }

    if (p >= &tcapbuf[TCAPSLEN])
    {
	puts ("Terminal description too big!\r");
	ttclose ();		/* fix in 1.13 */
	exit (1);
    }
    putpad (TI);		/* DR */
}

void
tcapclose ()			/* DR */
{
    putpad (TE);
}

void
tcapmove (row, col)
    register int row, col;
{
    putpad (tgoto (CM, col, row));
}

void
tcapeeol ()
{
    putpad (CE);
}

void
tcapeeop ()
{
    putpad (CL);
}

void
tcaprev (state)			/* change reverse video status */
    int state;			/* FALSE = normal video, TRUE = reverse video */

{
    if (state)
    {
	if (SO != NULL)
	    putpad (SO);
    }
    else if (SE != NULL)
	putpad (SE);
}

void
putnpad (str, n)
    char *str;
    int n;
{
    tputs (str, n, ttputc);
}

#endif
