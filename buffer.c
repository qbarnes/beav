/*
*       Buffer handling.
*/

#include    "def.h"

bool onebuf ();
bool killablebufs ();
bool _yankbuffer ();
char next_buf ();
bool bclear ();
bool addline ();
char makelist ();
bool popblist ();
char listbuffers ();
char _killbuffer ();
bool _usebuffer ();

extern ROW_FMT text_fmt;
extern char MSG_use_b[];
extern char MSG_kill_b[];
extern char MSG_not_fnd[];
extern char MSG_no_del_m[];
extern char MSG_buf_disp[];
extern char MSG_main[];
extern char MSG_l_buf_h[];
extern char MSG_l_buf_h1[];
extern char MSG_no_chg[];
extern char MSG_yank_b[];
extern char MSG_no_buf[];
extern char MSG_no_s_yank[];
extern char MSG_buf_nam[];
extern char MSG_bad_l[];
extern char MSG_pick[];
extern char MSG_siz_chg[];
extern char MSG_no_siz_chg[];
extern char MSG_up_arrow[];
extern char MSG_null[];
extern char MSG_save_buf[];
extern char MSG_cnt_al_b[];
extern char MSG_ins_cnt[];

BUFFER sav_buf;
LINE sav_line_h;
/*
* Attach a buffer to a window. The
* values of dot and mark come from the buffer
* if the use count is 0. Otherwise, they come
* from some other window.
*
* plus hacks for prev/next buffer and use-buffer-split  (jam)
* functions (like in file.c)
*/
char
usebuffer ()
{

    char bufn[NBUFN];
    register char s;

    if ((s = ereply (MSG_use_b, bufn, NBUFN, 0)) != TRUE)
	return (s);
    return (_usebuffer (bufn));
}

/* use buffer, split window first
*/
char
use_buffer ()
{
    char bufn[NBUFN];
    register char s;

    if ((s = ereply (MSG_use_b, bufn, NBUFN, 0)) != TRUE)
	return (s);
    splitwind ();
    return (_usebuffer (bufn));
}

/* does all the work for changing to a new buffer for use-buffer,
* use-buffer-split and prev-buff & next-buff
*/
bool
_usebuffer (bufn)
    char *bufn;
{
    register BUFFER *bp;
    register WINDOW *wp;

    if (strcmp (MSG_kill_b, bufn) == 0)	/* hack! */
	bp = blistp;
    else if ((bp = bfind (bufn, TRUE)) == NULL)
	return (FALSE);

    /* if current buffer is special and new buffer is normal */
    /* set to hex byte mode */
    if ((curbp == blistp) && (R_TYPE (curwp) == TEXT))
    {
	dispsize1 ();
	hexmode ();
    }

    if (--curbp->b_nwnd == 0)
    {
	/* Last use.         */
	curbp->b_dotp = curwp->w_dotp;
	curbp->b_doto = curwp->w_doto;
	curbp->b_unit_offset = curwp->w_unit_offset;	/* pvr */
	curbp->b_markp = curwp->w_markp;
	curbp->b_marko = curwp->w_marko;
    }
    curbp = bp;			/* Switch.       */
    curwp->w_bufp = bp;
    curwp->w_linep = bp->b_linep;	/* For macros, ignored.     */
    curwp->w_loff = 0;		/* pvr */
    curwp->w_flag |= WFMODE | WFFORCE | WFHARD;
    /* Quite nasty.      */
    if (bp->b_nwnd++ == 0)
    {
	/* First use.        */
	curwp->w_dotp = bp->b_dotp;
	curwp->w_doto = bp->b_doto;
	curwp->w_unit_offset = 0;	/* pvr */
	curwp->w_markp = bp->b_markp;
	curwp->w_marko = bp->b_marko;
	wind_on_dot (curwp);
	/* if we are in the funny TEXT mode then goto standard HEX mode */
	if (R_TYPE (curwp) == TEXT)
	    hexmode ();
	return (TRUE);
    }
    wp = wheadp;		/* Look for old.     */
    while (wp != NULL)
    {
	if (wp != curwp && wp->w_bufp == bp)
	{
	    curwp->w_dotp = wp->w_dotp;
	    curwp->w_doto = wp->w_doto;
	    curwp->w_unit_offset = wp->w_unit_offset;	/* pvr */
	    curwp->w_markp = wp->w_markp;
	    curwp->w_marko = wp->w_marko;
	    break;
	}
	wp = wp->w_wndp;
    }
    wind_on_dot (curwp);
    /* if we are in the funny TEXT mode then goto standard HEX mode */
    if (R_TYPE (curwp) == TEXT)
	hexmode ();
    return (TRUE);
}

/*
* Dispose of a buffer, by name.
* Ask for the name. Look it up (don't get too
* upset if it isn't there at all!). Get quite upset
* if the buffer is being displayed. Clear the buffer (ask
* if the buffer has been changed). Then free the header
* line and the buffer header. Bound to "C-X K".
*/
char
killbuffer ()
{
    register char s;
    char bufn[NBUFN];

    if ((s = ereply (MSG_kill_b, bufn, NBUFN, 0)) != TRUE)
	return (s);
    if (s = _killbuffer (bufn))
	writ_echo (okmsg);	/* verbose-ness (jam) */
    return (s);
}

char
_killbuffer (bufn)
    char *bufn;
{
    register BUFFER *bp, *bp1, *bp2;
    register char s, x = 0;

    if (((bp = bfind (bufn, FALSE)) == NULL) ||
	!strcmp (bufn, MSG_save_buf))
    {
	writ_echo (MSG_not_fnd);
	return (FALSE);
    }


    if (killablebufs (bp))	/* can't kill '?' if no other buffers */
    {
	writ_echo (MSG_no_del_m);
	return (FALSE);
    }

    /* see if the buffer to be killed is in a window */
    bp1 = bp;
    if (curbp == blistp && onebuf (bp))	/* Hack ! */
    {
	next_buf ();
	onlywind ();
	update ();
    }

    if (bp->b_nwnd > 0)
    {
	if ((s = eyesno (MSG_buf_disp)) != TRUE)
	    return (s);

	/* make the current window the only window if it is to die */
	onlywind ();
	if (curbp == bp)
	{
	    next_buf ();
	    if (curbp == bp)
		x++;
	}
    }
    if ((s = bclear (bp)) != TRUE)	/* Blow text away.      */
    {
	if (bp1 == blistp)	/* special buffer */
	    curbp = bp1;
	else if (!x)
	    _usebuffer (bp1->b_bname);
	/* back to original buffer (jam) */
	return (s);
    }
    if (x)
    {
	_usebuffer (MSG_main);
	x++;
    }

    free ((char *) bp->b_linep);/* Release header line.         */
    bp1 = NULL;			/* Find the header.     */
    bp2 = bheadp;
    while (bp2 != bp)
    {
	bp1 = bp2;
	bp2 = bp2->b_bufp;
    }
    bp2 = bp2->b_bufp;		/* Next one in chain.   */
    if (bp1 == NULL)		/* Unlink it.           */
	bheadp = bp2;
    else
	bp1->b_bufp = bp2;
    free ((char *) bp);		/* Release buffer block         */
    if (x)
	update ();
    /* update buffer display */
    if ((blistp->b_nwnd != 0) &&
	(blistp->b_type == BTLIST))
	listbuffers ();
    return (TRUE);
}

/*
* Display the buffer list. This is done
* in two parts. The "makelist" routine figures out
* the text, and puts it in the buffer whoses header is
* pointed to by the external "blistp". The "popblist"
* then pops the data onto the screen. Bound to
* "C-X C-B".
*/
char
listbuffers ()
{
    register char s;

    if ((s = makelist ()) != TRUE)
	return (s);
    return (popblist ());
}

/*
* Display the save buffer contents.
* Bound to "Meta C-W".
*/
char
showsavebuf ()
{
    WINDOW *wp;

    if (sav_buf.b_nwnd == 0)
    {
	splitwind ();
	_usebuffer (MSG_save_buf);
    }
    else
    {
	wp = wheadp;		/* Look for old.     */
	while (wp != NULL)
	{
	    if (wp->w_bufp == &sav_buf)
	    {
		wp->w_flag |= WFMODE | WFFORCE | WFHARD;
		break;
	    }
	    wp = wp->w_wndp;
	}
    }
    return (TRUE);
}

/*
* Pop the special buffer whose
* buffer header is pointed to by the external
* variable "blistp" onto the screen. This is used
* by the "listbuffers" routine (above) and by
* some other packages. Returns a status.
*/
bool
popblist ()
{
    register WINDOW *wp;
    register BUFFER *bp;

    if (blistp->b_nwnd == 0)	/* Not on screen yet.    */
    {
	if ((wp = wpopup ()) == NULL)
	    return (FALSE);
	bp = wp->w_bufp;
	if (--bp->b_nwnd == 0)
	{
	    bp->b_dotp = wp->w_dotp;
	    bp->b_doto = wp->w_doto;
	    bp->b_unit_offset = wp->w_unit_offset;	/* pvr */
	    bp->b_markp = wp->w_markp;
	    bp->b_marko = wp->w_marko;
	}
	curwp = wp;
	curbp = blistp;
	wp->w_bufp = blistp;
	++blistp->b_nwnd;
    }
    wp = wheadp;
    while (wp != NULL)
    {
	if (wp->w_bufp == blistp)
	{
	    wp->w_linep = lforw (blistp->b_linep);
	    wp->w_loff = 0;
	    wp->w_dotp = lforw (blistp->b_linep);
	    wp->w_doto = 0;
	    wp->w_unit_offset = 0;
	    wp->w_markp = NULL;
	    wp->w_marko = 0;
	    wp->w_disp_shift = 0;
	    wp->w_intel_mode = FALSE;
	    wp->w_fmt_ptr = &text_fmt;
	    wp->w_flag |= WFMODE | WFHARD;
	}
	wp = wp->w_wndp;
    }
    return (TRUE);
}

/*
* This routine rebuilds the
* text in the special secret buffer
* that holds the buffer list. It is called
* by the list buffers command. Return TRUE
* if everything works. Return FALSE if there
* is an error (if there is no memory).
*/
char
makelist ()
{
    register char *cp1;
    register char *cp2;
    register int c;
    register BUFFER *bp;
    register A32 nbytes;
    register char s;
    char b[8 + 1];
    char line[128];

    blistp->b_flag &= ~BFCHG;	/* Blow away old.    */
    if ((s = bclear (blistp)) != TRUE)
	return (s);
    blistp->b_flag |= BFVIEW;
    blistp->b_type = BTLIST;
    strcpy (blistp->b_fname, MSG_up_arrow);
    if (addline (MSG_l_buf_h) == FALSE
	|| addline (MSG_l_buf_h1) == FALSE)
	return (FALSE);
    bp = bheadp;		/* For all buffers   */
    while (bp != NULL)
    {
	cp1 = &line[0];		/* Start at left edge    */
	if ((bp->b_flag & BFCHG) != 0)	/* "*" if changed    */
	    *cp1++ = '*';
	else if (bp->b_flag & BFVIEW)	/* jam */
	    *cp1++ = 'R';	/* readonly */
	else
	    *cp1++ = ' ';
	*cp1++ = ' ';		/* Gap. */
	if ((bp->b_flag & BFBAD) != 0)	/* "?" if maybe trashed  */
	    *cp1++ = '?';
	else
	    *cp1++ = ' ';
	*cp1++ = ' ';		/* Gap. */
	nbytes = bp->b_linep->l_bp->l_file_offset +
	    bp->b_linep->l_bp->l_used;
	sprintf (b, "%8lx", nbytes);	/* 8 digit buffer size.   */
	cp2 = &b[0];
	while ((c = *cp2++) != 0)
	    *cp1++ = c;
	*cp1++ = ' ';		/* Gap.          */
	cp2 = &bp->b_bname[0];	/* Buffer name       */
	while ((c = *cp2++) != 0)
	    *cp1++ = c;
	*cp1++ = ' ';		/* Gap.          */
	*cp1++ = ' ';		/* Gap.          */
	cp2 = &bp->b_fname[0];	/* File name         */
	if (*cp2 != 0)
	{
	    while (cp1 < &line[1 + 1 + 1 + 1 + 6 + 1 + NBUFN + 1])
		*cp1++ = ' ';
	    while ((c = *cp2++) != 0)
	    {
		if (cp1 < &line[128 - 1])
		    *cp1++ = c;
	    }
	}
	while (cp1 < &line[80])	/* Fill out line to col 80 */
	    *cp1++ = ' ';

	*cp1 = 0;		/* Add to the buffer.    */
	if (addline (line) == FALSE)
	    return (FALSE);
	bp = bp->b_bufp;
    }
    return (TRUE);		/* All done      */
}

/*
* The argument "text" points to
* a string. Append this line to the
* buffer list buffer.
* Return TRUE if it worked and
* FALSE if you ran out of room.
*/
bool
addline (text)
    char *text;
{
    register LINE *lp;
    register int i, allocsize;
    register int ntext;

    ntext = strlen (text);
    allocsize = 128;

    if ((lp = lalloc (allocsize)) == NULL)
	return (FALSE);

    for (i = 0; i < ntext; ++i)
	lputc (lp, i, text[i]);

    for (; i < allocsize; ++i)	/* fill out line with spaces */
	lputc (lp, i, ' ');

    blistp->b_linep->l_bp->l_fp = lp;	/* Hook onto the end  */
    lp->l_bp = blistp->b_linep->l_bp;
    blistp->b_linep->l_bp = lp;
    lp->l_fp = blistp->b_linep;
    lp->l_size = allocsize;	/* line size is limited to 80 chars */
    lp->l_used = allocsize;
    lp->l_file_offset = lp->l_bp->l_file_offset + lp->l_bp->l_used;
    if (blistp->b_dotp == blistp->b_linep)	/* If "." is at the end    */
	blistp->b_dotp = lp;	/* move it to new line   */
    return (TRUE);
}

/*
* Look through the list of
* buffers. Return TRUE if there
* are any changed buffers. Special buffers
* like the buffer list buffer don't count, as
* they are not in the list. Return FALSE if
* there are no changed buffers.
*/
bool
anycb ()
{
    register BUFFER *bp;

    bp = bheadp;
    while (bp != NULL)
    {

	if ((bp->b_flag & BFCHG) != 0)
	    return (TRUE);
	bp = bp->b_bufp;
    }
    return (FALSE);
}

/*
* Search for a buffer, by name.
* If not found, and the "cflag" is TRUE,
* create a buffer and put it in the list of
* all buffers. Return pointer to the BUFFER
* block for the buffer.
*/
BUFFER *
bfind (bname, cflag)
    register char *bname;
    int cflag;
{
    register BUFFER *bp;

    bp = bheadp;
    while (bp != NULL)
    {
	if (strcmp (bname, bp->b_bname) == 0)
	    return (bp);
	bp = bp->b_bufp;
    }
    if (cflag != FALSE && (bp = bcreate (bname)) != NULL)
    {
	bp->b_bufp = bheadp;
	bheadp = bp;
    }
    return (bp);
}

/*
* Create a buffer, by name.
* Return a pointer to the BUFFER header
* block, or NULL if the buffer cannot
* be created. The BUFFER is not put in the
* list of all buffers; this is called by
* "edinit" to create the buffer list
* buffer.
*/
BUFFER *
bcreate (bname)
    register char *bname;
{

    register BUFFER *bp;
    register LINE *lp;

    if ((bp = (BUFFER *) malloc (sizeof (BUFFER))) == NULL)
    {
	err_echo (MSG_cnt_al_b);
	return (NULL);
    }
    if ((lp = lalloc (0)) == NULL)
    {
	free ((char *) bp);
	return (NULL);
    }
    bp->b_bufp = NULL;
    bp->b_dotp = lp;
    bp->b_doto = 0;
    bp->b_unit_offset = 0;	/* unit offset   pvr */
    bp->b_markp = NULL;
    bp->b_marko = 0;
    bp->b_flag = 0;
    bp->b_nwnd = 0;
    bp->b_linep = lp;
    strcpy (bp->b_fname, MSG_null);
    strcpy (bp->b_bname, bname);
    lp->l_fp = lp;
    lp->l_bp = lp;
    lp->l_file_offset = 0;	/* pvr */
    lp->l_used = 0;		/* pvr */
    lp->l_size = 0;		/* size of zero indicates the header line  pvr
							                                    */
    return (bp);
}

/*
* This routine blows away all of the text
* in a buffer. If the buffer is marked as changed
* then we ask if it is ok to blow it away; this is
* to save the user the grief of losing text. The
* window chain is nearly always wrong if this gets
* called; the caller must arrange for the updates
* that are required. Return TRUE if everything
* looks good.
*/
bool
bclear (bp)
    register BUFFER *bp;
{
    register LINE *lp;
    register char s;

    if ((bp->b_flag & BFCHG) != 0	/* Changed.       */
	&& (s = eyesno (MSG_no_chg)) != TRUE)
	return (s);
    bp->b_flag &= ~BFCHG;	/* Not changed       */
    while ((lp = lforw (bp->b_linep)) != bp->b_linep)
	lfree (lp);
    bp->b_dotp = bp->b_linep;	/* Fix "."      */
    bp->b_doto = 0;
    bp->b_unit_offset = 0;	/* pvr */
    bp->b_markp = NULL;		/* Invalidate mark  */
    bp->b_marko = 0;
    return (TRUE);
}

/* flip to next buffer in the list, wrap
* to beginning if required (wrap around)
* (skips buffers saved  by save-region)
*/
char
next_buf ()
{
    register BUFFER *bp;

    bp = curbp;
    while (TRUE)
    {
	if (!(bp = bp->b_bufp))
	    bp = bheadp;
	if ((bp->b_type == BTSAVE) ||
	    (bp->b_type == BTLIST) ||
	    (bp->b_type == BTHELP))
	    continue;
	break;
    }
    _usebuffer (bp->b_bname);
    return (TRUE);
}

/* flip to prev buffer in the list, wrap
* to end if required (wrap around)
* (does NOT skip buffers saved by save-region)
*/
char
prev_buf ()
{
    register BUFFER *sp;

    if ((sp = curbp) == bheadp)	/* front of list */
    {
	for (; sp->b_bufp; sp = sp->b_bufp)
	    ;
    }
    else
	/* cycle around */
    {
	for (sp = bheadp; sp->b_bufp; sp = sp->b_bufp)
	{
	    if (sp->b_bufp == curbp)
		break;
	}
    }
    return (_usebuffer (sp->b_bname));
}

/* yank a buffer into current buffer
*/
char
yank_buffer ()
{
    char bufn[NBUFN];

    if (ereply (MSG_yank_b, bufn, NBUFN, 0) != TRUE)
	return (FALSE);
    return (_yankbuffer (bufn));
}

bool
_yankbuffer (bufn)
    char *bufn;
{
    register LINE *lp;
    register BUFFER *bp = curbp;
    register int s;
    A32 cnt;
    char buf[NFILEN], buf1[NFILEN];

    if ((bp = bfind (bufn, FALSE)) == NULL)
    {
	writ_echo (MSG_no_buf);
	return (FALSE);
    }
    if (strcmp (bp->b_bname, curbp->b_bname) == 0)
    {
	writ_echo (MSG_no_s_yank);
	return (FALSE);
    }
    cnt = 0;
    lp = lforw (bp->b_linep);
    while (TRUE)
    {
	cnt += lp->l_used;
	for (s = 0; s < lp->l_used; s++)
	    if (linsert (1, lp->l_text[s]) == FALSE)
		return (FALSE);

	if ((lp = lforw (lp)) == bp->b_linep)
	{
	    break;
	}

	if ((cnt & 0x7ff) == 0)
	{
	    sprintf (buf1, MSG_ins_cnt, R_POS_FMT (curwp));
	    sprintf (buf, buf1, cnt);
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		l_fix_up (lp->l_bp);
		wind_on_dot_all ();
		if (ttgetc () == CTL_G)	/* was it an abort key? */
		    return (FALSE);
	    }
	}
    }
    writ_echo (okmsg);
    return (TRUE);
}

bool
buffername ()
{

    register WINDOW *wp;
    register char *p;
    register char s;
    char bname[NBUFN + 1];

    if ((s = ereply (MSG_buf_nam, bname, NBUFN, 0)) == ABORT)
	return (s);
    for (p = bname; *p && *p != ' '; p++)
	;
    *p = 0;			/* no blanks */
    strcpy (curbp->b_bname, bname);
    wp = wheadp;		/* Update mode lines.   */
    while (wp != NULL)
    {
	if (wp->w_bufp == curbp)
	    wp->w_flag |= WFMODE;
	wp = wp->w_wndp;
    }
    if ((blistp->b_nwnd != 0) &&/* update buffer display */
	(blistp->b_type == BTLIST))
	listbuffers ();
    return (TRUE);
}

/* any killable buffers around ? (jam)
*/
bool
killablebufs (bp)
    register BUFFER *bp;
{
    if (strcmp (bp->b_bname, MSG_main) == 0)	/* doomed buffer is 'empty' */
	if (bheadp == bp)	/* and is only buffer in list */
	    if (bheadp->b_bufp == 0)	/* then there are no killable buffers */
		return (TRUE);
    return (FALSE);
}

/* only 1 buffer around ?
*/
bool
onebuf (bp)
    register BUFFER *bp;
{
    if (strcmp (bp->b_bname, bheadp->b_bname) == 0)
	if (bheadp->b_bufp == 0)
	    return (TRUE);
    return (FALSE);
}

/* funky new name; real yukky!!!! (jam)
*/
void
funky_name (bname, n)
    register char *bname;
    int n;
{
    char num[10];
    register int i;
    register char *p;

    for (i = 0; i < 10; i++)
	num[i] = ' ';
    for (p = bname; *p; p++)
	*p = 0;
    *bname++ = '#';
    sprintf (num, "%lx", (long) n + 1);
    for (p = num; *p; p++)
	if (*p != ' ')
	    *bname++ = *p;
    *bname = 0;
}

/* pick a buffer to goto/kill
*/
#define BUFFEROFFSET (13)	/* depends on makelist !! */

bool
pickone ()
{
    register int s, i, c;
    register LINE *lp;
    char name[NBUFN + 1];
    char buf[3];
    WINDOW *wp;

    lp = curwp->w_dotp;		/* get the buffer name from the line */

    i = 0;
    if (!llength (lp))
    {
	writ_echo (MSG_bad_l);
	return (FALSE);
    }
    for (s = BUFFEROFFSET; (c = lgetc (lp, s)) != ' '; s++)
    {
	name[i++] = c;
	if (s >= llength (lp))
	    break;
    }
    name[i] = 0;
    if (!bfind (name, FALSE))
    {
	writ_echo (MSG_bad_l);
	return (FALSE);
    }
  loop:
    if ((s = ereply (MSG_pick, buf, 2, name)) != TRUE)
	return (FALSE);
    if (ISLOWER (buf[0]) != FALSE)
	buf[0] = TOUPPER (buf[0]);
    if (buf[0] == 'K')
	_killbuffer (name);
    else if (buf[0] == 'G')
	_usebuffer (name);
    else if (buf[0] == 'S')
    {
	_usebuffer (name);
	/* goto this buffer, but don't show the user */
	filesave ();
	_usebuffer (MSG_kill_b);
	/* jump back to this window - HACK ! */
	listbuffers ();		/* update the list */
    }
    else
	goto loop;
    writ_echo (MSG_null);
    return (TRUE);
}

/*
*   Toggle the buffer size lock bit.
*/
char
bufsizlock ()
{
    if (curbp->b_flag & BFSLOCK)
    {
	curbp->b_flag &= ~BFSLOCK;
	writ_echo (MSG_siz_chg);
    }
    else
    {
	if (insert_mode)
	    insert_toggle ();
	curbp->b_flag |= BFSLOCK;
	writ_echo (MSG_no_siz_chg);
    }
    return (TRUE);
}

/*
 *   Append the given line to the end of the given buffer.
 */
void
b_append_l (buf_p, lp)
    BUFFER *buf_p;
    LINE *lp;
{
    LINE *h_lp;

    h_lp = buf_p->b_linep;

    lp->l_fp = h_lp;
    lp->l_bp = h_lp->l_bp;
    lp->l_bp->l_fp = lp;
    h_lp->l_bp = lp;
    lp->l_file_offset = lp->l_bp->l_file_offset + lp->l_bp->l_used;
}

/*
 *   Append the given line to the end of the given buffer.
 */
bool
b_append_c (buf_p, ch)
    BUFFER *buf_p;
    D8 ch;
{
    LINE *lp;

    lp = buf_p->b_linep->l_bp;	/* get last line */
    /* do I need to get a new line? */
    if (lp->l_size <= lp->l_used)
    {
	if ((lp = lalloc (KBLOCK)) == NULL)
	    return (FALSE);

	lp->l_fp = buf_p->b_linep;
	lp->l_bp = buf_p->b_linep->l_bp;
	lp->l_bp->l_fp = lp;
	buf_p->b_linep->l_bp = lp;
	lp->l_file_offset = lp->l_bp->l_file_offset + lp->l_bp->l_used;
    }
    lp->l_text[lp->l_used++] = ch;

    return (TRUE);
}

/*
 * Initialize the save buffer.
 */
void
save_buf_init ()
{
    register BUFFER *bp;

    sav_line_h.l_fp = &sav_line_h;
    sav_line_h.l_bp = &sav_line_h;
    sav_line_h.l_file_offset = 0;
    sav_line_h.l_used = 0;
    sav_line_h.l_size = 0;

    sav_buf.b_type = BTSAVE;
    sav_buf.b_bufp = NULL;
    sav_buf.b_dotp = &sav_line_h;
    sav_buf.b_doto = 0;
    sav_buf.b_unit_offset = 0;
    sav_buf.b_markp = NULL;
    sav_buf.b_marko = 0;
    sav_buf.b_linep = &sav_line_h;
    sav_buf.b_nwnd = 0;
    sav_buf.b_flag = BFVIEW;
    sav_buf.b_begin_addr = 0;
    sav_buf.b_file_size = 0;
    sav_buf.b_fname[0] = 0;
    strcpy (sav_buf.b_bname, MSG_save_buf);

    /* put on end of chain */
    bp = bheadp;
    while ((bp->b_bufp) != NULL)
	bp = bp->b_bufp;

    bp->b_bufp = &sav_buf;

}

/*
 * Set the save buffer dot pointer to the begining.
 */
void
save_buf_home ()
{
    sav_buf.b_dotp = sav_buf.b_linep->l_fp;
    sav_buf.b_doto = 0;
    sav_buf.b_flag = BFVIEW;
}

D16
get_save_char ()
{
    D8 ch;

    /* are we past the end of the buffer */
    if (sav_buf.b_dotp == sav_buf.b_linep)
	return (-1);
    ch = sav_buf.b_dotp->l_text[sav_buf.b_doto++];
    if (sav_buf.b_doto >= sav_buf.b_dotp->l_used)
    {
	sav_buf.b_doto = 0;
	sav_buf.b_dotp = sav_buf.b_dotp->l_fp;
    }
    return ((D16) ch);
}
