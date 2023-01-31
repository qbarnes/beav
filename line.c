/*
*       Text line handling.
* The functions in this file
* are a general set of line management
* utilities. They are the only routines that
* touch the text. They also touch the buffer
* and window structures, to make sure that the
* necessary updating gets done. There are routines
* in this file that handle the kill buffer too.
* It isn't here for any good reason.
*
* Note that this code only updates the dot and
* mark values in the window list. Since all the code
* acts on the current window, the buffer that we
* are editing must be being displayed, which means
* that "b_nwnd" is non zero, which means that the
* dot and mark values in the buffer headers are
* nonsense.
*/

#include    "def.h"

void l_fix_up ();

extern char MSG_cnt_alloc[];
#if RUNCHK
extern char ERR_no_alloc[];
extern char ERR_db_dalloc[];
extern char ERR_lock[];
extern char ERR_lock_del[];
#endif

extern LINE *cur_pat;
extern LINE *cur_mask;
extern bool read_pat_mode;
extern BUFFER sav_buf;

/*
* This routine allocates a block
* of memory large enough to hold a LINE
* containing "size" characters. Return a pointer
* to the new block, or NULL if there isn't
* any memory left. Print a message in the
* message line if no space.
*/
LINE *
lalloc (size)
    register int size;
{
    register LINE *lp;
    char buf[NCOL], buf1[NCOL];
#if RUNCHK
    if (read_pat_mode)
	printf (ERR_no_alloc);
#endif

    if ((lp = (LINE *) malloc (sizeof (LINE) + size)) == NULL)
    {
	sprintf (buf1, MSG_cnt_alloc, R_POS_FMT (curwp));
	sprintf (buf, buf1, (A32) size);
	err_echo (buf);
	curbp->b_flag |= BFBAD;	/* may be trashed */
	curwp->w_flag |= WFMODE;
	update ();
	return (NULL);
    }
    lp->l_size = size;
    lp->l_used = 0;
    lp->l_file_offset = 0;	/* set resonable initial value */
    return (lp);
}

/*
* Delete line "lp". Fix all of the
* links that might point at it (they are
* moved to offset 0 of the next line.
* Unlink the line from whatever buffer it
* might be in. Release the memory. The
* buffers are updated too; the magic conditions
* described in the above comments don't hold
* here.
*/

void
lfree (lp)
    register LINE *lp;
{
    register BUFFER *bp;
    register WINDOW *wp;

#if RUNCHK
    if (read_pat_mode)
	printf (ERR_db_dalloc);
#endif

    wp = wheadp;
    while (wp != NULL)
    {
	if (wp->w_linep == lp)
	{
	    wp->w_linep = lp->l_fp;
	    wp->w_loff = 0;
	}

	if (wp->w_dotp == lp)
	{
	    wp->w_dotp = lp->l_fp;
	    wp->w_doto = 0;
	}

	if (wp->w_markp == lp)
	{
	    wp->w_markp = lp->l_fp;
	    wp->w_marko = 0;
	}

	wp = wp->w_wndp;
    }

    bp = bheadp;
    while (bp != NULL)
    {
	if (bp->b_nwnd == 0)
	{
	    if (bp->b_dotp == lp)
	    {
		bp->b_dotp = lp->l_fp;
		bp->b_doto = 0;
	    }

	    if (bp->b_markp == lp)
	    {
		bp->b_markp = lp->l_fp;
		bp->b_marko = 0;
	    }
	}
	bp = bp->b_bufp;
    }

    lp->l_bp->l_fp = lp->l_fp;
    lp->l_fp->l_bp = lp->l_bp;
    free ((char *) lp);
}

/*
* This routine gets called when
* a character is changed in place in the
* current buffer. It updates all of the required
* flags in the buffer and window system. The flag
* used is passed as an argument; if the buffer is being
* displayed in more than 1 window we change EDIT to
* HARD. Set MODE if the mode line needs to be
* updated (the "*" has to be set).
*/
void
lchange (flag)
    register int flag;
{
    register WINDOW *wp;

    if (curbp->b_nwnd != 1)	/* Ensure hard.     */
	flag = WFHARD;
    if ((curbp->b_flag & BFCHG) == 0)
    {
	/* First change, so     */
	flag |= WFMODE;		/* update mode lines.   */
	curbp->b_flag |= BFCHG;
    }

    wp = wheadp;
    while (wp != NULL)
    {
	if (wp->w_bufp == curbp)
	    wp->w_flag |= flag;
	wp = wp->w_wndp;
    }
}

/*
 *  Break the line "dotp" in two at the position "doto."
 */

LINE *
l_break_in_two (lp, lo, extra)
    register LINE *lp;
    register LPOS lo, extra;
{
    register LINE *new_lp;
    register D8 *cp1;
    register D8 *cp2;
    LPOS cnt, i;

    i = 0;
    cnt = lp->l_used - lo;
    if ((new_lp = lalloc (cnt + extra)) == NULL)
	return (NULL);

    cp1 = &lp->l_text[lo];	/* starting location, source */
    cp2 = &new_lp->l_text[0];	/* starting location, destination */

    /* kill bytes in the current line */
    while (i++ < cnt)
    {
	*cp2++ = *cp1++;
    }
    lp->l_used -= cnt;
    new_lp->l_used = cnt;
    new_lp->l_file_offset = new_lp->l_file_offset + lo;

    /* insert into chain */
    new_lp->l_fp = lp->l_fp;
    lp->l_fp = new_lp;
    new_lp->l_bp = lp;
    new_lp->l_fp->l_bp = new_lp;
    return (new_lp);
}

/*
* Insert "n" copies of the character "c"
* at the current location of dot. In the easy case
* all that happens is the text is stored in the line.
* Always allocate some extra space in line so that edit
* will be faster next time but will save space in the general case.
* In the hard case, the line has to be reallocated.
* When the window list is updated, take special
* care; I screwed it up once. You always update dot
* in the current window. You update mark, and a
* dot in another window, if it is greater than
* the place where you did the insert. Return TRUE
* if all is well, and FALSE on errors.
*/
bool
linsert (n, c)
    uchar c;
    int n;
{
    register D8 *cp1;
    register D8 *cp2;
    register LINE *lp1;
    register LINE *lp2;
    register short doto;
    register int i;
    register WINDOW *wp;

#if RUNCHK
    /* check that buffer size can be changed */
    if (curbp->b_flag & BFSLOCK)
    {
	writ_echo (ERR_lock);
	return (FALSE);
    }
#endif

    lchange (WFMOVE);
    lp1 = curwp->w_dotp;	/* Current line     */
    if (lp1 == curbp->b_linep)
    {
	/* At the end: special  */
	/* break the current line at the end */
	if ((lp2 = l_break_in_two (lp1, lp1->l_used, (LPOS) n + NBLOCK)) == NULL)
	    return (FALSE);
	for (i = 0; i < n; ++i)	/* Add the characters   */
	    lp2->l_text[i] = c;
	lp2->l_used = n;
	curwp->w_dotp = lp2;
	curwp->w_doto = n;
	return (TRUE);
    }

    doto = curwp->w_doto;	/* Save for later.  */
    if (lp1->l_used + n > lp1->l_size)
    {
	/* break the current line and let the normal insert do it */
	if ((lp2 = l_break_in_two (lp1, doto, (LPOS) n + NBLOCK)) == NULL)
	    return (FALSE);
	lp1->l_text[doto] = c;
	lp1->l_used++;
	curwp->w_doto++;
	if (curwp->w_doto >= lp1->l_used)
	{
	    curwp->w_dotp = lp2;
	    curwp->w_doto = 0;
	}
	if (n > 1)
	    return (linsert (n - 1, c));	/* handle the rest in normal maner */
    }
    else
    {
	/* Easy: in place   */
	lp2 = lp1;		/* Pretend new line */
	lp2->l_used += n;
	cp2 = &lp1->l_text[lp1->l_used];
	cp1 = cp2 - n;
	while (cp1 != &lp1->l_text[doto])
	    *--cp2 = *--cp1;
	for (i = 0; i < n; ++i)	/* Add the characters   */
	    lp2->l_text[doto + i] = c;
	move_ptr (curwp, (A32) n, TRUE, TRUE, TRUE);
    }

    wp = wheadp;		/* Update windows   */
    while (wp != NULL)
    {
	if ((wp->w_linep == lp1) && (wp->w_loff >= lp1->l_used))
	{
	    wp->w_linep = lp2;
	    wp->w_loff -= lp1->l_used;
	}

	/* move dot to next line but not to head line */
	if ((wp->w_dotp == lp1) && (wp->w_doto >= lp1->l_used) &&
	    (wp->w_dotp->l_fp->l_size != 0))
	{
	    wp->w_dotp = lp2;
	    wp->w_doto -= (lp1->l_used - 1);
	}

	if ((wp->w_markp == lp1) && (wp->w_marko >= lp1->l_used))
	{
	    wp->w_markp = lp2;
	    wp->w_marko -= (lp1->l_used - 1);
	}

	wp = wp->w_wndp;
    }
    l_fix_up (lp1);		/* re-adjust file offsets */
    return (TRUE);
}

/*
* This function deletes n_bytes,
* starting at dot. It understands how to deal
* with end of lines, etc. It returns TRUE if all
* of the characters were deleted, and FALSE if
* they were not (because dot ran into the end of
* the buffer). The "kflag" is TRUE if the text
* should be put in the kill buffer.
*/
bool
ldelete (n_bytes, kflag)
    A32 n_bytes;
    int kflag;
{
    register LINE *dotp, *lp, *lp_prev, *lp_next;
    register LPOS doto, l_cnt;
    register WINDOW *wp;
    D8 *cp1, *cp2;
    D32 dot_pos;
    uint n_byt;

#if RUNCHK
    /* check that buffer size can be changed */
    if (curbp->b_flag & BFSLOCK)
    {
	writ_echo (ERR_lock_del);
	return (FALSE);
    }
#endif
    lchange (WFMOVE);
    doto = curwp->w_doto;
    dotp = curwp->w_dotp;
    lp_prev = dotp->l_bp;
    dot_pos = DOT_POS (curwp);

    /* if at the end of the buffer then delete nothing */
    if (dot_pos >= BUF_SIZE (curwp))
    {
	l_fix_up (dotp);	/* re-adjust file offsets */
	return (TRUE);
    }

    /* save dot and mark positions for later restore */
    wp = wheadp;
    while (wp != NULL)
    {
	wp->w_dot_temp = DOT_POS (wp);
	if (wp->w_markp != NULL)/* mark may not be set */
	    wp->w_mark_temp = MARK_POS (wp);
	wp->w_wind_temp = WIND_POS (wp);
	wp = wp->w_wndp;
    }

    /* is delete wholy within one line? */
    if ((doto + n_bytes) <= dotp->l_used)
    {
	cp1 = &dotp->l_text[doto];	/* Scrunch text.    */
	cp2 = cp1 + n_bytes;

	/* put stuff to delete into the kill buffer */
	if (kflag != FALSE)
	{
	    /* Kill?        */
	    while (cp1 != cp2)
	    {
		if (b_append_c (&sav_buf, *cp1) == FALSE)
		    return (FALSE);
		++cp1;
	    }
	    cp1 = &dotp->l_text[doto];
	}
	/* kill bytes in the current line */
	while (cp2 < &dotp->l_text[dotp->l_used])
	    *cp1++ = *cp2++;

	dotp->l_used -= n_bytes;
    }
    else
    {				/* wholesale delete by moving lines to save buffer */
	if (doto != 0)
	{
	    if ((lp = l_break_in_two (dotp, doto, 0)) == NULL)
		return (FALSE);
	}
	else
	    lp = dotp;

	n_byt = n_bytes;
	/* now handle whole lines if necessary */
	while (n_byt > 0)
	{
	    lp_next = lp->l_fp;

	    if (n_byt < lp->l_used)
	    {
		/* get last piece of a line */
		lp_next = l_break_in_two (lp, n_byt, 0);
	    }
	    n_byt -= lp->l_used;
	    if (kflag)
	    {
		/* remove form linked list */
		lp->l_bp->l_fp = lp->l_fp;
		lp->l_fp->l_bp = lp->l_bp;
		/* append it to the save buffer */
		b_append_l (&sav_buf, lp);
	    }
	    else
		/* if we don't want it, free it */
		lfree (lp);
	    lp = lp_next;
	}
    }
    l_fix_up (lp_prev);		/* re-adjust file offsets */

    /* adjust dot and marks in other windows */
    /* this should be ok because the save buffer dosn't disturb l_file_offset */
    wp = wheadp;		/* Fix windows      */
    while (wp != NULL)
    {
	if (curbp == wp->w_bufp)
	{
	    A32 temp;

	    /* if dot is before delete position, do nothing */
	    if (dot_pos <= (temp = wp->w_dot_temp))
	    {
		/* step back to the previous line */
		wp->w_doto = 0;
		wp->w_dotp = lp_prev;

		/* if dot is in deleted range, set to dot position */
		if (temp > dot_pos + n_bytes)
		    /* if after deleted range, move back deleted ammount */
		    move_ptr (wp, temp - n_bytes, TRUE, TRUE, FALSE);
		else
		    /* if in the deleted range, move to curwp dot position */
		    move_ptr (wp, dot_pos, TRUE, TRUE, FALSE);
	    }
	    /* mark may not be set in some windows */
	    if (wp->w_markp != NULL)
	    {
		/* do the same for mark */
		if (dot_pos <= (temp = wp->w_mark_temp))
		{
		    /* if in or after the deleted range, move to curwp dot position */
		    wp->w_marko = curwp->w_doto;
		    wp->w_markp = curwp->w_dotp;

		    /* if mark after deleted range */
		    if (temp > dot_pos + n_bytes)
		    {
			/* if after deleted range, move back deleted ammount */
			/* move dot then swap with mark to produce result */
			move_ptr (wp, temp - n_bytes, TRUE, TRUE, FALSE);
			lp_next = wp->w_dotp;
			wp->w_dotp = wp->w_markp;
			wp->w_markp = lp_next;
			l_cnt = wp->w_doto;
			wp->w_doto = wp->w_marko;
			wp->w_marko = l_cnt;
		    }
		}
	    }
	    /* if window position is before delete position, do nothing */
	    if (dot_pos <= (temp = wp->w_wind_temp))
	    {
		/* set window position to dot position */
		wp->w_loff = 0;
		wp->w_linep = wp->w_dotp;
		wind_on_dot (wp);
	    }
	}
	wp = wp->w_wndp;
    }
    /* update buffer display */
    if ((blistp->b_nwnd != 0) &&
	(blistp->b_type == BTLIST))
	listbuffers ();
    return (TRUE);
}

/*
*   Replace character at dot position.
*/
void
lreplace (n, c)
    int n;
    char c;
{
    lchange (WFEDIT);
    while (n--)
    {
	DOT_CHAR (curwp) = c & 0xff;
	move_ptr (curwp, 1L, TRUE, FALSE, TRUE);
    }
}

/*
* Replace plen characters before dot with argument string.
*/
bool
lrepl_str (plen, rstr, mstr)

    register int plen;		/* length to remove     */
    register LINE *rstr;	/* replace string       */
    register LINE *mstr;	/* mask string       */
{
    register int i;		/* used for random characters   */
    register A32 dot_pos;	/* dot offset into buffer     */
    register int rlen;		/* rplace string length */
    register char c;		/* temp storage for char */
    register char mask;		/* temp storage for mask */

    /*
  * make the string lengths match (either pad the line
  * so that it will fit, or scrunch out the excess).
  * be careful with dot's offset.
  */
    /* get offset from begining of buffer */
    dot_pos = DOT_POS (curwp);
    rlen = rstr->l_used;
    if (plen > rlen)
    {
	ldelete ((A32) (plen - rlen), FALSE);
    }
    else if (plen < rlen)
    {
	if (linsert (rlen - plen, ' ') == FALSE)
	    return (FALSE);
    }
    /* must use move_ptr because delete may advance to next line */
    move_ptr (curwp, dot_pos, TRUE, FALSE, FALSE);

    /* do the replacement. */
    for (i = 0; i < rlen; i++)
    {
	c = DOT_CHAR (curwp);
	mask = mstr->l_text[i];
	DOT_CHAR (curwp) = (c & mask) | (rstr->l_text[i] & ~mask);
	move_ptr (curwp, 1L, TRUE, FALSE, TRUE);
    }
    move_ptr (curwp, dot_pos, TRUE, FALSE, FALSE);
    lchange (WFHARD);
    return (TRUE);
}

/*
*   Line fixup.
*   This fixes the 'l_file_offset' variable in
*   each line structure.
*   This is necessary after every change in the size
*   of the buffer.
*/
void
l_fix_up (line)

    LINE *line;			/* points to buffer header line */

{
    long offset;

    offset = line->l_file_offset;	/* starting offset */
    offset += line->l_used;
    for (;;)
    {
	line = line->l_fp;
	if (line->l_size == 0)
	    return;
	line->l_file_offset = offset;
	offset += line->l_used;
    }
}
