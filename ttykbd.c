/*
 *	  Wang PC keyboard handler
 */
#include	"def.h"

extern char MSG_sp_key[];
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
extern char MSG_hex_mode[];
extern char MSG_back_del_unit[];
extern char MSG_octal_mode[];
extern char MSG_display_version[];
extern char MSG_unit_size1[];
extern char MSG_unit_size2[];
extern char MSG_unit_size4[];
extern char MSG_unit_size8[];
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

#define SPECIAL 0x1F		/* Special keys		 */

typedef struct key_name_array
{
    int key_code;
    char *func_name_str;
    char *key_name_str;
} KEY_NAME_ARRAY;

#if MSDOS
extern bool wang_pc;
extern bool ibm_pc;
#endif
/*
 * The keyboard's special characters, those things that are prefixed with
 * a 0x1F, are placed into the keyboard tables as KCTRL || 0x800 || x, for some
 * x i.e. they have both the control and 0x800 bits set, so they won't conflict
 * with anything else on the keyboard.
 */


/*
 * Names for the keys with basic keycode
 * between KFIRST and KLAST (inclusive). This is used by
 * the key name routine in "kbd.c".
 */
#if MSDOS
KEY_NAME_ARRAY wang_keys[] =
{
    KCTRL | KFIRST | 0x00, MSG_bind_to_key, "Indent",
 /*  KCTRL | KFIRST | 0x01, NULL, "Page", */
    KCTRL | KFIRST | 0x02, MSG_reposition_window, "Center",
 /*  KCTRL | KFIRST | 0x03, NULL, "DecTab", */
 /*  KCTRL | KFIRST | 0x04, NULL, "Format", */
 /*  KCTRL | KFIRST | 0x05, NULL, "Merge", */
 /*  KCTRL | KFIRST | 0x06, NULL, "Note", */
    KCTRL | KFIRST | 0x07, MSG_set_mark, "Stop",
    KCTRL | KFIRST | 0x08, MSG_forw_search, "Search",
    KCTRL | KFIRST | 0x09, MSG_yank, "Replace",
    KCTRL | KFIRST | 0x0A, MSG_copy_region, "Copy",
    KCTRL | KFIRST | 0x0B, MSG_kill_region, "Move",
    KCTRL | KFIRST | 0x0C, MSG_extended_command, "Command",
    KCTRL | KFIRST | 0x0D, MSG_forw_window, "UpDown",
 /*  KCTRL | KFIRST | 0x0E, NULL, "BlankKey", */
    KCTRL | KFIRST | 0x0F, MSG_goto_line, "GoTo",
 /*  KCTRL | KFIRST | 0x10, NULL, "Sh-Indent", */
 /*  KCTRL | KFIRST | 0x11, NULL, "Sh-Page", */
 /*  KCTRL | KFIRST | 0x12, NULL, "Sh-Center", */
 /*  KCTRL | KFIRST | 0x13, NULL, "Sh-DecTab", */
 /*  KCTRL | KFIRST | 0x14, NULL, "Sh-Format", */
 /*  KCTRL | KFIRST | 0x15, NULL, "Sh-Merge", */
 /*  KCTRL | KFIRST | 0x16, NULL, "Sh-Note", */
 /*  KCTRL | KFIRST | 0x17, NULL, "Sh-Stop", */
    KCTRL | KFIRST | 0x18, MSG_search_again, "Sh-Search",
    KCTRL | KFIRST | 0x19, MSG_query_replace, "Sh-Replace",
 /*  KCTRL | KFIRST | 0x1A, NULL, "Sh-Copy", */
 /*  KCTRL | KFIRST | 0x1B, NULL, "Sh-Move", */
 /*  KCTRL | KFIRST | 0x1C, NULL, "Sh-Command", */
    KCTRL | KFIRST | 0x1D, MSG_split_window, "Sh-UpDown",
 /*  KCTRL | KFIRST | 0x1E, NULL, "Sh-BlankKey", */
 /*  KCTRL | KFIRST | 0x1F, NULL, "Sh-GoTo", */
    KCTRL | KFIRST | 0x40, MSG_back_line, "North",
    KCTRL | KFIRST | 0x41, MSG_forw_char, "East",
    KCTRL | KFIRST | 0x42, MSG_forw_line, "South",
    KCTRL | KFIRST | 0x43, MSG_back_char, "West",
    KCTRL | KFIRST | 0x44, MSG_byte_shift, "Home",
 /*  KCTRL | KFIRST | 0x45, NULL, "Execute", */
    KCTRL | KFIRST | 0x46, MSG_ins_toggle, "Insert",
    KCTRL | KFIRST | 0x47, MSG_forw_del_char, "Delete",
    KCTRL | KFIRST | 0x48, MSG_back_page, "PrevPage",
    KCTRL | KFIRST | 0x49, MSG_forw_page, "NextPage",
 /*  KCTRL | KFIRST | 0x4B, NULL, "Erase", */
 /*  KCTRL | KFIRST | 0x4D, NULL, "BackTab", */
 /*  KCTRL | KFIRST | 0x50, NULL, "Sh-North", */
    KCTRL | KFIRST | 0x51, MSG_forw_unit, "Sh-East",
 /*  KCTRL | KFIRST | 0x52, NULL, "Sh-South", */
    KCTRL | KFIRST | 0x53, MSG_back_unit, "Sh-West",
 /*  KCTRL | KFIRST | 0x54, NULL, "Sh-Home", */
    KCTRL | KFIRST | 0x55, MSG_execute_macro, "Sh-Execute",
 /*  KCTRL | KFIRST | 0x56, NULL, "Sh-Insert", */
    KCTRL | KFIRST | 0x57, MSG_forw_del_unit, "Sh-Delete",
    KCTRL | KFIRST | 0x58, MSG_goto_bob, "Sh-PrevPage",
    KCTRL | KFIRST | 0x59, MSG_goto_eob, "Sh-NextPage",
 /*  KCTRL | KFIRST | 0x5B, NULL, "Sh-Erase", */
 /*  KCTRL | KFIRST | 0x5C, NULL, "Sh-Tab", */
 /*  KCTRL | KFIRST | 0x5D, NULL, "Sh-BackTab", */
    KCTRL | KFIRST | 0x60, MSG_abort, "Cancel",
    KMETA | KCTRL | KFIRST | 0x60, MSG_abort, "Cancel",
    KCTLX | KCTRL | KFIRST | 0x60, MSG_abort, "Ctl-X Cancel",
    KCTRL | KFIRST | 0x61, MSG_display_bindings, "Help",
 /*  KCTRL | KFIRST | 0x62, NULL, "Glossary", */
    KCTRL | KFIRST | 0x63, MSG_print, "Print",
    KCTRL | KFIRST | 0x71, MSG_help, "Sh-Help",
 /*  KCTRL | KFIRST | 0x72, NULL, "Sh-Glossary", */
 /*  KCTRL | KFIRST | 0x73, NULL, "Sh-Print", */
    0, NULL, NULL
};
#endif

#if defined(MSDOS) || defined(OS2)
KEY_NAME_ARRAY ibm_keys[] =
{
    KCTLX | KFIRST | 0x3B, MSG_display_bindings, "F1",
    KCTLX | KFIRST | 0x3C, MSG_set_mark, "F2",
    KCTLX | KFIRST | 0x3D, MSG_forw_search, "F3",
    KCTLX | KFIRST | 0x3E, MSG_search_again, "F4",
    KCTLX | KFIRST | 0x3F, MSG_query_replace, "F5",
    KCTLX | KFIRST | 0x40, MSG_yank, "F6",
    KCTLX | KFIRST | 0x41, MSG_copy_region, "F7",
    KCTLX | KFIRST | 0x42, MSG_kill_region, "F8",
    KCTLX | KFIRST | 0x43, MSG_goto_line, "F9",
    KCTLX | KFIRST | 0x44, NULL, "F10",
    KCTLX | KFIRST | 0x85, NULL, "F11",
    KCTLX | KFIRST | 0x86, NULL, "F12",
    KCTLX | KFIRST | 0x54, MSG_help, "Sh-F1",
    KCTLX | KFIRST | 0x55, MSG_file_read, "Sh-F2",
    KCTLX | KFIRST | 0x56, MSG_file_save, "Sh-F3",
    KCTLX | KFIRST | 0x57, MSG_file_visit, "Sh-F4",
    KCTLX | KFIRST | 0x58, MSG_file_write, "Sh-F5",
    KCTLX | KFIRST | 0x59, MSG_flush_all, "Sh-F6",
    KCTLX | KFIRST | 0x5A, MSG_set_file_name, "Sh-F7",
    KCTLX | KFIRST | 0x5B, MSG_file_insert, "Sh-F8",
    KCTLX | KFIRST | 0x5C, MSG_exit_flush_all, "Sh-F9",
    KCTLX | KFIRST | 0x5D, MSG_quit, "Sh-F10",
    KCTLX | KFIRST | 0x87, NULL, "Sh-F11",
    KCTLX | KFIRST | 0x88, NULL, "Sh-F12",
    KCTLX | KFIRST | 0x5E, MSG_display_buffers, "Ctl-F1",
    KCTLX | KFIRST | 0x5F, MSG_use_buffer, "Ctl-F2",
    KCTLX | KFIRST | 0x60, MSG_kill_buffer, "Ctl-F3",
    KCTLX | KFIRST | 0x61, MSG_next_buff, "Ctl-F4",
    KCTLX | KFIRST | 0x62, MSG_prev_buff, "Ctl-F5",
    KCTLX | KFIRST | 0x63, MSG_yank_buffer, "Ctl-F6",
    KCTLX | KFIRST | 0x64, MSG_set_file_name, "Ctl-F7",
    KCTLX | KFIRST | 0x65, MSG_file_insert, "Ctl-F8",
    KCTLX | KFIRST | 0x66, MSG_exit_flush_all, "Ctl-F9",
    KCTLX | KFIRST | 0x67, MSG_quit, "Ctl-F10",
    KCTLX | KFIRST | 0x89, NULL, "Ctl-F11",
    KCTLX | KFIRST | 0x8A, NULL, "Ctl-F12",
    KCTLX | KFIRST | 0x68, NULL, "Alt-F1",
    KCTLX | KFIRST | 0x69, NULL, "Alt-F2",
    KCTLX | KFIRST | 0x6A, NULL, "Alt-F3",
    KCTLX | KFIRST | 0x6B, NULL, "Alt-F4",
    KCTLX | KFIRST | 0x6C, NULL, "Alt-F5",
    KCTLX | KFIRST | 0x6D, NULL, "Alt-F6",
    KCTLX | KFIRST | 0x6E, NULL, "Alt-F7",
    KCTLX | KFIRST | 0x6F, NULL, "Alt-F8",
    KCTLX | KFIRST | 0x70, NULL, "Alt-F9",
    KCTLX | KFIRST | 0x71, NULL, "Alt-F10",
    KCTLX | KFIRST | 0x8B, NULL, "Alt-F11",
    KCTLX | KFIRST | 0x8C, NULL, "Alt-F12",
    KCTLX | KFIRST | 0x48, MSG_back_line, "North",
    KCTLX | KFIRST | 0x4D, MSG_forw_char, "East",
    KCTLX | KFIRST | 0x50, MSG_forw_line, "South",
    KCTLX | KFIRST | 0x4B, MSG_back_char, "West",
    KCTLX | KFIRST | 0x8D, MSG_back_line, "Ctl-North",
    KCTLX | KFIRST | 0x74, MSG_forw_unit, "Ctl-East",
    KCTLX | KFIRST | 0x91, MSG_forw_line, "Ctl-South",
    KCTLX | KFIRST | 0x73, MSG_back_unit, "Ctl-West",
    KCTLX | KFIRST | 0x98, NULL, "Alt-North",
    KCTLX | KFIRST | 0x9D, NULL, "Alt-East",
    KCTLX | KFIRST | 0xA0, NULL, "Alt-South",
    KCTLX | KFIRST | 0x9B, NULL, "Alt-West",
    KCTLX | KFIRST | 0x49, MSG_back_page, "PageDown",
    KCTLX | KFIRST | 0x51, MSG_forw_page, "PageUp",
    KCTLX | KFIRST | 0x47, MSG_goto_bob, "Home",
    KCTLX | KFIRST | 0x4F, MSG_goto_eob, "End",
    KCTLX | KFIRST | 0x4C, NULL, "Center",
    KCTLX | KFIRST | 0x76, MSG_forw_window, "Ctl-PageDown",
    KCTLX | KFIRST | 0x84, MSG_back_window, "Ctl-PageUp",
    KCTLX | KFIRST | 0x77, MSG_goto_bob, "Ctl-Home",
    KCTLX | KFIRST | 0x75, MSG_goto_eob, "Ctl-End",
    KCTLX | KFIRST | 0x8F, NULL, "Ctl-Center",
    KCTLX | KFIRST | 0xA1, NULL, "Alt-PageDown",
    KCTLX | KFIRST | 0x99, NULL, "Alt-PageUp",
    KCTLX | KFIRST | 0x97, NULL, "Alt-Home",
    KCTLX | KFIRST | 0x9F, NULL, "Alt-End",
    KCTLX | KFIRST | 0x52, MSG_ins_toggle, "Insert",
    KCTLX | KFIRST | 0x53, MSG_forw_del_char, "Delete",
    KCTLX | KFIRST | 0x92, NULL, "Ctl-Insert",
    KCTLX | KFIRST | 0x93, NULL, "Ctl-Delete",
    KCTLX | KFIRST | 0xA2, NULL, "Alt-Insert",
    KCTLX | KFIRST | 0xA3, NULL, "Alt-Delete",
    KCTLX | KFIRST | 0x72, MSG_print, "Ctl-Print",
    KCTLX | KFIRST | 0x0F, NULL, "Sh-Tab",
    KCTLX | KFIRST | 0x94, NULL, "Ctl-Tab",
    KCTLX | KFIRST | 0xA5, NULL, "Alt-Tab",
    KCTLX | KFIRST | 0x95, NULL, "Ctrl-Num/",
    KCTLX | KFIRST | 0x96, NULL, "Ctrl-Num*",
    KCTLX | KFIRST | 0x8E, NULL, "Ctrl-Num-",
    KCTLX | KFIRST | 0x90, NULL, "Ctrl-Num+",
    KCTLX | KFIRST | 0xA4, NULL, "Alt-Num/",
    KCTLX | KFIRST | 0x37, NULL, "Alt-Num*",
    KCTLX | KFIRST | 0x4A, NULL, "Alt-Num-",
    KCTLX | KFIRST | 0x4E, NULL, "Alt-Num+",
    KCTLX | KFIRST | 0xA6, NULL, "Alt-NumEnter",
    KCTLX | KFIRST | 0x01, NULL, "Alt-Esc",
    KCTLX | KFIRST | 0x1C, NULL, "Alt-Enter",
    KCTLX | KFIRST | 0x0E, NULL, "Alt-Backspace",
    KCTLX | KFIRST | 0x1E, NULL, "Alt-A",
    KCTLX | KFIRST | 0x30, NULL, "Alt-B",
    KCTLX | KFIRST | 0x2E, NULL, "Alt-C",
    KCTLX | KFIRST | 0x20, NULL, "Alt-D",
    KCTLX | KFIRST | 0x12, NULL, "Alt-E",
    KCTLX | KFIRST | 0x21, NULL, "Alt-F",
    KCTLX | KFIRST | 0x22, NULL, "Alt-G",
    KCTLX | KFIRST | 0x23, NULL, "Alt-H",
    KCTLX | KFIRST | 0x17, NULL, "Alt-I",
    KCTLX | KFIRST | 0x24, NULL, "Alt-J",
    KCTLX | KFIRST | 0x25, NULL, "Alt-K",
    KCTLX | KFIRST | 0x26, NULL, "Alt-L",
    KCTLX | KFIRST | 0x32, NULL, "Alt-M",
    KCTLX | KFIRST | 0x31, NULL, "Alt-N",
    KCTLX | KFIRST | 0x18, NULL, "Alt-O",
    KCTLX | KFIRST | 0x19, NULL, "Alt-P",
    KCTLX | KFIRST | 0x10, NULL, "Alt-Q",
    KCTLX | KFIRST | 0x13, NULL, "Alt-R",
    KCTLX | KFIRST | 0x1F, NULL, "Alt-S",
    KCTLX | KFIRST | 0x14, NULL, "Alt-T",
    KCTLX | KFIRST | 0x16, NULL, "Alt-U",
    KCTLX | KFIRST | 0x2F, NULL, "Alt-V",
    KCTLX | KFIRST | 0x11, NULL, "Alt-W",
    KCTLX | KFIRST | 0x2D, NULL, "Alt-X",
    KCTLX | KFIRST | 0x15, NULL, "Alt-Y",
    KCTLX | KFIRST | 0x2C, NULL, "Alt-Z",
    KCTLX | KFIRST | 0x78, NULL, "Alt-1",
    KCTLX | KFIRST | 0x79, NULL, "Alt-2",
    KCTLX | KFIRST | 0x7A, NULL, "Alt-3",
    KCTLX | KFIRST | 0x7B, NULL, "Alt-4",
    KCTLX | KFIRST | 0x7C, NULL, "Alt-5",
    KCTLX | KFIRST | 0x7D, NULL, "Alt-6",
    KCTLX | KFIRST | 0x7E, NULL, "Alt-7",
    KCTLX | KFIRST | 0x7F, NULL, "Alt-8",
    KCTLX | KFIRST | 0x80, NULL, "Alt-9",
    KCTLX | KFIRST | 0x81, NULL, "Alt-0",
    0, NULL, NULL
};
#endif
/*
 * Read in a key, doing the low level mapping
 * of ASCII code to 12 bit code.  This level deals with
 * mapping the special keys into their spots in the C1
 * control area.  The C0 controls go right through, and
 * get remapped by "getkey".
 */
static int unkey = KRANDOM;	/* jam - for ungetkey */
void
ungetkey (k)
    int k;
{
    unkey = k;
}

int
getkbd ()
{
    register int c;

    if (unkey == KRANDOM)	/* jam */
	c = ttgetc ();
    else
    {
	c = unkey;
	unkey = KRANDOM;
    }
    if (c == SPECIAL)
    {
	c = ttgetc ();
	if ((c == 0xCD) || (c == 0xDD))	/* Backtab is meta */
	    return (METACH);
	return (c | KCTRL);
    }
    if (c == 0 || c == 0xE0)
    {
	c = ttgetc ();
	return (c | KFIRST | KCTLX);
    }
    return (c);
}

/*
 * Terminal specific keymap initialization.
 * Attach the special keys to the appropriate built
 * in functions.
 * As is the case of all the keymap routines, errors
 * are very fatal.
 */
void
ttykeymapinit ()
{
#ifdef MSDOS
    KEY_NAME_ARRAY *ptr;
    int i;
    char buf[NCOL];
    if (wang_pc)
	ptr = wang_keys;

    if (ibm_pc)
	ptr = ibm_keys;

    if (!wang_pc && !ibm_pc)
	return;

    i = 0;
    while (ptr->key_code != 0)
    {
	if (ptr->func_name_str != NULL)
	    keydup (ptr->key_code, ptr->func_name_str);
	ptr++;
	i++;
    }
    sprintf (buf, MSG_sp_key, i);
    writ_echo (buf);
#endif
#ifdef OS2
    KEY_NAME_ARRAY *ptr;
    int i;
    char buf[NCOL];
    ptr = ibm_keys;
    i = 0;
    while (ptr->key_code != 0)
    {
	if (ptr->func_name_str != NULL)
	    keydup (ptr->key_code, ptr->func_name_str);
	ptr++;
	i++;
    }
    /* sprintf (buf, MSG_sp_key, i);
    writ_echo (buf); */
#endif
}

/*
*   Search key name array for given key code.
*   return pointer to key name.
*/
char *
keystrings (key)
    int key;
{
#ifdef MSDOS
    KEY_NAME_ARRAY *ptr;

    if (wang_pc)
	ptr = wang_keys;

    if (ibm_pc)
	ptr = ibm_keys;

    if (!wang_pc && !ibm_pc)
	return (NULL);

    while (ptr->key_code != 0)
    {
	if (key == ptr->key_code)
	{
	    return (ptr->key_name_str);
	}
	ptr++;
    }
#endif
#ifdef OS2
    KEY_NAME_ARRAY *ptr = ibm_keys;

    while (ptr->key_code != 0)
    {
	if (key == ptr->key_code)
	{
	    return (ptr->key_name_str);
	}
	ptr++;
    }
#endif
    return (NULL);
}
