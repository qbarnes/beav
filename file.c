/*
*  File commands.
*/
#include	<sys/types.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include        "def.h"

char load_file ();
char readin ();
void makename ();
bool writeout ();
bool parse_f_name ();
A32 ffseek ();
A32 file_len ();

extern char MSG_rd_file[];
extern char MSG_trash[];
extern char MSG_ins_file[];
extern char MSG_not_fnd[];
extern char MSG_visit[];
extern char MSG_view[];
extern char MSG_buf_ex[];
extern char MSG_old_buf[];
extern char MSG_buf_nam[];
extern char MSG_cnt_cr[];
extern char MSG_reading[];
extern char MSG_read_lx[];
extern char MSG_no_mem_rd[];
extern char MSG_wr_file[];
extern char MSG_no_fn[];
extern char MSG_bk_err[];
extern char MSG_writing[];
extern char MSG_wrot_n[];
extern char MSG_fil_nam[];
extern char MSG_null[];
extern char ERR_parse_fn[];
extern char ERR_addr_neg[];
extern char ERR_f_size[];

static int ughlyflag = FALSE;

/*
* Read a file into the current
* buffer. This is really easy; all you do it
* find the name of the file, and call the standard
* "read a file into the current buffer" code.
*/
char
fileread ()
{
    register char s;
    char fname[NFILEN];
    A32 start, end;

    if ((s = ereply (MSG_rd_file, fname, NFILEN, NULL)) != TRUE)
	return (s);
    if (parse_f_name (fname, &start, &end))
    {
	adjustcase (fname);
	return (readin (fname, start, end));
    }
    return (TRUE);
}

/* insert file into current buffer - use readin, and yank
*/
char
fileinsert ()
{
    register char s;
    char bname[NBUFN], fname[NFILEN];
    A32 start, end;
    register char *trash = MSG_trash;

    strcpy (bname, curbp->b_bname);	/* save current buffer */
    if ((s = _usebuffer (trash)) == 0)	/* temp buffer */
	return (s);
    if ((s = ereply (MSG_ins_file, fname, NFILEN, NULL)) != TRUE)
	return (s);
    /* if file name and starting and ending addresses are good */
    if (parse_f_name (fname, &start, &end))
    {
	adjustcase (fname);
	if ((s = readin (fname, start, end)) == 0)
	{
	    writ_echo (MSG_not_fnd);
	    _usebuffer (bname);
	    _killbuffer (trash);
	    return (s);
	}
	if ((s = _usebuffer (bname)) == 0)
	{
	    _killbuffer (trash);
	    return (s);
	}
	if ((s = _yankbuffer (trash)) == 0)
	{
	    _killbuffer (trash);
	    return (s);
	}
	writ_echo (okmsg);
    }
    else
    {
	_usebuffer (bname);
	_killbuffer (trash);
	return (FALSE);
    }
    if ((s = _killbuffer (trash)) == 0)
	return (s);
    wind_on_dot (curwp);
    return (s);
}

/*
* Select a file for editing.
* Look around to see if you can find the
* fine in another buffer; if you can find it
* just switch to the buffer. If you cannot find
* the file, create a new buffer, read in the
* text, and switch to the new buffer.
*
* also various hacked versions for auto load, and
* file-vist with auto window split, and readonly (view-file) (jam)
*/
char
file_visit (f, n, k)
    int f, n, k;
{
    char fname[NFILEN];
    char s;
    A32 start, end;
    if ((s = ereply (MSG_visit, fname, NFILEN, NULL)) != TRUE)
	return (s);
    if (!parse_f_name (fname, &start, &end))
	return (FALSE);

    splitwind ();
    return (load_file (fname, start, end));
}

/* like filevisit, only read only
*/
char
viewfile ()
{
    char fname[NFILEN];
    char s;
    A32 start, end;

    if ((s = ereply (MSG_view, fname, NFILEN, NULL)) != TRUE)
	return (s);
    ughlyflag = TRUE;
    if (!parse_f_name (fname, &start, &end))
	return (FALSE);

    s = load_file (fname, start, end);
    if (s)
	curbp->b_flag |= BFVIEW;
    ughlyflag = FALSE;
    return (s);
}

char
filevisit ()
{
    char fname[NFILEN];
    char s;
    A32 start, end;

    if ((s = ereply (MSG_visit, fname, NFILEN, NULL)) != TRUE)
	return (s);
    if (!parse_f_name (fname, &start, &end))
	return (FALSE);

    return (load_file (fname, start, end));
}

char
load_file (fname, start, end)	/* jam */
    char *fname;
    A32 start, end;
{
    register BUFFER *bp;
    register WINDOW *wp;
    register LINE *lp;
    register int i;
    char s;
    char bname[NBUFN];
    extern int initial_load;	/* jam */
    static int append = 0;

    adjustcase (fname);
    for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
    {
	if (strcmp (bp->b_fname, fname) == 0)
	{
	    if (ughlyflag == TRUE)
	    {
		writ_echo (MSG_buf_ex);
		return (FALSE);
	    }
	    if (--curbp->b_nwnd == 0)
	    {
		curbp->b_type = BTFILE;
		curbp->b_dotp = curwp->w_dotp;
		curbp->b_doto = curwp->w_doto;
		curbp->b_unit_offset = curwp->w_unit_offset;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	    }
	    curbp = bp;
	    curwp->w_bufp = bp;
	    if (bp->b_nwnd++ == 0)
	    {
		curwp->w_dotp = bp->b_dotp;
		curwp->w_doto = bp->b_doto;
		curwp->w_unit_offset = bp->b_unit_offset;
		curwp->w_markp = bp->b_markp;
		curwp->w_marko = bp->b_marko;
	    }
	    else
	    {
		wp = wheadp;
		while (wp != NULL)
		{
		    if (wp != curwp && wp->w_bufp == bp)
		    {
			curwp->w_dotp = wp->w_dotp;
			curwp->w_doto = wp->w_doto;
			curwp->w_unit_offset = wp->w_unit_offset;
			curwp->w_markp = wp->w_markp;
			curwp->w_marko = wp->w_marko;
			break;
		    }
		    wp = wp->w_wndp;
		}
	    }
	    lp = curwp->w_dotp;
	    i = curwp->w_ntrows / 2;
	    while (i-- && lback (lp) != curbp->b_linep)
		lp = lback (lp);
	    curwp->w_linep = lp;
	    curwp->w_flag |= WFMODE | WFHARD;
	    if (kbdmop == NULL)
	    {
		writ_echo (MSG_old_buf);
	    }
	    return (TRUE);
	}
    }

    makename (bname, fname);	/* New buffer name.     */
    while ((bp = bfind (bname, FALSE)) != NULL)
    {
	if (initial_load)	/* patch old name */
	{
	    funky_name (bname, append++);
	    bp = NULL;
	    break;
	}
	s = ereply (MSG_buf_nam, bname, NBUFN, NULL);
	if (s == ABORT)		/* ^G to just quit      */
	    return (s);
	if (strcmp (bp->b_bname, bname) == 0 || s == FALSE)
	{
	    /* CR to clobber it     */
	    makename (bname, fname);
	    break;
	}
    }
    if (bp == NULL && (bp = bfind (bname, TRUE)) == NULL)
    {
	err_echo (MSG_cnt_cr);
	return (FALSE);
    }
    if (--curbp->b_nwnd == 0)
    {
	/* Undisplay.           */
	curbp->b_type = BTFILE;
	curbp->b_dotp = curwp->w_dotp;
	curbp->b_doto = curwp->w_doto;
	curbp->b_unit_offset = curwp->w_unit_offset;
	curbp->b_markp = curwp->w_markp;
	curbp->b_marko = curwp->w_marko;
    }
    curbp = bp;			/* Switch to it.        */
    curwp->w_bufp = bp;
    curbp->b_nwnd++;
    return (readin (fname, start, end));	/* Read it in.          */
}

/*
* Read the file "fname" into the current buffer.
* Make all of the text in the buffer go away, after checking
* for unsaved changes. This is called by the "read" command, the
* "visit" command, and the mainline (for "beav file"). If the
* BACKUP conditional is set, then this routine also does the read
* end of backup processing. The BFBAK flag, if set in a buffer,
* says that a backup should be taken. It is set when a file is
* read in, but not on a new file (you don't need to make a backup
* copy of nothing). Return a standard status. Print a summary
* (lines read, error message) out as well.
*/
char
readin (fname, start, end)
    char fname[];
    A32 start, end;
{
    register LINE *lp1;
    register LINE *lp2;
    register WINDOW *wp;
    register BUFFER *bp;
    register char s, m;
    long byte_cnt;
    LPOS req_chars;
    char buf[NCOL], buf1[NCOL];
    A32 temp;

    m = TRUE;
    byte_cnt = 0;
    bp = curbp;			/* Cheap.               */
    if ((s = bclear (bp)) != TRUE)	/* Might be old.        */
	return (s);
#if     BACKUP
    bp->b_flag &= ~(BFCHG | BFBAK);	/* No change, backup.   */
#else
    bp->b_flag &= ~BFCHG;	/* No change.           */
#endif
    if ((start == 0L) && (end == MAXPOS))
	strcpy (bp->b_fname, fname);
    else
	strcpy (bp->b_fname, MSG_null);
    bp->b_file_size = 0;
    bp->b_type = BTFILE;
    if ((s = ffropen (fname)) == FIOERR || s == FIOFNF)	/* jam */
	goto out;
    bp->b_file_size = file_len ();	/* get the file lenth */
    sprintf (buf, MSG_reading, fname);	/* jam */
    writ_echo (buf);
    temp = ffseek (start);
    if (temp != start)
    {
	sprintf (buf1, ERR_f_size, R_POS_FMT (curwp));
	sprintf (buf, buf1, temp);
	writ_echo (buf);
	return (FALSE);
    }
    /* only read the requested number of characters */
    if ((end - start) > NLINE)
	req_chars = NLINE;
    else
	req_chars = (int) (end - start);

    if ((lp1 = lalloc (req_chars)) == NULL)
    {
	bp->b_flag |= BFVIEW;	/* if no memory set to read only mode */

	m = FALSE;		/* flag memory allocation error */
    }
    else
    {
	while ((s = ffgetline (lp1->l_text, lp1->l_size, &lp1->l_used)) == FIOSUC)
	{
	    /* this code breaks rules for knowing how lines * are stored and linked
      together, oh well */
	    lp2 = lback (curbp->b_linep);
	    lp2->l_fp = lp1;
	    lp1->l_fp = curbp->b_linep;
	    lp1->l_bp = lp2;
	    curbp->b_linep->l_bp = lp1;
	    lp1->l_file_offset = byte_cnt;	/* file offset from begining */
	    byte_cnt += (long) lp1->l_used;	/* number of bytes read in    */
	    start += (long) lp1->l_used;
	    if (end <= start)
		break;
	    /* stop reading after the requested number of characters */
	    if (end < start + req_chars)
	    {
		req_chars = end - start;
	    }
	    if ((lp1 = lalloc (req_chars)) == NULL)
	    {
		bp->b_flag |= BFVIEW;	/* if no memory set to read only mode */

		m = FALSE;	/* flag memory allocation error */
		break;
	    }
	    if ((byte_cnt & 0x7fff) == 0)
	    {
		sprintf (buf1, MSG_read_lx, R_POS_FMT (curwp));
		sprintf (buf, buf1, (ulong) byte_cnt);
		writ_echo (buf);
		/* check if we should quit */
		if (ttkeyready ())
		{
		    wind_on_dot_all ();
		    if (ttgetc () == CTL_G)	/* was it an abort key? */
		    {
			s = FIOERR;
			break;
		    }
		}
	    }
	}
    }
    ffclose ();			/* Ignore errors.       */
    if (s == FIOEOF && kbdmop == NULL)
    {
	/* Don't zap an error.   */
	sprintf (buf1, MSG_read_lx, R_POS_FMT (curwp));
	sprintf (buf, buf1, byte_cnt);
	writ_echo (buf);
    }
    if (m == FALSE && kbdmop == NULL)
    {
	/* Don't zap an error.   */
	sprintf (buf, MSG_no_mem_rd);
	err_echo (buf);
    }

#if     BACKUP
    curbp->b_flag |= BFBAK;	/* Need a backup.       */
#endif
  out:
    for (wp = wheadp; wp != NULL; wp = wp->w_wndp)
    {
	if (wp->w_bufp == curbp)
	{
	    wp->w_linep = lforw (curbp->b_linep);
	    wp->w_dotp = lforw (curbp->b_linep);
	    wp->w_doto = 0;
	    wp->w_unit_offset = 0;
	    wp->w_markp = NULL;
	    wp->w_marko = 0;
	    wp->w_flag |= WFMODE | WFHARD;
	}
    }
    /* so tell yank-buffer about it */
    if ((blistp->b_nwnd != 0) &&/* update buffer display */
	(blistp->b_type == BTLIST))
	listbuffers ();
    if (s == FIOERR || s == FIOFNF)	/* False if error.      */
	return (FALSE);
    return (TRUE);
}

/*
* Take a file name, and from it
* fabricate a buffer name. This routine knows
* about the syntax of file names on the target system.
* BDC1         left scan delimiter.
* BDC2         optional second left scan delimiter.
* BDC3         optional right scan delimiter.
*/
void
makename (bname, fname)
    char bname[];
    char fname[];
{
    register char *cp1;
    register char *cp2;

    cp1 = &fname[0];
    while (*cp1 != 0)
	++cp1;
#ifdef  BDC2
    while (cp1 != &fname[0] && cp1[-1] != BDC1 && cp1[-1] != BDC2)
	--cp1;
#else
    while (cp1 != &fname[0] && cp1[-1] != BDC1)
	--cp1;
#endif
    cp2 = &bname[0];
#ifdef  BDC3
    while (cp2 != &bname[NBUFN - 1] && *cp1 != 0 && *cp1 != BDC3)
	*cp2++ = *cp1++;
#else
    while (cp2 != &bname[NBUFN - 1] && *cp1 != 0)
	*cp2++ = *cp1++;
#endif
    *cp2 = 0;
}

/*
* Ask for a file name, and write the
* contents of the current buffer to that file.
* Update the remembered file name and clear the
* buffer changed flag. This handling of file names
* is different from the earlier versions, and
* is more compatable with Gosling EMACS than
* with ITS EMACS.
*/
char
filewrite ()
{
    register WINDOW *wp;
    register char s;
    char fname[NFILEN];
    A32 start, end;

    if ((s = ereply (MSG_wr_file, fname, NFILEN, NULL)) != TRUE)
	return (s);
    if (!parse_f_name (fname, &start, &end))
	return (FALSE);

    adjustcase (fname);
    if ((s = writeout (fname, start, end, S_IREAD | S_IWRITE)) == TRUE)
    {
	strcpy (curbp->b_fname, fname);
	curbp->b_flag &= ~BFCHG;
	wp = wheadp;		/* Update mode lines.   */
	while (wp != NULL)
	{
	    if (wp->w_bufp == curbp)
		wp->w_flag |= WFMODE;
	    wp = wp->w_wndp;
	}
    }

#if     BACKUP
    curbp->b_flag &= ~BFBAK;	/* No backup.           */
#endif
    return (s);
}

/*
* Save the contents of the current buffer back into
* its associated file. Do nothing if there have been no changes
* (is this a bug, or a feature). Error if there is no remembered
* file name. If this is the first write since the read or visit,
* then a backup copy of the file is made.
*/
char
filesave ()
{
    register WINDOW *wp;
    register char s;
    struct stat st;

    if ((curbp->b_flag & BFCHG) == 0)	/* Return, no changes.  */
	return (TRUE);
    if (curbp->b_fname[0] == 0)	/* Must have a name.    */
    {
	if (!(curbp->b_type == BTSAVE))	/* yanked buffer */
	{
	    writ_echo (MSG_no_fn);
	}
	return (FALSE);
    }
    st.st_mode = S_IREAD | S_IWRITE;	/* set default */
#if     BACKUP
    if ((curbp->b_flag & BFBAK) != 0)
    {
	/* get the mode of the file */
	stat (curbp->b_fname, &st);

	s = fbackupfile (curbp->b_fname);
	if (s == ABORT)		/* Hard error.          */
	    return (s);
	if (s == FALSE		/* Softer error.        */
	    && (s = eyesno (MSG_bk_err)) != TRUE)
	    return (s);
    }

#endif
    if ((s = writeout (curbp->b_fname, 0L, MAXPOS, st.st_mode)) == TRUE)
    {
	curbp->b_flag &= ~BFCHG;/* No change.           */
	curbp->b_flag &= ~BFBAD;/* if it was trashed, forget it now */
	wp = wheadp;		/* Update mode lines.   */
	while (wp != NULL)
	{
	    if (wp->w_bufp == curbp)
		wp->w_flag |= WFMODE;
	    wp = wp->w_wndp;
	}
    }

#if     BACKUP
    curbp->b_flag &= ~BFBAK;	/* No backup.           */
#endif
    return (s);
}

/*
* This function performs the details of file
* writing. Uses the file management routines in the
* "fileio.c" package. The number of lines written is
* displayed. Sadly, it looks inside a LINE; provide
* a macro for this. Most of the grief is error
* checking of some sort.
* The file permissions are set as requested.
*/
bool
writeout (fn, start, end, mode)
    char *fn;
    A32 start, end;
    ushort mode;
{
    register int s, num_chars;
    register LINE *lp;
    register long nbytes;
    char buf[NCOL], buf1[NCOL];
    A32 temp;

    if ((s = ffwopen (fn, mode)) != FIOSUC)	/* Open writes message. */
	return (FALSE);
    temp = ffseek (start);
    if (temp != start)
    {
	sprintf (buf1, ERR_f_size, R_POS_FMT (curwp));
	sprintf (buf, buf1, temp);
	writ_echo (buf);
	return (FALSE);
    }
    sprintf (buf, MSG_writing, fn);	/* jam */
    writ_echo (buf);

    /* insure that the help screen reflects the latest bindings */
    if (curbp == blistp)
	wallchart (0, 0, 0);

    lp = lforw (curbp->b_linep);/* First line. */
    nbytes = 0;			/* Number of bytes.  */
    temp = end - start;		/* number of bytes to write */
    while (lp != curbp->b_linep)
    {
	if (curbp == blistp)
	{
	    /* special list buffer */
	    num_chars = HENDCOL;/* limit line length */
	    lp->l_text[num_chars - 1] = '\n';
	}
	else
	{
	    /* standard buffer */
	    if (nbytes + (long) llength (lp) > temp)
		num_chars = (int) (temp - nbytes);
	    else
		num_chars = llength (lp);
	}
	if ((s = ffputline (&lp->l_text[0], num_chars)) != FIOSUC)
	    break;
	nbytes += num_chars;
	if (temp <= nbytes)
	    break;
	lp = lforw (lp);

	if ((nbytes & 0x7fff) == 0)
	{
	    sprintf (buf1, MSG_wrot_n, R_POS_FMT (curwp));
	    sprintf (buf, buf1, (ulong) nbytes);
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		wind_on_dot_all ();
		if (ttgetc () == CTL_G)	/* was it an abort key? */
		{
		    s = FIOERR;
		    break;
		}
	    }
	}
    }
    if (s == FIOSUC)
    {
	/* No write error. */
	s = ffclose ();
	if (s == FIOSUC && kbdmop == NULL)
	{
	    sprintf (buf1, MSG_wrot_n, R_POS_FMT (curwp));
	    sprintf (buf, buf1, (long) nbytes);
	    writ_echo (buf);
	}
    }
    else			/* Ignore close error   */
	ffclose ();		/* if a write error.    */
    curbp->b_file_size = nbytes;/* update file size */
    if ((blistp->b_nwnd != 0) &&/* update buffer display */
	(blistp->b_type == BTLIST))
	listbuffers ();
    if (s != FIOSUC)		/* Some sort of error.  */
	return (FALSE);
    return (TRUE);
}

/*
* The command allows the user
* to modify the file name associated with
* the current buffer. It is like the "f" command
* in UNIX "ed". The operation is simple; just zap
* the name in the BUFFER structure, and mark the windows
* as needing an update. You can type a blank line at the
* prompt if you wish.
*/
char
filename ()
{
    register WINDOW *wp;
    register char s;
    char fname[NFILEN];
    A32 start, end;

    if ((s = ereply (MSG_fil_nam, fname, NFILEN, NULL)) == ABORT)
	return (s);
    if (!parse_f_name (fname, &start, &end))
	return (FALSE);

    adjustcase (fname);
    curbp->b_flag |= BFCHG;	/* jam - on name change, set modify */
    BUF_START (curwp) = start;
    l_fix_up (curbp->b_linep->l_fp);	/* adjust file offsets from first line */
    strcpy (curbp->b_fname, fname);	/* Fix name.            */
    wp = wheadp;		/* Update mode lines.   */
    while (wp != NULL)
    {
	if (wp->w_bufp == curbp)
	    wp->w_flag |= WFMODE;
	wp = wp->w_wndp;
    }
#if     BACKUP
    curbp->b_flag &= ~BFBAK;	/* No backup.           */
#endif
    return (TRUE);
}

/*
*   Get the length parameters that were entered with the file name.
*   There can be the file name only.
*   There can be a file name and a starting position.
*   There can be a name a starting position and an ending position.
*   There can be a name a starting position and a length.
*
*   input:
*       fn      pointer to file name string to parse.
*
*   output:
*       fn      pointer to null terminated file name.
*       start   pointer to the starting point in file (default = 0)
*       end     pointer to the end point in file (default = -1)
*       return  FALSE if file name or addresses are bad.
*/
bool
parse_f_name (fn, start, end)
    char *fn;
    A32 *start, *end;
{
    char buf[NFILEN], buf1[NCOL], fmt[NCOL];
    int i_cnt;

    /* build up format string according to the current screen format */
    sprintf (fmt, "%s %s %s", "%s", R_POS_FMT (curwp), R_POS_FMT (curwp));

    *start = 0L;
    *end = MAXPOS;
    sscanf (fn, fmt, buf, start, end);

    if (*end != MAXPOS)
    {
	for (i_cnt = strlen (fn) - 1; i_cnt >= 0; i_cnt--)
	{
	    if (fn[i_cnt] == '+')
	    {
		*end += *start;
		break;
	    }
	}
    }
    /* start should preceed end */
    if (*start > *end)
    {
	sprintf (buf1, ERR_parse_fn, R_POS_FMT (curwp), R_POS_FMT (curwp));
	sprintf (buf, buf1, *start, *end);
	writ_echo (buf);
	return (FALSE);
    }

    /* error if addresses are negative */
    if ((*start < 0) || (*end < 0))
    {
	writ_echo (ERR_addr_neg);
	return (FALSE);
    }

    /* deposit null terminated file name */
    strcpy (fn, buf);
    return (TRUE);
}
