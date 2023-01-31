/*
*       Window handling.
*/
#include    <stdlib.h>
#include    "def.h"

bool mvupwind ();
bool shrinkwind ();

extern char MSG_no_splt[];
extern char MSG_cnt_al_w[];
extern char MSG_one_w[];
extern char MSG_imp_chg[];
extern char MSG_scrn_rows[];
extern char MSG_bad_num[];

/*
* Reposition the window so as to center on the dot.
*/
bool
reposition ()
{
    long l_val;

    l_val = DOT_POS (curwp) - (curwp->w_ntrows * R_BYTES (curwp) / 2L);
    move_ptr (curwp, l_val, FALSE, TRUE, FALSE);
    curwp->w_flag |= WFHARD;
    return (TRUE);
}

/*
* The command make the next
* window (next => down the screen)
* the current window. There are no real
* errors, although the command does
* nothing if there is only 1 window on
* the screen.
*/
bool
nextwind ()
{

    register WINDOW *wp;

    if ((wp = curwp->w_wndp) == NULL)
	wp = wheadp;
    curwp = wp;
    curbp = wp->w_bufp;
    return (TRUE);
}

/*
* This command makes the previous
* window (previous => up the screen) the
* current window. There arn't any errors,
* although the command does not do a lot
* if there is 1 window.
*/
bool
prevwind ()
{

    register WINDOW *wp1;
    register WINDOW *wp2;

    wp1 = wheadp;
    wp2 = curwp;
    if (wp1 == wp2)
	wp2 = NULL;
    while (wp1->w_wndp != wp2)
	wp1 = wp1->w_wndp;
    curwp = wp1;
    curbp = wp1->w_bufp;
    return (TRUE);
}

/*
* This command moves the current
* window down by "arg" lines. Recompute
* the top line in the window. The move up and
* move down code is almost completely the same;
* most of the work has to do with reframing the
* window, and picking a new dot. We share the
* code by having "move down" just be an interface
* to "move up".
*/
bool
mvdnwind (f, n, k)
    int f, n, k;
{
    return (mvupwind (f, -n, KRANDOM));
}

/*
* Move the current window up by "arg"
* lines. Recompute the new top line of the window.
* Look to see if "." is still on the screen. If it is,
* you win. If it isn't, then move "." to center it
* in the new framing of the window (this command does
* not really move "."; it moves the frame).
*/
bool
mvupwind (f, n, k)
    int f, n, k;
{
    A32 l_val, l_bytes;

    l_bytes = (A32) R_BYTES (curwp);	/* number of bytes in a row */
    l_val = n * l_bytes;	/* number of bytes to move */
    move_ptr (curwp, l_val, FALSE, TRUE, TRUE);	/* move window */

    /* check that dot is in window */
    while (DOT_POS (curwp) < WIND_POS (curwp))
    {
	/* dot is before the first window line */
	move_ptr (curwp, l_bytes, TRUE, TRUE, TRUE);
    }
    while (DOT_POS (curwp) >=
	   ((l_bytes * curwp->w_ntrows) + WIND_POS (curwp)))
    {
	/* dot is after the last window line */
	move_ptr (curwp, -l_bytes, TRUE, TRUE, TRUE);
    }
    curwp->w_flag |= WFHARD;
    return (TRUE);
}

/*
* This command makes the current
* window the only window on the screen.
* Try to set the framing
* so that "." does not have to move on
* the display. Some care has to be taken
* to keep the values of dot and mark
* in the buffer structures right if the
* distruction of a window makes a buffer
* become undisplayed.
*/
bool
onlywind ()
{

    register WINDOW *wp;
    register LINE *lp;
    register int i;

    while (wheadp != curwp)
    {

	wp = wheadp;
	wheadp = wp->w_wndp;
	if (--wp->w_bufp->b_nwnd == 0)
	{

	    wp->w_bufp->b_dotp = wp->w_dotp;
	    wp->w_bufp->b_doto = wp->w_doto;
	    wp->w_bufp->b_markp = wp->w_markp;
	    wp->w_bufp->b_marko = wp->w_marko;
	}

	free ((char *) wp);
    }

    while (curwp->w_wndp != NULL)
    {

	wp = curwp->w_wndp;
	curwp->w_wndp = wp->w_wndp;
	if (--wp->w_bufp->b_nwnd == 0)
	{

	    wp->w_bufp->b_dotp = wp->w_dotp;
	    wp->w_bufp->b_doto = wp->w_doto;
	    wp->w_bufp->b_markp = wp->w_markp;
	    wp->w_bufp->b_marko = wp->w_marko;
	}

	free ((char *) wp);
    }

    lp = curwp->w_linep;
    i = curwp->w_toprow;
    while (i != 0 && lback (lp) != curbp->b_linep)
    {

	--i;
	lp = lback (lp);
    }

    curwp->w_toprow = 0;
    curwp->w_ntrows = nrow - 2;	/* 2 = mode, echo.  */
    curwp->w_linep = lp;
    curwp->w_flag |= WFMODE | WFHARD;
    return (TRUE);
}

/*
 * Delete the current window, placing its space in the window above,
 * or, if it is the top window, the window below. Bound to C-X 0.
 */

bool
delwind ()

{
    register WINDOW *wp;	/* window to recieve deleted space */
    register WINDOW *lwp;	/* ptr window before curwp */
    register int target;	/* target line to search for */

    /* if there is only one window, don't delete it */
    if (wheadp->w_wndp == NULL)
    {
	return (FALSE);
    }

    /* find window before curwp in linked list */
    wp = wheadp;
    lwp = NULL;
    while (wp != NULL)
    {
	if (wp == curwp)
	    break;
	lwp = wp;
	wp = wp->w_wndp;
    }

    /* find recieving window and give up our space */
    wp = wheadp;
    if (curwp->w_toprow == 0)
    {
	/* find the next window down */
	target = curwp->w_ntrows + 1;
	while (wp != NULL)
	{
	    if (wp->w_toprow == target)
		break;
	    wp = wp->w_wndp;
	}
	if (wp == NULL)
	    return (FALSE);
	wp->w_toprow = 0;
	wp->w_ntrows += target;
    }
    else
    {
	/* find the next window up */
	target = curwp->w_toprow - 1;
	while (wp != NULL)
	{
	    if ((wp->w_toprow + wp->w_ntrows) == target)
		break;
	    wp = wp->w_wndp;
	}
	if (wp == NULL)
	    return (FALSE);
	wp->w_ntrows += 1 + curwp->w_ntrows;
    }

    /* get rid of the current window */
    if (--curwp->w_bufp->b_nwnd == 0)
    {
	curwp->w_bufp->b_dotp = curwp->w_dotp;
	curwp->w_bufp->b_doto = curwp->w_doto;
	curwp->w_bufp->b_markp = curwp->w_markp;
	curwp->w_bufp->b_marko = curwp->w_marko;
    }
    if (lwp == NULL)
	wheadp = curwp->w_wndp;
    else
	lwp->w_wndp = curwp->w_wndp;
    free ((char *) curwp);
    curwp = wp;
    wp->w_flag |= WFMODE | WFHARD;
    curbp = wp->w_bufp;
    return (TRUE);
}

/*
* Split the current window. A window
* smaller than 3 lines cannot be split.
* The only other error that is possible is
* a "malloc" failure allocating the structure
* for the new window.
*/
bool
splitwind ()
{

    register WINDOW *wp;
    register int ntru;
    register int ntrl;
    //register int ntrd;
    //register WINDOW *wp1;
    //register WINDOW *wp2;
    char buf[NCOL], buf1[NCOL];

    if (curwp->w_ntrows < MIN_WIN_ROWS)
    {
	sprintf (buf1, MSG_no_splt, R_BYTE_FMT (curwp));
	sprintf (buf, buf1, curwp->w_ntrows);
	writ_echo (buf);
	return (FALSE);
    }

    if ((wp = (WINDOW *) malloc (sizeof (WINDOW))) == NULL)
    {
	err_echo (MSG_cnt_al_w);
	return (FALSE);
    }

    ++curbp->b_nwnd;		/* Displayed twice.  */
    wp->w_bufp = curbp;
    wp->w_dotp = curwp->w_dotp;
    wp->w_doto = curwp->w_doto;
    wp->w_unit_offset = curwp->w_unit_offset;
    wp->w_markp = curwp->w_markp;
    wp->w_marko = curwp->w_marko;
    wp->w_flag = 0;
    wp->w_disp_shift = curwp->w_disp_shift;
    wp->w_intel_mode = curwp->w_intel_mode;
    wp->w_fmt_ptr = curwp->w_fmt_ptr;
    ntru = (curwp->w_ntrows - 1) / 2;	/* Upper size         */
    ntrl = (curwp->w_ntrows - 1) - ntru;	/* Lower size      */

    curwp->w_ntrows = ntru;
    wp->w_wndp = curwp->w_wndp;
    curwp->w_wndp = wp;
    wp->w_toprow = curwp->w_toprow + ntru + 1;
    wp->w_ntrows = ntrl;

    wind_on_dot (curwp);	/* put window on the dot */
    wp->w_loff = curwp->w_loff;	/* do the same for the new window */
    wp->w_linep = curwp->w_linep;
    curwp->w_flag |= WFMODE | WFHARD;
    wp->w_flag |= WFMODE | WFHARD;
    return (TRUE);
}

/*
* Enlarge the current window.
* Find the window that loses space. Make
* sure it is big enough. If so, hack the window
* descriptions, and ask redisplay to do all the
* hard work. You don't just set "force reframe"
* because dot would move.
*/
bool
enlargewind (f, n, k)
    int f, n, k;
{
    register WINDOW *adjwp;
    register LINE *lp;
    register int i;

    if (n < 0)
	return (shrinkwind (f, -n, KRANDOM));
    if (wheadp->w_wndp == NULL)
    {

	writ_echo (MSG_one_w);
	return (FALSE);
    }

    if ((adjwp = curwp->w_wndp) == NULL)
    {
	adjwp = wheadp;
	while (adjwp->w_wndp != curwp)
	    adjwp = adjwp->w_wndp;
    }

    if (adjwp->w_ntrows <= n)
    {
	writ_echo (MSG_imp_chg);
	return (FALSE);
    }

    if (curwp->w_wndp == adjwp)
    {
	/* Shrink below.     */
	lp = adjwp->w_linep;
	for (i = 0; i < n && lp != adjwp->w_bufp->b_linep; ++i)
	    lp = lforw (lp);
	adjwp->w_linep = lp;
	adjwp->w_toprow += n;
    }
    else
    {
	/* Shrink above.     */
	lp = curwp->w_linep;
	for (i = 0; i < n && lback (lp) != curbp->b_linep; ++i)
	    lp = lback (lp);
	curwp->w_linep = lp;
	curwp->w_toprow -= n;
    }

    curwp->w_ntrows += n;
    adjwp->w_ntrows -= n;
    curwp->w_flag |= WFMODE | WFHARD;
    adjwp->w_flag |= WFMODE | WFHARD;
    return (TRUE);
}

/*
* Shrink the current window.
* Find the window that gains space. Hack at
* the window descriptions. Ask the redisplay to
* do all the hard work.
*/
bool
shrinkwind (f, n, k)
    int f, n, k;
{
    register WINDOW *adjwp;
    register LINE *lp;
    register int i;

    if (n < 0)
	return (enlargewind (f, -n, KRANDOM));
    if (wheadp->w_wndp == NULL)
    {
	writ_echo (MSG_one_w);
	return (FALSE);
    }

    if ((adjwp = curwp->w_wndp) == NULL)
    {
	adjwp = wheadp;
	while (adjwp->w_wndp != curwp)
	    adjwp = adjwp->w_wndp;
    }

    if (curwp->w_ntrows <= n)
    {
	writ_echo (MSG_imp_chg);
	return (FALSE);
    }

    if (curwp->w_wndp == adjwp)
    {
	/* Grow below.       */
	lp = adjwp->w_linep;
	for (i = 0; i < n && lback (lp) != adjwp->w_bufp->b_linep; ++i)
	    lp = lback (lp);
	adjwp->w_linep = lp;
	adjwp->w_toprow -= n;
    }
    else
    {
	/* Grow above.       */
	lp = curwp->w_linep;
	for (i = 0; i < n && lp != curbp->b_linep; ++i)
	    lp = lforw (lp);
	curwp->w_linep = lp;
	curwp->w_toprow += n;
    }

    curwp->w_ntrows -= n;
    adjwp->w_ntrows += n;
    curwp->w_flag |= WFMODE | WFHARD;
    adjwp->w_flag |= WFMODE | WFHARD;
    return (TRUE);
}

/*
* Pick a window for a pop-up.
* Split the screen if there is only
* one window. Pick the uppermost window that
* isn't the current window. An LRU algorithm
* might be better. Return a pointer, or
* NULL on error.
*/
WINDOW *
wpopup ()
{

    register WINDOW *wp;

    if (wheadp->w_wndp == NULL
	&& splitwind () == FALSE)
	return (NULL);
    wp = wheadp;		/* Find window to use    */
    while (wp != NULL && wp == curwp)
	wp = wp->w_wndp;
    return (wp);
}

/*
* Refresh the display.
* In the normal case the
* call to "update" in "main.c" refreshes the screen,
* and all of the windows need not be recomputed.
*/
bool
refresh ()
{
    sgarbf = TRUE;
    return (TRUE);
}

/*  pvr
* Set the number of rows on the screen.
* This sets the nrow value and fixes up the windows
* to account for the new size.
*/
bool
screen_rows (f, n, k)
    int f, n, k;
{
    int new_rows;
    register int s;
    char buf[80];
    WINDOW *wp, *cur_wp_sav;

    cur_wp_sav = curwp;

    new_rows = nrow;		/* default it if sscanf reads nothing */
    sprintf (buf, "%s [%d] ", MSG_scrn_rows, nrow);
    if (f == FALSE)
    {
	if ((s = ereply (buf, buf, sizeof (buf), 0) != TRUE))
	    return (s);
	sscanf (buf, "%d", &new_rows);
    }
    else
	new_rows = n;

    if (new_rows <= MIN_WIN_ROWS)
    {
	writ_echo (MSG_bad_num);
	return (FALSE);
    }

    ttmove (0, 0);
    tteeop ();
    refresh ();

    /* set new nrow value */
    nrow = new_rows;

    while (TRUE)
    {
	/* find last window */
	for (wp = wheadp; wp->w_wndp; wp = wp->w_wndp)
	    wp->w_flag |= WFHARD;	/* update all windows */

	wp->w_flag |= WFHARD;	/* update last windows */

	/* if no part of window can fit, kill it */
	if ((wp->w_toprow + MIN_WIN_ROWS) >= new_rows)
	{
	    /* don't kill last window */
	    if (wp == wheadp)
	    {
		nrow = MIN_WIN_ROWS + 2;	/* set min size */
		wp->w_ntrows = MIN_WIN_ROWS;
		wind_on_dot (wp);
		break;
	    }
	    curwp = wp;
	    curbp = wp->w_bufp;
	    delwind ();
	}
	else
	{
	    /* adjust window to fit */
	    wp->w_ntrows = new_rows - wp->w_toprow - 2;
	    wind_on_dot (wp);
	    break;
	}
    }
    /* free up old screen memory */
    vtfree ();
    /* recreate scerrn buffers */
    vtinit ();
    eerase ();			/* clean up prompt line */
    return (TRUE);
}
