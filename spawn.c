/*
*    MS-DOS spawn command.com
*/
#include        "def.h"

#ifdef OS2
#define MSDOS  1
#endif

extern char MSG_shell[];
extern char MSG_def_shell[];
extern char MSG_pmpt[];
extern char MSG_pt[];
extern char MSG_pme[];
extern char MSG_null[];

/* #include     <dos.h> */
#ifdef MSDOS
#include        "process.h"
#endif
char *cspec = NULL;		/* Command string.      */
char *pspec = NULL;

/*
* Create a subjob with a copy
* of the command intrepreter in it. When the
* command interpreter exits, mark the screen as
* garbage so that you do a full repaint.
*/
bool
spawncli (f, n, k)
    int f, n, k;
{
#ifdef MSDOS

    char *getenv ();
    char old_prompt[NCOL];
    char prompt_line[NCOL];

    ttcolor (CTEXT);		/* Normal color.        */
    ttmove (nrow - 1, 0);	/* Last line.           */
    ttflush ();
    ttcooked ();
#ifndef OS2
#ifndef IBM
    strcpy (prompt_line, MSG_pmpt);
    pspec = getenv (MSG_pt);
    strcpy (old_prompt, pspec);
    strcat (prompt_line, pspec);
    if (strlen (prompt_line - strlen (MSG_pme)) >= 64)
	/* VERY rude, but setenv not found */
    {
	if (putenv (MSG_pmpt) == -1)
	    exit (1);
    }
    else if (putenv (prompt_line) == -1)
	exit (1);

#endif
#endif
    if (!cspec && !(cspec = getenv (MSG_shell)))	/* jam */
	cspec = MSG_def_shell;
    spawnl (P_WAIT, cspec, MSG_null, NULL);
#ifndef OS2
#ifndef IBM
    putenv (MSG_pme);
    if (putenv (old_prompt) == -1)
	exit (1);
#endif
#endif
    ttraw ();
    sgarbf = TRUE;
#endif
    return (TRUE);
}
