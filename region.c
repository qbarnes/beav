/*
*       Region based commands.
* The routines in this file
* deal with the region, that magic space
* between "." and mark. Some functions are
* commands. Some functions are just for
* internal use.
*/
#include    "def.h"

bool getregion ();

extern char MSG_sv_in_b[];
extern char MSG_sav_slf[];
extern char MSG_no_mark[];
extern char MSG_procing[];

extern BUFFER sav_buf;

/*
* Kill the region. Ask "getregion"
* to figure out the bounds of the region.
* Move "." to the start, and kill the characters.
*/
char
killregion (f, n, k)
    int f, n, k;
{
    register char s;
    REGION region;
    int error;

    if ((s = getregion (&region)) != TRUE)
	return (s);
    if ((lastflag & CFKILL) == 0)	/* This is a kill type  */
	bclear (&sav_buf);
    thisflag |= CFKILL;		/* kill buffer stuff.   */
    curwp->w_dotp = region.r_linep;
    curwp->w_doto = region.r_offset;
    error = ldelete (region.r_size, TRUE);
    lchange (WFHARD);
    /* cause the save buffer display to be updated if needed */
    if (sav_buf.b_nwnd != 0)
	showsavebuf ();
    writ_echo (okmsg);
    return (error);
}

/*
* Copy all of the characters in the
* region to the kill buffer. Don't move dot
* at all. This is a bit like a kill region followed
* by a yank.
*/
char
copyregion (f, n, k)
    int f, n, k;
{
    register LINE *linep;
    register int loffs, j;
    register char s;
    REGION region;
    char buf[NCOL], buf1[NCOL];

    j = 0;
    if ((s = getregion (&region)) != TRUE)
	return (s);
    if ((lastflag & CFKILL) == 0)	/* Kill type command.   */
	bclear (&sav_buf);
    thisflag |= CFKILL;
    linep = region.r_linep;	/* Current line.    */
    loffs = region.r_offset;	/* Current offset.  */
    while (region.r_size--)
    {
	if ((s = b_append_c (&sav_buf, (uchar) lgetc (linep, loffs))) != TRUE)
	    return (s);
	++loffs;
	/* step to next line */
	if (linep->l_used <= loffs)
	{
	    linep = linep->l_fp;
	    loffs = 0;
	}

	if ((j++ & 0x2ff) == 0)
	{
	    sprintf (buf1, MSG_procing, R_POS_FMT (curwp));
	    sprintf (buf, buf1, DOT_POS (curwp));
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		wind_on_dot_all ();
		if (ttgetc () == '*')
		    return (FALSE);
	    }
	}
    }
    /* cause the save buffer display to be updated if needed */
    if (sav_buf.b_nwnd != 0)
	showsavebuf ();
    /* update buffer display */
    if ((blistp->b_nwnd != 0) &&
	(blistp->b_type == BTLIST))
	listbuffers ();
    writ_echo (okmsg);
    return (TRUE);
}

/*
* This routine figures out the bound of the region
* in the current window, and stores the results into the fields
* of the REGION structure. Dot and mark are usually close together,
* but I don't know the order. The size is kept in a long. At the
* end, after the size is figured out, it is assigned to the size
* field of the region structure. If this assignment loses any bits,
* then we print an error. This is "type independent" overflow
* checking. All of the callers of this routine should be ready to
* get an ABORT status, because I might add a "if regions is big,
* ask before clobberring" flag.
*/
bool
getregion (rp)
    register REGION *rp;
{
    if (curwp->w_markp == NULL)
    {
	writ_echo (MSG_no_mark);
	return (FALSE);
    }

    if (DOT_POS (curwp) < MARK_POS (curwp))
    {
	rp->r_linep = curwp->w_dotp;
	rp->r_offset = curwp->w_doto;
	rp->r_size = MARK_POS (curwp) - DOT_POS (curwp);
    }
    else
    {
	rp->r_linep = curwp->w_markp;
	rp->r_offset = curwp->w_marko;
	rp->r_size = DOT_POS (curwp) - MARK_POS (curwp);
    }
    return (TRUE);
}

/* save some region in a buffer
* (use _usebuffer to handle non-existent buffers)
*
* hack as it uses kill buffer to transfer stuff (quick and dirty!)
* and doesn't do clever things at all with dot in destination buffer!
*/
char
save_region (f, n, k)
    int f, n, k;
{
    char bufn[NBUFN];
    char oldbufn[NBUFN];
    register char s;

    if ((s = ereply (MSG_sv_in_b, bufn, NBUFN, NULL)) != TRUE)
	return (s);

    if (strcmp (bufn, curbp->b_bname) == 0)
    {
	writ_echo (MSG_sav_slf);
	return (FALSE);
    }

    /* save this name for ughly reversal */
    strcpy (oldbufn, curbp->b_bname);

    /* copy stuff using killbuffer as work space -  hack !! * than move it to
  named place using yank - Quick AND Dirty */
    copyregion (f, n, k);
    _usebuffer (bufn);
    curbp->b_type = BTSAVE;	/* mark as a saved buffer */

    yank (f, n, k);
    _usebuffer (oldbufn);
    writ_echo (okmsg);
    return (TRUE);
}
