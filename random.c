/*
*              Assorted commands.
* The file contains the command
* processors for a large assortment of unrelated
* commands. The only thing they have in common is
* that they are all command processors.
*/

#include    <string.h>
#include    "def.h"


extern char MSG_sh_pos[];
extern char MSG_sh_pos1[];
extern char MSG_f_str[];
extern char MSG_3u[];
extern char MSG_5u[];
extern char MSG_lu[];
extern char MSG_03u[];
extern char MSG_05u[];
extern char MSG_010lu[];
extern char MSG_116e[];
extern char MSG_106e[];
extern char MSG_lnk[];
extern char MSG_unlink[];
extern char MSG_link[];
extern char MSG_bad_key[];
extern char MSG_esc[];
extern char MSG_ctl_x[];
extern char MSG_ctl[];
extern char MSG_key_code[];
extern char char_str[];
extern char MSG_w_not_empty[];
extern char MSG_procing[];
extern char MSG_ok[];
extern char MSG_edit_float[];
#if RUNCHK
extern char ERR_rnd_1[];
extern char ERR_rnd_2[];
extern char ERR_rnd_3[];
extern char ERR_rnd_4[];
extern char ERR_rnd_5[];
extern char ERR_rnd_6[];
extern char ERR_rnd_7[];
#endif

extern ROW_FMT ascii_fmt;
extern ROW_FMT ebcdic_fmt;
extern ROW_FMT binary_8_fmt;
extern ROW_FMT binary_16_fmt;
extern ROW_FMT binary_32_fmt;
extern ROW_FMT octal_8_fmt;
extern ROW_FMT octal_16_fmt;
extern ROW_FMT octal_32_fmt;
extern ROW_FMT decimal_8_fmt;
extern ROW_FMT decimal_16_fmt;
extern ROW_FMT decimal_32_fmt;
#if	FLOAT_DISP
extern ROW_FMT float_32_fmt;
extern ROW_FMT float_64_fmt;
#endif
extern ROW_FMT hex_8_fmt;
extern ROW_FMT hex_16_fmt;
extern ROW_FMT hex_32_fmt;

extern bool read_pat_mode;
extern bool dont_repeat;
extern BUFFER sav_buf;


/*
* Display a bunch of useful information about
* the current location of dot and mark.
* The position of the dot and mark and the difference between them.
* The total buffer size is displayed.
* This is normally bound to "C-X =".
*/
bool
showcpos (f, n, k)
    int f, n, k;
{

    A32 dotoff, markoff, fsize, bsize;
    char buf[NCOL * 2], buf1[NCOL * 2];

    dotoff = curwp->w_dotp->l_file_offset;
    dotoff += curwp->w_doto;

    if (curwp->w_markp != NULL)
    {
	markoff = curwp->w_markp->l_file_offset;
	markoff += curwp->w_marko;
    }

    bsize = curwp->w_bufp->b_linep->l_bp->l_file_offset;
    bsize += curwp->w_bufp->b_linep->l_bp->l_used;
    fsize = curbp->b_file_size;

    if (curwp->w_markp != NULL)
    {
	/* build format string */
	sprintf (buf1, MSG_sh_pos, R_POS_FMT (curwp), R_POS_FMT (curwp),
		 R_POS_FMT (curwp), R_POS_FMT (curwp));
	sprintf (buf, buf1, dotoff, markoff, bsize, fsize);
    }
    else
    {
	/* build format string */
	sprintf (buf1, MSG_sh_pos1, R_POS_FMT (curwp), R_POS_FMT (curwp),
		 R_POS_FMT (curwp));
	sprintf (buf, buf1, dotoff, bsize, fsize);
    }

    sprintf (&buf[strlen (buf)], MSG_f_str, curbp->b_fname);
    writ_echo (buf);

    return (TRUE);
}

/*
* Twiddle the two characters on either side of
* dot. If dot is at the end of the line twiddle the
* two characters before it. Return with an error if dot
* is at the beginning of line; it seems to be a bit
* pointless to make this work. This fixes up a very
* common typo with a single stroke. Normally bound
* to "C-T". This always works within a line, so
* "WFEDIT" is good enough.
*/
bool
twiddle ()
{

    register LINE *dotp;
    register short doto;
    unsigned char b_per_u, f_buf[4], s_buf[4], i;

    dotp = curwp->w_dotp;
    doto = curwp->w_doto;
    b_per_u = curwp->w_fmt_ptr->r_b_per_u;
    /* try to move back one unit */
    if (!move_ptr (curwp, (long) -b_per_u, TRUE, TRUE, TRUE))
    {
	curwp->w_dotp = dotp;	/* if fail then restore dot and quit */
	curwp->w_doto = doto;
	ttbeep ();
	return (FALSE);
    }
    /* pick up first unit byte by byte */
    for (i = 0; i < b_per_u; i++)
    {
	f_buf[(int)i] = DOT_CHAR (curwp);
	move_ptr (curwp, 1L, TRUE, FALSE, TRUE);
    }
    /* move to the end of the second unit */
    if (!move_ptr (curwp, (long) (b_per_u - 1), TRUE, FALSE, TRUE))
    {
	curwp->w_dotp = dotp;	/* if fail then restore dot and quit */
	curwp->w_doto = doto;
	ttbeep ();
	return (FALSE);
    }
    /* pick up second unit (reverse order) and deposit second unit */
    for (i = 0; i < b_per_u; i++)
    {
	s_buf[(int)i] = DOT_CHAR (curwp);
	DOT_CHAR (curwp) = f_buf[(int)b_per_u - 1 - i];
	move_ptr (curwp, -1L, TRUE, FALSE, TRUE);
    }
    /* deposit first unit */
    for (i = 0; i < b_per_u; i++)
    {
	DOT_CHAR (curwp) = s_buf[(int)i];
	move_ptr (curwp, -1L, TRUE, FALSE, TRUE);
    }
    curwp->w_dotp = dotp;
    curwp->w_doto = doto;
    lchange (WFHARD);
    return (TRUE);
}

/*
* Quote the next character, and insert it into the buffer.
* All the characters are taken literally.
* The character is always read, even if it is inserted 0 times, for
* regularity.
*/
bool
quote (f, n, k)
    int f, n, k;
{
    register int c;

    if (kbdmop != NULL)
	c = *kbdmop++;
    else
    {
	c = ttgetc ();
	if (kbdmip != NULL)
	{
	    if (kbdmip > &kbdm[NKBDM - 4])
	    {
		ctrlg (FALSE, 0, KRANDOM);
		return (ABORT);
	    }

	    *kbdmip++ = c;
	}

    }

    if (n < 0)
	return (FALSE);
    if (n == 0)
	return (TRUE);

    return (linsert (n, (uchar) c));
}

/*
* Toggle the insert mode.  Insert mode is used only in ASCII or EBCDIC modes.
*/
bool
insert_toggle ()		/* toggle routine for selfinsert */
{
    register WINDOW *wp;

    if (curbp->b_flag & BFSLOCK)
	return (TRUE);

    if (read_pat_mode)
	dont_repeat = TRUE;

    insert_mode = !insert_mode;
    for (wp = wheadp; wp; wp = wp->w_wndp)
	wp->w_flag |= WFMODE;	/* force mode line update */
    return (TRUE);
}

/*
* Ordinary text characters are bound to this function,
* which inserts them into the buffer. Characters marked as control
* characters (using the CTRL flag) may be remapped to their ASCII
* equivalent. This makes TAB (C-I) work right, and also makes the
* world look reasonable if a control character is bound to this
* this routine by hand. Any META or CTLX flags on the character
* are discarded.
*
*   Edit the unit under the cursor.
*   Check that the character is valid for the current display mode.
*/

bool
selfinsert (f, n, k)
    int f, n, k;
{
    register int c;
    char edt_buf[8], i_chr, b_per_u, u_offs, u_roffs, bit_shf, i;
    LINE *l_ptr;
    short d_offs;
    int bytes, temp_int;
    long dot_shf, l_mask, l_val;
    D64 d_val;
    char text_buf[32];
    static char max_dec_8[] = "255";
    static char max_dec_16[] = "65535";
    static char max_dec_32[] = "4294967295";
    int cur_col;

    bool intel;

    if (n < 0)
    {
	ttbeep ();
	return (FALSE);
    }
    if (n == 0)
    {
	ttbeep ();
	return (TRUE);
    }
    c = k & KCHAR;
    if ((k & KCTRL) != 0 && c >= '@' && c <= '_')	/* ASCII-ify.           */
	c -= '@';
    b_per_u = curwp->w_fmt_ptr->r_b_per_u;
    u_offs = curwp->w_unit_offset;
    u_roffs = curwp->w_fmt_ptr->r_chr_per_u - u_offs - 1;
    intel = curwp->w_intel_mode;

    cur_col = ttcol;

    switch ((uchar) (curwp->w_fmt_ptr->r_type))
    {
    case EBCDIC:
	c = to_ebcdic ((uchar) c);	/* convert ASCII to EBCDIC */
    case ASCII:
	if ((insert_mode) || (DOT_POS (curwp) == BUF_SIZE (curwp)))
	    linsert (n, (uchar) c);
	else
	    lreplace (n, (uchar) c);
	break;

    case HEX:
	if ((c >= '0') && (c <= '9'))
	{
	    i_chr = c - '0';	/* convert to binary */
	}
	else if ((c >= 'A') && (c <= 'F'))
	{
	    i_chr = c - 'A' + 10;	/* convert to binary */
	}
	else if ((c >= 'a') && (c <= 'f'))
	{
	    i_chr = c - 'a' + 10;	/* convert to binary */
	}
	else
	{
	    bad_key (k);
	    return (FALSE);
	}
	fill_out ();		/* expand buffer if necessary */

	/* position dot to byte to be altered */
	if (intel)
	    dot_shf = u_roffs >> 1;
	else
	    dot_shf = u_offs >> 1;

	/* save dot position for later */
	l_ptr = curwp->w_dotp;
	d_offs = curwp->w_doto;
	move_ptr (curwp, dot_shf, TRUE, FALSE, TRUE);

	if (u_offs & 1)
	{			/* lower nibble in byte */
	    i_chr &= 0x0f;
	    DOT_CHAR (curwp) &= 0xf0;
	    DOT_CHAR (curwp) |= i_chr;
	}
	else
	{			/* upper nibble in byte */
	    i_chr <<= 4;
	    i_chr &= 0xf0;
	    DOT_CHAR (curwp) &= 0x0f;
	    DOT_CHAR (curwp) |= i_chr;
	}

	/* restore dot position */
	curwp->w_dotp = l_ptr;
	curwp->w_doto = d_offs;
	forwchar (0, 1, KRANDOM);	/* advance the cursor */
	break;

    case BINARY:
	if ((c != '0') && (c != '1'))
	{
	    bad_key (k);
	    return (FALSE);
	}

	/* position dot to byte to be altered */
	if (intel)
	    dot_shf = u_roffs >> 3;
	else
	    dot_shf = u_offs >> 3;

	fill_out ();		/* expand buffer if necessary */

	/* save dot position for later */
	l_ptr = curwp->w_dotp;
	d_offs = curwp->w_doto;
	move_ptr (curwp, dot_shf, TRUE, FALSE, TRUE);

	bit_shf = u_roffs & 0x07;

	if (c == '0')
	{
	    DOT_CHAR (curwp) &= ~(1 << bit_shf);
	}
	else
	{
	    DOT_CHAR (curwp) |= 1 << bit_shf;
	}

	/* restore dot position */
	curwp->w_dotp = l_ptr;
	curwp->w_doto = d_offs;
	forwchar (0, 1, KRANDOM);	/* advance the cursor */
	break;

    case OCTAL:
	if (c < '0')
	{
	    bad_key (k);
	    return (FALSE);
	}
	else if ((c > '1') && (u_offs == 0) &&
		 ((curwp->w_fmt_ptr->r_size) == WORDS))
	{
	    bad_key (k);
	    return (FALSE);
	}
	else if ((c > '3') && (u_offs == 0))
	{
	    bad_key (k);
	    return (FALSE);
	}
	else if (c > '7')
	{
	    bad_key (k);
	    return (FALSE);
	}

	dot_shf = (c - '0') & 7;/* get binary value */
	l_mask = 7;		/* create bit mask */

	dot_shf <<= (u_roffs * 3);
	l_mask <<= (u_roffs * 3);

	fill_out ();		/* expand buffer if necessary */

	/* save dot position for later */
	l_ptr = curwp->w_dotp;
	d_offs = curwp->w_doto;

	/* position dot to the byte to be altered */
	if (intel)
	{
	    for (i = 0; i < b_per_u; i++)
	    {
		DOT_CHAR (curwp) &= ~((D8) l_mask & 0xff);
		DOT_CHAR (curwp) |= (D8) dot_shf & 0xff;
		l_mask >>= 8;
		dot_shf >>= 8;
		move_ptr (curwp, 1L, TRUE, FALSE, TRUE);
	    }
	}
	else
	{
	    move_ptr (curwp, (long) (b_per_u - 1), TRUE, FALSE, TRUE);
	    /* move to last byte */
	    for (i = 0; i < b_per_u; i++)
	    {
		DOT_CHAR (curwp) &= ~((D8) l_mask & 0xff);
		DOT_CHAR (curwp) |= (D8) dot_shf & 0xff;
		l_mask >>= 8;
		dot_shf >>= 8;
		move_ptr (curwp, -1L, TRUE, FALSE, TRUE);	/* step back one byte */
	    }
	}

	/* restore dot position */
	curwp->w_dotp = l_ptr;
	curwp->w_doto = d_offs;
	forwchar (0, 1, KRANDOM);	/* advance the cursor */
	break;

    case DECIMAL:
	fill_out ();		/* expand buffer if necessary */

	/* save dot position for later */
	l_ptr = curwp->w_dotp;
	d_offs = curwp->w_doto;

	bytes = fill_buf (curwp, l_ptr, d_offs, edt_buf, b_per_u);
	/* if last unit is not full and must be extended */
	for (; bytes < b_per_u; bytes++)
	{
	    edt_buf[3] = edt_buf[2];	/* shuffle bytes down */
	    edt_buf[2] = edt_buf[1];
	    edt_buf[1] = edt_buf[0];
	    edt_buf[0] = 0;
	}
	switch (curwp->w_fmt_ptr->r_size)
	{
	case BYTES:
	    sprintf (text_buf, MSG_03u, (int) (edt_buf[0] & 0xff));
	    if (!dec_chr_ok (text_buf, max_dec_8, (char) c, u_offs))
	    {
		bad_key (k);
		return (TRUE);	/* TRUE so that mask will be same len */
	    }
	    sscanf (text_buf, MSG_3u, &i);	/* convert back to binary */
	    l_val = (long) i & 0xff;
	    break;

	case WORDS:
	    l_val = get_int (edt_buf);	/* do intel swap */
	    sprintf (text_buf, MSG_05u, (int) (l_val & 0xFFFF));
	    if (!dec_chr_ok (text_buf, max_dec_16, (char) c, u_offs))
	    {
		bad_key (k);
		return (TRUE);	/* TRUE so that mask will be same len */
	    }
	    sscanf (text_buf, MSG_5u, &temp_int);
	    /* convert back to binary */
	    l_val = get_int ((char *) &temp_int);	/* do intel swap */
	    break;

	case DWORDS:
	    l_val = get_long (edt_buf);	/* do intel swap */
	    sprintf (text_buf, MSG_010lu, l_val);
	    if (!dec_chr_ok (text_buf, max_dec_32, (char) c, u_offs))
	    {
		bad_key (k);
		return (TRUE);	/* TRUE so that mask will be same len */
	    }
	    sscanf (text_buf, MSG_lu, &l_val);
	    /* convert back to binary */
	    l_val = get_long ((char *) &l_val);	/* do intel swap */
	    break;
#if RUNCHK
	default:
	    writ_echo (ERR_rnd_2);
	    break;
#endif
	}

	DOT_CHAR (curwp) = (char) l_val & 0xff;
	for (i = 1; i < b_per_u; i++)
	{
	    l_val >>= 8;
	    move_ptr (curwp, 1L, TRUE, FALSE, TRUE);	/* step forward one byte */
	    DOT_CHAR (curwp) = (char) l_val & 0xff;
	}

	/* restore dot position */
	curwp->w_dotp = l_ptr;
	curwp->w_doto = d_offs;
	forwchar (0, 1, KRANDOM);	/* advance the cursor */
	break;

    case FLOAT:
	{
	    char temp_text[32];
	    D64 d_temp;

	    /* couldn't make this work so error it out */
	    writ_echo (MSG_edit_float);
	    return (FALSE);

	    fill_out ();	/* expand buffer if necessary */

	    /* save dot position for later */
	    l_ptr = curwp->w_dotp;
	    d_offs = curwp->w_doto;

	    bytes = fill_buf (curwp, l_ptr, d_offs, edt_buf, b_per_u);
	    /* if last unit is not full and must be extended */
	    for (; bytes < b_per_u; bytes++)
	    {
		edt_buf[7] = edt_buf[6];	/* shuffle bytes down */
		edt_buf[6] = edt_buf[5];
		edt_buf[5] = edt_buf[4];
		edt_buf[4] = edt_buf[3];
		edt_buf[3] = edt_buf[2];
		edt_buf[2] = edt_buf[1];
		edt_buf[1] = edt_buf[0];
		edt_buf[0] = 0;
	    }
	    d_val = get_double (edt_buf);	/* do intel swap */
	    /* get a text representation of the float */
	    sprintf (text_buf, MSG_116e, d_val);
	    /* insert the character that was typed */
	    text_buf[(int)u_offs] = c;
	    /* see if scanf get get back a good number */

	    /************************************************
            This is where the problem lies.   The sscanf would
            not read in the same number that the sprinf would
            spit out.   Maybe the sscanf did not like the
            scientific notation at maximum precesion.  pvr 7/4/94
            *************************************************/

	    if (sscanf (text_buf, MSG_116e, &d_temp) != 1)
	    {
		bad_key (k);
		return (TRUE);	/* TRUE so that mask will be same len */
	    }
	    /* printf the the number */
	    sprintf (temp_text, MSG_116e, d_temp);
	    /* see if the text strings match */
	    if (strcmp (temp_text, text_buf))
	    {
		bad_key (k);
		return (TRUE);	/* TRUE so that mask will be same len */
	    }
	    sscanf (text_buf, MSG_116e, &d_val);
	    /* convert back to binary */
	    d_val = get_double ((char *) &d_val);	/* do intel swap */

	    DOT_CHAR (curwp) = (char) d_val & 0xff;
	    for (i = 1; i < b_per_u; i++)
	    {
		D8 *ptr;

		ptr = (D8 *) & d_val;
		move_ptr (curwp, 1L, TRUE, FALSE, TRUE);	/* step forward one byte */
		DOT_CHAR (curwp) = ptr[(int)i] & 0xff;
	    }

	    /* restore dot position */
	    curwp->w_dotp = l_ptr;
	    curwp->w_doto = d_offs;
	    forwchar (0, 1, KRANDOM);	/* advance the cursor */
	    break;
	}
#if RUNCHK
    default:
	writ_echo (ERR_rnd_3);
	break;
#endif
    }
    /* if cursor has wrapped to the next line then previous line
	    will not be refreshed with WFEDIT so do a WFHARD */
    if (cur_col > get_curcol (curwp))
	lchange (WFHARD);
    else
	lchange (WFEDIT);

    return (TRUE);
}

/*
*   Insert one unit of zeros at the current dot position.
*/
bool
insertunit (f, n, k)
    int f, n, k;
{
    lchange (WFEDIT);
    linsert ((R_B_PER_U (curwp) * n), 0);
    return (TRUE);
}

/*
*   Increase the size of the buffer if necessary.
*   If dot is at the byte after the last full unit
*   then add enough bytes to the buffer to create
*   a full unit at the end.
*/

bool
fill_out ()
{
    long buf_size, dot_pos, last_unit;
    int b_per_u;
    char stat, shift;
    int insert_val;

    buf_size = BUF_SIZE (curwp);
    dot_pos = DOT_POS (curwp);
    b_per_u = R_B_PER_U (curwp);
    shift = curwp->w_disp_shift;
    stat = TRUE;
    insert_val = 0;
    last_unit = buf_size & ~((long) (b_per_u - 1));
    /* there is an even number of units step back one */
    if (last_unit == buf_size)
	last_unit -= b_per_u;
    last_unit += shift;

    /* if dot is one byte past the end of the buffer */
    if (dot_pos > last_unit)
    {
	insert_val = b_per_u;
    }

    /* if dot is pointed at the end of the buffer */
    else if (dot_pos == last_unit)
    {
	insert_val = b_per_u - (buf_size - last_unit);
    }

    /* if insert is necessary then do it */
    if (insert_val != 0)
    {
	lchange (WFHARD);
	move_ptr (curwp, buf_size, TRUE, FALSE, FALSE);	/* move dot to end */
	stat = linsert (insert_val, 0);
	move_ptr (curwp, dot_pos, TRUE, TRUE, FALSE);	/* put dot back */
    }
    return (stat);
}

/*
*   This checks that an entered character is ok
*   for the position given.
*/

char
dec_chr_ok (char_buf, max_str, chr, pos)

    char chr, pos, *char_buf, *max_str;

{
    char i;

    if ((chr < '0') || (chr > '9'))
	return (FALSE);

    char_buf[(int)pos] = chr;	/* insert typed char */

    /* check if number is too big */
    for (i = 0; max_str[(int)i] != 0; i++)
    {
	if (char_buf[(int)i] < max_str[(int)i])
	    break;		/* if char is smaller then must be ok */

	if (char_buf[(int)i] > max_str[(int)i])
	    return (FALSE);	/* val is too large; ERROR */
    }
    return (TRUE);
}

/*
* Set the rest of the variables for the mode change.
*/
void
set_mode_vars ()
{
    curwp->w_disp_shift = 0;	/* shift to 0 when changing mode */
    curwp->w_unit_offset = 0;	/* go to end of unit */
    /* if we are in the middle of a search then use the proper format struc */
    if (read_pat_mode)
	curwp->w_fmt_ptr = curwp->w_fmt_ptr->r_srch_fmt;

    wind_on_dot (curwp);
    curwp->w_flag = WFHARD;
    update ();
}

/*
* Change the display mode to ASCII.
* The default binding is META C-A.
*/
bool
asciimode ()
{
    curwp->w_fmt_ptr = &ascii_fmt;
    set_mode_vars ();
    return (TRUE);
}

/*
* Change the display mode to EBCDIC.
* The default binding is META C-E.
*/
bool
ebcdicmode ()
{
    curwp->w_fmt_ptr = &ebcdic_fmt;
    set_mode_vars ();
    return (TRUE);
}

#if	FLOAT_DISP
/*
* Change the display mode to FLOAT.
* The default binding is META C-F.
*/
bool
floatmode ()
{
    switch (curwp->w_fmt_ptr->r_size)
    {
    case DWORDS:
      curwp->w_fmt_ptr = &float_32_fmt;
      break;
    case DOUBLES:
    curwp->w_fmt_ptr = &float_64_fmt;
      break;
    }
    set_mode_vars ();
    return (TRUE);
}

#endif

/*
* Change the display mode to DECIMAL.
* The default binding is META C-D.
*/
bool
decimalmode ()
{
    switch (curwp->w_fmt_ptr->r_size)
    {
    case BYTES:
	curwp->w_fmt_ptr = &decimal_8_fmt;
	break;
    case WORDS:
	curwp->w_fmt_ptr = &decimal_16_fmt;
	break;

    case DOUBLES:
    case DWORDS:
	curwp->w_fmt_ptr = &decimal_32_fmt;
	break;
#if RUNCHK
    default:
	writ_echo (ERR_rnd_4);
	break;
#endif
    }
    set_mode_vars ();
    return (TRUE);
}

/*
* Change the display mode to HEXADECIMAL.
* The default binding is META C-H.
*/
bool
hexmode ()
{
    switch (curwp->w_fmt_ptr->r_size)
    {
    case BYTES:
	curwp->w_fmt_ptr = &hex_8_fmt;
	break;
    case WORDS:
	curwp->w_fmt_ptr = &hex_16_fmt;
	break;
    case DWORDS:
    case DOUBLES:
	curwp->w_fmt_ptr = &hex_32_fmt;
	break;
#if RUNCHK
    default:
	writ_echo (ERR_rnd_5);
	break;
#endif
    }
    set_mode_vars ();
    return (TRUE);
}

/*
* Change the display mode to OCTAL.
* The default binding is META C-O.
*/
bool
octalmode ()
{
    switch (curwp->w_fmt_ptr->r_size)
    {
    case BYTES:
	curwp->w_fmt_ptr = &octal_8_fmt;
	break;

    case WORDS:
	curwp->w_fmt_ptr = &octal_16_fmt;
	break;

    case DOUBLES:
    case DWORDS:
	curwp->w_fmt_ptr = &octal_32_fmt;
	break;
#if RUNCHK
    default:
	writ_echo (ERR_rnd_6);
	break;
#endif
    }
    set_mode_vars ();
    return (TRUE);
}

/*
* Change the display mode to BINARY.
* The default binding is META C-B.
*/
bool
binarymode ()
{
    switch (curwp->w_fmt_ptr->r_size)
    {
    case BYTES:
	curwp->w_fmt_ptr = &binary_8_fmt;
	break;
    case WORDS:
	curwp->w_fmt_ptr = &binary_16_fmt;
	break;
    case DOUBLES:
    case DWORDS:
	curwp->w_fmt_ptr = &binary_32_fmt;
	break;
#if RUNCHK
    default:
	writ_echo (ERR_rnd_7);
	break;
#endif
    }
    set_mode_vars ();
    return (TRUE);
}

/*
* Change the display shift.
* Circularly rotate through display shift of 0 through 3.
* This value is used to shift the display by the designated number of bytes.
* This is used to cause WORD and DWORD values to be calculated
* from the correct offset.
*/
bool
dispshift (f, n, k)
    int f, n, k;
{
    uchar mode, size;

    if (read_pat_mode)
	return (TRUE);		/* no shift is allowed in search mode */


    mode = curwp->w_fmt_ptr->r_type;
    size = curwp->w_fmt_ptr->r_size;

    if (((mode == HEX) ||
	 (mode == FLOAT) ||
	 (mode == DECIMAL) ||
	 (mode == BINARY) ||
	 (mode == OCTAL)) &&
	(size != BYTES))
    {
	if ((size == WORDS) &&
	    (curwp->w_disp_shift >= 1))
	{			/* roll over on words */
	    curwp->w_disp_shift = 0;
	}
	else
	{
	    if ((size == DWORDS) &&
		(curwp->w_disp_shift >= 3))
	    {			/* roll over on longs */
		curwp->w_disp_shift = 0;
	    }
	    else
	    {
		if ((size == DOUBLES) &&
		    (curwp->w_disp_shift >= 7))
		{		/* roll over on doubles */
		    curwp->w_disp_shift = 0;
		}
		else
		{
		    curwp->w_disp_shift++;	/* increment shift */
		}
	    }
	}
    }
    else
    {
	curwp->w_disp_shift = 0;/* set to no shift */
    }
    move_ptr (curwp, 0L, TRUE, TRUE, TRUE);
    wind_on_dot (curwp);
    curwp->w_flag = WFHARD;	/* force full window refresh */
    return (TRUE);
}

/*
* Delete forward. This is real
* easy, because the basic delete routine does
* all of the work. Watches for negative arguments,
* and does the right thing. If any argument is
* present, it kills rather than deletes, to prevent
* loss of text if typed with a big argument.
* Normally bound to "C-D".
*/
char
forwdel (f, n, k)
    int f, n, k;
{
    char s;

    if (n < 0)
	return (backdel (f, -n, KRANDOM));

    s = FALSE;
    if (R_SIZE (curwp) == BYTES)
    {
	if (f != FALSE)
	{
	    /* Really a kill.       */
	    if ((lastflag & CFKILL) == 0)
		bclear (&sav_buf);
	    thisflag |= CFKILL;
	}
	s = ldelete ((A32) n, f);
	curwp->w_unit_offset = 0;
    }
    return (s);
}

/*
* Delete backwards. This is quite easy too,
* because it's all done with other functions. Just
* move the cursor back, and delete forwards.
* Like delete forward, this actually does a kill
* if presented with an argument.
*/
char
backdel (f, n, k)
    int f, n, k;
{

    int u_off;
    char s;

    if (n < 0)
	return (forwdel (f, -n, KRANDOM));

    s = FALSE;
    if (R_SIZE (curwp) == BYTES)
    {
	u_off = curwp->w_unit_offset;
	curwp->w_unit_offset = 0;
	if ((s = backchar (f, n * R_CHR_PER_U (curwp), KRANDOM)) == TRUE)
	{
	    s = ldelete ((A32) n, f);
	    if (f != FALSE)
	    {
		/* Really a kill.       */
		if ((lastflag & CFKILL) == 0)
		    bclear (&sav_buf);
		thisflag |= CFKILL;
	    }
	}
	curwp->w_unit_offset = u_off;
    }
    return (s);
}

/*
* Change the size of the display unit to BYTE.
* Adjust byte shift to the allowable range.
* Normally bound to "META-1".
*/
bool
dispsize1 ()
{
    curwp->w_disp_shift = 0;	/* shift to 0 when changing size */
    curwp->w_unit_offset = 0;	/* go to end of unit */

    switch ((uchar) (R_TYPE (curwp)))
    {
    case OCTAL:
	curwp->w_fmt_ptr = &octal_8_fmt;
	break;

    case DECIMAL:
	curwp->w_fmt_ptr = &decimal_8_fmt;
	break;

    case HEX:
	curwp->w_fmt_ptr = &hex_8_fmt;
	break;

    case BINARY:
	curwp->w_fmt_ptr = &binary_8_fmt;
	break;

    default:
	return (TRUE);
	break;
    }

    /* if we are in the middle of a search then use the proper format struc */
    if (read_pat_mode)
	curwp->w_fmt_ptr = curwp->w_fmt_ptr->r_srch_fmt;

    move_ptr (curwp, 0L, TRUE, TRUE, TRUE);
    wind_on_dot (curwp);
    curwp->w_flag = WFHARD;
    update ();
    return (TRUE);
}

/*
* Change the size of the display unit to WORD.
* Adjust byte shift to the allowable range.
* Normally bound to "META-2".
*/
bool
dispsize2 ()
{
    curwp->w_disp_shift = 0;	/* shift to 0 when changing size */
    curwp->w_unit_offset = 0;	/* go to end of unit */

    switch ((uchar) (R_TYPE (curwp)))
    {
    case OCTAL:
	curwp->w_fmt_ptr = &octal_16_fmt;
	break;

    case DECIMAL:
	curwp->w_fmt_ptr = &decimal_16_fmt;
	break;

    case HEX:
	curwp->w_fmt_ptr = &hex_16_fmt;
	break;

    case BINARY:
	curwp->w_fmt_ptr = &binary_16_fmt;
	break;

    default:
	return (TRUE);
	break;
    }

    /* if we are in the middle of a search then use the proper format struc */
    if (read_pat_mode)
	curwp->w_fmt_ptr = curwp->w_fmt_ptr->r_srch_fmt;

    move_ptr (curwp, 0L, TRUE, TRUE, TRUE);
    wind_on_dot (curwp);
    curwp->w_flag = WFHARD;
    update ();
    return (TRUE);
}

/*
* Change the size of the display unit to long.
* Adjust byte shift to the allowable range.
* Normally bound to "META-4".
*/
bool
dispsize4 ()
{
    curwp->w_disp_shift = 0;	/* shift to 0 when changing size */
    curwp->w_unit_offset = 0;	/* go to end of unit */

    switch ((uchar) (R_TYPE (curwp)))
    {
    case OCTAL:
	curwp->w_fmt_ptr = &octal_32_fmt;
	break;

    case DECIMAL:
	curwp->w_fmt_ptr = &decimal_32_fmt;
	break;

    case HEX:
	curwp->w_fmt_ptr = &hex_32_fmt;
	break;

    case BINARY:
	curwp->w_fmt_ptr = &binary_32_fmt;
	break;

    case FLOAT:
	curwp->w_fmt_ptr = &float_32_fmt;
	break;

    default:
	return (TRUE);
	break;
    }

    /* if we are in the middle of a search then use the proper format struc */
    if (read_pat_mode)
	curwp->w_fmt_ptr = curwp->w_fmt_ptr->r_srch_fmt;

    move_ptr (curwp, 0L, TRUE, TRUE, TRUE);
    wind_on_dot (curwp);
    curwp->w_flag = WFHARD;
    update ();
    return (TRUE);
}

/*
* Change the size of the display unit to double.
* Adjust byte shift to the allowable range.
* Normally bound to "META-8".
*/
bool
dispsize8 ()
{
    curwp->w_disp_shift = 0;	/* shift to 0 when changing size */
    curwp->w_unit_offset = 0;	/* go to end of unit */

    switch ((uchar) (R_TYPE (curwp)))
    {
    case FLOAT:
	curwp->w_fmt_ptr = &float_64_fmt;
	break;

    default:
	return (TRUE);
	break;
    }

    /* if we are in the middle of a search then use the proper format struc */
    if (read_pat_mode)
	curwp->w_fmt_ptr = curwp->w_fmt_ptr->r_srch_fmt;

    move_ptr (curwp, 0L, TRUE, TRUE, TRUE);
    wind_on_dot (curwp);
    curwp->w_flag = WFHARD;
    update ();
    return (TRUE);
}

/*
* Display byte swaped.   This command causes the bytes
* that are displayed in WORD and DWORD mode to be swaped
* in the way that the INTEL microprocessors do it.
*/
bool
dispswapbyte (f, n, k)
    int f, n, k;
{
    if ((curwp->w_fmt_ptr->r_size) == BYTES)
	return (TRUE);

    if (curwp->w_intel_mode)
	curwp->w_intel_mode = FALSE;
    else
	curwp->w_intel_mode = TRUE;

    curwp->w_flag = WFHARD;
    update ();
    return (TRUE);
}

/*
* Yank text back from the kill buffer. This
* is really easy. All of the work is done by the
* standard insert routines. All you do is run the loop,
* and check for errors.
* An attempt has been made to fix the cosmetic bug
* associated with a yank when dot is on the top line of
* the window (nothing moves, because all of the new
* text landed off screen).
*/
bool
yank (f, n, k)
    int f, n, k;
{
    register D16 c;
    register A32 i;
    char buf[NCOL], buf1[NCOL];

    if (n < 0)
	return (FALSE);
    while (n--)
    {
	i = 0;
	save_buf_home ();
	while ((c = get_save_char ()) != (D16) - 1)
	{
	    if (linsert (1, (uchar) c) == FALSE)
		return (FALSE);
	    if ((i & 0x2ff) == 0)
	    {
		sprintf (buf1, MSG_procing, R_POS_FMT (curwp));
		sprintf (buf, buf1, (ulong) i);
		writ_echo (buf);
		/* check if we should quit */
		if (ttkeyready ())
		{
		    wind_on_dot_all ();
		    if (ttgetc () == CTL_G)
			return (FALSE);
		}
	    }
	    ++i;
	}
    }
    /* update buffer display */
    if ((blistp->b_nwnd != 0) &&
	(blistp->b_type == BTLIST))
	listbuffers ();

    curwp->w_flag |= WFHARD;
    return (TRUE);
}

/*
*   Link windows.   pvr
*   This function toggles the window linking function.
*   When linking is enabled all windows that look at
*   the same buffer will be forced to have the same
*   dot position.   Each window is then moved to be
*   positioned on the dot.   Thus when a user moves
*   arround a buffer all other views into that buffer
*   will follow.
*/

bool
linkwind ()

{
    char buf[NCOL];

    if (curwp->w_bufp->b_flag & BFLINK)
    {
	curwp->w_bufp->b_flag &= ~(BFLINK & 0xff);
	sprintf (buf, MSG_lnk, curwp->w_bufp->b_bname, MSG_unlink);
    }
    else
    {
	curwp->w_bufp->b_flag |= BFLINK;
	sprintf (buf, MSG_lnk, curwp->w_bufp->b_bname, MSG_link);
    }
    writ_echo (buf);
    return (TRUE);
}

/*
*   Print all bad keys to the screen and beep
*/
void
bad_key (key)
    int key;
{
    char buf[NCOL];

    ttbeep ();
    sprintf (buf, MSG_bad_key);
    keyname (&buf[strlen (buf)], key);
    sprintf (&buf[strlen (buf)], ", %X", key);
    writ_echo (buf);
}

/*
 *	Combine sequential bytes from the rest of the windows
 *	into this window.   This is useful in combining PROM
 *	image files from odd and even bytes into one file.
 */
bool
n_way_combine (f, n, k)
    int f, n, k;
{
    WINDOW *dest_wp, *src_wp;
    BUFFER *src_bp;
    A32 dotp;
    D8 byt;
    int j = 0;
    char buf[NCOL], buf1[NCOL];

    /* save the destination window for later restore */
    dest_wp = curwp;

    if ((BUF_SIZE (curwp)) != (A32) 0)
    {
	writ_echo (MSG_w_not_empty);
	return (FALSE);
    }
    /* Current window must be empty, modifiable and not the only one. */
    if ((BUF_SIZE (curwp) != 0) ||
	(curwp->w_wndp == NULL) ||
	(curwp->w_bufp->b_flag & (BFVIEW | BFSLOCK)))
    {
	writ_echo (MSG_w_not_empty);
	return (FALSE);
    }




    for (;;)
    {
	/* step to the next window after the destination window */
	nextwind ();

	/* as I cycle around the windows skip the destination window */
	if (curwp == dest_wp)
	{
	    continue;
	}
	byt = DOT_CHAR (curwp) & 0xff;
	dotp = DOT_POS (curwp);	/* get the current dot position */
	/* move the dot position ahead in current buffer */
	if (move_ptr (curwp, 1L, TRUE, FALSE, TRUE) == FALSE)
	{
	    /* did we advance? */
	    if (DOT_POS (curwp) == dotp)
	    {
		wind_on_dot_all ();
		writ_echo (MSG_ok);
		return (TRUE);	/* done all that we could */
	    }
	}

	src_wp = curwp;
	src_bp = curwp->w_bufp;
	curwp = dest_wp;
	curbp = dest_wp->w_bufp;
	if (linsert (1, byt) == FALSE)
	{
	    wind_on_dot_all ();
	    return (FALSE);	/* insert failed for some reason */
	}
	curwp = src_wp;
	curbp = src_bp;
	if ((j++ & 0x2ff) == 0)
	{
	    sprintf (buf1, MSG_procing, R_POS_FMT (curwp));
	    sprintf (buf, buf1, dotp);
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		wind_on_dot_all ();
		if (ttgetc () == CTL_G)
		    return (FALSE);
	    }
	}
    }
}

/*
 *	Split the current buffer into the rest of the windows.
 *	This is useful in splitting a binary file into PROM
 *	image files.
 */
bool
n_way_split (f, n, k)
    int f, n, k;
{
    WINDOW *src_wp;
    A32 b_size;
    D8 byt;
    int j = 0;
    char buf[NCOL], buf1[NCOL];

    /* save the source window and buffer for later restore */
    src_wp = curwp;

    /* step to the next window after the source window */
    nextwind ();

    /* check that all the destination windows are empty and modifiable */
    for (;;)
    {
	if ((BUF_SIZE (curwp) != 0) ||
	    (curwp->w_bufp->b_flag & (BFVIEW | BFSLOCK)))
	{
	    writ_echo (MSG_w_not_empty);
	    return (FALSE);
	}

	/* force all windows to be refreshed */
	lchange (WFHARD);
	/* step to the next window */
	nextwind ();
	/* stop after one pass around the windows */
	if (curwp == src_wp)
	    break;
    }

    b_size = BUF_SIZE (src_wp);	/* get the buffer size */

    /* do the split until source is exhausted */
    for (;;)
    {
	/* step to the next window after the source window */
	nextwind ();

	/* current window cannot be the source */
	if (curwp == src_wp)
	    continue;

	byt = DOT_CHAR (src_wp) & 0xff;	/* get the byte to copy */

	/* are we at the end of the buffer */
	if (b_size == DOT_POS (src_wp))
	{
	    wind_on_dot_all ();
	    writ_echo (MSG_ok);
	    return (TRUE);
	}
	if (linsert (1, byt) == FALSE)
	{
	    wind_on_dot_all ();
	    return (FALSE);
	}
	if ((j++ & 0x2ff) == 0)
	{
	    sprintf (buf1, MSG_procing, R_POS_FMT (src_wp));
	    sprintf (buf, buf1, DOT_POS (src_wp));
	    writ_echo (buf);
	    /* check if we should quit */
	    if (ttkeyready ())
	    {
		wind_on_dot_all ();
		if (ttgetc () == CTL_G)
		    return (FALSE);
	    }
	}
	if (move_ptr (src_wp, 1L, TRUE, FALSE, TRUE) == FALSE)
	{
	    wind_on_dot_all ();
	    writ_echo (MSG_ok);
	    return (TRUE);	/* hit the end of the source buffer */
	}
    }
}

void
wind_on_dot_all ()
{
    WINDOW *wp;

    wp = curwp;
    do
    {
	wind_on_dot (curwp);
	nextwind ();
    }
    while (wp != curwp);
}
