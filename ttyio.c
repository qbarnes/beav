/*
*
*   MS-DOS terminal I/O.               TTYIO.C
*/

#include        "def.h"
#ifdef	MSDOS


void ttopen ();
void ttclose ();		/* stub */
int  ttputc ();
void putline ();
void ttflush ();		/* stub */
int ttkeyready ();
int ttgetc ();
void ttraw ();
void ttcooked ();
void set_crt_type ();

#include    "dos.h"

int slot;
int scr_type;
#define SCREEN_PORT (video_port)
static int video_port =
{
    0x1010
};

extern bool wang_pc;
extern bool ibm_pc;
int nrow;			/* Terminal size, rows.         */
int ncol;			/* Terminal size, columns.      */
int last_key;
uchar attr = 0x0f;		/* saved color attribute, default
				 * white on black */

/*
* Initialization.
* for MS-DOS.
*/
void
ttopen ()
{
    uchar *ptr;
    uchar data[64];

    if (wang_pc && !ibm_pc)
	set_crt_type ();
    nrow = NROW;
    ncol = NCOL;
    if (ibm_pc)
    {
	union REGS inregs, outregs;
	struct SREGS segs;
	int i;

	for (i = 0; i < 64; i++)
	    data[i] = 0;
	ptr = data;
	inregs.h.ah = 0x1b;
	inregs.h.al = 0;
	inregs.x.bx = 0;
	inregs.x.di = (int) data;
#ifdef	FP_SEG	/* this is for MSC 5.1 */
	segs.es = FP_SEG (ptr);
#else	/* this is for MSC 6.0 or 7.0 */
	segs.es = _FP_SEG (ptr);
#endif
	
	int86x (0x10, &inregs, &outregs, &segs);	/* get number of rows */

	/* if that failed then use the default */
	if ((outregs.h.al != 0x1b) || (data[0x22] == 0))
	    return;
	nrow = data[0x22];

	/* get current attributes */
	inregs.h.ah = 0x8;
	inregs.h.al = 0;
	inregs.h.bh = 0;

	int86 (0x10, &inregs, &outregs);
	attr = outregs.h.ah & 0x7f;	/* don't want blink */

    }
}
void
ttclose ()
{
}
void
ttflush ()
{
}

/*
* Write character.
*/
int
ttputc (c)
    int c;
{
    bdos (6, c, 0);
    return c;
}

void
putline (row, startcol, stringsize, string)
    int row, startcol, stringsize;
    char *string;
{
    extern int tthue;
    unsigned short *screen;
    int x, attribute;
    char c_row, c_col, i;
    union REGS regs;

    if (ibm_pc)
    {
	c_row = row - 1;
	c_col = startcol - 1;
	for (i = 0; i < stringsize; i++)
	{
	    regs.h.ah = 2;
	    regs.h.dh = c_row;
	    regs.h.dl = c_col;
	    regs.h.bh = 0;
	    int86 (0x10, &regs, &regs);	/* set cursor position */

	    if (tthue == CTEXT)
		regs.h.bl = attr;
	    if (tthue == CMODE)
		regs.h.bl = ((0x70 & attr) >> 4) | ((0x07 & attr) << 4);
	    regs.h.ah = 9;
	    regs.h.bh = 0;
	    regs.h.al = string[i];
	    regs.x.cx = 1;
	    int86 (0x10, &regs, &regs);	/* set cursor position */
	    c_col++;
	}
    }
    else if (wang_pc)
    {
	if (tthue == CTEXT)
	    attribute = 0x00;
	else
	    attribute = 0x02;

	x = stringsize;
	screen = (unsigned short *) WANG_CHARACTER_SCREEN;
	screen += ((row - 1) * 80) + startcol - 1;
	outp (SCREEN_PORT, 01);
	while (x--)
	{
	    *screen = (*string++ << 8) | attribute;
	    screen++;
	}
	outp (SCREEN_PORT, 00);
    }
}

/*
*   return with a TRUE if key was struck.
*/
int
ttkeyready ()
{
    int cnt;

    if (last_key != 0)
	return (1);

    last_key = bdos (6, 0xff, 0);
    last_key &= 0xff;
    if (last_key == 0)
	return (0);
    else
	return (1);
}

/*
* Read character.
*/
int
ttgetc ()
{
    int c;
    if (last_key != 0)
    {
	c = last_key;
	last_key = 0;
	return (c);
    }
    ttcooked ();
    c = (bdos (7, 0, 0) & 0xFF);
    ttraw ();
    return (c);
}

/* disable nasty cntrl-c during disk io!
*/
void
ttraw ()
{
    union REGS inregs, outregs;

    inregs.h.al = 1;
    inregs.h.ah = 0x33;
    inregs.h.dl = 0;
    intdos (&inregs, &outregs);
    /*
  cntrlcoff();
  */
}

/* re enable cntrl-c for keyboard
*/
void
ttcooked ()
{
    union REGS inregs, outregs;

    inregs.h.al = 1;
    inregs.h.ah = 0x33;
    intdos (&inregs, &outregs);
    inregs.h.dl = 1;
    /*
  cntrlcon();
  */
}

/* switch physical monitors
*/
static char str[] =
{
    0x1b, '/', 1, 's'
};

void
set_crt_type ()
{
    char active_screen;

    active_screen = getscreenstate ();
    slot = active_screen & 0x0f;
    scr_type = (active_screen & 0x70) >> 4;
    video_port = 0x1010 | (slot << 8);
}

#endif
