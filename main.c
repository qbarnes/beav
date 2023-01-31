/*
*	BEAV is based on the source for emacs for display and keyboard handling
* functions.   The binary file handling and display formats are special
* to BEAV.   There is a full manual included in this release.   There
* are makefiles for unix and MSC 5.1 under DOS.   The old Wang PC is
* supported.   This release is for unix.   The def_unix.h file is the
* header for unix and the def_dos.h file is the header for dos.   Rename
* the appropriate .h file to def.h to convert to your os.
* 	I am willing to maintain BEAV and will entertain suggestions for
* modifications and/or bug fixes.   I can be reached at;
*
* 		pvr@wang.com
*
* or at;
*
* 		Peter Reilley
* 		19 Heritage Cir.
* 		Hudson, N.H. 03051
*/

/*
*	Bug fix log
*	3/04/91		1.20		pvr
*		Create new file with read/write permisions.
*		Fix polled mode system hog tty bug.
*		Add ANSI define for DOS.
*		Define short for D16 type.
*		Call ttclose on error exit.
*		Limit nrow and ncol to actual array size.
*		Added beavrc key binding functionallity.
*		Added delete current window command.
*		Support VT100 type function keys for binding.
*/
/*
*
*     Mainline, macro commands.
*/
#include        "def.h"

bool execute ();
void edinit ();
char flush_all ();
char quit ();
char ctrlg ();
void _lowercase ();


extern char MSG_ok[];
extern char MSG_main[];
extern char MSG_prog_name[];
extern char MSG_init_file[];
extern char MSG_init_var[];
extern char MSG_init_path[];
extern char MSG_no_mod[];
extern char MSG_no_s_chg[];
extern char MSG_auto_fl[];
extern char MSG_quit[];
extern char MSG_not_now[];
extern char MSG_st_mac[];
extern char MSG_end_mac[];
extern char MSG_num_mod[];
extern char MSG_null[];

int thisflag;			/* Flags, this command      */
int lastflag;			/* Flags, last command          */
int curgoal;			/* Goal column                  */
int com_line_flags;		/* Count of cmd line switches   */
BUFFER *curbp;			/* Current buffer               */
WINDOW *curwp;			/* Current window               */
BUFFER *bheadp;			/* BUFFER listhead              */
WINDOW *wheadp;			/* WINDOW listhead              */
BUFFER *blistp;			/* Buffer list BUFFER           */
short kbdm[NKBDM] =
{
    (KCTLX | ')')};		/* Macro (fitz)  */
short *kbdmip;			/* Input  for above             */
short *kbdmop;			/* Output for above             */
SYMBOL *symbol[NSHASH];		/* Symbol table listhead.       */
SYMBOL *binding[NKEYS];		/* Key bindings.                */
extern ROW_FMT hex_8_fmt;
extern bool ibm_pc, mem_map;

char *okmsg =
{
    MSG_ok};
int insert_mode =
{
    TRUE};
int extend_buf =
{
    FALSE};

extern bool srch_mode;
extern bool rplc_mode;
extern char *getenv ();
int initial_load = 0;
int flush_count = 0;
int flush_num = 500;
int auto_update = 0;

void
main (argc, argv)
    char *argv[];
    int argc;
{

    register int c;
    register int f;
    register int n;
    register int mflag;
    char bname[NBUFN];
    char initfile[NFILEN];

#if MSDOS
    is_wang ();			/* Check for computer type */
#endif

    init_fmt ();		/* initialize format arrays */
    strcpy (bname, MSG_main);	/* Get buffer name.     */
    ttopen ();			/* set default screen size */
    ttinit ();			/* set terminal raw mode */
    vtinit ();			/* Virtual terminal.    */
    keymapinit ();		/* Symbols, bindings.   */

#ifdef OS2
#ifdef __EMX__
    _response (&argc, &argv);
    _wildcard (&argc, &argv);
#endif

    _searchenv (MSG_init_file, MSG_init_var, initfile);
    if (initfile[0])
	check_extend (initfile);
    else
    {
	_searchenv (MSG_init_file, MSG_init_path, initfile);
	if (initfile[0])
	    check_extend (initfile);
	else
	    check_extend (getenv (MSG_prog_name));
    }
#else
    check_extend (NULL);	/* check for extended keys */
#endif

    if (argc == 1)
    {
	edinit (bname);
	eerase ();
	update ();
    }

    else
    {
	com_line_flags = 0;
	initial_load = 1;
	n = (argc - 1);		/* Load  them backwards */
	if (n > com_line_flags)
	{
	    /*            _lowercase (argv[n]); */
	    makename (bname, argv[n]);
	    edinit (bname);	/* Buffers, windows.    */
	    eerase ();
	    update ();
	    readin (argv[n--], 0L, MAXPOS);
	    for (; n > com_line_flags; n--)
	    {
		/*                _lowercase (argv[n]); */
		load_file (argv[n], 0L, MAXPOS);
	    }
	}
	else
	{
	    edinit (bname);
	    eerase ();
	    update ();
	}

	initial_load = 0;
    }

    save_buf_init ();		/* initialize save buffer */
    lastflag = 0;		/* Fake last flags.     */

  loop:
    update ();
    c = getkey ();
    if (epresf != FALSE)
    {
	eerase ();
	update ();
    }
    f = FALSE;
    n = 1;
    if (c == (KCTRL | 'U'))
    {
	/* ^U, start argument.  */
	f = TRUE;
	n = 4;
	while ((c = getkey ()) == (KCTRL | 'U'))
	    n *= 4;
	if ((c >= '0' && c <= '9') || c == '-')
	{
	    if (c == '-')
	    {
		n = 0;
		mflag = TRUE;
	    }
	    else
	    {
		n = c - '0';
		mflag = FALSE;
	    }
	    while ((c = getkey ()) >= '0' && c <= '9')
		n = 10 * n + c - '0';
	    if (mflag != FALSE)
		n = -n;
	}
    }
    if (kbdmip != NULL)
    {
	/* Save macro strokes.  */
	if (c != (KCTLX | ')') && kbdmip > &kbdm[NKBDM - 6])
	{
	    ctrlg (FALSE, 0, KRANDOM);
	    goto loop;
	}
	if (f != FALSE)
	{
	    *kbdmip++ = (KCTRL | 'U');
	    *kbdmip++ = n;
	}
	*kbdmip++ = c;
    }
    execute (c, f, n);		/* Do it.               */
    goto loop;
}

/*
* Command execution. Look up the binding in the the
* binding array, and do what it says. Return a very bad status
* if there is no binding, or if the symbol has a type that
* is not usable (there is no way to get this into a symbol table
* entry now). Also fiddle with the flags.
*/
char
execute (c, f, n)
    int c, f, n;
{

    register SYMBOL *sp;
    register int status;

    if ((sp = binding[c]) != NULL)
    {
	thisflag = 0;
	if (sp->s_modify & SMOD && (curbp->b_flag & BFVIEW))
	{
	    writ_echo (MSG_no_mod);
	    return (ABORT);
	}
	if (sp->s_modify & SSIZE && (curbp->b_flag & BFSLOCK))
	{
	    writ_echo (MSG_no_s_chg);
	    return (ABORT);
	}
	if ((srch_mode && !(sp->s_modify & SSRCH)) ||
	    (rplc_mode && !(sp->s_modify & SRPLC)))
	{
	    ttbeep ();
	    return (TRUE);
	}

	status = (*sp->s_funcp) (f, n, c);
	if (sp->s_modify & SMOD)
	    flush_count++;

	if (flush_count >= flush_num && auto_update)
	    if (!(kbdmip != NULL || kbdmop != NULL))	/* not during macro */
	    {
		ttbeep ();
		writ_echo (MSG_auto_fl);
		flush_all ();
	    }
	lastflag = thisflag;
	return (status);
    }
    else
	bad_key (c);

    lastflag = 0;
    return (ABORT);
}

/*
* Initialize all of the buffers
* and windows. The buffer name is passed down as
* an argument, because the main routine may have been
* told to read in a file by default, and we want the
* buffer name to be right.
*/
void
edinit (bname)
    char bname[];
{

    register BUFFER *bp;
    register WINDOW *wp;

    bp = bfind (bname, TRUE);	/* Text buffer.         */
    blistp = bcreate (MSG_null);/* Special list buffer. */
    wp = (WINDOW *) malloc (sizeof (WINDOW));	/* Initial window.      */
    if (bp == NULL || wp == NULL || blistp == NULL)
	abort ();
    curbp = bp;			/* Current ones.        */
    wheadp = wp;
    curwp = wp;
    wp->w_wndp = NULL;		/* Initialize window.   */
    wp->w_bufp = bp;
    bp->b_nwnd = 1;		/* Displayed.           */
    wp->w_fmt_ptr = &hex_8_fmt;	/* HEX 8 bit display       pvr  */
    wp->w_linep = bp->b_linep;
    wp->w_dotp = bp->b_linep;
    wp->w_doto = 0;		/* set dot pos  pvr */
    wp->w_markp = NULL;
    wp->w_marko = 0;
    wp->w_toprow = 0;
    wp->w_ntrows = nrow - 2;	/* 2 = mode, echo.      */
    wp->w_flag = WFMODE | WFHARD;	/* Full.                */
    wp->w_intel_mode = FALSE;	/* default is no byte swap     pvr  */
    wp->w_disp_shift = 0;	/* default to no byte shift    pvr  */
    wp->w_loff = 0;		/* starting line offset        pvr  */
    wp->w_unit_offset = 0;	/* dot offset from file start  pvr  */
}

/*
* Flush all the dirty buffers that have file names
* associated with them.
*/
char
flush_all ()
{
    register BUFFER *bp, *savbp = curbp;

    for (bp = bheadp; bp != NULL; bp = bp->b_bufp)
	if (bp->b_fname != NULL)
	{
	    curbp = bp;		/* jam */
	    filesave ();
	    update ();
	}
    flush_count = 0;
    writ_echo (okmsg);
    curbp = savbp;
    if (blistp->b_nwnd != 0)	/* update buffer display */
	listbuffers ();
    update ();
    return (TRUE);
}

/* call flush_all to empty the buffers
* and quit
*/
bool
flushnquit (f, n, k)
    int f, n, k;
{
    flush_all ();
    quit (f, n, k);
    return (TRUE);
}

/*
* Quit command. If an argument, always
* quit. Otherwise confirm if a buffer has been
* changed and not written out. Normally bound
* to "C-X C-C".
*/
char
quit (f, n, k)
    int f, n, k;
{

    register char s;

    if (f != FALSE		/* Argument forces it.  */
	|| anycb () == FALSE	/* All buffers clean.   */
	|| (s = eyesno (MSG_quit)) == TRUE)	/* User says it's OK.   */
    {

	vttidy ();
	exit (GOOD);
    }

    return (s);
}

/*
* Begin a keyboard macro.
* Error if not at the top level
* in keyboard processing. Set up
* variables and return.
*/
bool
ctlxlp (f, n, k)
    int f, n, k;
{

    if (kbdmip != NULL || kbdmop != NULL)
    {

	writ_echo (MSG_not_now);
	return (FALSE);
    }

    writ_echo (MSG_st_mac);
    kbdmip = &kbdm[0];
    return (TRUE);
}

/*
* End keyboard macro. Check for
* the same limit conditions as the
* above routine. Set up the variables
* and return to the caller.
*/
bool
ctlxrp (f, n, k)
    int f, n, k;
{

    if (kbdmip == NULL)
    {

	writ_echo (MSG_not_now);
	return (FALSE);
    }

    writ_echo (MSG_end_mac);
    kbdmip = NULL;
    return (TRUE);
}

/*
* Execute a macro.
* The command argument is the
* number of times to loop. Quit as
* soon as a command gets an error.
* Return TRUE if all ok, else
* FALSE.
*/
bool
ctlxe (f, n, k)
    int f, n, k;
{

    register int c;
    register int af;
    register int an;
    register int s;

    if (kbdmip != NULL || kbdmop != NULL)
    {

	writ_echo (MSG_not_now);
	return (FALSE);
    }

    if (n <= 0)
	return (TRUE);
    do
    {

	kbdmop = &kbdm[0];
	do
	{

	    af = FALSE;
	    an = 1;
	    if ((c = *kbdmop++) == (KCTRL | 'U'))
	    {

		af = TRUE;
		an = *kbdmop++;
		c = *kbdmop++;
	    }

	    s = TRUE;
	}
	while (c != (KCTLX | ')') && (s = execute (c, af, an)) == TRUE);
	kbdmop = NULL;
    }
    while (s == TRUE && --n);
    return (s);
}

/*
* Abort.
* Beep the beeper.
* Kill off any keyboard macro,
* etc., that is in progress.
* Sometimes called as a routine,
* to do general aborting of
* stuff.
*/
char
ctrlg (f, n, k)
    int f, n, k;
{
    /*    ttbeep (); */
    if (kbdmip != NULL)
    {
	kbdm[0] = (KCTLX | ')');
	kbdmip = NULL;
    }
    return (ABORT);
}

/*
* Display the version. All this does
* is copy the text in the external "version" array into
* the message system, and call the message reading code.
* Don't call display if there is an argument.
*/
char
showversion (f, n, k)
    int f, n, k;
{
    static char *cp;
    char buf[NCOL];

    cp = version;
    sprintf (buf, cp);
    writ_echo (buf);
    return (TRUE);
}

/* ughly to_lower function for
* files read in under MSDOS setargv function
*/
void
_lowercase (s)
    register char *s;
{

#ifdef MSDOS
    for (; *s; s++)
	if (ISUPPER (*s))
	    *s = TOLOWER (*s);
#endif
}

/* autosave control
*/
bool
autosave ()
{
    register WINDOW *wp;
    int n;
    char buf[NCOL];

    if ((ereply (MSG_num_mod, buf, sizeof (buf), NULL)) == TRUE)
    {

	n = atoi (buf);
	if (n >= 0)
	    auto_update = flush_num = n;	/* not 0! */
	else
	    auto_update = 0;
    }

    for (wp = wheadp; wp; wp = wp->w_wndp)
	if (wp->w_bufp == curbp)
	    wp->w_flag |= WFMODE;
    return (TRUE);
}
