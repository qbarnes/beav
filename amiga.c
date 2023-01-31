/* -*-C-*-
 *
 * Module : amiga.c
 *
 * Author : Simon J Raybould.    (sie@fulcrum.bt.co.uk).
 *
 * Date   : Tuesday 11th June 1991.
 *
 * Desc   : amiga specifics for beav binary editor.
 *
 *
 * This file is public domain and you can do what you want with it, even roll
 * it up into a ball and toss it for your cat to chase. I accept no
 * resposibility for it being unfit for any purpose (including a feline toy).
 * Any bugs you can either fix them yourself or tell me and I'll do it.
 * Any major fixes should be reported to me and I will inform the main keeper
 * of beav to be sure they are fixed in the next release. This only applies to
 * bugs in THIS FILE or in AMIGA sections of other files. Any other bugs to the
 * original author.
 *
 * SJR - 25.Aug.91
 *
 *
 */

#ifdef AMIGA

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <libraries/dosextens.h>
#include <exec/memory.h>
#include <intuition/intuition.h>

#include "def.h"

#define SCRBUFSIZ        1024	/* buffered screen io */

struct NewWindow nw =
{
    0, 0, 640, 256, -1, -1, NULL,
    WINDOWDEPTH | WINDOWDRAG | SMART_REFRESH | ACTIVATE | BORDERLESS,
    NULL, NULL,
    "BEAV Amiga Port by S.J.Raybould (sie@fulcrum.bt.co.uk)  Sep 1991",
    NULL, NULL, 0, 0, 0, 0, WBENCHSCREEN
};

/* Opens/allocations we'll need to clean up */
struct Library *IntuitionBase = NULL;
struct Window *win = NULL, *OpenWindow ();
struct IOStdReq *writeReq = NULL;	/* I/O request block pointer */
struct MsgPort *writePort = NULL;	/* replyport for writes      */

struct IOStdReq *readReq = NULL;/* I/O request block pointer */
struct MsgPort *readPort = NULL;/* replyport for reads       */

struct MsgPort *CreatePort ();
BOOL OpenedConsole = FALSE;
UBYTE ibuf;
BYTE OpenConsole ();
void CloseConsole (), QueueRead (), ConWrite ();


int nrow;			/* Terminal size, rows.         */
int ncol;			/* Terminal size, columns.      */
#ifdef CRAP
int tceeol = 3;			/* Costs.                       */
#endif /* CRAP */

int kbdpoll;			/* in O_NDELAY mode         */
int kbdqp;			/* there is a char in kbdq  */

char scrbuf[SCRBUFSIZ];		/* buffered screen io */
short scrbufpos = 0;		/* current write position */

/* CODE TO REPLACE STUFF IN termio.c */

/*
 * This function gets called just before we go back home to the command
 * interpreter. On VMS it puts the terminal back in a reasonable state.
 * Another no-operation on CPM.
 */
void
ttclose ()
{
    /* Put TTY back in sesible state */
    if (!(CheckIO (readReq)))
	AbortIO (readReq);
    WaitIO (readReq);
    if (OpenedConsole)
	CloseConsole (writeReq);
    if (readReq)
	DeleteExtIO (readReq);
    if (readPort)
	DeletePort (readPort);
    if (writeReq)
	DeleteExtIO (writeReq);
    if (writePort)
	DeletePort (writePort);
    if (win)
	CloseWindow (win);
    if (IntuitionBase)
	CloseLibrary (IntuitionBase);
}

/*
 * Flush terminal buffer. Does real work where the terminal output is buffered
 * up. A no-operation on systems where byte at a time terminal I/O is done.
 */
void
ttflush ()
{
    if (scrbufpos > 0)
    {
	ConWrite (writeReq, scrbuf, scrbufpos);
	scrbufpos = 0;
    }
}

/*
 * Write a character to the display. On VMS, terminal output is buffered, and
 * we just put the characters in the big array, after checking for overflow.
 * On CPM terminal I/O unbuffered, so we just write the byte out. Ditto on
 * MS-DOS (use the very very raw console output routine).
 */
void
ttputc (c)
{
    if (scrbufpos < SCRBUFSIZ)
	scrbuf[scrbufpos++] = c;
    else
    {
	ConWrite (writeReq, scrbuf, scrbufpos);
	scrbufpos = 0;
	scrbuf[scrbufpos++] = c;
    }
}

void
ttputs (char *str)
{
    while (*str)
	ttputc (*str++);
}

/*
 * Read a character from the terminal, performing no editing and doing no echo
 * at all. More complex in VMS that almost anyplace else, which figures. Very
 * simple on CPM, because the system can do exactly what you want.
 */
ttgetc ()
{
    char c, ConGetChar ();
    static char Buffer[8], ri = 0, wi = 0;

    if (kbdqp)
	kbdqp = FALSE;
    /* If we stil have chars from last time, return them */
    if (ri < wi)
	return Buffer[ri++] & 0x7f;

    /* Else empty the buffer and start a new read */
    ri = wi = 0;
    c = ConGetChar (readPort, &ibuf);
    /*
  * Attempt some translations !
  * This is the place to extend, if you wish to add some more.
  * SEE RKM L&D 1.3 pg 654 for more info.
  */
    if ((unsigned char) c == (unsigned char) 0x9b)
    {				/* ANSI esc start */
	c = ConGetChar (readPort, &ibuf);
	switch (c)
	{
	case 'A':		/* UP */
	    Buffer[wi++] = 0x10;/* ^P */
	    break;
	case 'B':		/* DOWN */
	    Buffer[wi++] = 0x0e;/* ^N */
	    break;
	case 'C':		/* RIGHT */
	    Buffer[wi++] = 0x06;/* ^F */
	    break;
	case 'D':		/* LEFT */
	    Buffer[wi++] = 0x02;/* ^B */
	    break;
	case '0':		/* F1 */
	    ConGetChar (readPort, &ibuf);	/* discard tilde */
	    Buffer[wi++] = 0x1b;/* HELP = "ESC ?" */
	    Buffer[wi++] = '?';
	    break;
	case '1':		/* F2 or SHIFTED function key */
	    c = ConGetChar (readPort, &ibuf);	/* Get next char to see if it's a tilde */
	    switch (c)
	    {
	    case '~':		/* was definately F2 */
		Buffer[wi++] = 0x1b;	/* mark-set = "ESC ." */
		Buffer[wi++] = '.';
		break;
	    case '0':		/* SHIFTED F1 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* binding-for-key = "Ctl-X ?" */
		Buffer[wi++] = '?';
		break;
	    case '1':		/* SHIFTED F2 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* file-read = "Ctl-X Ctl-R" */
		Buffer[wi++] = 0x12;
		break;
	    case '2':		/* SHIFTED F3 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* file-save = "Ctl-X Ctl-S" */
		Buffer[wi++] = 0x13;
		break;
	    case '3':		/* SHIFTED F4 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* file-visit = "Ctl-X Ctl-V" */
		Buffer[wi++] = 0x16;
		break;
	    case '4':		/* SHIFTED F5 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* file-write = "Ctl-X Ctl-W" */
		Buffer[wi++] = 0x17;
		break;
	    case '5':		/* SHIFTED F6 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* save-all-buffers = "Ctl-X return" */
		Buffer[wi++] = '\r';
		break;
	    case '6':		/* SHIFTED F7 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* buffer-set-file-name = "Ctl-X Ctl-F" */
		Buffer[wi++] = 0x06;
		break;
	    case '7':		/* SHIFTED F8 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* insert-file = "Ctl-X TAB" */
		Buffer[wi++] = '\t';
		break;
	    case '8':		/* SHIFTED F9 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x18;	/* quit-save-all = "Ctl-X Ctl-E" */
		Buffer[wi++] = 0x05;
		break;
	    case '9':		/* SHIFTED F10 */
		ConGetChar (readPort, &ibuf);	/* Discard the tilde */
		Buffer[wi++] = 0x03;	/* quit-no-save = "Ctl-C" */
		break;
	    }
	    break;
	case '2':		/* F3 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x1b;/* search-forv = "ESC s" */
	    Buffer[wi++] = 's';
	    break;
	case '3':		/* F4 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x1b;/* search-again = "ESC t" */
	    Buffer[wi++] = 't';
	    break;
	case '4':		/* F5 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x1b;/* replace = "ESC %" */
	    Buffer[wi++] = '%';
	    break;
	case '5':		/* F6 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x19;/* yank = "Ctl-Y" */
	    break;
	case '6':		/* F7 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x1b;/* copy-mark-to-cursor = "ESC w" */
	    Buffer[wi++] = 'w';
	    break;
	case '7':		/* F8 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x17;/* delete-mark-to-cursor = "Ctl-W" */
	    break;
	case '8':		/* F9 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x18;/* move-to-byte = "Ctl-X G" */
	    Buffer[wi++] = 'G';
	    break;
	case '9':		/* F10 */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x07;/* abort-cmd = "Ctl-G" */
	    break;
	case '?':		/* HELP */
	    ConGetChar (readPort, &ibuf);	/* Discard the tilde */
	    Buffer[wi++] = 0x1b;/* help = "ESC ?" */
	    Buffer[wi++] = '?';
	    break;
	}
	return Buffer[ri++] & 0x7f;
    }
    else			/* not an ANSI sequence */
	return c & 0x7f;
}

/*
 * This function is called once to set up the terminal device streams.
 * On VMS, it translates TT until it finds the terminal, then assigns
 * a channel to it and sets it raw. On CPM it is a no-op.
 */

void
ttopen ()
{
    int Sig;
    ULONG conreadsig, windowsig;
    BYTE error;
    struct Screen Screen;	/* get a copy of WBENCHSCREEN in here */


    if (!(IntuitionBase = (struct IntuitionBase *) OpenLibrary ("intuition.library", 0)))
    {
	printf ("Can't open intuition\n");
	ttclose ();
	exit (10);
    }
    /* Create reply port and io block for writing to console */
    if (!(writePort = CreatePort ("LARN.console.write", 0)))
    {
	printf ("Can't create write port\n");
	ttclose ();
	exit (10);
    }
    if (!(writeReq = (struct IOStdReq *)
	  CreateExtIO (writePort, (LONG) sizeof (struct IOStdReq))))
    {
	printf ("Can't create write request\n");
	ttclose ();
	exit (10);
    }
    /* Create reply port and io block for reading from console */
    if (!(readPort = CreatePort ("LARN.console.read", 0)))
    {
	printf ("Can't create read port\n");
	ttclose ();
	exit (10);
    }
    if (!(readReq = (struct IOStdReq *)
	  CreateExtIO (readPort, (LONG) sizeof (struct IOStdReq))))
    {
	printf ("Can't create read request\n");
	ttclose ();
	exit (10);
    }
    if (!GetScreenData (&Screen, sizeof (struct Screen), WBENCHSCREEN, NULL))
    {
	printf ("Can't get screen size\n");
	ttclose ();
	exit (10);
    }
    nrow = Screen.Height / 8 - 3;
    ncol = Screen.Width / 8;
    nw.Height = Screen.Height;
    nw.Width = Screen.Width;

    /* don't allow a larger number of rows than we can handle */
    if (nrow > NROW)
	nrow = NROW;
    /* don't allow a larger number of cols than we can handle */
    if (ncol > NCOL)
	ncol = NCOL;

    /* Open a window */
    if (!(win = OpenWindow (&nw)))
    {
	printf ("Can't open window\n");
	ttclose ();
	exit (10);
    }
    /* Now, attach a console to the window */
    if (error = OpenConsole (writeReq, readReq, win))
    {
	printf ("Can't open console.device\n");
	ttclose ();
	exit (10);
    }
    else
	OpenedConsole = TRUE;

    QueueRead (readReq, &ibuf);	/* send the first console read request */
    conreadsig = 1 << readPort->mp_SigBit;
    windowsig = 1 << win->UserPort->mp_SigBit;
    for (Sig = 0; Sig < NSIG; Sig++)
	signal (Sig, SIG_IGN);

    kbdpoll = FALSE;
    /* on all screens we are not sure of the initial position of the cursor */
    ttrow = 999;
    ttcol = 999;
}

/* END OF TERMIO REPLACEMENT CODE */

/* Attach console device to an open Intuition window.
 * This function returns a value of 0 if the console
 * device opened correctly and a nonzero value (the error
 * returned from OpenDevice) if there was an error.
 */
BYTE
OpenConsole (writereq, readreq, window)
    struct IOStdReq *writereq;
    struct IOStdReq *readreq;
    struct Window *window;
{
    BYTE error;

    writereq->io_Data = (APTR) window;
    writereq->io_Length = sizeof (struct Window);
    error = OpenDevice ("console.device", 0, writereq, 0);
    readreq->io_Device = writereq->io_Device;	/* clone required parts */
    readreq->io_Unit = writereq->io_Unit;
    return (error);
}

void
CloseConsole (struct IOStdReq *writereq)
{
    CloseDevice (writereq);
}

/* Output a single character to a specified console
 */
void
ConPutChar (struct IOStdReq *writereq, UBYTE character)
{
    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR) & character;
    writereq->io_Length = 1;
    DoIO (writereq);
    /* command works because DoIO blocks until command is done
  * (otherwise ptr to the character could become invalid)
  */
}

/* Output a stream of known length to a console
 */
void
ConWrite (struct IOStdReq *writereq, UBYTE * string, LONG length)
{
    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR) string;
    writereq->io_Length = length;
    DoIO (writereq);
    /* command works because DoIO blocks until command is done
  * (otherwise ptr to string could become invalid in the meantime)
  */
}

/* Output a NULL-terminated string of characters to a console
 */
void
ConPuts (struct IOStdReq *writereq, UBYTE * string)
{
    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR) string;
    writereq->io_Length = -1;	/* means print till terminating null */
    DoIO (writereq);
}

/* Queue up a read request to console, passing it pointer
 * to a buffer into which it can read the character
 */
void
QueueRead (struct IOStdReq *readreq, UBYTE * whereto)
{
    readreq->io_Command = CMD_READ;
    readreq->io_Data = (APTR) whereto;
    readreq->io_Length = 1;
    SendIO (readreq);
}

struct IOStdReq *readreq;	/* ttkeyready() needs to be able to see this */

/* Wait for a character
 */
char
ConGetChar (struct MsgPort *msgport, UBYTE * whereto)
{
    register temp;

    WaitPort (msgport);
    readreq = (struct IOStdReq *) GetMsg (msgport);
    temp = *whereto;		/* get the character */
    QueueRead (readreq, whereto);	/* then re-use the request block*/
    return ((char) temp);
}

/* typahead():    Check to see if any characters are already in the
        keyboard buffer
   On the amiga, we do this by checking if the outstanding read request
   has been satisfied yet by calling CheckIO().
*/
ttkeyready ()
{
    if (!kbdqp)
	kbdqp = CheckIO (readreq) ? 1 : 0;
    return kbdqp;
}

/* UNIX support stuff */

#define BLOCKSIZE  4096

link (char *SPath, char *DPath)
{
    int sfd, dfd, Bytes;
    char BlkBuf[BLOCKSIZE];

    if ((sfd = open (SPath, O_RDONLY)) == -1)
	return -1;
    if ((dfd = open (DPath, O_WRONLY | O_CREAT | O_TRUNC)) == -1)
	return -1;
    while ((Bytes = read (sfd, BlkBuf, BLOCKSIZE)) > 0)
	write (dfd, BlkBuf, Bytes);
    close (sfd);
    close (dfd);
    return 0;
}

#endif /* AMIGA */
