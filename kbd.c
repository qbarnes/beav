/*                      KBD.C
*       Terminal independent keyboard handling.
*/
#include    <string.h>
#include    "def.h"

char *keystrings ();

extern char MSG_tab[];
extern char MSG_esc[];
extern char MSG_ctl_x[];
extern char MSG_ctl[];
extern char MSG_fn[];
extern char MSG_ret[];
extern char MSG_bksp[];
extern char MSG_space[];
extern char MSG_rubout[];


/*
* Read in a key, doing the terminal
* independent prefix handling. The terminal specific
* "getkbd" routine gets the first swing, and may return
* one of the special codes used by the special keys
* on the keyboard. The "getkbd" routine returns the
* C0 controls as received; this routine moves them to
* the right spot in 11 bit code.
*/
int
getkey ()
{

    register int c;
    c = getkbd ();
    if (c == METACH)		/* M-           */
    {
	c = KMETA | getctl ();
#ifdef VT100KEY
	if ((c & KCHAR) == '[')
	    c = KMETA | KCTRL | KCTLX | getctl ();	/* flag VT100 sequence */
#endif
    }
    else if (c == CTRLCH)	/* C-           */
	c = KCTRL | getctl ();
    else if (c == CTMECH)	/* C-M-         */
	c = KCTRL | KMETA | getctl ();
    else if (c >= 0x00 && c <= 0x1F)	/* Relocate control.    */
	c = KCTRL | (c + '@');

    if (c == (KCTRL | 'X'))	/* C-X          */
	c = KCTLX | getctl ();
    return (c);
}

/*
* Used above.
*/
int
getctl ()
{

    register int c;

#if 1
    c = getkbd ();
    if (c == METACH)		/* M-           */
	c = KMETA | getctl ();
    else if (c == CTRLCH)	/* C-           */
	c = KCTRL | getctl ();
    else if (c == CTMECH)	/* C-M-         */
	c = KCTRL | KMETA | getctl ();
    else if (c >= 0x00 && c <= 0x1F)	/* Relocate control.    */
	c = KCTRL | (c + '@');
#else
    c = getkey ();		/* Note recursion   */
    if (ISLOWER (c & 0xFF))
	c = (c & ~0xFF) | TOUPPER (c & 0xFF);
    if (c >= 0x00 && c <= 0x1F)	/* Relocate control.    */
	c = KCTRL | (c + '@');
#endif
    if (ISLOWER (c & 0xFF))
	c = (c & ~0xFF) | TOUPPER (c & 0xFF);
    return (c);
}

/*
* Transform a key code into a name,
* using a table for the special keys and combination
* of some hard code and some general processing for
* the rest. None of this code is terminal specific any
* more. This makes adding keys easier.
*/
void
keyname (cp, k)
    register char *cp;
    register int k;
{
    register char *np;
    char nbuf[3];

    static char hex[] =
    {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'A', 'B',
	'C', 'D', 'E', 'F'
    };
    *cp = 0;			/* terminate previous string */
#ifdef VT100KEY
    if ((k & (KMETA | KCTRL | KCTLX)) == (int) (KMETA | KCTRL | KCTLX))
    {
	sprintf (&cp[strlen (cp)], MSG_fn);
	sprintf (&cp[strlen (cp)], "%c", k & KCHAR);
	return;
    }
#endif
    if (k & KFIRST)
    {
	if ((np = keystrings (k)) != NULL)
	{
	    if ((k & KMETA) != 0)
		sprintf (&cp[strlen (cp)], MSG_esc);

	    strcat (cp, np);
	}
	else
	    cp[strlen (cp)] = 0;/* null string */
	return;
    }

    if ((k & KCTLX) != 0)
    {
	/* Ctl-X prefix.      */
	sprintf (&cp[strlen (cp)], MSG_ctl_x);
	k &= ~KCTLX;
    }

    if ((k & KMETA) != 0)
    {
	/* Add Esc- mark.     */
	sprintf (&cp[strlen (cp)], MSG_esc);
	k &= ~KMETA;
    }

    if (k == (KCTRL | 'I'))	/* Some specials.   */
	np = MSG_tab;
    else
    {
	if (k == (KCTRL | 'M'))
	    np = MSG_ret;
	else if (k == (KCTRL | 'H'))
	    np = MSG_bksp;
	else if (k == ' ')
	    np = MSG_space;
	else if (k == 0x7F)
	    np = MSG_rubout;
	else
	{
	    if ((k & KCTRL) != 0)
	    {
		/* Add Ctl- mark.     */
		sprintf (&cp[strlen (cp)], MSG_ctl);
	    }
	    np = &nbuf[0];
	    if (((k & KCHAR) >= 0x20 && (k & KCHAR) <= 0x7E)
		|| ((k & KCHAR) >= 0xA0 && (k & KCHAR) <= 0xFE))
	    {
		nbuf[0] = k & KCHAR;	/* Graphic.     */
		nbuf[1] = 0;
	    }
	    else
	    {
		/* Non graphic.     */
		nbuf[0] = hex[(k >> 4) & 0x0F];
		nbuf[1] = hex[k & 0x0F];
		nbuf[2] = 0;
	    }
	}
    }
    strcat (cp, np);
}
