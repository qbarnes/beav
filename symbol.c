/*
*			  Symbol table stuff.
* Symbol tables, and keymap setup.
* The terminal specific parts of building the
* keymap has been moved to a better place.
*/
#include		"def.h"

void keyadd ();
void keydup ();


extern char MSG_byte_shift[];
extern char MSG_back_char[];
extern char MSG_quit[];
extern char MSG_forw_del_char[];
extern char MSG_toggle_swap[];
extern char MSG_forw_char[];
extern char MSG_abort[];
extern char MSG_ins_self[];
extern char MSG_back_del_char[];
extern char MSG_refresh[];
extern char MSG_forw_line[];
extern char MSG_back_line[];
extern char MSG_quote[];
extern char MSG_recall[];
extern char MSG_twiddle[];
extern char MSG_forw_page[];
extern char MSG_kill_region[];
extern char MSG_yank[];
extern char MSG_down_window[];
extern char MSG_ins_toggle[];
extern char MSG_display_buffers[];
extern char MSG_quit[];
extern char MSG_exit_flush_all[];
extern char MSG_set_file_name[];
extern char MSG_file_insert[];
extern char MSG_buf_size_lock[];
extern char MSG_flush_all[];
extern char MSG_down_window[];
extern char MSG_up_window[];
extern char MSG_file_read[];
extern char MSG_file_save[];
extern char MSG_file_visit[];
extern char MSG_file_write[];
extern char MSG_swap_dot_and_mark[];
extern char MSG_shrink_window[];
extern char MSG_display_position[];
extern char MSG_start_macro[];
extern char MSG_end_macro[];
extern char MSG_help[];
extern char MSG_only_window[];
extern char MSG_del_window[];
extern char MSG_split_window[];
extern char MSG_use_buffer[];
extern char MSG_spawn_cli[];
extern char MSG_execute_macro[];
extern char MSG_goto_line[];
extern char MSG_ins_unit[];
extern char MSG_kill_buffer[];
extern char MSG_load_bindings[];
extern char MSG_forw_window[];
extern char MSG_back_window[];
extern char MSG_view_file[];
extern char MSG_enlarge_window[];
extern char MSG_ascii_mode[];
extern char MSG_binary_mode[];
extern char MSG_buffer_name[];
extern char MSG_decimal_mode[];
extern char MSG_ebcdic_mode[];
#if	FLOAT_DISP
extern char MSG_float_mode[];
#endif
extern char MSG_hex_mode[];
extern char MSG_back_del_unit[];
extern char MSG_octal_mode[];
extern char MSG_display_version[];
extern char MSG_unit_size1[];
extern char MSG_unit_size2[];
extern char MSG_unit_size4[];
extern char MSG_reposition_window[];
extern char MSG_set_mark[];
extern char MSG_goto_eob[];
extern char MSG_goto_bob[];
extern char MSG_next_buff[];
extern char MSG_prev_buff[];
extern char MSG_query_replace[];
extern char MSG_display_bindings[];
extern char MSG_auto_save[];
extern char MSG_back_unit[];
extern char MSG_compare[];
extern char MSG_forw_del_unit[];
extern char MSG_forw_unit[];
extern char MSG_link_windows[];
extern char MSG_print[];
extern char MSG_back_search[];
extern char MSG_forw_search[];
extern char MSG_back_page[];
extern char MSG_copy_region[];
extern char MSG_extended_command[];
extern char MSG_up_window[];
extern char MSG_search_again[];
extern char MSG_bind_to_key[];
extern char MSG_file_visit_split[];
extern char MSG_yank_buffer[];
extern char MSG_save_region[];
extern char MSG_use_buffer_split[];
extern char MSG_no_f_tb[];
extern char MSG_n_split[];
extern char MSG_n_combine[];
extern char MSG_show_save_buf[];
extern char MSG_scr_row[];

/*
* Defined by "main.c".
*/
extern char ctrlg ();		/* Abort out of things	  */
extern char quit ();		/* Quit			 */
extern char ctlxlp ();		/* Begin macro		  */
extern char ctlxrp ();		/* End macro			*/
extern char ctlxe ();		/* Execute macro		*/
extern char showversion ();	/* Show version numbers, etc.   */
extern char flushnquit ();	/* Flush buffers & exit (fitz)  */
extern char flush_all ();	/* Flush buffers (jam)	  */
extern char autosave ();	/* autosave function (jam)  */

/*
* Defined by "search.c".
*/
extern char forwsearch ();	/* Search forward	   */
extern char backsearch ();	/* Search backwards	 */
extern char searchagain ();	/* Repeat last search command   */
extern char queryrepl ();	/* Query replace		*/
extern char compare ();		/* Compare two windows  */
extern char recall ();		/* Recall last search string  */

/*
* Defined by "basic.c".
*/
extern char backchar ();	/* Move backward by characters  */
extern char forwchar ();	/* Move forward by characters   */
extern char gotobob ();		/* Move to start of buffer  */
extern char gotoeob ();		/* Move to end of buffer	*/
extern char forwline ();	/* Move forward by lines	*/
extern char backline ();	/* Move backward by lines   */
extern char forwpage ();	/* Move forward by pages	*/
extern char backpage ();	/* Move backward by pages   */
extern char setmark ();		/* Set mark		 */
extern char swapmark ();	/* Swap "." and mark		*/
extern char gotoline ();	/* Go to a specified line.  */

/*
* Defined by "buffer.c".
*/
extern char listbuffers ();	/* Display list of buffers  */
extern char showsavebuf ();	/* Show the save buffer contents */
extern char usebuffer ();	/* Switch a window to a buffer  */
extern char use_buffer ();	/* ditto, plus window split */
extern char killbuffer ();	/* Make a buffer go away.   */
extern char next_buf ();	/* goto next buffer	 */
extern char prev_buf ();	/* goto prev buffer	 */
extern char yank_buffer ();	/* yank buffer by name	  */
extern char buffername ();	/* change buffer name	   */
extern char bufsizlock ();	/* lock buffer size		 */

/*
* Defined by "file."
*/
extern char fileread ();	/* Get a file, read only	*/
extern char filevisit ();	/* Get a file, read write   */
extern char file_visit ();	/* ditto , plus window split	*/
extern char filewrite ();	/* Write a file		 */
extern char filesave ();	/* Save current file		*/
extern char filename ();	/* Adjust file name	 */
extern char fileinsert ();	/* insert file to cursor (jam ) */
extern char viewfile ();	/* readonly file visit (jam)	*/

/*
* Defined by "random.c".
*/

extern char dispshift ();	/* Increment display shift   */
extern char selfinsert ();	/* Insert character  */
extern char insert_toggle ();	/* toggle insert mode  (jam)	*/
extern char insertunit ();	/* insert unit  (pvr)	*/
extern char showcpos ();	/* Show the cursor position */
extern char twiddle ();		/* Twiddle units		*/
extern char forwdel ();		/* Forward delete	   */
extern char backdel ();		/* Backward delete	  */
extern char quote ();		/* Insert literal	   */
extern char asciimode ();	/* display ASCII data   */
extern char ebcdicmode ();	/* display EBCDIC data   */
extern char decimalmode ();	/* display DECIMAL data   */
#if	FLOAT_DISP
extern char floatmode ();	/* display FLOATING POINT data   */
#endif
extern char hexmode ();		/* display HEX data   */
extern char octalmode ();	/* display OCTAL data   */
extern char binarymode ();	/* display BINARY data   */
extern char dispsize1 ();	/* display in BYTE format */
extern char dispsize2 ();	/* display in WORD format */
extern char dispsize4 ();	/* display in DWORD format*/
extern char dispswapbyte ();	/* Display swaped bytes	pvr   */
extern char yank ();		/* Yank back from killbuffer.   */
extern char linkwind ();	/* Link all windows on one buffer. */
extern char n_way_split ();	/* Split buffer into n buffers. */
extern char n_way_combine ();	/* Combine n buffers into one. */

/*
* Defined by "region.c".
*/
extern char killregion ();	/* Kill region.		 */
extern char copyregion ();	/* Copy region to kill buffer.  */
extern char save_region ();	/* Save region in named buffer. */

/*
* Defined by "spawn.c".
*/
extern char spawncli ();	/* Run CLI in a subjob.	 */
extern char clock ();		/* display time in modeline */

/*
* Defined by "window.c".
*/
extern char reposition ();	/* Reposition window		*/
extern char refresh ();		/* Refresh the screen	   */
extern char nextwind ();	/* Move to the next window  */
extern char prevwind ();	/* Move to the previous window  */
extern char mvdnwind ();	/* Move window down	 */
extern char mvupwind ();	/* Move window up	   */
extern char onlywind ();	/* Make current window only one */
extern char delwind ();		/* Delete current window */
extern char splitwind ();	/* Split current window	 */
extern char enlargewind ();	/* Enlarge display window.  */
extern char shrinkwind ();	/* Shrink window.	   */
extern char screen_rows ();	/* Set the screen size  */

/*
* Defined by "word.c".
*/
extern char backunit ();	/* Backup by units	  */
extern char forwunit ();	/* Advance by units	 */
extern char delfunit ();	/* Delete forward unit. */
extern char delbunit ();	/* Delete backward unit.	*/

/*
* Defined by "extend.c".
*/
extern char extend ();		/* Extended commands.	   */
extern char help ();		/* Help key.			*/
extern char bindtokey ();	/* Modify key bindings.	 */
extern char wallchart ();	/* Make wall chart.	 */
extern void check_extend ();	/* load extended key file   */
extern char load_extend ();	/* load extended file by name   */

/*
* Defined by "display.c
*/
extern char print ();		/* print window from mark to dot */

typedef struct
{
    short k_key;		/* Key to bind.				 */
    char (*k_funcp) ();		/* Function.			*/
    char *k_name;		/* Function name string.		*/
    char k_modify;		/* modify bit */
} KEY;

/*
* Default key binding table. This contains
* the function names, the symbol table name, and (possibly)
* a key binding for the builtin functions. There are no
* bindings for C-U or C-X. These are done with special
* code, but should be done normally.
*/
KEY key[] =
{
    KCTRL | 'A', dispshift, MSG_byte_shift, 0,
    KCTRL | 'B', backchar, MSG_back_char, SSRCH | SRPLC,
    KCTRL | 'C', quit, MSG_quit, 0,	/* pvr */
    KCTRL | 'D', forwdel, MSG_forw_del_char, SMOD | SSIZE | SSRCH | SRPLC,
    KCTRL | 'E', dispswapbyte, MSG_toggle_swap, SSRCH | SRPLC,	/* pvr */
    KCTRL | 'F', forwchar, MSG_forw_char, SSRCH | SRPLC,
    KCTRL | 'G', ctrlg, MSG_abort, SSRCH | SRPLC,
    KCTRL | 'I', selfinsert, MSG_ins_self, SMOD | SSRCH | SRPLC,
    KCTRL | 'H', backdel, MSG_back_del_char, SMOD | SSIZE | SSRCH | SRPLC,
    KCTRL | 'L', refresh, MSG_refresh, SSRCH | SRPLC,
    KCTRL | 'N', forwline, MSG_forw_line, SSRCH | SRPLC,
    KCTRL | 'P', backline, MSG_back_line, SSRCH | SRPLC,
    KCTRL | 'Q', quote, MSG_quote, 0,
    KCTRL | 'R', recall, MSG_recall, SSRCH | SRPLC,
    KCTRL | 'T', twiddle, MSG_twiddle, SMOD | SSRCH | SRPLC,
    KCTRL | 'V', forwpage, MSG_forw_page, SRPLC,
    KCTRL | 'W', killregion, MSG_kill_region, SMOD | SSIZE,
    KCTRL | 'Y', yank, MSG_yank, SMOD | SSIZE,
    KCTRL | 'Z', mvdnwind, MSG_down_window, 0,	/* fitz */
    KCTLX | KCTRL | 'A', insert_toggle, MSG_ins_toggle, SSRCH | SRPLC,
    KCTLX | KCTRL | 'B', listbuffers, MSG_display_buffers, 0,
    KCTLX | KCTRL | 'C', quit, MSG_quit, 0,
    KCTLX | KCTRL | 'E', flushnquit, MSG_exit_flush_all, 0,	/* fitz */
    KCTLX | KCTRL | 'F', filename, MSG_set_file_name, SMOD,	/* jam */
    KCTLX | KCTRL | 'I', fileinsert, MSG_file_insert, SMOD | SSIZE,
    KCTLX | KCTRL | 'L', bufsizlock, MSG_buf_size_lock, 0,
    KCTLX | KCTRL | 'M', flush_all, MSG_flush_all, 0,
    KCTLX | KCTRL | 'N', mvdnwind, MSG_down_window, 0,
    KCTLX | KCTRL | 'P', mvupwind, MSG_up_window, 0,
    KCTLX | KCTRL | 'R', fileread, MSG_file_read, 0,
    KCTLX | KCTRL | 'S', filesave, MSG_file_save, 0,
    KCTLX | KCTRL | 'V', filevisit, MSG_file_visit, 0,
    KCTLX | KCTRL | 'W', filewrite, MSG_file_write, 0,
    KCTLX | KCTRL | 'X', swapmark, MSG_swap_dot_and_mark, 0,
    KCTLX | KCTRL | 'Z', shrinkwind, MSG_shrink_window, 0,
    KCTLX | '=', showcpos, MSG_display_position, 0,
    KCTLX | '(', ctlxlp, MSG_start_macro, 0,
    KCTLX | ')', ctlxrp, MSG_end_macro, 0,
    KCTLX | '?', help, MSG_help, 0,
    KCTLX | '0', delwind, MSG_del_window, 0,
    KCTLX | '1', onlywind, MSG_only_window, 0,
    KCTLX | '2', splitwind, MSG_split_window, 0,
    KCTLX | 'B', usebuffer, MSG_use_buffer, 0,
    KCTLX | 'C', spawncli, MSG_spawn_cli, 0,	/* fitz */
    KCTLX | 'E', ctlxe, MSG_execute_macro, 0,
    KCTLX | 'G', gotoline, MSG_goto_line, 0,
    KCTLX | 'I', insertunit, MSG_ins_unit, SMOD | SSIZE | SSRCH | SRPLC,
    KCTLX | 'K', killbuffer, MSG_kill_buffer, 0,
    KCTLX | 'L', load_extend, MSG_load_bindings, 0,
    KCTLX | 'N', nextwind, MSG_forw_window, 0,
    KCTLX | 'P', prevwind, MSG_back_window, 0,
    KCTLX | 'V', viewfile, MSG_view_file, 0,	/* jam */
    KCTLX | 'Z', enlargewind, MSG_enlarge_window, 0,
    KMETA | KCTRL | 'A', asciimode, MSG_ascii_mode, SSRCH | SRPLC,	/* pvr */
    KMETA | KCTRL | 'B', binarymode, MSG_binary_mode, SSRCH | SRPLC,	/* pvr */
    KMETA | KCTRL | 'D', decimalmode, MSG_decimal_mode, SSRCH | SRPLC,	/* pvr */
    KMETA | KCTRL | 'E', ebcdicmode, MSG_ebcdic_mode, SSRCH | SRPLC,	/* pvr */
#if	FLOAT_DISP
    KMETA | KCTRL | 'F', floatmode, MSG_float_mode, SSRCH | SRPLC,	/* pvr */
#endif
    KMETA | KCTRL | 'H', hexmode, MSG_hex_mode, SSRCH | SRPLC,	/* pvr */
    KMETA | KCTRL | 'K', delbunit, MSG_back_del_unit, SMOD | SSIZE | SSRCH | SRPLC,
    KMETA | KCTRL | 'N', buffername, MSG_buffer_name, 0,
    KMETA | KCTRL | 'O', octalmode, MSG_octal_mode, SSRCH | SRPLC,	/* pvr */
    KMETA | KCTRL | 'P', n_way_combine, MSG_n_combine, SSIZE | SMOD,	/* pvr */
    KMETA | KCTRL | 'R', screen_rows, MSG_scr_row, 0,	/* pvr */
    KMETA | KCTRL | 'S', n_way_split, MSG_n_split, 0,	/* pvr */
    KMETA | KCTRL | 'V', showversion, MSG_display_version, 0,
    KMETA | KCTRL | 'W', showsavebuf, MSG_show_save_buf, 0,
    KMETA | '1', dispsize1, MSG_unit_size1, SSRCH | SRPLC,	/* pvr */
    KMETA | '2', dispsize2, MSG_unit_size2, SSRCH | SRPLC,	/* pvr */
    KMETA | '4', dispsize4, MSG_unit_size4, SSRCH | SRPLC,	/* pvr */
    KMETA | '!', reposition, MSG_reposition_window, 0,
    KMETA | '.', setmark, MSG_set_mark, 0,
    KMETA | '>', gotoeob, MSG_goto_eob, SSRCH | SRPLC,
    KMETA | '<', gotobob, MSG_goto_bob, SSRCH | SRPLC,
    KMETA | '+', next_buf, MSG_next_buff, 0,
    KMETA | '-', prev_buf, MSG_prev_buff, 0,
    KMETA | '%', queryrepl, MSG_query_replace, SMOD,
    KMETA | '?', wallchart, MSG_display_bindings, 0,
    KMETA | 'A', autosave, MSG_auto_save, 0,
    KMETA | 'B', backunit, MSG_back_unit, SSRCH | SRPLC,
    KMETA | 'C', compare, MSG_compare, 0,
    KMETA | 'D', delfunit, MSG_forw_del_unit, SMOD | SSIZE | SSRCH | SRPLC,
    KMETA | 'F', forwunit, MSG_forw_unit, SSRCH | SRPLC,
    KMETA | 'G', use_buffer, MSG_use_buffer_split, 0,
    KMETA | 'K', bindtokey, MSG_bind_to_key, 0,
    KMETA | 'L', linkwind, MSG_link_windows, 0,
    KMETA | 'O', save_region, MSG_save_region, 0,
    KMETA | 'P', print, MSG_print, 0,
    KMETA | 'R', backsearch, MSG_back_search, 0,
    KMETA | 'S', forwsearch, MSG_forw_search, 0,
    KMETA | 'T', searchagain, MSG_search_again, 0,
    KMETA | 'U', file_visit, MSG_file_visit_split, 0,
    KMETA | 'V', backpage, MSG_back_page, SRPLC,
    KMETA | 'W', copyregion, MSG_copy_region, 0,
    KMETA | 'X', extend, MSG_extended_command, 0,
    KMETA | 'Y', yank_buffer, MSG_yank_buffer, SMOD | SSIZE,
    KMETA | 'Z', mvupwind, MSG_up_window, 0
};

#define NKEY	(sizeof(key) / sizeof(key[0]))

/*
* Symbol table lookup.
* Return a pointer to the SYMBOL node, or NULL if
* the symbol is not found.
*/
SYMBOL *
symlookup (cp)
    register char *cp;
{
    register SYMBOL *sp;

    sp = symbol[symhash (cp)];
    while (sp != NULL)
    {
	if (strcmp (cp, sp->s_name) == 0)
	    return (sp);
	sp = sp->s_symp;
    }
    return (NULL);
}

/*
* Take a string, and compute the symbol table
* bucket number. This is done by adding all of the characters
* together, and taking the sum mod NSHASH. The string probably
* should not contain any GR characters; if it does the "*cp"
* may get a nagative number on some machines, and the "%"
* will return a negative number!
*/
int
symhash (cp)
    register char *cp;
{
    register int c;
    register int n;

    n = 0;
    while ((c = *cp++) != 0)
	n += c;
    return (n % NSHASH);
}

/*
* Build initial keymap. The funny keys
* (commands, odd control characters) are mapped using
* a big table and calls to "keyadd". The printing characters
* are done with some do-it-yourself handwaving. The terminal
* specific keymap initialization code is called at the
* very end to finish up. All errors are fatal.
*/
void
keymapinit ()
{
    register SYMBOL *sp;
    register KEY *kp;
    register int i;

    for (i = 0; i < NKEYS; ++i)
	binding[i] = NULL;
    for (kp = &key[0]; kp < &key[NKEY]; ++kp)
	keyadd (kp->k_key, kp->k_funcp, kp->k_name, kp->k_modify);
    keydup (KCTLX | KCTRL | 'G', MSG_abort);
    keydup (KMETA | KCTRL | 'G', MSG_abort);
    keydup (0x7F, MSG_back_del_char);
    keydup (KMETA | 'Q', MSG_quote);
    keydup (KMETA | 0x7F, MSG_back_del_unit);
    /*
  * Should be bound by "tab" already.
  */
    if ((sp = symlookup (MSG_ins_self)) == NULL)
	abort ();
    for (i = 0x20; i < 0x7F; ++i)
    {
	if (binding[i] != NULL)
	    abort ();
	binding[i] = sp;
	++sp->s_nkey;
    }
    ttykeymapinit ();
}

/*
* Create a new builtin function "name"
* with function "funcp". If the "new" is a real
* key, bind it as a side effect. All errors
* are fatal.
*/
void
keyadd (new, funcp, name, modify)
    short new;
#ifdef	NOPROTO
bool (*funcp) ();
#else
bool (*funcp) (void);
#endif
    char *name;
    int modify;
{
    register SYMBOL *sp;
    register int hash;

    if ((sp = (SYMBOL *) malloc (sizeof (SYMBOL))) == NULL)
	abort ();
    hash = symhash (name);
    sp->s_symp = symbol[hash];
    symbol[hash] = sp;
    sp->s_nkey = 0;
    sp->s_name = name;
    sp->s_funcp = funcp;
    sp->s_modify = modify;
    if (new >= 0)
    {
	/* Bind this key.	   */
	if (binding[new] != NULL)
	    abort ();
	binding[new] = sp;
	++sp->s_nkey;
    }
}

/*
* Bind key "new" to the existing
* routine "name". If the name cannot be found,
* or the key is already bound, abort.
*/
void
keydup (new, name)
    register int new;
    char *name;
{
    register SYMBOL *sp;

    if (binding[new] != NULL || (sp = symlookup (name)) == NULL)
    {
	printf (MSG_no_f_tb, name);
	abort ();
    }
    binding[new] = sp;
    ++sp->s_nkey;
}
