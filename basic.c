/*
*      Basic cursor motion commands.
* The routines in this file are the basic
* command functions for moving the cursor around on
* the screen, setting mark, and swapping dot with
* mark. Only moves between lines, which might make the
* current buffer framing bad, are hard.
*/

#include    "def.h"

extern char MSG_mark_set[];
extern char MSG_no_mark[];
extern char MSG_go_b_n[];
extern char MSG_bad_num[];
#if RUNCHK
extern char ERR_bas_1[];
#endif
extern char MSG_lX[];
extern char MSG_lO[];
extern char MSG_lD[];


extern bool rplc_mode;

/*  pvr
* Move cursor backwards. Do the
* right thing if the count is less than
* 0. Error if you try to move back from
* the beginning of the buffer.
*/
bool
backchar (f, n, k)
    register int f, n, k;
{
    if (n < 0)
	return (forwchar (f, -n, KRANDOM));

    while (n--)
    {
	if (curwp->w_unit_offset == 0)
	{
	    if (!move_ptr (curwp, -(long) R_B_PER_U (curwp),
			   TRUE, TRUE, TRUE))
		return (FALSE);

	    /* step to previous unit */
	    curwp->w_unit_offset = R_CHR_PER_U (curwp) - 1;

	    /* if before first line in window then move window */
	    wind_on_dot (curwp);
	}
	else
	    curwp->w_unit_offset--;
    }
    curwp->w_flag |= WFMODE;	/* update mode line */
    return (TRUE);
}

/*  pvr
* Move cursor forwards. Do the
* right thing if the count is less than
* 0. Error if you try to move forward
* from the end of the buffer.
*/
bool
forwchar (f, n, k)
    register int f, n, k;
{
    if (n < 0)
	return (backchar (f, -n, KRANDOM));

    curwp->w_flag |= WFMODE;	/* update mode line */
    while (n--)
    {
	if (curwp->w_unit_offset >= (R_CHR_PER_U (curwp) - 1))
	{
	    /* move to the mext unit */
	    curwp->w_unit_offset = 0;

	    if (!move_ptr (curwp, (long) R_B_PER_U (curwp),
			   TRUE, TRUE, TRUE))
	    {
		/* I am at the the end of the buffer */
		return (FALSE);
	    }

	    /* if after the last line in window then move window */
	    wind_on_dot (curwp);
	}
	else if			/* if at last byte of buffer then do not step  */
	    (DOT_POS (curwp) < BUF_SIZE (curwp))
	    curwp->w_unit_offset++;	/* step within unit */
    }
    return (TRUE);
}

/*  pvr
*   This function moves the specified pointer by the ammount specified
*   in 'len'.   Move the dot pointer is 'dot' is true, else move
*   the window pointer.  Do the fix up if 'fix' is TRUE.
*   This is a relative move if 'rel' is TRUE, else it is an
*   absolute move.
*/

bool
move_ptr (wp, len, dot, fix, rel)
    WINDOW *wp;
    long len;
    bool dot, fix, rel;
{
    A32 cur_pos, dest_pos, fix_val, last_pos;
    long rel_pos;
    A32 last_fixed_pos, align;
    LINE **line;
    int *l_off;
    char shift;
    bool no_limit;

    no_limit = TRUE;
    if (dot)
    {				/* move dot position */
	l_off = (int *) &wp->w_doto;
	line = &wp->w_dotp;
	align = R_SIZE (wp);	/* bytes -1 in a unit */
    }
    else
    {				/* move window position */
	l_off = (int *) &wp->w_loff;
	line = &wp->w_linep;
	align = R_ALIGN (wp) - 1;	/* interval of bytes to align window */
    }

    /* get the current position in the buffer */
    cur_pos = (*line)->l_file_offset + *l_off;

    if (rel)
    {
	rel_pos = len;
	dest_pos = len + cur_pos;	/* destination position */
    }
    else
    {
	rel_pos = len - cur_pos;/* relative move amount */
	dest_pos = len;		/* destination position */
    }
    if (fix)
    {
	shift = wp->w_disp_shift;

	/* limit at begining */
	if ((long) dest_pos < (long) shift)
	{
	    rel_pos = shift - cur_pos;
	    no_limit = FALSE;
	}
	else
	{
	    /* calculate fixed up destination position */
	    fix_val = dest_pos &= ~align;
	    fix_val += shift;

	    /* calculate the last position in the buffer */
	    last_pos = BUF_SIZE (wp);
	    if (last_pos < (last_fixed_pos = (last_pos & ~align) + shift))
		last_pos = last_fixed_pos - align - 1;

	    /* if we are going to limit at the end of the buffer */
	    if (last_pos < fix_val)
	    {
		fix_val = last_pos;
		no_limit = FALSE;
	    }
	    rel_pos = fix_val - cur_pos;
	}
    }
    while (TRUE)
    {
	if (rel_pos < 0)	/* move  backward through buffer */
	{
	    /* current line? */
	    if (*l_off + rel_pos >= 0)
	    {
		*l_off += (short) rel_pos;
		return (no_limit);
	    }
	    /* are we at the first line */
	    if ((*line)->l_bp->l_size != 0)
	    {			/* no, so step back */
		rel_pos += *l_off;
		(*line) = (*line)->l_bp;	/* move back one line */
		*l_off = (*line)->l_used;
	    }
	    else
	    {			/* yes, limit at the begining */
		*l_off = 0;
		return (FALSE);
	    }
	}
	else
	    /* move forward through buffer */
	{
	    /* is in current line? */
	    if (((A32) (*l_off) + rel_pos) < ((A32) ((*line)->l_used)))
	    {
		*l_off += (short) rel_pos;
		return (no_limit);
	    }
	    if ((*line)->l_fp->l_size != 0)
	    {
		rel_pos -= (*line)->l_used - *l_off;
		*l_off = 0;
		(*line) = (*line)->l_fp;	/* move forward one line */
	    }
	    else
	    {
		*l_off = (*line)->l_used;	/* at last line so limit it */
		return (FALSE);
	    }
	}
    }
}

/*  pvr
*   Move the window so that the dot is within it's
*   area.   Return TRUE if window was moved.
*/

bool
wind_on_dot (wp)

    WINDOW *wp;
{
    long diff, incr;
    A32 d_offs, w_start, bytes, align;

    /* number of bytes in a row */
    bytes = R_BYTES (wp);
    /* number of bytes to align on */
    align = R_ALIGN (wp);
    /* offset of window from start of the buffer */
    w_start = WIND_POS (wp);
    /* offset of dot from start of the buffer */
    d_offs = DOT_POS (wp);
    /* calculate the amount to move that is 1/3 of the window */
    incr = bytes * wp->w_ntrows / 3;
    /* if dot is before first line in window */
    if ((diff = (d_offs - w_start)) < 0)	/* diff used later */
    {
	move_ptr (wp, diff - incr, FALSE, TRUE, TRUE);
	wp->w_flag |= WFHARD;
	return (TRUE);
    }
    /* if dot is after the last line in window */
    if (0 < (diff -= (wp->w_ntrows * bytes - 1)))
    {
	if (align != 1)
	    diff = (diff & ~(align - 1)) + align;
	move_ptr (wp, diff + incr, FALSE, TRUE, TRUE);
	wp->w_flag |= WFHARD;
	return (TRUE);
    }
    /* is window aligned? */
    if (w_start != ((w_start & ~(align - 1)) + wp->w_disp_shift))
    {				/* if no then move into alignment */
	move_ptr (wp, 0L, FALSE, TRUE, TRUE);
	wp->w_flag |= WFHARD;
	return (TRUE);
    }
    return (FALSE);
}

/*  pvr
* Go to the beginning of the
* buffer. Setting WFHARD is conservative,
* but almost always the case.
*/
bool
gotobob ()
{
    move_ptr (curwp, 0L, TRUE, TRUE, FALSE);	/* move dot */
    move_ptr (curwp, 0L, FALSE, TRUE, FALSE);	/* move window */
    curwp->w_unit_offset = 0;
    curwp->w_flag |= WFHARD;
    return (TRUE);
}

/*  pvr
* Go to the end of the buffer.
* Setting WFHARD is conservative, but
* almost always the case.
* Dot is one byte past the end of the buffer.
*/
bool
gotoeob ()
{
    move_ptr (curwp, BUF_SIZE (curwp), TRUE, TRUE, FALSE);	/* move dot */
    curwp->w_unit_offset = 0;
    wind_on_dot (curwp);
    return (TRUE);
}

/*  pvr
* Move forward by full lines.
* If the number of lines to move is less
* than zero, call the backward line function to
* actually do it. The last command controls how
* the goal column is set.
*/
bool
forwline (f, n, k)
    int f, n, k;
{
    if (n < 0)
	return (backline (f, -n, KRANDOM));

    if (rplc_mode)
    {
	next_pat ();
    }
    else
    {
	/* move dot */
	if (!move_ptr (curwp, (long) R_BYTES (curwp) * n,
		       TRUE, TRUE, TRUE))
	    curwp->w_unit_offset = 0;
	wind_on_dot (curwp);
	curwp->w_flag |= WFMODE;/* update mode line */
    }
    return (TRUE);
}

/*  pvr
* This function is like "forwline", but
* goes backwards. The scheme is exactly the same.
* Check for arguments that are less than zero and
* call your alternate. Figure out the new line and
* call "movedot" to perform the motion.
*/
bool
backline (f, n, k)
    int f, n, k;
{
    if (n < 0)
	return (forwline (f, -n, KRANDOM));

    if (rplc_mode)
    {
	next_pat ();
    }
    else
    {
	if (!move_ptr (curwp, -((long) (R_BYTES (curwp) * n)),
		       TRUE, TRUE, TRUE))
	    curwp->w_unit_offset = 0;

	/* is dot before the top of window? */
	wind_on_dot (curwp);
	curwp->w_flag |= WFMODE;/* update mode line */
    }
    return (TRUE);
}

/*  pvr
* Scroll forward by a specified number
* of lines, or by a full page if no argument.
* (KRW) Added cursor (dot) weighting to force cursor
*       to same position on new page.
*/
bool
forwpage (f, n, k)
    int f, n, k;
{
    long mov_lines;

    if (rplc_mode)
	next_pat ();
    else
    {
	if (curwp->w_ntrows <= 2)
	    mov_lines = 2;
	else
	    mov_lines = curwp->w_ntrows - 2;

	/* check if last line is already displayed */
	if (WIND_POS (curwp) + (R_BYTES (curwp) * curwp->w_ntrows) <
	    curwp->w_bufp->b_linep->l_bp->l_file_offset +
	    curwp->w_bufp->b_linep->l_bp->l_used)
	{
	    move_ptr (curwp, (long) (R_BYTES (curwp) * mov_lines),
		      FALSE, TRUE, TRUE);
	}
	/* move dot by same amount */
	if (!move_ptr (curwp, (long) (R_BYTES (curwp) * mov_lines),
		       TRUE, TRUE, TRUE))
	    curwp->w_unit_offset = 0;

	curwp->w_flag |= WFHARD;
    }
    return (TRUE);
}

/*  pvr
* This command is like "forwpage",
* but it goes backwards.
*/
bool
backpage (f, n, k)
    int f, n, k;
{
    long mov_lines;

    if (rplc_mode)
	next_pat ();
    else
    {
	if (curwp->w_ntrows <= 2)
	    mov_lines = 2;
	else
	    mov_lines = curwp->w_ntrows - 2;

	/* move window */
	move_ptr (curwp, -(long) (R_BYTES (curwp) * mov_lines),
		  FALSE, TRUE, TRUE);
	/* move dot by same amount */
	if (!move_ptr (curwp, -(long) (R_BYTES (curwp) * mov_lines),
		       TRUE, TRUE, TRUE))
	    curwp->w_unit_offset = 0;

	curwp->w_flag |= WFHARD;
    }
    return (TRUE);
}

/*
* Set the mark in the current window
* to the value of dot. A message is written to
* the echo line unless we are running in a keyboard
* macro, when it would be silly.
*/
bool
setmark ()
{

    if (curbp == blistp)	/* jam - hack to do goto/kill */
	pickone ();
    else
    {
	curwp->w_markp = curwp->w_dotp;
	curwp->w_marko = curwp->w_doto;
	if (kbdmop == NULL)
	{
	    writ_echo (MSG_mark_set);
	}
    }
    return (TRUE);
}

/*  pvr
* Swap the values of "dot" and "mark" in
* the current window. This is pretty easy, because
* all of the hard work gets done by the standard routine
* that moves the mark about. The only possible
* error is "no mark".
*/
bool
swapmark ()
{
    register short odoto;
    register LINE *odotp;

    if (curwp->w_markp == NULL)
    {
	writ_echo (MSG_no_mark);
	return (FALSE);
    }

    odotp = curwp->w_dotp;
    curwp->w_dotp = curwp->w_markp;
    curwp->w_markp = odotp;
    odoto = curwp->w_doto;
    curwp->w_doto = curwp->w_marko;
    curwp->w_marko = odoto;
    wind_on_dot (curwp);
    curwp->w_flag |= WFMODE;	/* update mode line */
    return (TRUE);
}

/*  pvr
* Go to a specific byte position in buffer.
* If an argument is present, then
* it is the byte number, else prompt for a byte number
* to use.
*/
bool
gotoline (f, n, k)
    int f, n, k;
{
    A32 index;
    register int s;
    char buf[32];

    if (f == FALSE)
    {

	if ((s = ereply (MSG_go_b_n, buf, sizeof (buf), 0) != TRUE))
	    return (s);
	switch (R_TYPE (curwp))
	{
	case TEXT:
	case ASCII:
	case EBCDIC:
	case BINARY:
	case HEX:
	    sscanf (buf, MSG_lX, &index);
	    break;
	case OCTAL:
	    sscanf (buf, MSG_lO, &index);
	    break;
	case DECIMAL:
#if	FLOAT_DISP
	case FLOAT:
#endif
	    sscanf (buf, MSG_lD, &index);
	    break;
#if RUNCHK
	default:
	    writ_echo (ERR_bas_1);
	    break;
#endif
	}
    }

    if (n <= 0)
    {
	writ_echo (MSG_bad_num);
	return (FALSE);
    }

    move_ptr (curwp, index, TRUE, TRUE, FALSE);
    curwp->w_unit_offset = 0;

    curwp->w_flag |= WFMODE;	/* update mode line */

    wind_on_dot (curwp);
    return (TRUE);
}
