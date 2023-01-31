/*
*       Word mode commands.
* The routines in this file
* implement commands that work unit at
* a time. There are all sorts of unit mode
* commands. If I do any sentence and/or paragraph
* mode commands, they are likely to be put in
* this file.
*/
#include    "def.h"

extern BUFFER sav_buf;
char forwunit ();

/*
* Move the cursor backward by
* "n" units. All of the details of motion
* are performed by the "backchar" and "forwchar"
* routines. Error if you try to move beyond
* the buffers.
*/
char
backunit (f, n, k)
    int f, n, k;
{
    char ret = 0;

    if (n < 0)
	return (forwunit (f, -n, KRANDOM));

    curwp->w_unit_offset = 0;
    while (n--)
    {
	ret = move_ptr (curwp, -(long) R_B_PER_U (curwp), TRUE, TRUE, TRUE);
    }
    wind_on_dot (curwp);
    curwp->w_flag |= WFMODE;	/* update mode line */
    return (ret);
}

/*
* Move the cursor forward by
* the specified number of units. All of the
* motion is done by "forwchar". Error if you
* try and move beyond the buffer's end.
*/
char
forwunit (f, n, k)
    int f, n, k;
{

    if (n < 0)
	return (backunit (f, -n, KRANDOM));

    curwp->w_unit_offset = 0;
    while (n--)
    {
	move_ptr (curwp, (long) R_B_PER_U (curwp), TRUE, TRUE, TRUE);
    }
    wind_on_dot (curwp);
    curwp->w_flag |= WFMODE;	/* update mode line */
    return (TRUE);
}

/*
* Kill forward by "n" units. The rules for final
* status are now different. It is not considered an error
* to delete fewer units than you asked. This lets you say
* "kill lots of units" and have the command stop in a reasonable
* way when it hits the end of the buffer.
*/
bool
delfunit (f, n, k)
    int f, n, k;
{
    if (n < 0)
	return (FALSE);
    if ((lastflag & CFKILL) == 0)	/* Purge kill buffer.   */
	bclear (&sav_buf);
    thisflag |= CFKILL;
    while (n--)
    {
	ldelete ((A32) (R_B_PER_U (curwp)), TRUE);
    }
    curwp->w_flag |= WFHARD;
    curwp->w_unit_offset = 0;
    return (TRUE);
}

/*
* Kill backwards by "n" units. The rules
* for success and failure are now different, to prevent
* strange behavior at the start of the buffer. The command
* only fails if something goes wrong with the actual delete
* of the characters. It is successful even if no characters
* are deleted, or if you say delete 5 units, and there are
* only 4 units left. I considered making the first call
* to "backchar" special, but decided that that would just
* be wierd. Normally this is bound to "M-Rubout" and
* to "M-Backspace".
*/
bool
delbunit (f, n, k)
    int f, n, k;
{
    int size;

    if (n < 0)
	return (FALSE);
    if ((lastflag & CFKILL) == 0)	/* Purge kill buffer.   */
	bclear (&sav_buf);
    thisflag |= CFKILL;
    size = R_B_PER_U (curwp);
    while (n--)
    {
	if (move_ptr (curwp, -((long) size), TRUE, TRUE, TRUE))
	    ldelete ((A32) size, TRUE);
    }
    curwp->w_flag |= WFHARD;
    return (TRUE);
}
