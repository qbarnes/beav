/*
* The functions in this file handle redisplay. The
* redisplay system knows almost nothing about the editing
* process; the editing functions do, however, set some
* hints to eliminate a lot of the grinding. There is more
* that can be done; the "vtputc" interface is a real
* pig.  The MEMMAP
* changes things around for memory mapped video. With
* both off, the terminal is a VT52.
*/

#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"def.h"

D64 get_double ();
F32 get_float ();
D32 get_long ();
D16 get_int ();
void writ_echo ();
void modeline ();
void bin_to_text ();
uint fill_buf ();
uint get_currow ();
uint get_curcol ();
#ifndef NOPROTO
void ucopy (struct vid *vvp, struct vid *pvp);
void uline (int row, struct vid *vvp, struct vid *pvp);
#else
void uline ();
void ucopy ();
#endif
#if MSDOS
void mem_line (int row, struct vid *vvp);
#endif

extern char MSG_prn_to[];
extern char MSG_disp_r_n[];
extern char MSG_11lX[];
extern char MSG_11lo[];
extern char MSG_11ld[];
extern char MSG_116e[];
extern char MSG_106e[];
extern char MSG_03o[];
extern char MSG_06o[];
extern char MSG_011lo[];
extern char MSG_03u[];
extern char MSG_05u[];
extern char MSG_010lu[];
extern char MSG_02X[];
extern char MSG_04X[];
extern char MSG_08lX[];
extern char MSG_prog_name[];
extern char MSG_disp_b_lst[];
extern char MSG_file[];
extern char MSG_RO[];
extern char MSG_WL[];
extern char MSG_RW[];
extern char MSG_AU[];
extern char MSG_NOT_AU[];
extern char MSG_curs_asc[];
extern char MSG_curs_ebc[];
extern char MSG_curs_hex[];
extern char MSG_curs_bin[];
extern char MSG_curs_dec[];
extern char MSG_curs_flt[];
extern char MSG_curs_oct[];
extern char MSG_siz_8[];
extern char MSG_siz_16[];
extern char MSG_siz_32[];
extern char MSG_siz_64[];
extern char MSG_siz_null[];
extern char MSG_int_shift[];
extern char MSG_mot_shift[];
extern char MSG_print1[];
extern char MSG_print2[];
extern char MSG_cnt_al_w[];
#if RUNCHK
extern char ERR_disp_1[];
extern char ERR_disp_2[];
extern char ERR_disp_3[];
extern char ERR_disp_4[];
extern char ERR_disp_5[];
extern char ERR_disp_6[];
#endif

extern char ebcdic_table[];

extern bool mem_map;

/*
* You can change these back to the types
* implied by the name if you get tight for space. If you
* make both of them "int" you get better code on the VAX.
* They do nothing if this is not Gosling redisplay, except
* for change the size of a structure that isn't used.
* A bit of a cheat.
*/
#define XCHAR   int
#define XSHORT  int

/*
* A video structure always holds
* an array of characters whose length is equal to
* the longest line possible. Only some of this is
* used if "ncol" isn't the same as "NCOL".
*/
typedef struct vid
{
    short v_hash;		/* Hash code, for compares.	 */
    short v_flag;		/* Flag word.				   */
    short v_color;		/* Color of the line.		   */
    char v_text[NCOL];		/* The actual characters.	   */
} VIDEO;

#define VFCHG   0x0001		/* Changed.					 */
#define VFHBAD  0x0002		/* Hash and cost are bad.	   */

/*
* SCORE structures hold the optimal
* trace trajectory, and the cost of redisplay, when
* the dynamic programming redisplay code is used.
* If no fancy redisplay, this isn't used. The trace index
* fields can be "char", and the score a "short", but
* this makes the code worse on the VAX.
*/
typedef struct
{
    XCHAR s_itrace;		/* "i" index for track back.	*/
    XCHAR s_jtrace;		/* "j" index for trace back.	*/
    XSHORT s_cost;		/* Display cost.				*/
} SCORE;

int sgarbf = TRUE;		/* TRUE if screen is garbage.   */
int vtrow = 0;			/* Virtual cursor row.		  */
int vtcol = 0;			/* Virtual cursor column.	   */
int tthue = CNONE;		/* Current color.			   */
int ttrow = HUGE;		/* Physical cursor row.		 */
int ttcol = HUGE;		/* Physical cursor column.	  */
int tttop = HUGE;		/* Top of scroll region.		*/
int ttbot = HUGE;		/* Bottom of scroll region.	 */
char file_off_bad = FALSE;	/* Have file offsets been changed */

VIDEO **vscreen;		/* Edge vector, virtual.		*/
VIDEO **pscreen;		/* Edge vector, physical.	   */
VIDEO *video;			/* Actual screen data.		  */
VIDEO blanks;			/* Blank line image.			*/

/*
* Initialize the data structures used
* by the display code. The edge vectors used
* to access the screens are set up. The operating
* system's terminal I/O channel is set up. Fill the
* "blanks" array with ASCII blanks. The rest is done
* at compile time. The original window is marked
* as needing full update, and the physical screen
* is marked as garbage, so all the right stuff happens
* on the first call to redisplay.
*/
void
vtinit ()
{
    register VIDEO *vp;
    register int i;

    /* allocate memory for screen images */
    if (((vscreen = (VIDEO **) malloc (sizeof (VIDEO *) * nrow)) == NULL) ||
	((pscreen = (VIDEO **) malloc (sizeof (VIDEO *) * nrow)) == NULL) ||
	((video = (VIDEO *) malloc (sizeof (VIDEO) * 2 * nrow)) == NULL))
    {
	err_echo (MSG_cnt_al_w);
	exit (1);		/* can't continue */
    }

    memset(video, 0, sizeof (VIDEO) * 2 * nrow);

    vp = &video[0];
    for (i = 0; i < nrow; ++i)
    {
	vscreen[i] = vp;
	++vp;
	pscreen[i] = vp;
	++vp;
    }
    blanks.v_color = CTEXT;
    for (i = 0; i < NCOL; ++i)
	blanks.v_text[i] = ' ';
}

/*
*	Free memory used by the screen buffers
*/
void
vtfree ()
{
    /* release old screen memory */
    free (video);
    free (pscreen);
    free (vscreen);
}

/*
* Tidy up the virtual display system
* in anticipation of a return back to the host
* operating system. Right now all we do is position
* the cursor to the last line, erase the line, and
* close the terminal channel.
*/
void
vttidy ()
{
    ttcolor (CTEXT);
    ttnowindow ();		/* No scroll window.	*/
    ttmove (nrow - 1, 0);	/* Echo line.		   */
    tteeol ();
    tttidy ();
    ttflush ();
    ttclose ();
}

/*
* Move the virtual cursor to an origin
* 0 spot on the virtual display screen. I could
* store the column as a character pointer to the spot
* on the line, which would make "vtputc" a little bit
* more efficient. No checking for errors.
*/
void
vtmove (row, col)
    int row, col;
{
    vtrow = row;
    vtcol = col;
}

/*
* Write a character to the virtual display,
* dealing with long lines and the display of unprintable
* things like control characters. Also expand tabs every 8
* columns. This code only puts printing characters into
* the virtual display image. Special care must be taken when
* expanding tabs. On a screen whose width is not a multiple
* of 8, it is possible for the virtual cursor to hit the
* right margin before the next tab stop is reached. This
* makes the tab code loop if you are not careful.
* Three guesses how we found this.
*/
void
vtputc (c)
    register char c;
{
    register VIDEO *vp;

    vp = vscreen[vtrow];
    if (vtcol >= ncol)
	vp->v_text[ncol - 1] = '$';
    else if (ISCTRL (c) != FALSE)
    {
	vtputc ('^');
	vtputc ((char) (c + 0x40));
    }
    else
    {
	vp->v_text[vtcol] = c;
	vtcol++;
    }
}

/*
* Write an entire screen line in the correct format.	pvr
*
* This code only puts printing characters into
* the virtual display image.
* Return TRUE if something was printed to the line.
*/
#define REGI  register
bool
vtputd (wp, row)
    WINDOW *wp;
    int row;			/* line # to print to v screen */

{
    REGI VIDEO *vp;
    REGI uchar mode;
    REGI A32 row_offst;
    REGI uint chrs_per_row, lin_offset, i, chrs_in_lin;
    LINE *cur_line;
    static char w_buf[128];	/* temp buffer for data */

    vp = vscreen[vtrow];	/* point to VIDEO structure to print into */
    mode = R_TYPE (wp);		/* get type of format structure */

    /* get number of bytes per row */
    chrs_per_row = R_BYTES (wp);

    /* determine the offset from begining of the buffer for this line */
    row_offst = WIND_POS (wp) + (row * chrs_per_row);

    /* search for and point to first character in buffer to be printed */
    cur_line = wp->w_linep;	/* start with current first window line */
    while (TRUE)
    {				/* find line with desired character */
	if (cur_line == wp->w_bufp->b_linep)
	{			/* at end of buffer? */
	    return (FALSE);
	}
	if (cur_line->l_file_offset > row_offst)
	{
	    /* if less than current line */
	    cur_line = cur_line->l_bp;	/* step back */
	}
	else if ((cur_line->l_file_offset + cur_line->l_used) <= row_offst)
	{
	    cur_line = cur_line->l_fp;	/* step ahead */
	}
	else
	    break;
    }
    lin_offset = row_offst - cur_line->l_file_offset;	/* offset into line */

    /* get index into the current line to start reading the current row's data */
    /* copy line text into buffer */
    chrs_in_lin = fill_buf (wp, cur_line, lin_offset, w_buf, chrs_per_row);

    /* limit line length to screen width, used in TEXT mode only */
    if (chrs_in_lin > NCOL)
	chrs_in_lin = NCOL;

    /* Clear the line to spaces */
    for (i = 0; i < NCOL; i++)
    {
	vp->v_text[i] = ' ';
    }
    switch (mode)
    {
    case TEXT:
	break;
    case ASCII:
    case EBCDIC:
    case BINARY:
    case HEX:
	/* print the row offset from the start of the file in HEX */
	sprintf (vp->v_text, MSG_11lX, row_offst);	/* to vid buf */
	break;
    case OCTAL:
	/* print the row offset from the start of the file */
	sprintf (vp->v_text, MSG_11lo, row_offst);	/* to vid buf */
	break;
#if	FLOAT_DISP
    case FLOAT:
#endif
    case DECIMAL:
	/* print the row offset from the start of the file */
	sprintf (vp->v_text, MSG_11ld, row_offst);	/* to vid buf */
	break;
#if RUNCHK
    default:
	writ_echo (ERR_disp_1);
	break;
#endif
    }

    /* print the binary data to the text line */
    bin_to_text (w_buf, vp->v_text, chrs_in_lin, wp->w_fmt_ptr);

    vtcol = NCOL;
    return (TRUE);
}

/*
*   Print the contents of then binary data buffer bin_buf
*   into the proper mode of text into txt_buf.
*   Process 'len' bytes.
*
*   input:
*		   bin_buf	 pointer to buffer of binary data to process.
*		   txt_buf	 pointer to output buffer to print text result into.
*		   len		 length in bytes of data in bin_buf to process.
*		   fmt_ptr	 pointer to a ROW_FMT to use to format the data
*					   conversion and printing process.
*   output:
*		   none.
*/

void
bin_to_text (bin_buf, txt_buf, len, fmt_ptr)

    char *bin_buf, *txt_buf;
    uint len;
    ROW_FMT *fmt_ptr;

{
    uchar i, ch, k, j, mode, size, *posn;
    uint temp_int;
    ulong temp_long;

    mode = fmt_ptr->r_type;	/* get type of format structure */
    size = fmt_ptr->r_size;	/* get size of format structure */
    posn = fmt_ptr->r_positions;/* pointer to array of display positions */

    switch (mode)
    {
    case TEXT:
    case ASCII:
	for (i = 0; i < len; i++)
	{
	    ch = bin_buf[i];
	    if ((ch >= ' ') && (ch < 0x7f))
		txt_buf[posn[0] + i] = ch;
	    else
		txt_buf[posn[0] + i] = '.';
	}
	break;

    case EBCDIC:
	for (i = 0; i < len; i++)
	{
	    txt_buf[posn[0] + i] =
		0xff & ebcdic_table[0xff & bin_buf[i]];
	}
	break;

    case OCTAL:
	switch (size)
	{
	case BYTES:		/* print octal bytes */
	    for (i = 0; i < len; i++)
	    {
		sprintf (&txt_buf[
				     posn[i]], MSG_03o, 0xff & bin_buf[i]);
	    }
	    break;

	case WORDS:		/* print octal words */
	    k = 0;
	    for (i = 0; i < len;
		 i += 2)
	    {
		temp_int = get_int (&bin_buf[i]);
		sprintf (&txt_buf[posn[k++]], MSG_06o, temp_int);
	    }
	    break;

	case DWORDS:		/* print octal double words */
	    k = 0;
	    for (i = 0; i < len;
		 i += 4)
	    {
		temp_long = get_long (&bin_buf[i]);
		sprintf (&txt_buf[posn[k++]], MSG_011lo, temp_long);
	    }
	    break;
	}
	break;

    case DECIMAL:
	switch (size)
	{
	case BYTES:		/* print decimal bytes */
	    for (i = 0; i < len; i++)
	    {
		sprintf (&txt_buf[posn[i]], MSG_03u, 0xff & bin_buf[i]);
	    }
	    break;

	case WORDS:		/* print decimal words */
	    k = 0;
	    for (i = 0; i < len;
		 i += 2)
	    {
		temp_int = get_int (&bin_buf[i]);
		sprintf (&txt_buf[posn[k++]], MSG_05u, temp_int);
	    }
	    break;

	case DWORDS:		/* print decimal double words */
	    k = 0;
	    for (i = 0; i < len; i += 4)
	    {
		temp_long = get_long (&bin_buf[i]);
		sprintf (&txt_buf[posn[k++]], MSG_010lu, temp_long);
	    }
	    break;
	}
	break;
#if	FLOAT_DISP
    case FLOAT:
	switch (size)
	{
	case DWORDS:
	  {
	    k = 0;
	    for (i = 0; i < len; i += sizeof (F32))
	      {
		F32 temp_d;
		
		temp_d = get_float (&bin_buf[i]);
		sprintf (&txt_buf[posn[k++]], MSG_106e, temp_d);
	      }
	  }
	break;
	case DOUBLES:
	{
	    /*
            *	The Intel floating point representation is;
            *		bit 0  - 52		significand	(53 bits)
            *		bit 53 - 62		biased exponent (11 bits)
            *		bit	63			sign
            *	maximum range;	10^-308 <= X <= 10^+308
            *	obviously, not all patterns are legal floating point numbers.
            *	There can be up to 16 decimal digits of significand.
            *	There are only 3 decimal digits of exponent (308 max).
            */
	    k = 0;
	    for (i = 0; i < len; i += sizeof (D64))
	    {
		D64 temp_d;

		temp_d = get_double (&bin_buf[i]);
		sprintf (&txt_buf[posn[k++]], MSG_116e, temp_d);
	    }
	}
	break;
	}
	break;
#endif
    case HEX:
	switch (size)
	{
	case BYTES:		/* print hex bytes and ascii chars */
	    for (i = 0; i < len; i++)
	    {
		if ((bin_buf[i] >= ' ') && (bin_buf[i] < 0x7f))
		    txt_buf[posn[i + 16]] = 0xff & bin_buf[i];
		else
		    txt_buf[posn[i + 16]] = '.';
		sprintf (&txt_buf[posn[i]], MSG_02X, 0xff & bin_buf[i]);
	    }
	    break;

	case WORDS:		/* print hex words */
	    k = 0;
	    for (i = 0; i < len; i += 2)
	    {
		temp_int = get_int (&bin_buf[i]);
		sprintf (&txt_buf[
				     posn[k++]], MSG_04X, temp_int);
	    }
	    break;

	case DWORDS:		/* print hex double words */
	    k = 0;
	    for (i = 0; i < len; i += 4)
	    {
		temp_long = get_long (&bin_buf[i]);
		sprintf (&txt_buf[
				     posn[k++]], MSG_08lX, temp_long);
	    }
	    break;
	}
	break;

    case BINARY:
	switch (size)
	{
	case BYTES:		/* print binary bits */
	    for (i = 0; i < len; i++)
	    {
		ch = bin_buf[i];/* get char to convert */
		for (k = 0; k < 8; k++)
		{
		    if (ch & 0x80)
			txt_buf[posn[i] + k] = '1';
		    else
			txt_buf[posn[i] + k] = '0';
		    ch = ch << 1;	/* slide next bit into place */
		}
	    }
	    break;

	case WORDS:
	    j = 0;
	    for (i = 0; i < len; i += 2)
	    {
		temp_int = get_int (&bin_buf[i]);

		for (k = 0; k < 16; k++)
		{
		    if (temp_int & 0x8000)
			txt_buf[posn[j] + k] = '1';
		    else
			txt_buf[posn[j] + k] = '0';
		    temp_int = temp_int << 1;
		    /* slide next bit into place */
		}
		j++;
	    }
	    break;

	case DWORDS:
	    j = 0;
	    for (i = 0; i < len; i += 4)
	    {
		temp_long = get_long (&bin_buf[i]);
		for (k = 0; k < 32; k++)
		{
		    if (temp_long & 0x80000000)
			txt_buf[posn[j] + k] = '1';
		    else
			txt_buf[posn[j] + k] = '0';
		    temp_long = temp_long << 1;
		    /* slide next bit into place */
		}
		j++;
	    }
	    break;
	}
	break;
#if RUNCHK
    default:
	writ_echo (ERR_disp_2);
	break;
#endif
    }
    len *= (fmt_ptr->r_chr_per_u + 1);
    /* Clean up any garbage characters left by the sprintf's */
    for (i = 0; i < NCOL; i++)
    {
	if (txt_buf[i] == 0)
	    txt_buf[i] = ' ';
    }
}

/*
*   Get an int from the buffer.
*   Perform the Intel byte shuffle if necessary
*/

D16
get_int (w_buf)
    uchar *w_buf;

{
    int temp_int;

    if (curwp->w_intel_mode)
    {
	temp_int = 0xff & w_buf[1];
	temp_int = temp_int << 8;
	temp_int |= 0xff & w_buf[0];
    }
    else
    {
	temp_int = 0xff & w_buf[0];
	temp_int = temp_int << 8;
	temp_int |= 0xff & w_buf[1];
    }
    return (temp_int);
}

/*
*   Get an long from the buffer.
*   Perform the Intel byte shuffle if necessary
*/

D32
get_long (w_buf)
    uchar *w_buf;

{
    long temp_long;

    if (curwp->w_intel_mode)
    {
	temp_long = 0xff & w_buf[3];
	temp_long = temp_long << 8;
	temp_long |= 0xff & w_buf[2];
	temp_long = temp_long << 8;
	temp_long |= 0xff & w_buf[1];
	temp_long = temp_long << 8;
	temp_long |= 0xff & w_buf[0];
    }
    else
    {
	temp_long = 0xff & w_buf[0];
	temp_long = temp_long << 8;
	temp_long |= 0xff & w_buf[1];
	temp_long = temp_long << 8;
	temp_long |= 0xff & w_buf[2];
	temp_long = temp_long << 8;
	temp_long |= 0xff & w_buf[3];
    }
    return (temp_long);
}

#if	FLOAT_DISP

/*
*   Get a float from the buffer.
*   Perform the Intel byte shuffle if necessary
*/

F32
get_float (w_buf)
    uchar *w_buf;

{
    uchar temp_float[sizeof (F32)];
    F32 *fp;
    int i, siz;

    fp = (F32 *) temp_float;
    siz = sizeof (F32);

    if (curwp->w_intel_mode)
    {
	for (i = 0; i <= siz-1; i++)
	{
	    temp_float[i] = 0xff & w_buf[i];
	}
    }
    else
    {
	for (i = 0; i <= siz-1; i++)
	{
	    temp_float[(siz - 1) - i] = 0xff & w_buf[i];
	}
    }
    return (*fp);
}

/*
*   Get a double from the buffer.
*   Perform the Intel byte shuffle if necessary
*/
D64
get_double (w_buf)
    uchar *w_buf;
{
    uchar temp_doub[sizeof (D64)];
    D64 *dp;
    int i, siz;

    dp = (D64 *) temp_doub;
    siz = sizeof (D64);

    if (curwp->w_intel_mode)
    {
	for (i = 0; i <= siz - 1; i++)
	{
	    temp_doub[i] = 0xff & w_buf[i];
	}
    }
    else
    {
	for (i = 0; i <= siz-1; i++)
	{
	    temp_doub[(siz - 1) - i] = 0xff & w_buf[i];
	}
    }
    return (*dp);
}

#endif

/*
*   Copy a length of bytes from the buffer LINEs into the designated
*   buffer.   If the current LINE does not have enough bytes then
*   advance to the next.   Return the actual number of bytes copied.
*   The number copied would be less than the number requested if
*   end of file is reached.
*/

uint
fill_buf (wp, lin, lin_off, w_buff, cnt)
    WINDOW *wp;
    LINE *lin;
    uint lin_off, cnt;
    char *w_buff;
{
    REGI uint src, dest, i;

    src = lin_off;		/* initialize source line index */
    dest = 0;			/* initialize destination buffer index */

    while (TRUE)
    {
	while (src < lin->l_used)
	{
	    w_buff[dest++] = lin->l_text[src++];	/* copy byte */
	    if (dest == cnt)
	    {			/* if done */
		return (cnt);	/* then leave */
	    }
	}
	if (R_TYPE (wp) == TEXT)
	    return (dest);	/* in text mode don't advance to next line */

	lin = lin->l_fp;	/* move to the next line */
	if (lin == wp->w_bufp->b_linep)
	{			/* if past last line */
	    for (i = dest; i < cnt; ++i)
		w_buff[i] = 0;	/* fill rest of buffer with zeros */
	    return (dest);	/* return number of chars copied */
	}
	src = 0;		/* start next LINE at first byte */
    }
}

/*
* Erase from the end of the
* software cursor to the end of the
* line on which the software cursor is
* located. The display routines will decide
* if a hardware erase to end of line command
* should be used to display this.
*/
void
vteeol ()
{
    register VIDEO *vp;

    vp = vscreen[vtrow];
    while (vtcol < ncol)
	vp->v_text[vtcol++] = ' ';
}

/*
* Make sure that the display is
* right. This is a three part process. First,
* scan through all of the windows looking for dirty
* ones. Check the framing, and refresh the screen.
* Second, make the
* virtual and physical screens the same.
*/
void
update ()
{
    register WINDOW *wp;
    register VIDEO *vp1;
    register VIDEO *vp2;
    register uint i;
    register int hflag;

    hflag = FALSE;		/* Not hard.			*/
    wp = wheadp;
    while (wp != NULL)
    {
	/* is this window to be displayed in linked mode */
	if ((curbp->b_flag & BFLINK) &&
	    (wp->w_bufp == curbp))
	{			/* move dot to current window's dot position */
	    wp->w_dotp = curwp->w_dotp;
	    wp->w_doto = curwp->w_doto;
	    move_ptr (wp, 0L, TRUE, TRUE, TRUE);	/* insure dot is aligned */
	    wind_on_dot (wp);	/* move window to new dot position */
	}

	if (wp->w_flag != 0)
	{			/* Need update.		 */
	    move_ptr (wp, 0L, FALSE, TRUE, TRUE);	/* window on row boundary */
	    move_ptr (wp, 0L, TRUE, TRUE, TRUE);	/* dot on unit boundary */
	    if ((wp->w_flag & WFFORCE) == 0)
	    {
		wind_on_dot (wp);	/* position window on dot */
	    }
	    i = get_currow (wp);/* Redo this one line, mabey.	*/
	    if ((wp->w_flag & ~WFMODE) == WFEDIT)
	    {
		vscreen[i]->v_color = CTEXT;
		vscreen[i]->v_flag |= (VFCHG | VFHBAD);
		vtmove (i, 0);
		vtputd (wp, i - wp->w_toprow);	/* print line to the screen */
	    }
	    else if ((wp->w_flag & ~WFMODE) == WFMOVE)
	    {
		while (i < wp->w_toprow + wp->w_ntrows)
		{
		    /* paint entire window */
		    vscreen[i]->v_color = CTEXT;
		    vscreen[i]->v_flag |= (VFCHG | VFHBAD);
		    vtmove (i, 0);
		    /* print line to the screen */
		    if (!vtputd (wp, i - wp->w_toprow))
			vteeol ();
		    ++i;
		}
	    }
	    else if ((wp->w_flag & (WFEDIT | WFHARD)) != 0)
	    {
		hflag = TRUE;
		i = wp->w_toprow;
		while (i < wp->w_toprow + wp->w_ntrows)
		{
		    /* paint entire window */
		    vscreen[i]->v_color = CTEXT;
		    vscreen[i]->v_flag |= (VFCHG | VFHBAD);
		    vtmove (i, 0);
		    /* print line to the screen */
		    if (!vtputd (wp, i - wp->w_toprow))
			vteeol ();
		    ++i;
		}
	    }
	    if ((wp->w_flag & WFMODE) ||
		(wp->w_flag & WFMOVE) ||
		(wp->w_flag & WFHARD))
		modeline (wp);
	    wp->w_flag = 0;
	}
	wp = wp->w_wndp;
    }
    if (sgarbf != FALSE)
    {				/* Screen is garbage.   */
	sgarbf = FALSE;		/* Erase-page clears	*/
	epresf = FALSE;		/* the message area.	*/
	tttop = HUGE;		/* Forget where you set */
	ttbot = HUGE;		/* scroll region.	   */
	tthue = CNONE;		/* Color unknown.	   */
	ttmove (0, 0);
	tteeop ();
#if MSDOS
	if (mem_map)
	{
	    for (i = 0; i < nrow; ++i)
	    {
		mem_line (i, vscreen[i]);
	    }
	}
	else
	{
#endif
	    for (i = 0; i < nrow; ++i)
	    {
		uline (i, vscreen[i], &blanks);
		ucopy (vscreen[i], pscreen[i]);
	    }
#if MSDOS
	}
#endif
	ttmove (get_currow (curwp), get_curcol (curwp));
	ttflush ();
	return;
    }
    for (i = 0; i < nrow; ++i)
    {				/* Easy update.		 */
	vp1 = vscreen[i];
	vp2 = pscreen[i];
	if ((vp1->v_flag & VFCHG) != 0)
	{
#if MSDOS
	    if (mem_map)
		mem_line (i, vp1);
	    else
#endif
	    {
		uline (i, vp1, vp2);
		ucopy (vp1, vp2);
	    }
	}
    }
    ttmove (get_currow (curwp), get_curcol (curwp));
    ttflush ();
}

/*
*   Get the window relative row in which the cursor will
*   appear. pvr
*/
uint
get_currow (wp)
    WINDOW *wp;
{
    A32 row;
    /* number of bytes from start of window */
    row = DOT_POS (wp) - WIND_POS (wp);
    /* number of rows down in window */
    row /= R_BYTES (wp);
    row += wp->w_toprow;
#if RUNCHK
    if (row < wp->w_toprow)
	printf (ERR_disp_3);
    if (row > (wp->w_ntrows + wp->w_toprow))
	printf (ERR_disp_4);
#endif
    return (row & 0xffff);
}

/*
*   Get the window relative column in which the cursor will
*   appear. pvr
*/
uint
get_curcol (wp)
    WINDOW *wp;
{
    long offset, index;
    uint b_per_u, pos;

    b_per_u = R_B_PER_U (wp);
    /* dot offset from start of buffer */
    offset = DOT_POS (wp);
    offset -= wp->w_disp_shift;
    offset &= ~(b_per_u - 1);
    /* calculate mod of the current file position */
    index = offset & (R_BYTES (wp) - 1);
    index /= b_per_u;
    /* limit to window width */
    if (index >= NCOL)
	index = NCOL;
    pos = wp->w_fmt_ptr->r_positions[index] + wp->w_unit_offset;
    return (pos);
}

#if MSDOS
void
mem_line (row, vvp)
    int row;
    VIDEO *vvp;
{
    vvp->v_flag &= ~VFCHG;	/* Changes done.		*/
    ttcolor (vvp->v_color);
    putline (row + 1, 1, ncol, &vvp->v_text[0]);
}

#endif
/*
* Update a saved copy of a line,
* kept in a VIDEO structure. The "vvp" is
* the one in the "vscreen". The "pvp" is the one
* in the "pscreen". This is called to make the
* virtual and physical screens the same when
* display has done an update.
*/
void
ucopy (vvp, pvp)
    register VIDEO *vvp;
    register VIDEO *pvp;
{
    register int i;

    vvp->v_flag &= ~VFCHG;	/* Changes done.		*/
    pvp->v_flag = vvp->v_flag;	/* Update model.		*/
    pvp->v_hash = vvp->v_hash;
    pvp->v_color = vvp->v_color;
    for (i = 0; i < ncol; ++i)
	pvp->v_text[i] = vvp->v_text[i];
}

/*
* Update a single line. This routine only
* uses basic functionality (no insert and delete character,
* but erase to end of line). The "vvp" points at the VIDEO
* structure for the line on the virtual screen, and the "pvp"
* is the same for the physical screen. Avoid erase to end of
* line when updating CMODE color lines, because of the way that
* reverse video works on most terminals.
*/
void
uline (row, vvp, pvp)
    int row;
    VIDEO *vvp;
    VIDEO *pvp;
{
    register char *cp1;
    register char *cp2;
    register char *cp3;
    register char *cp4;
    register char *cp5;
    register int nbflag;

    if (vvp->v_color != pvp->v_color)
    {				/* Wrong color, do a	*/
	ttmove (row, 0);	/* full redraw.		 */
	ttcolor (vvp->v_color);
	cp1 = &vvp->v_text[0];
	cp2 = &vvp->v_text[ncol];
	while (cp1 != cp2)
	{
	    ttputc (*cp1++);
	    ++ttcol;
	}
	return;
    }
    cp1 = &vvp->v_text[0];	/* Compute left match.  */
    cp2 = &pvp->v_text[0];
    while (cp1 != &vvp->v_text[ncol] && cp1[0] == cp2[0])
    {
	++cp1;
	++cp2;
    }
    if (cp1 == &vvp->v_text[ncol])	/* All equal.		   */
	return;
    nbflag = FALSE;
    cp3 = &vvp->v_text[ncol];	/* Compute right match. */
    cp4 = &pvp->v_text[ncol];
    while (cp3[-1] == cp4[-1])
    {
	--cp3;
	--cp4;
	if (cp3[0] != ' ')	/* Note non-blanks in   */
	    nbflag = TRUE;	/* the right match.	 */
    }
    cp5 = cp3;			/* Is erase good?	   */
    if (nbflag == FALSE && vvp->v_color == CTEXT)
    {
	while (cp5 != cp1 && cp5[-1] == ' ')
	    --cp5;
	/* Alcyon hack */
	if ((int) (cp3 - cp5) <= tceeol)
	    cp5 = cp3;
    }
    /* Alcyon hack */
    ttmove (row, (int) (cp1 - &vvp->v_text[0]));
    ttcolor (vvp->v_color);
    while (cp1 != cp5)
    {
	ttputc (*cp1++);
	++ttcol;
    }
    if (cp5 != cp3)		/* Do erase.			*/
	tteeol ();
}

/*
* Redisplay the mode line for
* the window pointed to by the "wp".
* This is the only routine that has any idea
* of how the modeline is formatted. You can
* change the modeline format by hacking at
* this routine. Called by "update" any time
* there is a dirty window.
*/

void
modeline (wp)
    register WINDOW *wp;
{
    register char *cp, size, u_posn, *s;
    uchar mode;
    register char c;
    register int n;
    register BUFFER *bp;
    register A32 posn;

    static char posn_buf[30] =
    {
	0
    };				/* krw */

    mode = wp->w_fmt_ptr->r_type;	/* get type of format structure */
    size = wp->w_fmt_ptr->r_size;	/* get size of format structure */

    n = wp->w_toprow + wp->w_ntrows;	/* Location.			*/
    vscreen[n]->v_color = CMODE;/* Mode line color.	 */
    vscreen[n]->v_flag |= (VFCHG | VFHBAD);	/* Recompute, display.  */
    vtmove (n, 0);		/* Seek to right line.  */
    bp = wp->w_bufp;

    cp = MSG_prog_name;		/* Program name.  pvr    */
    n = 5;
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }

    if ((bp->b_flag & BFBAD) != 0)	/* "?" if trashed.      */
	vtputc ('?');
    else
	vtputc (' ');

    if ((bp->b_flag & BFCHG) != 0)	/* "*" if changed.      */
	vtputc ('*');
    else
	vtputc (' ');

    if (insert_mode)		/* "I" if insert mode  */
	vtputc ('I');
    else
	vtputc ('O');

    if (bp == blistp)
    {				/* special list */
	cp = MSG_disp_b_lst;
	while ((c = *cp++) != 0)
	{
	    vtputc (c);
	    ++n;
	}
	goto pad;
    }

    /* Buffer name */
    vtputc (' ');
    ++n;
    cp = &bp->b_bname[0];
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }
    while ((int) (cp - &bp->b_bname[0]) < NBUFN)
    {
	vtputc (' ');
	n++;
	cp++;
    }

    /* File name.           */
    vtputc (' ');
    ++n;
    cp = MSG_file;
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }
    cp = &bp->b_fname[0];
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }
    cp--;
    while ((int) (cp - &bp->b_fname[0]) < NFILE)
    {
	vtputc (' ');
	n++;
	cp++;
    }

    if (bp->b_flag & BFVIEW)
	s = MSG_RO;
    else if (bp->b_flag & BFSLOCK)
	s = MSG_WL;
    else
	s = MSG_RW;

    while (*s)
    {				/* krw */
	vtputc (*s++);
	++n;
    }

    if (auto_update && !(bp->b_flag & BFVIEW) && bp->b_bname[0])	/* jam */
	s = MSG_AU;
    else
	s = MSG_NOT_AU;
    for (; *s && n < NCOL;)
    {
	vtputc (*s++);
	++n;
    }

    /* Insert current dot position into mode line. */
    posn = DOT_POS (wp);
    u_posn = R_CHR_PER_U (wp) - wp->w_unit_offset - 1;
    if (u_posn < 0)
	u_posn = 0;
    switch (mode)
    {
    case TEXT:
    case ASCII:
	sprintf (posn_buf, MSG_curs_asc, posn);
	break;
    case EBCDIC:
	sprintf (posn_buf, MSG_curs_ebc, posn);
	break;
    case HEX:
	sprintf (posn_buf, MSG_curs_hex, posn, u_posn);
	break;
    case BINARY:
	sprintf (posn_buf, MSG_curs_bin, posn, u_posn);
	break;
    case DECIMAL:
	sprintf (posn_buf, MSG_curs_dec, posn, u_posn);
	break;
#if	FLOAT_DISP
    case FLOAT:
#endif
	sprintf (posn_buf, MSG_curs_flt, posn, u_posn);
	break;
    case OCTAL:
	sprintf (posn_buf, MSG_curs_oct, posn, u_posn);
	break;
#if RUNCHK
    default:
	writ_echo (ERR_disp_5);
	break;
#endif
    }

    cp = posn_buf;
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }


    if ((mode == HEX) ||
	(mode == DECIMAL) ||
	(mode == FLOAT) ||
	(mode == OCTAL))
    {
	switch (size)
	{
	case BYTES:
	    sprintf (posn_buf, MSG_siz_8);
	    break;
	case WORDS:
	    sprintf (posn_buf, MSG_siz_16);
	    break;
	case DWORDS:
	    sprintf (posn_buf, MSG_siz_32);
	    break;
	case DOUBLES:
	    sprintf (posn_buf, MSG_siz_64);
	    break;
#if RUNCHK
	default:
	    writ_echo (ERR_disp_6);
	    break;
#endif
	}
    }
    else
	sprintf (posn_buf, MSG_siz_null);

    cp = posn_buf;
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }

    if (wp->w_intel_mode)
	sprintf (posn_buf, MSG_int_shift, wp->w_disp_shift);
    else
	sprintf (posn_buf, MSG_mot_shift, wp->w_disp_shift);
    cp = posn_buf;
    while ((c = *cp++) != 0)
    {
	vtputc (c);
	++n;
    }


    /* pad out */
  pad:
    while (n < ncol)
    {
	vtputc (' ');
	++n;
    }
}

/*
* write text to the echo line
*/
void
writ_echo (buf)
    char *buf;
{
    int i;
    char *vpp;
    bool fill_spac;

    fill_spac = FALSE;
    vpp = vscreen[nrow - 1]->v_text;
    vscreen[nrow - 1]->v_color = CTEXT;
    vscreen[nrow - 1]->v_flag |= VFCHG;
    epresf = TRUE;

    for (i = 0; i < NCOL; i++)
    {
	if (buf[i] == 0)
	    fill_spac = TRUE;
	if (fill_spac)
	    vpp[i] = ' ';
	else
	    vpp[i] = buf[i];
    }
#if MSDOS
    if (mem_map)
    {
	mem_line (nrow - 1, vscreen[nrow - 1]);
    }
    else
#endif
    {
	uline (nrow - 1, vscreen[nrow - 1], pscreen[nrow - 1]);
	uline (nrow - 1, vscreen[nrow - 1], &blanks);
	ucopy (vscreen[nrow - 1], pscreen[nrow - 1]);
	ttflush ();
    }
}

/*
* Print the current buffer from mark to dot using the
* current window's display format.
* Prompt for file name or io device to print to.
*/

bool
print ()
{
    LINE *dot_l_sav, *mark_l_sav, *wind_l_sav;
    int dot_off_sav, mark_off_sav, wind_off_sav, i;
    char s;
    char fname[NFILEN];
    register int nline;
    char buf[NFILEN], buf1[NFILEN];

    /* save the original window state */
    dot_l_sav = curwp->w_dotp;
    dot_off_sav = curwp->w_doto;
    mark_l_sav = curwp->w_markp;
    mark_off_sav = curwp->w_marko;
    wind_l_sav = curwp->w_linep;
    wind_off_sav = curwp->w_loff;

    /* if mark is not set then set it to location zero */
    if (curwp->w_markp == NULL)
    {
	curwp->w_markp = curwp->w_bufp->b_linep->l_fp;
	curwp->w_marko = 0;
    }

    nline = 0;
    if ((s = ereply (MSG_prn_to, fname, NFILEN, NULL)) == ABORT)
	return (s);
    adjustcase (fname);
    if ((s = ffwopen (fname, S_IREAD | S_IWRITE)) != FIOSUC)	/* Open writes message. */
	return (FALSE);

    sprintf (buf, MSG_print1, fname);
    writ_echo (buf);
    /* make dot before mark */
    if (DOT_POS (curwp) > MARK_POS (curwp))
	swapmark ();		/* make mark first */

    while (DOT_POS (curwp) <= MARK_POS (curwp))
    {
	/* check if we should quit */
	if (ttkeyready ())
	{
	    if (ttgetc () == CTL_G)	/* quit if abort was hit */
		break;
	}
	nline++;
	/* move window so that first line is on dot */
	move_ptr (curwp, DOT_POS (curwp), FALSE, TRUE, FALSE);

	if (vtputd (curwp, 0))	/* print line into video buffer */
	{
	    for (i = NCOL; (vscreen[vtrow]->v_text[i] < '!') ||
		 (vscreen[vtrow]->v_text[i] > '~'); i--)
		;
	    i++;
	    if ((s = ffputline (vscreen[vtrow]->v_text, i)) != FIOSUC)
		break;
	    if ((s = ffputline (MSG_disp_r_n, 2)) != FIOSUC)
		break;
	}
	else
	    break;
	forwline (0, 1, KRANDOM);	/* advance to next line */
    }
    ffclose ();
    sprintf (buf1, MSG_print2, R_POS_FMT (curwp));
    sprintf (buf, buf1, (long) nline);
    writ_echo (buf);

    /* restore the original window state */
    curwp->w_dotp = dot_l_sav;
    curwp->w_doto = dot_off_sav;
    curwp->w_markp = mark_l_sav;
    curwp->w_marko = mark_off_sav;
    curwp->w_linep = wind_l_sav;
    curwp->w_loff = wind_off_sav;
    curwp->w_flag |= WFHARD;	/* insure that window is still presentable */
    return (TRUE);
}
