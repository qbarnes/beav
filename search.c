/*
*       Search commands.
* The functions in this file implement the
* search commands (both plain and incremental searches
* are supported) and the query-replace command.
*/
#include    <string.h>
#include    "def.h"

char replaceit ();
char forwsrch ();
char backsrch ();
char readpattern ();
void next_pat ();

extern char MSG_sch_str[];
extern char MSG_bsrc_str[];
extern char MSG_rpl_str[];
extern char MSG_pat_fnd[];
extern char MSG_no_srch[];
extern char MSG_fnd_at[];
extern char MSG_no_rpl[];
extern char MSG_1_rpl[];
extern char MSG_n_rpl[];
extern char MSG_srcing[];
extern char MSG_curs[];
extern char MSG_cmp_end[];
extern char MSG_cmp_term[];
extern char MSG_cmp_dif[];
extern char MSG_only_2[];
extern char MSG_cmping[];
extern char MSG_not_fnd[];
#if RUNCHK
extern char ERR_rdpat[];
extern char ERR_mask[];
extern char ERR_m_cl[];
#endif

#define CCHR(x)     ((x)-'@')

#define SRCH_BEGIN  (0)		/* Search sub-codes.    */
#define SRCH_FORW   (-1)
#define SRCH_BACK   (-2)
#define SRCH_PREV   (-3)
#define SRCH_NEXT   (-4)
#define SRCH_NOPR   (-5)
#define SRCH_ACCM   (-6)

typedef struct
{
    int s_code;
    LINE *s_dotp;
    short s_doto;
} SRCHCOM;

#define MAX_PAT 260

extern ROW_FMT hex_s_8_fmt;
extern ROW_FMT ascii_s_fmt;

bool recall_flag = FALSE;
bool read_pat_mode = FALSE;
bool srch_mode = FALSE;
bool rplc_mode = FALSE;
bool dont_repeat = FALSE;	/* used to prevent toggling commands from */
/* failing in read_pattern */
static char srch_patb[MAX_PAT];
static char srch_maskb[MAX_PAT];
static char rplc_patb[MAX_PAT];
static char rplc_maskb[MAX_PAT];

static LINE *srch_pat = (LINE *) srch_patb;
static LINE *srch_mask = (LINE *) srch_maskb;
static LINE *cur_pat;
static LINE *cur_mask;
static LINE *rplc_pat = (LINE *) rplc_patb;
static LINE *rplc_mask = (LINE *) rplc_maskb;

static int old_srch_pat_size = 0;	/* for pattern recall */
static int old_rplc_pat_size = 0;
static ROW_FMT *old_fmt = &hex_s_8_fmt;

char *cur_prompt;

int srch_lastdir = SRCH_NOPR;	/* Last search flags.   */

/*
* Search forward.
* Get a search string from the user, and search for it,
* starting at ".". If found, "." gets moved to the
* first matched character, and display does all the hard stuff.
* If not found, it just prints a message.
*/
char
forwsearch ()
{
    register char s;
    char buf[NCOL], buf1[NCOL];

    srch_mode = TRUE;
    rplc_mode = FALSE;
    cur_prompt = MSG_sch_str;
    if ((s = readpattern ()) != TRUE)
    {
	srch_mode = FALSE;
	eerase ();		/* clear message line */
	return (s);
    }
    if (forwsrch () == FALSE)
    {
	writ_echo (MSG_not_fnd);
	srch_mode = FALSE;
	return (FALSE);
    }
    srch_lastdir = SRCH_FORW;
    curwp->w_flag |= WFMODE;	/* update mode line */
    curwp->w_unit_offset = 0;
    /* build format */
    sprintf (buf1, MSG_pat_fnd, R_POS_FMT (curwp));
    sprintf (buf, buf1, curwp->w_dotp->l_file_offset +
	     curwp->w_doto);
    writ_echo (buf);
    srch_mode = FALSE;
    return (TRUE);
}

/*
* Reverse search.
* Get a search string from the  user, and search, starting at "."
* and proceeding toward the front of the buffer. If found "." is left
* pointing at the first character of the pattern [the last character that
* was matched].
*/
char
backsearch ()
{
    register char s;
    char buf[NCOL], buf1[NCOL];

    srch_mode = TRUE;
    rplc_mode = FALSE;
    cur_prompt = MSG_bsrc_str;
    if ((s = readpattern ()) != TRUE)
    {
	srch_mode = FALSE;
	eerase ();		/* clear message line */
	return (s);
    }
    if (backsrch () == FALSE)
    {
	writ_echo (MSG_not_fnd);
	srch_mode = FALSE;
	return (FALSE);
    }
    srch_lastdir = SRCH_BACK;
    curwp->w_flag |= WFMODE;	/* update mode line */
    curwp->w_unit_offset = 0;
    sprintf (buf1, MSG_pat_fnd, R_POS_FMT (curwp));
    sprintf (buf, buf1, curwp->w_dotp->l_file_offset +
	     curwp->w_doto);
    writ_echo (buf);
    srch_mode = FALSE;
    return (TRUE);
}

/*
* Search again, using the same search string
* and direction as the last search command. The direction
* has been saved in "srch_lastdir", so you know which way
* to go.
*/
char
searchagain ()
{
    char buf[NCOL], buf1[NCOL];
    long dot_pos;
    srch_mode = TRUE;
    rplc_mode = FALSE;

    dot_pos = DOT_POS (curwp);
    if (srch_lastdir == SRCH_FORW)
    {
	/* advance one unit so we don't find the same thing again */
	move_ptr (curwp, dot_pos + 1, TRUE, FALSE, FALSE);
	if (forwsrch () == FALSE)
	{			/* go back to orig pt */
	    move_ptr (curwp, dot_pos, TRUE, FALSE, FALSE);
	    writ_echo (MSG_not_fnd);
	    srch_mode = FALSE;
	    return (FALSE);
	}
	curwp->w_flag |= WFMODE;/* update mode line */
	curwp->w_unit_offset = 0;
	sprintf (buf1, MSG_pat_fnd, R_POS_FMT (curwp));
	sprintf (buf, buf1, curwp->w_dotp->l_file_offset +
		 curwp->w_doto);
	writ_echo (buf);
	srch_mode = FALSE;
	return (TRUE);
    }
    if (srch_lastdir == SRCH_BACK)
    {
	/* step back one unit so we don't find the same thing again */
	move_ptr (curwp, dot_pos - 1, TRUE, FALSE, FALSE);
	if (backsrch () == FALSE)
	{			/* go back to orig pt */
	    move_ptr (curwp, dot_pos, TRUE, FALSE, FALSE);
	    writ_echo (MSG_not_fnd);
	    srch_mode = FALSE;
	    return (FALSE);
	}
	curwp->w_flag |= WFMODE;/* update mode line */
	curwp->w_unit_offset = 0;
	sprintf (buf1, MSG_pat_fnd, R_POS_FMT (curwp));
	sprintf (buf, buf1, curwp->w_dotp->l_file_offset +
		 curwp->w_doto);
	writ_echo (buf);
	srch_mode = FALSE;
	return (TRUE);
    }
    writ_echo (MSG_no_srch);
    srch_mode = FALSE;
    return (FALSE);
}

/*
* Query Replace.
*   Replace strings selectively.  Does a search and replace operation.
*   A space or a comma replaces the string, a period replaces and quits,
*   an n doesn't replace, a C-G quits.
* (note typical hack to add a function with minimal code)
*/
char
queryrepl (f, n, k)
    int f, n, k;
{

    register char s;

    srch_mode = FALSE;
    rplc_mode = TRUE;
    cur_prompt = MSG_sch_str;
    if ((s = readpattern ()))
    {
	replaceit ();
    }
    srch_mode = FALSE;
    rplc_mode = FALSE;
    return (s);
}

char
replaceit ()
{
    int rcnt = 0;		/* Replacements made so far */
    int plen;			/* length of found string   */
    int rlen;			/* length of replace string   */
    long abs_dot_p;		/* absolute dot position */
    long abs_mark_p;		/* absolute mark position */
    char buf[NCOL], buf1[NCOL];

    /*
  * Search forward repeatedly, checking each time whether to insert
  * or not.  The "!" case makes the check always true, so it gets put
  * into a tighter loop for efficiency.
  *
  * If we change the line that is the remembered value of dot, then
  * it is possible for the remembered value to move.  This causes great
  * pain when trying to return to the non-existant line.
  *
  * possible fixes:
  * 1) put a single, relocated marker in the WINDOW structure, handled
  *    like mark.  The problem now becomes a what if two are needed...
  * 2) link markers into a list that gets updated (auto structures for
  *    the nodes)
  * 3) Expand the mark into a stack of marks and add pushmark, popmark.
  */

    plen = srch_pat->l_used;
    rlen = rplc_pat->l_used;

    abs_dot_p = DOT_POS (curwp);/* save current dot position */
    if (curwp->w_markp != NULL)	/* mark may not be set */
	abs_mark_p = MARK_POS (curwp);

    while (forwsrch () == TRUE)
    {
      retry:
	sprintf (buf1, MSG_fnd_at, R_POS_FMT (curwp));
	sprintf (buf, buf1, DOT_POS (curwp));
	writ_echo (buf);
	curwp->w_flag |= WFMODE;/* update mode line */
	update ();
	switch (ttgetc ())
	{
	case 'r':
	case 'R':
	case ' ':
	case ',':
	    /* update has fixedup the dot position so move to found byte */
	    /* go and do the replace */
	    if (lrepl_str (plen, rplc_pat, rplc_mask) == FALSE)
		return (FALSE);
	    /* begin searching after replace string */
	    move_ptr (curwp, (long) rlen, TRUE, FALSE, TRUE);
	    rcnt++;
	    break;

	case 'o':
	case 'O':
	case '.':
	    if (lrepl_str (plen, rplc_pat, rplc_mask) == FALSE)
		return (FALSE);
	    /* begin searching after replace string */
	    move_ptr (curwp, (long) rlen, TRUE, FALSE, TRUE);
	    rcnt++;
	    goto stopsearch;

	case 'q':
	case 'Q':
	case CCHR ('G'):
	    ctrlg (FALSE, 0, KRANDOM);
	    goto stopsearch;

	case 'a':
	case 'A':
	case '!':
	    do
	    {
		if (lrepl_str (plen, rplc_pat, rplc_mask) == FALSE)
		    return (FALSE);
		/* begin searching after replace string */
		move_ptr (curwp, (long) rlen, TRUE, FALSE, TRUE);
		rcnt++;
	    }
	    while (forwsrch () == TRUE);
	    goto stopsearch;

	case 's':
	case 'S':
	case 'n':
	    /* begin searching after this byte */
	    move_ptr (curwp, 1L, TRUE, FALSE, TRUE);
	    break;

	default:
	    ttbeep ();
	    goto retry;
	}
    }

  stopsearch:
    move_ptr (curwp, abs_dot_p, TRUE, TRUE, FALSE);
    if (curwp->w_markp != NULL)
    {
	swapmark ();
	/* insure that the mark points to the same byte as before */
	if (abs_mark_p > abs_dot_p)
	    move_ptr (curwp, abs_mark_p + rlen - plen, TRUE, FALSE, FALSE);
	else
	    move_ptr (curwp, abs_mark_p, TRUE, FALSE, FALSE);
	swapmark ();
    }
    curwp->w_flag |= WFHARD;
    update ();
    if (rcnt == 0)
    {
	writ_echo (MSG_no_rpl);
    }
    else if (rcnt == 1)
    {
	writ_echo (MSG_1_rpl);
    }
    else
    {
	sprintf (buf1, MSG_n_rpl, R_POS_FMT (curwp));
	sprintf (buf, buf1, (ulong) rcnt);
	writ_echo (buf);
    }
    flush_count += rcnt;	/* jam for auto write buffers */
    return (TRUE);
}

/*
* This routine does the real work of a
* forward search. The pattern is sitting in the external
* variable "srch_pat" the mask if in "srch_mask".
* If found, dot is updated, the window system
* is notified of the change, and TRUE is returned. If the
* string isn't found, FALSE is returned.
*/
char
forwsrch ()
{
    register LINE *save_dotp, *save2_dotp;
    register int save_doto, save2_doto;
    register D8 *pat_ptr, *mask_ptr;
    register int i, j, pat_cnt;
    register D8 first_pat, first_mask;
    char buf[NCOL], buf1[NCOL];

    save_dotp = curwp->w_dotp;	/* save dot position for later */
    save_doto = curwp->w_doto;
    pat_ptr = srch_pat->l_text;
    mask_ptr = srch_mask->l_text;
    pat_cnt = srch_pat->l_used;
    first_mask = mask_ptr[0];
    first_pat = pat_ptr[0] | first_mask;
    j = (int) DOT_POS (curwp) & 0xffff;

    do
    {
	if ((j++ & 0x2ff) == 0)
	{
	    sprintf (buf1, MSG_srcing, R_POS_FMT (curwp));
	    sprintf (buf, buf1, DOT_POS (curwp));
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		if (ttgetc () == CTL_G)
		    break;
	    }
	}
	if (first_pat ==
	    ((DOT_CHAR (curwp) | first_mask) & 0xff))
	{
	    save2_dotp = curwp->w_dotp;	/* save dot position for later */
	    save2_doto = curwp->w_doto;
	    for (i = 1; i < pat_cnt; i++)
	    {
		if (!move_ptr (curwp, 1L, TRUE, FALSE, TRUE) ||
		    ((pat_ptr[i] & ~mask_ptr[i]) !=
		     (DOT_CHAR (curwp) & ~mask_ptr[i])))
		{		/* not found */
		    curwp->w_dotp = save2_dotp;	/* restore dot position */
		    curwp->w_doto = save2_doto;
		    break;
		}
	    }
	    if (i == pat_cnt)	/* found */
	    {			/* move back to the first matching unit */
		move_ptr (curwp, -(long) pat_cnt + 1, TRUE, FALSE, TRUE);
		wind_on_dot (curwp);
		return (TRUE);
	    }
	}
    }
    while (move_ptr (curwp, 1L, TRUE, FALSE, TRUE));

    curwp->w_dotp = save_dotp;	/* restore dot position */
    curwp->w_doto = save_doto;
    return (FALSE);
}

/*
* This routine does the real work of a
* backward search. The pattern is sitting in the external
* variable "srch_pat". If found, dot is updated, the window system
* is notified of the change, and TRUE is returned. If the
* string isn't found, FALSE is returned.
*/
char
backsrch ()
{
    register LINE *save_dotp, *save_p;
    register LPOS save_doto, save_o;
    register D8 *pat_ptr, *mask_ptr;
    register int i, j, pat_cnt;
    register char first_pat, first_mask;
    char buf[NCOL], buf1[NCOL];

    save_dotp = curwp->w_dotp;	/* save dot position for later */
    save_doto = curwp->w_doto;
    pat_ptr = srch_pat->l_text;
    mask_ptr = srch_mask->l_text;
    pat_cnt = srch_pat->l_used;
    first_mask = mask_ptr[0];
    first_pat = pat_ptr[0] | first_mask;
    j = (int) DOT_POS (curwp) & 0xffff;

    do
    {
	/* check if we should quit */
	if (ttkeyready ())
	{
	    if (ttgetc () == CTL_G)
		break;
	}
	if ((j-- & 0x2ff) == 0)
	{
	    sprintf (buf1, MSG_srcing, R_POS_FMT (curwp));
	    sprintf (buf, buf1, DOT_POS (curwp));
	    writ_echo (buf);
	}
	if (first_pat ==
	    (curwp->w_dotp->l_text[curwp->w_doto] | first_mask))
	{

	    save_p = curwp->w_dotp;
	    save_o = curwp->w_doto;
	    for (i = 1; i < pat_cnt; i++)
	    {
		if (!move_ptr (curwp, 1L, TRUE, FALSE, TRUE) ||
		    ((pat_ptr[i] & ~mask_ptr[i]) !=
		     (DOT_CHAR (curwp) & ~mask_ptr[i])))
		{		/* not found */
		    curwp->w_dotp = save_p;	/* restore ptr to continue */

		    curwp->w_doto = save_o;
		    break;
		}
	    }
	    if (i == pat_cnt)	/* found */
	    {			/* move back to the first matching unit */
		move_ptr (curwp, -(long) pat_cnt + 1, TRUE, FALSE, TRUE);
		wind_on_dot (curwp);
		return (TRUE);
	    }
	}
    }
    while (move_ptr (curwp, -1L, TRUE, FALSE, TRUE));

    curwp->w_dotp = save_dotp;	/* restore dot position */
    curwp->w_doto = save_doto;
    return (FALSE);
}

/*
* Read a pattern.
* Display and edit in the form of the current window.
* Slide the displayed line back and forth when the cursor hits a boundary.
* Manage the mask buffer. When a '*' (wild card) is entered mask all
* bits in that unit and display all '?'s.
*/
char
readpattern ()
{
    int cod, mask_cod, curs_pos, curs_pos1, prt_siz, i, doto, loff;
    WINDOW srch_wind, *save_wind;
    BUFFER srch_buf, *save_buf;
    LINE head_line;
    int r_type, siz_prompt2, u_off;
    bool first_time, stat;
    char disp_buf[180], mask_buf[180], buf1[NCOL];


    save_wind = curwp;		/* save current window for later */
    save_buf = curbp;		/* save current buffer for later */

    curwp = &srch_wind;		/* search window is current window during
							                                       search */
    curbp = &srch_buf;
    cur_pat = srch_pat;		/* set global variables for LINE finctions */
    cur_mask = srch_mask;

    recall_flag = FALSE;
    first_time = TRUE;
    read_pat_mode = TRUE;
    curwp->w_wndp = NULL;
    curwp->w_bufp = curbp;
    curwp->w_linep = cur_pat;
    curwp->w_loff = 0;
    curwp->w_dotp = cur_pat;
    curwp->w_doto = 0;
    curwp->w_unit_offset = 0;
    curwp->w_toprow = 24;
    curwp->w_ntrows = 1;
    curwp->w_intel_mode = save_wind->w_intel_mode;
    curwp->w_disp_shift = 0;
    if (R_TYPE (save_wind) == TEXT)
	curwp->w_fmt_ptr = &ascii_s_fmt;
    else
	curwp->w_fmt_ptr = save_wind->w_fmt_ptr->r_srch_fmt;

    srch_buf.b_bufp = NULL;
    srch_buf.b_linep = &head_line;
    srch_buf.b_unit_offset = 0;	/* unit offset   pvr */
    srch_buf.b_markp = NULL;
    srch_buf.b_marko = 0;
    srch_buf.b_flag = 0;
    srch_buf.b_nwnd = 1;
    srch_buf.b_fname[0] = 0;
    srch_buf.b_bname[0] = 0;

    head_line.l_fp = cur_pat;
    head_line.l_bp = cur_pat;
    head_line.l_file_offset = 0;/* pvr */
    head_line.l_used = 0;
    head_line.l_size = 0;

    cur_pat->l_fp = &head_line;
    cur_pat->l_bp = &head_line;
    cur_pat->l_size = 266;	/* leave some extra past 256 */
    cur_pat->l_used = 0;
    cur_pat->l_file_offset = 0;

    cur_mask->l_fp = &head_line;
    cur_mask->l_bp = &head_line;
    cur_mask->l_size = 266;	/* leave some extra past 256 */
    cur_mask->l_used = 0;
    cur_mask->l_file_offset = 0;

    rplc_pat->l_fp = &head_line;
    rplc_pat->l_bp = &head_line;
    rplc_pat->l_size = 266;	/* leave some extra past 256 */
    rplc_pat->l_used = 0;
    rplc_pat->l_file_offset = 0;

    rplc_mask->l_fp = &head_line;
    rplc_mask->l_bp = &head_line;
    rplc_mask->l_size = 266;	/* leave some extra past 256 */
    rplc_mask->l_used = 0;
    rplc_mask->l_file_offset = 0;

    sprintf (buf1, MSG_curs, cur_prompt, R_BYTE_FMT (curwp),
	     R_BYTE_FMT (curwp), R_BYTE_FMT (curwp));
    sprintf (disp_buf, buf1, curwp->w_doto,
	     curwp->w_fmt_ptr->r_chr_per_u - curwp->w_unit_offset - 1,
	     curwp->w_dotp->l_used);

    siz_prompt2 = strlen (disp_buf);	/* save prompt length for later */

    for (i = siz_prompt2; i < NCOL; i++)	/* clear rest of buffer */
	disp_buf[i] = ' ';

    writ_echo (disp_buf);

    r_type = R_TYPE (curwp);

    while (TRUE)
    {
	/* position cursor */
	curs_pos = curwp->w_doto - curwp->w_loff;
	if (curwp->w_fmt_ptr->r_size == 1)
	{
	    curs_pos = curs_pos >> 1;
	}
	else if (curwp->w_fmt_ptr->r_size == 3)
	{
	    curs_pos = curs_pos >> 2;
	}
	curs_pos1 = curwp->w_fmt_ptr->r_positions[curs_pos] +
	    curwp->w_unit_offset + siz_prompt2;
	ttmove (nrow - 1, curs_pos1);
	ttflush ();

	cod = getkey ();

	if (cod == 0x014D || cod == 0x014A)	/* check for return or linefeed */
	{
	    if ((rplc_mode == TRUE) && (cur_prompt == MSG_sch_str))
	    {
		next_pat ();
		dont_repeat = FALSE;	/* fix up */
		goto next_loop;
	    }
	    else
	    {
		old_srch_pat_size = srch_pat->l_used;	/* save for restore */
		if (rplc_mode == TRUE)
		    old_rplc_pat_size = rplc_pat->l_used;

		old_fmt = curwp->w_fmt_ptr;
		curwp = save_wind;	/* restore current window */
		curbp = save_buf;	/* restore current buffer */
		read_pat_mode = FALSE;
		return (TRUE);
	    }
	}

	if ((cod >= ' ') && (cod < 0x7f))
	{
	    if ((r_type == ASCII) || (r_type == EBCDIC))
	    {
		mask_cod = '9';	/* use 9 as dummy char that will get through */
	    }
	    else if ((r_type == DECIMAL) || (r_type == FLOAT))
	    {
		mask_cod = '0';	/* clear mask byte */
	    }
	    else if (cod == '?')
	    {
		cod = '0';
		switch (r_type)
		{
		case OCTAL:
		    if (curwp->w_unit_offset == 0)	/* if first char */
		    {
			if (R_SIZE (curwp) == WORDS)
			    mask_cod = '1';
			else
			    mask_cod = '3';
		    }
		    else
			mask_cod = '7';
		    break;

		case HEX:
		    mask_cod = 'F';
		    break;

		case BINARY:
		    mask_cod = '1';
		    break;
#if RUNCHK
		default:
		    printf (ERR_rdpat);
		    break;
#endif
		}
	    }
	    else
	    {
		mask_cod = '0';
	    }
	}
	else
	    mask_cod = cod;	/* must be control; do the same to the mask */

	/* save current dot and window positions */
	doto = curwp->w_doto;
	u_off = curwp->w_unit_offset;
	loff = curwp->w_loff;
	stat = execute (cod, FALSE, 1);

	if (stat == ABORT)
	{
	    old_srch_pat_size = srch_pat->l_used;	/* save for restore */
	    if (rplc_mode == TRUE)
		old_rplc_pat_size = rplc_pat->l_used;
	    old_fmt = curwp->w_fmt_ptr;
	    curwp = save_wind;	/* restore current window */
	    curbp = save_buf;	/* restore current buffer */
	    read_pat_mode = FALSE;
	    return (FALSE);
	}

	/* If key is recall then reset the size variables */
	if (first_time)
	{
	    first_time = FALSE;
	    if (recall_flag)
	    {
		srch_pat->l_used = old_srch_pat_size;
		srch_mask->l_used = old_srch_pat_size;
		rplc_pat->l_used = old_rplc_pat_size;
		rplc_mask->l_used = old_rplc_pat_size;
		curwp->w_fmt_ptr = old_fmt;
		recall_flag = FALSE;
	    }
	}

	/* if it was a toggling command, don't do it again */
	if (!dont_repeat &&
	    (stat == TRUE))
	{
	    head_line.l_fp = cur_mask;	/* point to mask */
	    head_line.l_bp = cur_mask;
	    curwp->w_linep = cur_mask;
	    curwp->w_dotp = cur_mask;
	    curwp->w_loff = loff;
	    curwp->w_doto = doto;
	    curwp->w_unit_offset = u_off;
	    execute (mask_cod, FALSE, 1);

	    head_line.l_fp = cur_pat;	/* restore pointers */
	    head_line.l_bp = cur_pat;
	    curwp->w_linep = cur_pat;
	    curwp->w_dotp = cur_pat;
	}
	else
	    dont_repeat = FALSE;

	/* limit at 256 bytes */
	if (cur_pat->l_used >= 256)
	{
	    cur_mask->l_used = 255;
	    cur_pat->l_used = 255;
	    if (curwp->w_doto >= 256)
	    {
		move_ptr (curwp, 255L, TRUE, TRUE, FALSE);	/* last position */
	    }
	}

	/* if buffer is size locked then replace pattern must be the */
	/* same size as the search pattern */
	if (rplc_mode && (save_buf->b_flag & BFSLOCK))
	{
	    rplc_pat->l_used = srch_pat->l_used;
	    rplc_mask->l_used = srch_pat->l_used;
	}

	r_type = R_TYPE (curwp);
#if RUNCHK
	/* check that the pattern and the mask are the same size */
	if (cur_pat->l_used != cur_mask->l_used)
	{
	    printf (ERR_mask, cur_pat->l_used, cur_mask->l_used);
	}

	/* check that in ascii mode the byte that will be set to zero */
	/* is the dummy char 9 */
	/*        if (((r_type == ASCII) &&
    (cur_mask -> l_text[curwp -> w_doto - 1] != '9'))
    ||
    ((r_type == EBCDIC) &&
    (cur_mask -> l_text[curwp -> w_doto - 1] != to_ebcdic('9'))))
    printf (ERR_m_cl);
    */
#endif
	if (((r_type == ASCII) ||
	     (r_type == EBCDIC)) &&
	    ((cod >= ' ') && (cod < 0x7f)))
	    cur_mask->l_text[doto] = 0;	/* clear mask byte */

      next_loop:
	sprintf (buf1, MSG_curs, cur_prompt, R_BYTE_FMT (curwp),
		 R_BYTE_FMT (curwp), R_BYTE_FMT (curwp));
	sprintf (disp_buf, buf1, curwp->w_doto,
		 curwp->w_fmt_ptr->r_chr_per_u - curwp->w_unit_offset - 1,
		 curwp->w_dotp->l_used);

	siz_prompt2 = strlen (disp_buf);	/* save prompt length for later */

	for (i = siz_prompt2; i < NCOL; i++)
	{
	    disp_buf[i] = ' ';
	    mask_buf[i] = ' ';
	}

	if ((curbp->b_flag & BFSLOCK) &&
	    (rplc_pat->l_used != srch_pat->l_used))
	{
	    rplc_pat->l_used = srch_pat->l_used;
	    /* if dot is past the end then move it back, replace string only */
	    if (DOT_POS (curwp) > srch_pat->l_used)
		move_ptr (curwp, (long) srch_pat->l_used, TRUE, TRUE, FALSE);
	}

	wind_on_dot (curwp);

	/* figure number of bytes to convert to text */
	if ((cur_pat->l_used - curwp->w_loff) <
	    (prt_siz = curwp->w_fmt_ptr->r_bytes))
	    prt_siz = cur_pat->l_used - curwp->w_loff;

	bin_to_text (&cur_pat->l_text[curwp->w_loff],
		     &disp_buf[siz_prompt2],
		     prt_siz, curwp->w_fmt_ptr);

	/* change any char to a ? if any bit is set in the mask buffer */
	if ((r_type != ASCII) && (r_type != EBCDIC))
	{
	    /* print the contents of the mask to a invisible buffer */
	    bin_to_text (&cur_mask->l_text[curwp->w_loff],
			 &mask_buf[siz_prompt2],
			 prt_siz, curwp->w_fmt_ptr);

	    for (i = siz_prompt2; (disp_buf[i] != 0) && (i < NCOL); i++)
	    {
		if ((mask_buf[i] != '0') &&
		    (mask_buf[i] != ' '))
		    disp_buf[i] = '?';
	    }
	}
	else
	{
	    for (i = 0; i < prt_siz; i++)
	    {
		if (cur_mask->l_text[curwp->w_loff + i] != 0)
		    disp_buf[i + siz_prompt2] = '?';
	    }
	}
	writ_echo (disp_buf);
    }
}

/*
*   Recall the last contents of the search string
*/
bool
recall ()
{
    recall_flag = TRUE;
    return (TRUE);
}

/*
*   Switch between search pattern and replace pattern and their
*   respective masks
*/
void
next_pat ()
{
    if (cur_pat == srch_pat)
    {
	cur_prompt = MSG_rpl_str;
	cur_pat = rplc_pat;	/* point to replace pattern */
	cur_mask = rplc_mask;
    }
    else
    {
	cur_prompt = MSG_sch_str;
	cur_pat = srch_pat;	/* point to search pattern */
	cur_mask = srch_mask;
    }
    curwp->w_dotp = cur_pat;
    curwp->w_linep = cur_pat;
    curbp->b_linep->l_fp = cur_pat;
    curbp->b_linep->l_bp = cur_pat;

    if (curwp->w_doto > cur_pat->l_used)
    {
	curwp->w_doto = cur_pat->l_used;
	curwp->w_unit_offset = 0;
    }
    if (curwp->w_loff > cur_pat->l_used)
	curwp->w_loff = cur_pat->l_used;
    dont_repeat = TRUE;
}

/*
* Compare the contents of two windows.
* There must be exactly two windows displayed.
* The bytes under the cursor in each window are compared and if
* a difference is found then the loop is stopped with the dot
* position in each window pointing to the difference.
* The two windows can be pointing at the same or different buffers.
*/
bool
compare ()

{
    WINDOW *wp1, *wp2;
    bool move1, move2;
    int j;
    char *term_str = MSG_cmp_dif;
    char buf[NCOL], buf1[NCOL];

    if (wheadp->w_wndp->w_wndp != NULL)
    {
	writ_echo (MSG_only_2);
	return (FALSE);
    }

    wp1 = wheadp;
    wp2 = wheadp->w_wndp;
    j = (int) DOT_POS (curwp) & 0xffff;

    wp1->w_flag |= WFMOVE;
    wp2->w_flag |= WFMOVE;

    while (DOT_CHAR (wp1) == DOT_CHAR (wp2))
    {
	if ((j++ & 0xff) == 0)
	{
	    sprintf (buf1, MSG_cmping, R_POS_FMT (curwp));
	    sprintf (buf, buf1, DOT_POS (curwp));
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		if (ttgetc () == CTL_G)
		{
		    term_str = MSG_cmp_term;
		    break;
		}
	    }
	}
	move1 = move_ptr (wp1, 1L, TRUE, FALSE, TRUE);
	move2 = move_ptr (wp2, 1L, TRUE, FALSE, TRUE);

	if (!(move1 && move2))
	{
	    term_str = MSG_cmp_end;
	    break;
	}
    }
    writ_echo (term_str);
    wind_on_dot (wp1);
    wind_on_dot (wp2);
    return (TRUE);
}
