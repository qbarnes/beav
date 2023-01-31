/*
*   Extended (M-X) commands.
*/
#include    <string.h>
#include    "def.h"

extern char MSG_not_now[];
extern char MSG_func[];
extern char MSG_unk_func[];
extern char MSG_cmd_t_ex[];
extern char MSG_unk_ext[];
extern char MSG_d_b[];
extern char MSG_unbd[];
extern char MSG_bnd_to[];
extern char MSG_ins_self[];
extern char MSG_bnd_file[];
extern char MSG_bld_wall[];
extern char MSG_wall_head[];
extern char MSG_beavrc[];
extern char MSG_null[];
extern char MSG_extended_command[];
extern char MSG_unk_rc[];


#ifdef CUSTOMIZE

char *flook ();

//static char *bindnm =
//{
//    0
//};				/* file name for customized key bindings */
#endif

/*
* This function modifies the keyboard
* binding table, by adjusting the entries in the
* big "bindings" array. Most of the grief deals with the
* prompting for additional arguments. This code does not
* work right if there is a keyboard macro floating around.
* Should be fixed.
*/
bool
bindtokey ()
{

    register int s;
    register SYMBOL *sp;
    register int c;
    char xname[NXNAME];
#ifdef CUSTOMIZE
    char xname2[NXNAME];
#endif

    if (kbdmip != NULL || kbdmop != NULL)
    {
	writ_echo (MSG_not_now);
	return (FALSE);
    }

    if ((s = eread (MSG_func, xname, NXNAME, EFAUTO, NULL)) != TRUE)
	return (s);
    if ((sp = symlookup (xname)) == NULL)
    {
	writ_echo (MSG_unk_func);
	return (FALSE);
    }

#ifdef CUSTOMIZE
    strcpy (xname2, xname);
#endif
    eputc (' ');
    eputc ('K');
    eputc ('e');
    eputc ('y');
    eputc (':');
    eputc (' ');
    ttflush ();
    c = getkey ();		/* Read key.        */
    keyname (xname, c);		/* Display keyname. */
    eputs (xname);
    ttflush ();
    if (binding[c] != NULL)	/* Unbind old, and  */
	--binding[c]->s_nkey;
    binding[c] = sp;		/* rebind new.      */
    ++sp->s_nkey;
    sp->s_modify |= SBOUND;	/* flag as altered key binding */

    return (TRUE);
}

/*
* Extended command. Call the message line
* routine to read in the command name and apply autocompletion
* to it. When it comes back, look the name up in the symbol table
* and run the command if it is found and has the right type.
* Print an error if there is anything wrong.
*/
char
extend (f, n, k)
    int f, n, k;
{

    register SYMBOL *sp;
    register char s;
    char xname[NXNAME];

    if ((s = eread (MSG_cmd_t_ex, xname, NXNAME, EFNEW | EFAUTO, NULL)) != TRUE)
	return (s);
    if ((sp = symlookup (xname)) != NULL)
	return ((*sp->s_funcp) (f, n, KRANDOM));
    writ_echo (MSG_unk_ext);
    return (ABORT);
}

/*
* Read a key from the keyboard, and look it
* up in the binding table. Display the name of the function
* currently bound to the key. Say that the key is not bound
* if it is indeed not bound, or if the type is not a
* "builtin". This is a bit of overkill, because this is the
* only kind of function there is.
*/
bool
help ()
{
    register SYMBOL *sp;
    register int c;
    char b[20];
    char buf[80];

    writ_echo (MSG_d_b);

    c = getkey ();
    keyname (b, c);
    if ((sp = binding[c]) == NULL)
    {
	sprintf (buf, MSG_unbd, b);
	writ_echo (buf);
    }
    else
    {
	sprintf (buf, MSG_bnd_to, b, sp->s_name);
	writ_echo (buf);
    }
    return (TRUE);
}

/*
*   Sort the lines in the buffer.
*/
void
sort_buf (b_ptr, cnt)
    BUFFER *b_ptr;
    int cnt;
{
    LINE *lp1, *lp2;
    bool no_swap;
    int loop1, loop2;

    for (loop1 = cnt; loop1 > 0; loop1--)
    {
	no_swap = TRUE;
	lp1 = b_ptr->b_linep->l_fp;	/* point to first line */
	lp2 = lp1->l_fp;	/* point to next line */
	for (loop2 = 0; loop2 <= loop1; loop2++)
	{
	    /* compare strings and swap if necessary */
	    if (0 < strcmp (&lp1->l_text[HFUNCCOL], &lp2->l_text[HFUNCCOL]))
	    {
		lp1->l_bp->l_fp = lp2;	/* get pointer to first string */
		lp2->l_fp->l_bp = lp1;	/* make it point to second string */

		lp1->l_fp = lp2->l_fp;
		lp2->l_bp = lp1->l_bp;

		lp1->l_bp = lp2;
		lp2->l_fp = lp1;

		lp2->l_file_offset = lp1->l_file_offset;
		lp1->l_file_offset = lp2->l_file_offset + lp2->l_used;

		no_swap = FALSE;
	    }
	    else
	    {
		/* if no swap then advance both pointers */
		lp1 = lp2;
	    }
	    lp2 = lp1->l_fp;
	}
	/* quick exit if sort is finished sooner than expected */
	if (no_swap)
	{
	    return;
	}
    }
}

/*
* This function creates a table, listing all
* of the command keys and their current bindings, and stores
* the table in the standard pop-op buffer (the one used by the
* directory list command, the buffer list command, etc.). This
* lets the editor produce it's own wall chart. The bindings to
* "ins-self" are only displayed if there is an argument.
*/
char
wallchart (f, n, k)
    int f, n, k;
{

    register char s;
    register int key, i, j;
    register SYMBOL *sp;
    register char *cp1;
    register char *cp2;
    char buf[64];
    WINDOW *wp;

    if ((s = bclear (blistp)) != TRUE)	/* Clear it out.    */
	return (s);
    i = 0;
    (void) strcpy (blistp->b_fname, MSG_null);
    blistp->b_flag = BFVIEW;
    blistp->b_type = BTHELP;
    writ_echo (MSG_bld_wall);
    sprintf (buf, MSG_wall_head);
    if (addline (buf) == FALSE)
	return (FALSE);
    for (key = 0; key < NKEYS; ++key)
    {
	/* For all keys.    */
	sp = binding[key];
	if (sp != NULL &&
	    (f != FALSE || strcmp (sp->s_name, MSG_ins_self) != 0))
	{
	    cp1 = &buf[0];
	    while (cp1 < &buf[HFUNCCOL])	/* Goto column 3.  */
		*cp1++ = ' ';
	    if ((sp->s_modify & SBOUND) == 0)	/* comment out default binding */
		buf[0] = '#';
	    cp2 = sp->s_name;	/* Add function name.   */
	    while ((*cp1++ = *cp2++))
		;
	    cp1--;
	    while (cp1 < &buf[HKEY])	/* Goto column 32.  */
		*cp1++ = ' ';
	    keyname (&buf[HKEY], key);
	    cp1 = &buf[strlen (buf)];
	    while (cp1 < &buf[HKEYCODE])	/* Goto column 50.  */
		*cp1++ = ' ';
	    sprintf (&buf[HKEYCODE], "%4X", key);
	    if (addline (buf) == FALSE)
		break;		/* lets go with what we have */
	    i++;
	}
    }

    /* list unbound functions lest they get lost */
    for (j = 0; j < NSHASH; j++)
    {
	sp = symbol[j];
	while (sp != NULL)
	{
	    if (sp->s_nkey == 0)
	    {
		cp1 = &buf[0];
		while (cp1 < &buf[HFUNCCOL])	/* Goto column 3.  */
		    *cp1++ = ' ';
		buf[0] = '#';
		cp2 = sp->s_name;	/* Add function name.   */
		while ((*cp1++ = *cp2++))
		    ;
		cp1--;
		while (cp1 < &buf[HENDCOL])
		    *cp1++ = ' ';
		*cp1 = 0;
		i++;
		if (addline (buf) == FALSE)
		    break;	/* lets go with what we have */
	    }
	    sp = sp->s_symp;
	}
    }
    sort_buf (blistp, i);	/* sort buffer lines */
    popblist ();
    writ_echo (MSG_null);
    /* make new window the current window */
    wp = wheadp;
    while (wp != NULL)
    {
	if (wp->w_bufp == blistp)
	{
	    curwp = wp;
	    curbp = wp->w_bufp;
	    return (TRUE);
	}
	wp = wp->w_wndp;
    }
    return (TRUE);
}

/* check for RC file and read it in if found
* - also, set local file variable for bindtokey for saving new defs
* (this is some what of a hack as it only handles 'bindtokey' changes at
* this time - also local file io !!!)
*/
void
check_extend (sfname)

    char *sfname;		/* name of startup file (null if default) */

{
    char *fname;		/* resulting file name to execute */
    char rc_name[NFILEN];	/* fixed up name of rc file */
    char *term;
    char *getenv ();
    register SYMBOL *sp;
    char funcname[NXNAME + 1];
    char keybind[NXNAME + 1];
    int keyval;
    FILE *bindf;

    /* look up the startup file */
    if ((sfname != NULL) && (*sfname != 0))
	fname = flook (sfname, TRUE);
    else
    {
#ifdef UNIX
	/* hidden file under unix */
	strcpy (&rc_name[0], ".");
	strcpy (&rc_name[1], MSG_beavrc);

	if ((term = getenv ("TERM")) != 0)
	{
	    strcpy (&rc_name[strlen (rc_name)], ".");
	    strcpy (&rc_name[strlen (rc_name)], term);
	}
	fname = flook (rc_name, TRUE);
	/* if fixed up name is not there then check original */
	if (fname == NULL)
	{
	    /* hidden file under unix */
	    strcpy (&rc_name[0], ".");
	    strcpy (&rc_name[1], MSG_beavrc);
	    fname = flook (rc_name, TRUE);
	}
#else
	strcpy (rc_name, MSG_beavrc);
	fname = flook (rc_name, TRUE);
#ifdef AMIGA
	/* look for .beavrc in the current directory */
	if (!fname)
	{
	    rc_name[0] = '.';
	    strcpy (&rc_name[1], MSG_beavrc);
	    fname = flook (rc_name, TRUE);
	}
	/* look for .beavrc in S: */
	if (!fname)
	{
	    /* Have a look in startup directory */
	    rc_name[0] = 'S';
	    rc_name[1] = ':';
	    rc_name[2] = '.';
	    strcpy (&rc_name[3], MSG_beavrc);
	    fname = flook (rc_name, TRUE);
	}
#endif /* AMIGA */
#endif
    }
    /* if it isn't around, don't sweat it */
    if (fname == NULL)
	return;

    if ((bindf = fopen (fname, "r")))
    {
	char buffr[80];
	char *buffp;

	buffp = buffr;
	while (fread (buffp++, sizeof (char), 1, bindf) == 1)
	{
	    /* scanf is unhappy with commas */
	    if (buffp[-1] == ',')
		buffp[-1] = '-';

	    /* did we get a whole line */
	    if (buffp[-1] == '\n')
	    {
		*buffp = 0;	/* terminate line */
		buffp = buffr;
		sscanf (buffr, "%s %s %x", funcname, keybind, &keyval);
		if ((buffr[0] == '#') || (keyval == 0))
		    continue;
		/* check if this is a command to execute */
		if ((strcmp (funcname, MSG_extended_command) == 0) &&
		    (keybind[0] > 'Z'))
		{
		    if ((sp = symlookup (keybind)) != NULL)
			(*sp->s_funcp) (TRUE, keyval, KRANDOM);
		    else
		    {
			char temp_b[68];

			sprintf (temp_b, "%s %s", MSG_unk_rc, keybind);
			writ_echo (temp_b);
		    }
		}
		else
		{
		    if ((sp = symlookup (funcname)))
		    {
			if (binding[keyval] != NULL)	/* Unbind old, and  */
			    --binding[keyval]->s_nkey;
			binding[keyval] = sp;	/* rebind new.      */
			++sp->s_nkey;
			sp->s_modify |= SBOUND;	/* flag as altered key binding */
		    }
		}
	    }
	}
	fclose (bindf);
    }
}

/*	Look up the existance of a file along the normal or PATH
	environment variable. Look first in the HOME directory if
	asked and possible
*/

char *
flook (fname, hflag)

    char *fname;		/* base file name to search for */
    int hflag;			/* Look in the HOME environment variable first? */

{
    register char *home;	/* path to home directory */
    register char *path;	/* environmental PATH variable */
    register char *sp;		/* pointer into path spec */
    //register int i;		/* index */
    static char fspec[NFILEN * 2];	/* full path spec to search */
    char *getenv ();
    FILE *bindf;

    if (hflag)
    {
	home = getenv ("HOME");
	if (home != NULL)
	{
	    /* build home dir file spec */
	    strcpy (fspec, home);
	    if (fspec[strlen (fspec) - 1] != '/')
		strcat (fspec, "/");
	    strcat (fspec, fname);

	    /* and try it out */
	    if ((bindf = fopen (fspec, "r")))
	    {
		fclose (bindf);
		return (fspec);
	    }
	}
    }

    /* always try the current directory first */
    if ((bindf = fopen (fname, "r")))
    {
	fclose (bindf);
	return (fname);
    }

    /* get the PATH variable */
    path = getenv ("PATH");
    if (path != NULL)
	while (*path)
	{

	    /* build next possible file spec */
	    sp = fspec;
	    while (*path && (*path != PATHCHR))
		*sp++ = *path++;

	    /* add a terminating dir separator if we need it */
	    if (sp[-1] != SEPCHAR)
		*sp++ = SEPCHAR;

	    *sp = 0;
	    strcat (fspec, fname);

	    /* and try it out */
	    if ((bindf = fopen (fspec, "r")))
	    {
		fclose (bindf);
		return (fspec);
	    }

	    if (*path == PATHCHR)
		++path;
	}

    return (NULL);		/* no such luck */
}

/* interactive method for loading binding file
* (uses above routine, obviously)
*/
char
load_extend ()
{

#ifdef CUSTOMIZE
    register char s;
    char fname[NFILEN];

    if ((s = ereply (MSG_bnd_file, fname, NFILEN, NULL)) != TRUE)
	return (s);
    check_extend (fname);
    writ_echo (okmsg);
#endif
    return (TRUE);
}

int
find_keyval (name)
    char *name;
{
    SYMBOL *sp;
    int key;

    for (key = 0; key < NKEYS; ++key)
    {
	/* For all keys.    */
	sp = binding[key];
	if (sp != NULL && (strcmp (sp->s_name, name) == 0))
	    return (key);
    }
    return (0);
}
