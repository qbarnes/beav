/*
*    file I/O.
*/

#ifdef UNIX
#include	<unistd.h>
#include	<sys/types.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#endif
#ifdef AMIGA
#include	<sys/types.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#endif
#include        "def.h"

extern char MSG_cnt_wr[];
extern char MSG_cnt_rd[];
extern char MSG_wr_io_er[];
extern char MSG_rd_er[];
extern char MSG_bak[];
extern char MSG_backup[];
extern char MSG_back_er[];
extern char MSG_back_of[];

#ifdef MSDOS
static FILE *ffp;
#endif

#ifdef UNIX
static int ffp;
#endif

#ifdef AMIGA
static int ffp;
#endif

/*
* Open a file for reading.
*/
char
ffropen (fn)
    char *fn;
{
#ifdef MSDOS
    if ((ffp = fopen (fn, "rb")) == NULL)	/* pvr */
#endif
#ifdef OS2
	if ((ffp = open (fn, O_RDONLY | O_BINARY)) == -1)	/* pvr */
#else
#ifdef UNIX
	if ((ffp = open (fn, O_RDONLY)) == -1)	/* pvr */
#endif
#endif
#ifdef AMIGA
	    if ((ffp = open (fn, O_RDONLY)) == -1)	/* pvr */
#endif
		return (FIOERR);
    return (FIOSUC);
}

/*
*   Get the file length
*/
#ifdef AMIGA
A32
file_len (char *fname)
{
    struct stat st;

    if (stat (fname, &st) == -1)
	return (-1);
    return (st.st_size);
}

#else /* AMIGA */
A32
file_len ()
{
#ifdef MSDOS
    return (filelength (fileno (ffp)));
#endif
#ifdef UNIX
    struct stat st;

    if (fstat (ffp, &st) == -1)
	return (-1);
    return (st.st_size);
#endif
}

#endif /* AMIGA */

/*
* Open a file for writing.
* Set file permissions as requested
* Return TRUE if all is well, and
* FALSE on error (cannot create).
*/
bool
ffwopen (fn, mode)
    char *fn;
    int mode;
{
#ifdef MSDOS
    if ((ffp = fopen (fn, "wb")) == NULL)	/* pvr */
#endif
#ifdef OS2
	mode &= (S_IREAD | S_IWRITE);
    mode |= S_IREAD;
    if ((ffp = open (fn, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, mode)) == -1)
#else
#ifdef UNIX
	/* set perms as in original file 1.31 */
	if ((ffp = open (fn, O_WRONLY | O_CREAT, mode)) == -1)
#endif
#endif
#ifdef AMIGA
	    /* set perms as in original file 1.31 */
	    if ((ffp = open (fn, O_WRONLY | O_CREAT, mode)) == -1)	/* pvr */
#endif
	    {
		err_echo (MSG_cnt_wr);
		return (FIOERR);
	    }
    return (FIOSUC);
}

/*
* Close a file.
* Should look at the status.
*/
char
ffclose ()
{
#ifdef MSDOS
    fclose (ffp);
#endif
#ifdef UNIX
    close (ffp);
#endif
#ifdef AMIGA
    close (ffp);
#endif
    return (FIOSUC);
}

/*
* Write a line to the already
* opened file. The "buf" points to the
* buffer, and the "nbuf" is its length.   pvr
* Return the status.
*/
char
ffputline (buf, nbuf)
    register char buf[];
    int nbuf;
{
    register int i;

#ifdef MSDOS
    i = fwrite (buf, 1, nbuf, ffp);
#endif
#ifdef UNIX
    i = write (ffp, buf, nbuf);
#endif
#ifdef AMIGA
    i = write (ffp, buf, nbuf);
#endif

    if ((i != nbuf)
#ifdef MSDOS
	|| (ferror (ffp) != FALSE))
#else
	)
#endif
    {
	err_echo (MSG_wr_io_er);
	return (FIOERR);
    }
    return (FIOSUC);
}

/*
* Read a line from a file, and store the bytes
* in the supplied buffer. Stop on end of file or after 'nbuf' characters. pvr
* the first byte in the buffer is the length in bytes.
*/
char
ffgetline (buf, nbuf, rbuf)
    register char *buf;
    register LPOS *rbuf, nbuf;
{
#ifdef MSDOS
    *rbuf = fread (buf, 1, nbuf, ffp);
#endif

#ifdef UNIX
    *rbuf = read (ffp, buf, nbuf);
#endif
#ifdef AMIGA
    *rbuf = read (ffp, buf, nbuf);
#endif

    /* End of file.         */
#ifdef MSDOS
    if (ferror (ffp) != FALSE)
    {
	err_echo (MSG_rd_er);
	return (FIOERR);
    }
#endif
    if (*rbuf == 0)
	return (FIOEOF);

    return (FIOSUC);
}

/*
*   Seek to specified position in file.
*   Return the actual position in the file.
*/
A32
ffseek (posn)
    A32 posn;
{
#ifdef MSDOS
    fseek (ffp, posn, SEEK_SET);
    return (ftell (ffp));
#endif
#ifdef UNIX
    return (lseek (ffp, posn, 0));
#endif
#ifdef AMIGA
    return (lseek (ffp, posn, 0));
#endif
}

/*
* Some backup user on MS-DOS might want
* to determine some rule for doing backups on that
* system, and fix this. I don't use MS-DOS, so I don't
* know what the right rules would be. Return TRUE so
* the caller does not abort a write.
* Under UNIX just append the .bak postfix.
*/
#ifdef BACKUP
bool
fbackupfile (fname)
    char *fname;
{
    unsigned char backname[NFILEN];
    unsigned char *source, *backup;
    unsigned char buf[NCOL];

    source = fname;
    backup = backname;
    while ((*source > 0)
#if defined(MSDOS) || defined(OS2)
	   && (*source != '.'))
#else
	)
#endif
    {
	*backup = *source;
	backup++;
	source++;
	*backup = 0;
    }
#ifdef OS2
    strcpy (backup, source);
    strcat (backup, "~");
    if (!isvalid (backname))
	strcpy (backup, ".bak");
#else
    strcat (backname, MSG_bak);
#endif
    sprintf (buf, MSG_backup, fname, backname);
    writ_echo (buf);
    unlink (backname);
#ifdef NORENAME
    if ((link (fname, backname) != 0) || (unlink (fname) != 0))
#else
    if (rename (fname, backname) > 0)
#endif
    {
	sprintf (buf, MSG_back_er, fname, backname);
	err_echo (buf);
	return (FALSE);
    }
    return (TRUE);		/* Hack.                */
}

#endif

/*
* The string "fn" is a file name.
* Perform any required case adjustments. All systems
* we deal with so far have case insensitive file systems.
* We zap everything to lower case. The problem we are trying
* to solve is getting 2 buffers holding the same file if
* you visit one of them with the "caps lock" key down.
* On UNIX and AMIGA file names are dual case, so we leave
* everything alone.
*/
void
adjustcase (fn)
    register char *fn;
{
    register int c;

    while ((c = *fn) != 0)
    {
	if (c >= 'A' && c <= 'Z')
	    *fn = c + 'a' - 'A';
	++fn;
    }
}

#ifdef OS2

#define INCL_NOPM
#define INCL_ERRORS
#include <os2.h>

int 
isvalid (char *name)
{
    HFILE hf;
#ifdef __32BIT__
    ULONG uAction;
#else
    USHORT uAction;
#endif

    switch (DosOpen (name, &hf, &uAction, 0, 0, FILE_OPEN,
		     OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, 0))
    {
    case ERROR_INVALID_NAME:
    case ERROR_FILENAME_EXCED_RANGE:
	return FALSE;
    case NO_ERROR:
	DosClose (hf);
    default:
	return TRUE;
    }
}

#endif
