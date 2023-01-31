/*
 *     Common header file.
 *
 * This file is the general header file for all parts
 * of the display editor. It contains all of the
 * general definitions and macros. It also contains some
 * conditional compilation flags. All of the per-system and
 * per-terminal definitions are in special header files.
 * The most common reason to edit this file would be to zap
 * the definition of CVMVAS or BACKUP.
 */
#define LINT_ARGS   1		/* enable lint type checking */
#include        "stdio.h"

#ifdef UNIX
#include        "sys/types.h"
#endif /* UNIX */

#define BACKUP  1		/* Make backup file.            */
#define RUNCHK  1		/* Do additional checking at run time */

#ifndef	uchar
#define uchar   unsigned    char
#endif

#ifndef	uint
#define uint    unsigned    int
#endif

#ifndef	ushort
#define ushort  unsigned    short
#endif

#ifndef	ulong
#define ulong   unsigned    long
#endif

/* these defines are reserved for handling data values from the buffer */
#define     D8  uchar		/* this had better be a 8 bit quantity */
#define     D16 ushort		/* this had better be a 16 bit quantity */
#define     D32 ulong		/* this had better be a 32 bit quantity */
#define     D64 double		/* this had better be a 64 bit quantity */

/* this define is reserved for the address of a location in the buffer */
#define     A32 ulong		/* this is a 32 bit address into the buffer */

#define     bool char		/* used for boolean values      */
#define     bits char		/* used for boolean bit flags   */

/* this define is reserved for the byte location in the a LINE structure */
#define     LPOS uint		/* this is a 32 bit address into the buffer */

/*
 *      MS-DOS system header file.
 */
#if	MSDOS
#define LARGE   1		/* Large model.         */
#endif
#define PCC 1			/* "[]" won't work.     */
#define GOOD    0		/* Indicate hunkydoryhood   */

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define BDC1    ':'		/* Buffer names.        */
#define BDC2    '/'		/* Buffer names. jam    */

#ifdef UNIX
#define PATHCHR ':'
#define	SEPCHAR '/'
#else
#define PATHCHR ';'
#define	SEPCHAR 0x5c		/* this is a \ char */
#endif

/*
*	This enables the float mode of BEAV.   Some systems may handle
*	floats in a way that is not compatible with BEAV.   BEAV was
*	written on Intel machines.
*
*	The Intel floating point representation is;
*		bit 0  - 52		significand	(53 bits)
*		bit 53 - 62		biased exponent (11 bits)
*		bit	63			sign
*	maximum range;	10^-308 <= X <= 10^+308
*	obviously, not all patterns are legal floating point numbers.
*	There can be up to 16 decimal digits of significand.
*	There are only 3 decimal digits of exponent (308 max).
*
*	BEAV uses printf to display floating point numbers so it should
*	transport with minimal work.   The printf format string is
*	called "MSG_116e" and is defined in text.c.   I have specified
*	16 digits of precision and scientific notation.   If you need
*	too many more digits of precision the displayed columns may
*	overwrite each other.   This can be fixed by editing the
*	format.c file.   The display format can be altered by changing
*	the "float_64_fmt" data.   This is more complicated, however.
*
*	If it cannot be made to work, turn this off.
*/
#define	FLOAT_DISP	1

/*
 *      Digital ANSI terminal header file
 */
#ifdef MSDOS
#define	ANSI	1		/* send ANSI escape codes */
#endif

#ifdef	OS2
#define NCOL    132		/* Columns.     */
#else
#define NCOL    80		/* Columns.     */
#endif

#define NROW    24		/* default rows */

#define CUSTOMIZE		/* compile switch for extended key
																				binding in extend.c           */
#define COSMETIC		/* cosmetic screen stuff on
																				insert off screen             */
#ifdef MSDOS
#define WANG_CHARACTER_SCREEN 0xf0000000L
#endif
/*
 * Table sizes, etc.
 */
#define NSHASH  31		/* Symbol table hash size.      */
#define NFILEN  256		/* Length, file name.           */
#define NBUFN   13		/* Length, buffer name.     */
#define NFILE   12          /* Length, file name.  */	/* krw */
#define NKBDM   256		/* Length, keyboard macro.      */
#define NMSG    512		/* Length, message buffer.      */
#define NPAT    80		/* Length, pattern.             */
#define HUGE    1000		/* A rather large number.       */
#define NSRCH   128		/* Undoable search commands.    */
#define NXNAME  64		/* Length, extended command.    */
#define MAXPOS  0x7FFFFFFF	/* Maximum positive long value  */
#define MIN_WIN_ROWS 3		/* Minimum number of rows in a window */
/*
 * This is the initial allocation for user data storage.
 * It has should be in the range of 1 to less than half the size
 * of an int.   The define LPOS is used to index into an array of this size.
 * This is main tunable parameter for the speed of beav.
 * If it is too large inserts and deletes will be slow but
 * file loads will be fast and memory will be efficiently used.
 * If it is too small the reverse will be true.
 * This seems like a good number, but I have not tested it for performance.
 */
#define NLINE   0x1000		/* Length, line.      pvr  */

/*
 * When memory must be reallocated this is added to the allocation
 * request to allow for a little slop on areas that are being worked on.
 * This should reduce the number of allocations done.
 */
#define NBLOCK  0x1000		/* Line block extra size        */

/*
 * This is the number of bytes that are allocated for the kill buffer
 * when data cannot be moved by changing the pointers.
 */
#define KBLOCK  0x1000		/* Kill buffer block size.  */

/*
 * Universal.
 */
#define FALSE   0		/* False, no, bad, etc.         */
#define TRUE    1		/* True, yes, good, etc.        */
#define ABORT   2		/* Death, ^G, abort, etc.       */

/*
 * These flag bits keep track of
 * some aspects of the last command.
 * The CFKILL flag controls the clearing versus appending
 * of data in the kill buffer.
 */
#define CFKILL  0x0002		/* Last command was a kill      */

/*
 * File I/O.
 */
#define FIOSUC  0		/* Success.                     */
#define FIOFNF  1		/* File not found.              */
#define FIOEOF  2		/* End of file.                 */
#define FIOERR  3		/* Error.                       */

/*
 * Directory I/O.
 */
#define DIOSUC  0		/* Success.                     */
#define DIOEOF  1		/* End of file.                 */
#define DIOERR  2		/* Error.                       */

/*
 * Display colors.
 */
#define CNONE   0		/* Unknown color.               */
#define CTEXT   1		/* Text color.                  */
#define CMODE   2		/* Mode line color.             */

/*
 * Flags for "eread".
 */
#define EFNEW   0x0001		/* New prompt.                  */
#define EFAUTO  0x0002		/* Autocompletion enabled.      */
#define EFCR    0x0004		/* Echo CR at end; last read.   */

/*
 * Keys are represented inside using an 12 bit
 * keyboard code. The transformation between the keys on
 * the keyboard and 12 bit code is done by terminal specific
 * code in the "kbd.c" file. The actual character is stored
 * in 8 bits (DEC multinationals work); there is also a control
 * flag KCTRL, a meta flag KMETA, and a control-X flag KCTLX.
 * ASCII control characters are always represented using the
 * KCTRL form. Although the C0 control set is free, it is
 * reserved for C0 controls because it makes the communication
 * between "getkey" and "getkbd" easier. The funny keys get
 * mapped into the C1 control area.
 */
#define NKEYS   4096		/* 12 bit code.                 */

#define METACH  0x1B		/* M- prefix,   Control-[, ESC  */
#define CTMECH  0x1C		/* C-M- prefix, Control-\       */
#define EXITCH  0x1D		/* Exit level,  Control-]       */
#define CTRLCH  0x1E		/* C- prefix,   Control-^       */
#define HELPCH  0x1F		/* Help key,    Control-_       */
#define CTL_G   0x07		/* Abort command key            */

#define KCHAR   0x00FF		/* The basic character code.    */
#define KCTRL   0x0100		/* Control flag.                */
#define KMETA   0x0200		/* Meta flag.                   */
#define KCTLX   0x0400		/* Control-X flag.              */

#define KFIRST  0x0800		/* First special.       fitz    */
#define KLAST   0x0873		/* Last special.                */

#define KRANDOM 0x0080		/* A "no key" code.             */

/*
*	This causes the key sequence ESC [ <key> to be delevered as
*	KCTRL | KMETA | KCTLX | <key>.   This allows VT100 function keys
*	to be bound.
*/
#define	VT100KEY

/*
 *	These define the column used in the help (wallchart) function
 */
#define	HFUNCCOL	3
#define	HKEY    	32
#define	HKEYCODE	50
#define	HENDCOL 	55

/*
 * These flags, and the macros below them,
 * make up a do-it-yourself set of "ctype" macros that
 * understand the DEC multinational set, and let me ask
 * a slightly different set of questions.
 */
#define _W      0x01		/* Word.                        */
#define _U      0x02		/* Upper case letter.           */
#define _L      0x04		/* Lower case letter.           */
#define _C      0x08		/* Control.                     */

#define ISCTRL(c)       ((cinfo[(c)]&_C)!=0)
#define ISUPPER(c)      ((cinfo[(c)]&_U)!=0)
#define ISLOWER(c)      ((cinfo[(c)]&_L)!=0)
#define TOUPPER(c)      ((c)-0x20)
#define TOLOWER(c)      ((c)+0x20)

#define BUF_SIZE(wp)    (wp -> w_bufp -> b_linep -> l_bp -> l_file_offset + \
                        wp -> w_bufp -> b_linep -> l_bp -> l_used)
#define BUF_START(wp)   (wp -> w_bufp -> b_linep -> l_fp -> l_file_offset)
#define DOT_POS(wp)     (wp -> w_dotp -> l_file_offset + wp -> w_doto)
#define MARK_POS(wp)    (wp -> w_markp -> l_file_offset + wp -> w_marko)
#define DOT_CHAR(wp)    (wp -> w_dotp -> l_text[wp -> w_doto])
#define WIND_POS(wp)    (wp -> w_linep -> l_file_offset + wp -> w_loff)
#define R_TYPE(wp)      (wp -> w_fmt_ptr -> r_type)
#define R_SIZE(wp)      (wp -> w_fmt_ptr -> r_size)
#define R_UNITS(wp)     (wp -> w_fmt_ptr -> r_units)
#define R_BYTES(wp)     (wp -> w_fmt_ptr -> r_bytes)
#define R_ALIGN(wp)     (wp -> w_fmt_ptr -> r_align)
#define R_B_PER_U(wp)   (wp -> w_fmt_ptr -> r_b_per_u)
#define R_CHR_PER_U(wp) (wp -> w_fmt_ptr -> r_chr_per_u)
#define R_FLAGS(wp)     (wp -> w_fmt_ptr -> r_flags)
#define R_UNIT_FMT(wp)  (wp -> w_fmt_ptr -> r_unit_fmt)
#define R_POS_FMT(wp)	(wp -> w_fmt_ptr -> r_pos_fmt)
#define R_BYTE_FMT(wp)	(wp -> w_fmt_ptr -> r_byte_fmt)
#define R_POSITIONS(wp) (wp -> w_fmt_ptr -> r_positions)

/*
 * The symbol table links editing functions
 * to names. Entries in the key map point at the symbol
 * table entry. A reference count is kept, but it is
 * probably next to useless right now. The old type code,
 * which was not being used and probably not right
 * anyway, is all gone.
 */
typedef struct SYMBOL
{
    struct SYMBOL *s_symp;	/* Hash chain.                  */
    short s_nkey;		/* Count of keys bound here.    */
    char *s_name;		/* Name.            */
         bool (*s_funcp) ();	/* Function.                    */
    bits s_modify;		/* modify bit */
} SYMBOL;

/*
*   These are the legal values for 's_modify' and 'k_modify'
*/
#define SMOD    0x01		/* command modifies the buffer  */
#define SSIZE   0x02		/* command changes buffer size  */
#define SSRCH   0x04		/* command valid in search  */
#define SRPLC   0x08		/* command valid in replace */
#define SBOUND  0x10		/* key was bound by user or rc file */

/*
 * There is a window structure allocated for
 * every active display window. The windows are kept in a
 * big list, in top to bottom screen order, with the listhead at
 * "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands
 * to guide redisplay; although this is a bit of a compromise in
 * terms of decoupling, the full blown redisplay is just too
 * expensive to run for every input character.
 */
typedef struct WINDOW
{
    struct WINDOW *w_wndp;	/* Next window                  */
    struct BUFFER *w_bufp;	/* Buffer displayed in window   */
    struct LINE *w_linep;	/* Top line in the window       */
    LPOS w_loff;		/* Offset into line for start pvr  */
    A32 w_wind_temp;		/* temp storage for window file pos */
    struct LINE *w_dotp;	/* Line containing "."          */
    LPOS w_doto;		/* Offset into line for "." */
    A32 w_dot_temp;		/* temp storage for dot file pos */
    struct LINE *w_markp;	/* Line containing "mark"       */
    LPOS w_marko;		/* Byte offset for "mark"       */
    A32 w_mark_temp;		/* temp storage for mark file pos */
    char w_unit_offset;		/* Byte offset for "." into unit pvr */
    char w_toprow;		/* Origin 0 top row of window   */
    char w_ntrows;		/* # of rows of text in window  */
    bits w_flag;		/* Flags.                       */
    char w_disp_shift;		/* Display byte shift; 0-3  pvr */
    bool w_intel_mode;		/* Display byte swaped.     pvr */
    struct ROW_FMT *w_fmt_ptr;	/* Pointer to display format pvr */
} WINDOW;

/*
 * Window flags are set by command processors to
 * tell the display system what has happened to the buffer
 * mapped by the window. Setting "WFHARD" is always a safe thing
 * to do, but it may do more work than is necessary. Always try
 * to set the simplest action that achieves the required update.
 * Because commands set bits in the "w_flag", update will see
 * all change flags, and do the most general one.
 */
#define WFFORCE 0x01		/* Force reframe.               */
#define WFMOVE  0x02		/* Movement from line to line.  */
#define WFEDIT  0x04		/* Editing within a line.       */
#define WFHARD  0x08		/* Better to a full display.    */
#define WFMODE  0x10		/* Update mode line.        */
/*
*   This structure contains how a row is constructed.   pvr
*/

typedef struct ROW_FMT
{
    uchar r_type;		/* format type nibbles          */
    uchar r_size;		/* format size: must be 0,1,3,7,15, etc */
    uchar r_units;		/* number of units per window row: must be 1,2,4,8,16*/
    uchar r_bytes;		/* number of bytes per window row: must be 1,2,4,8,16*/
    uchar r_align;		/* number of bytes per align row: must be 1,2,4,8,16*/
    uchar r_b_per_u;		/* number of bytes per unit: must be 1,2,4,8,16 */
    uchar r_chr_per_u;		/* displayed chars per unit     */
    bits r_flags;		/* flags controlling format     */
    char *r_unit_fmt;		/* print format for unit */
    char *r_pos_fmt;		/* print format for buffer position, always a long */
    char *r_byte_fmt;		/* print format for bytes */
    uchar *r_positions;		/* list of unit positions   */
    struct ROW_FMT *r_srch_fmt;	/* pointer to search display format */
} ROW_FMT;

/* legal values for 'r_size'  (values significant; used as bit mask) pvr */

#define BYTES   0x00		/* Display as byte; 8 bits  */
#define WORDS   0x01		/* Display as word; 16 bits  */
#define DWORDS  0x03		/* Display as long; 32 bits  */
#define DOUBLES 0x07		/* Display as doubles; 64 bits  */

/* legal values for 'r_type'   pvr */
#define ASCII   0x10		/* Display as ascii     */
#define OCTAL   0x20		/* Display as octal values  */
#define DECIMAL 0x30		/* Display as decimal values    */
#define HEX     0x40		/* Display as hex values    */
#define BINARY  0x50		/* Display as binary values */
#define EBCDIC  0x60		/* Display as ebcdic        */
#define TEXT    0x70		/* Display as normal text   */
#if	FLOAT_DISP
#define FLOAT   0x80		/* Display as floating point number */
#endif
/*
 * Text is kept in buffers. A buffer header, described
 * below, exists for every buffer in the system. The buffers are
 * kept in a big list, so that commands that search for a buffer by
 * name can find the buffer header. There is a safe store for the
 * dot and mark in the header, but this is only valid if the buffer
 * is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with
 * a pointer to the header line in "b_linep".
 */
typedef struct BUFFER
{
    bits b_type;		/* Type of buffer       */
    struct BUFFER *b_bufp;	/* Link to next BUFFER          */
    struct LINE *b_dotp;	/* Link to "." LINE structure   */
    LPOS b_doto;		/* Offset of "." in above LINE  */
    char b_unit_offset;		/* Offset into unit for "." pvr */
    struct LINE *b_markp;	/* The same as the above two,   */
    LPOS b_marko;		/* but for the "mark"       */
    struct LINE *b_linep;	/* Link to the header LINE      */
    char b_nwnd;		/* Count of windows on buffer   */
    bits b_flag;		/* Flags            */
    A32 b_begin_addr;		/* File address of begining of buffer */
    A32 b_end_addr;		/* File address of end of buffer */
    A32 b_file_size;		/* Size of file */
    char b_fname[NFILEN];	/* File name                    */
    char b_bname[NBUFN];	/* Buffer name                  */
} BUFFER;

/* Values for 'buf_type' */
#define BTFILE   0x00		/* Buffer contains a file   */
#define BTDISK   0x01		/* Buffer points to a disk  */
#define BTMEMORY 0x02		/* Buffer points to memory  */
#define BTSAVE   0x03		/* This is the save buffer */
#define BTLIST   0x04		/* This is the buffer list */
#define BTHELP   0x05		/* This is the help buffer */

/* Values for 'b_flag' */

#define BFCHG   0x01		/* Changed.         */
#define BFBAK   0x02		/* Need to make a backup.       */
#define BFBAD   0x04		/* may be trashed alloc error?  */
#define BFINMEM 0x08		/* File is entirely in memory */
#define BFVIEW  0x10		/* read only (jam)               */
#define BFLINK  0x20		/* Linked mode    pvr        */
#define BFSLOCK 0x40		/* Lock buffer size   pvr    */
/*
 * This structure holds the starting position
 * (as a line/offset pair) and the number of characters in a
 * region of a buffer. This makes passing the specification
 * of a region around a little bit easier.
 * There have been some complaints that the short in this
 * structure is wrong; that a long would be more appropriate.
 * I'll await more comments from the folks with the little
 * machines; I have a VAX, and everything fits.
 */
typedef struct reg
{
    struct LINE *r_linep;	/* Origin LINE address.         */
    LPOS r_offset;		/* Origin LINE offset.          */
    A32 r_size;			/* Length in characters.        */
} REGION;

/*
 * All text is kept in circularly linked
 * lists of "LINE" structures. These begin at the
 * header line (which is the blank line beyond the
 * end of the buffer). This line is pointed to by
 * the "BUFFER". Each line contains a the number of
 * bytes in the line (the "used" size), the size
 * of the text array, and the text. The end of line
 * is not stored as a byte; it's implied. Future
 * additions will include update hints, and a
 * list of marks into the line.
 */
typedef struct LINE
{
    struct LINE *l_fp;		/* Link to the next line        */
    struct LINE *l_bp;		/* Link to the previous line    */
    A32 l_file_offset;		/* Offset from begining of file pvr */
    LPOS l_size;		/* Allocated size           */
    LPOS l_used;		/* Used size            */
#if     PCC
    D8 l_text[1];		/* A bunch of characters.       */
#else
    D8 l_text[];		/* A bunch of characters.       */
#endif
} LINE;

/*
 * The rationale behind these macros is that you
 * could (with some editing, like changing the type of a line
 * link from a "LINE *" to a "REFLINE", and fixing the commands
 * like file reading that break the rules) change the actual
 * storage representation of lines to use something fancy on
 * machines with small address spaces.
 */
#define lforw(lp)       ((lp)->l_fp)
#define lback(lp)       ((lp)->l_bp)
#define lgetc(lp, n)    ((lp)->l_text[(n)]&0xFF)
#define lputc(lp, n, c) ((lp)->l_text[(n)]=(c))
#define llength(lp)     ((lp)->l_used)

/*
 * Externals.
 */
extern int thisflag;
extern int lastflag;
extern int curgoal;
extern int epresf;
extern int sgarbf;
extern WINDOW *curwp;
extern BUFFER *curbp;
extern WINDOW *wheadp;
extern BUFFER *bheadp;
extern BUFFER *blistp;
extern short kbdm[];
extern short *kbdmip;
extern short *kbdmop;
extern SYMBOL *symbol[];
extern SYMBOL *binding[];
extern BUFFER *bfind ();
extern BUFFER *bcreate ();
extern WINDOW *wpopup ();
extern LINE *lalloc ();
extern int nrow;
extern int ncol;
extern char version[];
extern int ttrow;
extern int ttcol;
extern int tceeol;
extern int tcinsl;
extern int tcdell;
extern char cinfo[];
extern SYMBOL *symlookup ();
extern int nmsg;
extern int curmsgf;
extern int newmsgf;
extern char msg[];

/* jam
 */
extern char *okmsg;
extern int insert_mode;
extern int extend_buf;
extern int flush_num;
extern int auto_update;
extern int flush_count;
extern int rowb;
extern char file_off_bad;

/*
 * Standard I/O.
 */
#ifndef linux
extern char *malloc ();
extern char *strcpy ();
extern char *strcat ();
#else
#include <stdio.h>
#include <string.h>
#endif

#ifndef NOPROTO
#include "prototyp.h"
#endif /* NOPROTO */
