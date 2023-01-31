/*
*       Echo line reading and writing.
* Common routines for reading
* and writing characters in the echo line area
* of the display screen. Used by the entire
* known universe.
*/
#include    "def.h"

void eerase ();
char ereply ();
char eread ();
void eformat ();
void eputi ();
void eputs ();
void eputc ();


extern char MSG_null[];
extern char MSG_y_n[];
extern char MSG_hex_dig[];
extern char MSG_hit_key[];

int epresf = FALSE;		/* Stuff in echo line flag. */

/*
* Erase the echo line.
*/
void
eerase ()
{
    writ_echo (MSG_null);	/* clear the echo line */
    epresf = FALSE;
}

/*
* Ask "yes" or "no" question.
* Return ABORT if the user answers the question
* with the abort ("^G") character. Return FALSE
* for "no" and TRUE for "yes". No formatting
* services are available.
*/
char
eyesno (sp)
    char *sp;
{

    register char s;
    char buf[NCOL];

    for (;;)
    {

	s = ereply (MSG_y_n, buf, sizeof (buf), sp);
	if (s == ABORT)
	    return (ABORT);
	if (s != FALSE)
	{

	    if (buf[0] == 'y' || buf[0] == 'Y')
		return (TRUE);
	    if (buf[0] == 'n' || buf[0] == 'N')
		return (FALSE);
	}

    }

}

/*
* Write out a prompt, and read back a
* reply. The prompt is now written out with full "eprintf"
* formatting, although the arguments are in a rather strange
* place. This is always a new message, there is no auto
* completion, and the return is echoed as such.
*/
/* VARARGS3 */
char
ereply (fp, buf, nbuf, arg)
    char *fp;
    char *buf;
    int nbuf;
    char *arg;
{
    return (eread (fp, buf, nbuf, EFNEW | EFCR, arg));
}

/*
* This is the general "read input from the
* echo line" routine. The basic idea is that the prompt
* string "prompt" is written to the echo line, and a one
* line reply is read back into the supplied "buf" (with
* maximum length "len"). The "flag" contains EFNEW (a
* new prompt), an EFAUTO (autocomplete), or EFCR (echo
* the carriage return as CR).
*/
char
eread (fp, buf, nbuf, flag, ap)
    char *fp;
    char *buf;
    char *ap;
    int nbuf, flag;
{

    register int cpos;
    register SYMBOL *sp1;
    register SYMBOL *sp2;
    register int i;
    register int c;
    register int h;
    register int nhits;
    register int nxtra;
    register int bxtra;

    int quote_flag;

    quote_flag = 0;
    cpos = 0;
    if (kbdmop != NULL)
    {
	/* In a macro.      */
	while ((c = *kbdmop++) != '\0')
	    buf[cpos++] = c;
	buf[cpos] = '\0';
	goto done;
    }

    if ((flag & EFNEW) != 0 || ttrow != nrow - 1)
    {

	ttcolor (CTEXT);
	ttmove (nrow - 1, 0);
	epresf = TRUE;
    }
    else
	eputc (' ');
    eformat (fp, ap);
    tteeol ();
    ttflush ();
    for (;;)
    {
	c = getkey ();
	if (c == ' ' && (flag & EFAUTO) != 0)
	{
	    nhits = 0;
	    nxtra = HUGE;
	    for (h = 0; h < NSHASH; ++h)
	    {
		sp1 = symbol[h];
		while (sp1 != NULL)
		{
		    for (i = 0; i < cpos; ++i)
		    {
			if (buf[i] != sp1->s_name[i])
			    break;
		    }

		    if (i == cpos)
		    {
			if (nhits == 0)
			    sp2 = sp1;
			++nhits;
			bxtra = getxtra (sp1, sp2, cpos);
			if (bxtra < nxtra)
			    nxtra = bxtra;
		    }

		    sp1 = sp1->s_symp;
		}
	    }

	    if (nhits == 0)	/* No completion.   */
		continue;
	    for (i = 0; i < nxtra && cpos < nbuf - 1; ++i)
	    {
		c = sp2->s_name[cpos];
		buf[cpos++] = c;
		eputc (c);
	    }

	    ttflush ();
	    if (nhits != 1)	/* Fake a CR if there   */
		continue;	/* is 1 choice.     */
	    c = (KCTRL | 'M');
	}
	if (quote_flag)
	{
	    c = c & 0x1f;
	    quote_flag = 0;
	}


	switch (c)
	{
	case (KCTRL | 'Q'):
	    quote_flag = 1;
	    break;
	case (KCTRL | 'M'):	/* Return, done.    */
	case (KCTRL | 'J'):	/* Linefeed, done.  */
	    buf[cpos] = '\0';
	    if (kbdmip != NULL)
	    {
		if (kbdmip + cpos + 1 > &kbdm[NKBDM - 3])
		{
		    (void) ctrlg (FALSE, 0, KRANDOM);
		    ttflush ();
		    return (ABORT);
		}

		for (i = 0; i < cpos; ++i)
		    *kbdmip++ = buf[i];
		*kbdmip++ = '\0';
	    }

	    if ((flag & EFCR) != 0)
	    {
		ttputc (0x0D);
		ttflush ();
	    }

	    goto done;

	case (KCTRL | 'G'):	/* Bell, abort.     */
	    eputc (0x07);
	    (void) ctrlg (FALSE, 0, KRANDOM);
	    ttflush ();
	    return (ABORT);

	case 0x7F:		/* Rubout, erase.   */
	case (KCTRL | 'H'):	/* Backspace, erase.    */
	    if (cpos != 0)
	    {
		ttputc ('\b');
		ttputc (' ');
		ttputc ('\b');
		--ttcol;
		if (ISCTRL (buf[--cpos]) != FALSE)
		{
		    ttputc ('\b');
		    ttputc (' ');
		    ttputc ('\b');
		    --ttcol;
		}

		ttflush ();
	    }
	    break;

	case (KCTRL | 'U'):	/* C-U, kill line.  */
	    while (cpos != 0)
	    {
		ttputc ('\b');
		ttputc (' ');
		ttputc ('\b');
		--ttcol;
		if (ISCTRL (buf[--cpos]) != FALSE)
		{
		    ttputc ('\b');
		    ttputc (' ');
		    ttputc ('\b');
		    --ttcol;
		}

	    }

	    ttflush ();
	    break;

	default:		/* All the rest.    */
	    if ((cpos < nbuf - 1) && ((c & ~KCHAR) == 0))
	    {
		buf[cpos++] = c;
		eputc (c);
		ttflush ();
	    }
	}			/* End switch */

    }

  done:
    if (buf[0] == '\0')
	return (FALSE);
    return (TRUE);
}

/*
* The "sp1" and "sp2" point to extended command
* symbol table entries. The "cpos" is a horizontal position
* in the name. Return the longest block of characters that can
* be autocompleted at this point. Sometimes the two symbols
* are the same, but this is normal.
*/
int
getxtra (sp1, sp2, cpos)
    SYMBOL *sp1;
    SYMBOL *sp2;
    int cpos;
{

    register int i;

    i = cpos;
    for (;;)
    {

	if (sp1->s_name[i] != sp2->s_name[i])
	    break;
	if (sp1->s_name[i] == '\0')
	    break;
	++i;
    }

    return (i - cpos);
}

/*
* Printf style formatting. This is
* called by both "eprintf" and "ereply", to provide
* formatting services to their clients. The move to the
* start of the echo line, and the erase to the end of
* the echo line, is done by the caller.
*/
void
eformat (fp, ap)
    char *fp;
    char *ap;
{

    register int c;

    while ((c = *fp++) != '\0')
    {

	if (c != '%')
	    eputc (c);
	else
	{

	    c = *fp++;
	    switch (c)
	    {

	    case 'd':
		eputi (*(int *) ap, 10);
		ap += sizeof (int);
		break;

	    case 'x':		/* krw */
		eputi (*(int *) ap, 16);
		ap += sizeof (int);
		break;

	    case 'o':
		eputi (*(int *) ap, 8);
		ap += sizeof (int);
		break;

	    case 's':
		eputs (ap);
		ap += sizeof (char *);
		break;

	    default:
		eputc (c);
	    }

	}

    }

}

/*
* Put integer, in radix "r".
*/
void
eputi (i, r)
    int i;
    int r;
{
    static char *convert =
    {
	MSG_hex_dig
    };


    register int q;

    if ((q = i / r) != 0)
	eputi (q, r);
    eputc (convert[i % r]);

}

/*
* Put string.
*/
void
eputs (s)
    char *s;
{
    register int c;

    while ((c = *s++) != '\0')
	eputc (c);
}

/*
* Put character. Watch for
* control characters, and for the line
* getting too long.
*/
void
eputc (c)
    int c;
{

    if (ttcol < ncol)
    {

	if (ISCTRL (c) != FALSE)
	{

	    eputc ('^');
	    c ^= 0x40;
	}

	ttputc (c);
	++ttcol;
    }

}

/*
 *   Print warning message and wait for the user to hit a key.
 */
void
err_echo (buf)
    char *buf;
{
    char ch[NCOL * 2];

    strcpy (ch, buf);
    strcat (ch, MSG_hit_key);
    writ_echo (ch);
    ttbeep ();
    while (ttgetc () != CTL_G);
    {
	ttbeep ();
	ttflush ();
    }
}
