/*
 * The functions in this file negotiate with the operating system for
 * characters, and write characters in a barely buffered fashion on the display.
 * All operating systems.
 */

#include	<sys/types.h>	/* 1.13 */

#ifdef UNIX			/* System V */

#include    <stdio.h>
#include    <signal.h>
#ifdef BSD
#include    <sys/ioctl.h>
#else
#ifdef OS2
#ifndef __EMX__
#define INCL_NOPM
#define INCL_DOS
#define INCL_KBD
#include    <os2.h>
#endif
#include    <io.h>
#else
#ifdef MINIX
#include    <sgtty.h>
#define O_NDELAY O_NONBLOCK
#else
#include    <termio.h>
#endif /* MINIX */
#endif /* OS2 */
#endif /* BSD */
#include    <errno.h>
#include    <fcntl.h>
#include    "def.h"
int kbdflgs;			/* saved keyboard fd flags  */
int kbdpoll;			/* in O_NDELAY mode         */
int kbdqp;			/* there is a char in kbdq  */
char kbdq;			/* char we've already read  */

#ifdef BSD
struct sgttyb otermb;
struct sgttyb ntermb;
#else
#ifdef OS2
#ifndef __EMX__
KBDINFO kbst, kbst_std;
#endif
#else
#ifdef MINIX
struct sgttyb otermio;		/* original terminal characteristics */
struct sgttyb ntermio;		/* charactoristics to use inside */
struct tchars tchars, tcharsorig;
#else
struct termio otermio;		/* original terminal characteristics */
struct termio ntermio;		/* charactoristics to use inside */
#endif /* MINIX */
#endif /* OS2 */
#endif /* BSD */

#ifndef OS2
extern errno;			/* System error number -- Necessary when compiling in BSD 1.13 */
#endif

int nrow;			/* Terminal size, rows.         */
int ncol;			/* Terminal size, columns.      */

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */

void
ttopen ()
{
#ifdef BSD
#ifdef ULTRIX
    struct winsize ttysize;
#else
    struct ttysize ttysize;
#endif

    ioctl (0, TIOCGETP, &otermb);	/* save settings	*/
    ntermb = otermb;		/* setup new settings	*/
    ntermb.sg_flags &= ~ECHO;
    ntermb.sg_flags |= RAW;
    ioctl (0, TIOCSETP, &ntermb);	/* and activate them	*/
    kbdpoll = FALSE;

    /* on all screens we are not sure of the initial position
   of the cursor                    */
    ttrow = 999;
    ttcol = 999;
#ifdef ULTRIX
    if (ioctl (0, TIOCGWINSZ, &ttysize) == 0)
    {
	nrow = ttysize.ws_row;
	ncol = ttysize.ws_col;
#else
    if (ioctl (0, TIOCGSIZE, &ttysize) == 0)
    {
	nrow = ttysize.ts_lines;
	ncol = ttysize.ts_cols;
#endif
    }
    else
    {
	nrow = NROW;
	ncol = NCOL;
    }
#else
#ifdef OS2
    setmode (1, O_BINARY);
#else
#ifdef MINIX
    ioctl (0, TIOCGETP, &otermio);
    ntermio = otermio;
    ntermio.sg_flags &= ~ECHO;
    ntermio.sg_flags |= RAW;
    ioctl (0, TIOCSETP, &ntermio);
    ioctl (0, TIOCGETC, &tcharsorig);
    tchars = tcharsorig;
    tchars.t_intrc = tchars.t_quitc = tchars.t_startc =
	tchars.t_stopc = tchars.t_eofc = tchars.t_brkc = -1;
    ioctl (0, TIOCSETC, &tchars);
#else
    ioctl (0, TCGETA, &otermio);/* save old settings */
    ntermio.c_iflag = 0;	/* setup new settings */
    ntermio.c_oflag = 0;
    ntermio.c_cflag = otermio.c_cflag;
    ntermio.c_lflag = 0;
    ntermio.c_line = otermio.c_line;
    ntermio.c_cc[VMIN] = 1;
    ntermio.c_cc[VTIME] = 0;
    ioctl (0, TCSETAW, &ntermio);	/* and activate them */
#endif /* MINIX */
    kbdflgs = fcntl (0, F_GETFL, 0);
    kbdpoll = FALSE;
#endif /* OS2 */
    /* on all screens we are not sure of the initial position of the cursor */
    ttrow = 999;
    ttcol = 999;
    nrow = NROW;
    ncol = NCOL;
#endif /* BSD */
}

 /*
* This function gets called just before we go back home to the command
* interpreter. On VMS it puts the terminal back in a reasonable state.
* Another no-operation on CPM.
*/
void
ttclose ()
{
#ifdef BSD
    if (ioctl (0, TIOCSETP, &otermb) == -1)	/* restore terminal settings */
	printf ("closing ioctl on dev 0 failure, error = %d\n", errno);
#else
#ifdef OS2
    setmode (1, O_TEXT);
#else
#ifdef MINIX
    if (ioctl (0, TIOCSETP, &otermio) == -1 ||
	ioctl (0, TIOCSETC, &tcharsorig) == -1)
	printf ("closing ioctl on dev 0 failure, error = %d\n", errno);
#else
    if (ioctl (0, TCSETAW, &otermio) == -1)	/* restore terminal settings */
	printf ("closing ioctl on dev 0 failure, error = %d\n", errno);
#endif /* MINIX */
    if (fcntl (0, F_SETFL, kbdflgs) == -1)
	printf ("closing fcntl on dev 0 failure, error = %d\n", errno);
#endif /* OS2 */
#endif /* BSD */
}

#ifdef OS2
void 
ttraw (void)
{
#ifdef __32BIT__
    signal (SIGINT, SIG_IGN);
    signal (SIGBREAK, SIG_IGN);
#else
    PFNSIGHANDLER oldhandler;
    USHORT oldact;

    DosSetSigHandler ((PFNSIGHANDLER) NULL, &oldhandler, &oldact,
		      SIGA_IGNORE, SIG_CTRLBREAK);
    DosSetSigHandler ((PFNSIGHANDLER) NULL, &oldhandler, &oldact,
		      SIGA_IGNORE, SIG_CTRLC);
#endif

#ifndef __EMX__
    kbst_std.cb = sizeof (kbst_std);
    KbdGetStatus (&kbst_std, 0);
    kbst = kbst_std;
    kbst.fsMask &= ~(KEYBOARD_ECHO_ON | KEYBOARD_ASCII_MODE |
		     KEYBOARD_SHIFT_REPORT);
    kbst.fsMask |= (KEYBOARD_ECHO_OFF | KEYBOARD_BINARY_MODE);
    KbdSetStatus (&kbst, 0);
#endif
}

void 
ttcooked (void)
{
#ifndef __EMX__
    KbdSetStatus (&kbst_std, 0);
#endif
}

#endif

 /*
* Write a character to the display. On VMS, terminal output is buffered, and
* we just put the characters in the big array, after checking for overflow.
* On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
* MS-DOS (use the very very raw console output routine).
*/

#ifdef OS2
int tty_io_size = 0;
char tty_io_buffer[2048];
#endif

int
ttputc (c)
{
#ifdef OS2
    if (tty_io_size == sizeof (tty_io_buffer))
    {
	write (1, tty_io_buffer, tty_io_size);
	tty_io_size = 0;
    }
    tty_io_buffer[tty_io_size++] = c;
#else
    fputc (c, stdout);
#endif
    return c;
}

 /*
* Flush terminal buffer. Does real work where the terminal output is buffered
* up. A no-operation on systems where byte at a time terminal I/O is done.
*/
void
ttflush ()
{
#ifdef OS2
    if (tty_io_size)
    {
	write (1, tty_io_buffer, tty_io_size);
	tty_io_size = 0;
    }
#else
    fflush (stdout);
#endif
}

 /*
* Read a character from the terminal, performing no editing and doing no echo
* at all. More complex in VMS that almost anyplace else, which figures. Very
* simple on CPM, because the system can do exactly what you want.
*/

#ifdef OS2
#ifdef __EMX__
static int chr = -1;
#endif
#endif

ttgetc ()
{
#ifdef OS2
#ifdef __EMX__
    if (chr != -1)
    {
	int c = chr;
	chr = -1;
	return c;
    }
    else
	return _read_kbd (0, 1, 0);
#else
    static int ext, scan, chr;
    KBDKEYINFO ki;

    if (ext)
    {
	ext = 0;
	return scan;
    }
    else
    {
	ttflush ();
	KbdCharIn (&ki, IO_WAIT, 0);

	if (ki.chChar == 0 || ki.chChar == 0xE0)
	{
	    ext = 1;
	    scan = ki.chScan;
	    return 0xE0;
	}
	else
	    return ki.chChar;
    }
#endif
#else
    if (kbdqp)
	kbdqp = FALSE;
    else
    {
#ifdef BSD
	int count;

	if (kbdpoll && (ioctl (0, FIONREAD, &count), count == 0))
	    return FALSE;
	read (0, &kbdq, 1);
#else
	if (kbdpoll && fcntl (0, F_SETFL, kbdflgs) < 0)
	    return FALSE;
	kbdpoll = FALSE;
	while (read (0, &kbdq, 1) != 1)
	    ;
#endif
    }
    return (kbdq & 127);
#endif /* OS2 */
}

 /* typahead():    Check to see if any characters are already in the
 keyboard buffer
*/
ttkeyready ()
{
#ifdef OS2
#ifdef __EMX__
    chr = _read_kbd (0, 0, 0);
    return (chr != -1);
#else
    KBDKEYINFO ki;

    KbdPeek (&ki, 0);
    return (ki.fbStatus != 0);
#endif
#else
    if (!kbdqp)
    {
#ifdef BSD
	int count;

	if (!kbdpoll && (ioctl (0, FIONREAD, &count), count == 0))
	    return FALSE;
	kbdpoll = TRUE;		/*  fix in 1.13 */
	kbdqp = TRUE;
#else
#ifdef X_MINIX
	/* MINIX has non-blocking mode but it doesn't work !?!? */
	return FALSE;
#else
	if (!kbdpoll && fcntl (0, F_SETFL, kbdflgs | O_NDELAY) < 0)
	    return (FALSE);
	kbdpoll = TRUE;		/*  fix in 1.13 */
	kbdqp = (1 == read (0, &kbdq, 1));
#endif /* MINIX */
#endif /* BSD */

    }
    return (kbdqp);
#endif /* OS2 */
}

#endif
