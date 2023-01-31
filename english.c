/*
*   This file contains all English language text srtings
*/
#include    "def.h"

/* in basic.c */
char MSG_mark_set[] = "Mark set";
char MSG_no_mark[] = "No mark in this window";
char MSG_go_b_n[] = "Goto byte number: ";
char MSG_bad_num[] = "ERROR: Bad number";

/* in buffer.c */
char MSG_use_b[] = "Enter name of buffer to goto: ";
char MSG_kill_b[] = "Enter name of buffer to delete: ";
char MSG_no_del_m[] = "ERROR: Can't delete 'main'";
char MSG_buf_disp[] = "ERROR: Buffer is displayed - continue";
char MSG_main[] = "main";
char MSG_l_buf_h[] = "S T    Size  Buffer           File";
char MSG_l_buf_h1[] = "- -   ------ ------           ----";
char MSG_no_chg[] = "Discard changes";
char MSG_yank_b[] = "Yank from buffer: ";
char MSG_no_buf[] = "ERROR: no such buffer";
char MSG_no_s_yank[] = "ERROR: can't yank to self!";
char MSG_buf_nam[] = "Buffer name: ";
char MSG_bad_l[] = "ERROR: Bad line!";
char MSG_pick[] = "%s: G(oto) or K(ill) S(ave) ";
char MSG_siz_chg[] = "Current buffer size may be changed";
char MSG_no_siz_chg[] = "Current buffer size is locked";
char MSG_cnt_al_b[] = "ERROR: Cannot allocate BUFFER block";
char MSG_ins_cnt[] = "Insert %s bytes, Hit Ctl-G to quit.";

/* in display.c */
char MSG_prn_to[] = "Print to: ";
char MSG_prog_name[] = "BEAV";
char MSG_disp_b_lst[] = " Buffer List ";
char MSG_file[] = "File: ";
char MSG_RO[] = " [RO]";
char MSG_WL[] = " [WL]";
char MSG_RW[] = " [RW]";
char MSG_AU[] = "[AU]";
char MSG_NOT_AU[] = "    ";
char MSG_curs_asc[] = "CURSOR=%08lX,   ASCII";
char MSG_curs_ebc[] = "CURSOR=%08lX,  EBCDIC";
char MSG_curs_hex[] = "CURSOR=%08lX,%2X   HEX";
char MSG_curs_bin[] = "CURSOR=%08lX,%2X   BIN";
char MSG_curs_dec[] = "CURSOR=%08lu,%2u   DEC";
char MSG_curs_flt[] = "CURSOR=%08lu,%2u FLOAT";
char MSG_curs_oct[] = "CURSOR=%08lo,%2o OCTAL";
char MSG_print1[] = "Writing: %s; Hit Ctl-G to quit";
char MSG_print2[] = "Wrote %s lines";
char MSG_cnt_al_v[] = "ERROR: Cannot allocate memory fot VIDEO";

/*  in ebcdic.c */
#if RUNCHK
char ERR_ebcdic[] =
"ERROR: Character %s not found in EBCDIC table\n";
#endif

/* in echo.c */
char MSG_y_n[] = "%s [y/n]? ";
char MSG_hit_key[] = "  Hit Ctl-G to continue";

/* in extend.c */
char MSG_not_now[] = "Not now";
char MSG_func[] = "Function: ";
char MSG_unk_func[] = "ERROR: Unknown function for binding";
char MSG_cmd_t_ex[] = "Command to execute: ";
char MSG_unk_ext[] = "ERROR: Unknown extended command";
char MSG_unk_rc[] = "ERROR: Unknown rc command: ";
char MSG_d_b[] =
"Display key binding for which key? (hit key now!)";
char MSG_unbd[] = "%s is unbound";
char MSG_bnd_to[] = "%s is bound to %s";
char MSG_ins_self[] = "ins-self";
char MSG_bnd_file[] = "Binding file: ";
char MSG_bld_wall[] = "Building help buffer";
char MSG_wall_head[] = "#  Function name                Key binding        Key code";
char MSG_beavrc[] = "beavrc";


/* in file.c */
char MSG_rd_file[] = "Read file: ";
char MSG_trash[] = "#tempbuf#";
char MSG_ins_file[] = "Insert file: ";
char MSG_not_fnd[] = "Not found";
char MSG_visit[] = "Visit file: ";
char MSG_view[] = "View file (read only): ";
char MSG_buf_ex[] = "ERROR: Buffer exists";
char MSG_old_buf[] = "ERROR: Old buffer";
char MSG_cnt_cr[] = "ERROR: Cannot create buffer";
char MSG_reading[] = "reading <%s>";
char MSG_read_lx[] = "Read %s bytes, Hit Ctl-G to quit.";
char MSG_no_mem_rd[] =
"ERROR: Insufficient memory, buffer set to read only";
char MSG_wr_file[] = "Write file: ";
char MSG_no_fn[] = "ERROR: No file name";
char MSG_bk_err[] = "ERROR: Backup error, save anyway";
char MSG_writing[] = "writing <%s>";
char MSG_wrot_n[] = "Wrote %s bytes, Hit Ctl-G to quit.";
char MSG_fil_nam[] = "File name: ";
#if RUNCHK
char ERR_parse_fn[] =
"ERROR: Starting address (%s) must preceede ending address (%s)";
char ERR_addr_neg[] = "ERROR: Addresses cannot be negative";
char ERR_f_size[] =
"ERROR: Cannot access past end of file. (file size = %s)";
#endif

/* in fileio.c */
char MSG_cnt_wr[] = "ERROR: Cannot open file for writing";
char MSG_wr_io_er[] = "ERROR: Write I/O error";
char MSG_rd_er[] = "ERROR: File read error";
#if UNIX
char MSG_bak[] = ".~";
#else
char MSG_bak[] = ".bak";
#endif
char MSG_backup[] = "Back-up of %s to %s";
char MSG_back_er[] = "ERROR: Back-up of %s to %s FAILED !!";
char MSG_back_of[] = "%s - Back-up of  <%s> to <%s>\n";

/* in kbd.c */
char MSG_tab[] = "Tab";
char MSG_ret[] = "Return";
char MSG_bksp[] = "Backspace";
char MSG_space[] = "Space";
char MSG_rubout[] = "Rubout";

/* in line.c */
char MSG_cnt_alloc[] = "ERROR: Cannot allocate %s bytes for a line";
char MSG_save_buf[] = "save-buffer";

/* in main.c */
char MSG_ok[] = "ok";
char MSG_no_mod[] = "ERROR: Buffer can not be modified";
char MSG_no_s_chg[] = "ERROR: Buffer size can not be changed";
char MSG_auto_fl[] = "Doing auto buffer flush";
char MSG_quit[] = "quit-no-save";
char MSG_st_mac[] = "Start macro";
char MSG_end_mac[] = "End macro";
char MSG_num_mod[] = "Number of modifcations per update: ";
char version[] = "BEAV, Ver 1.40,  March 31, 1993";

/*  in random.c */
char MSG_sh_pos[] =
"Cursor: %s, Mark: %s,  Buffer Size: %s, File Size: %s";
char MSG_sh_pos1[] =
"Cursor: %s, No Mark, Buffer Size: %s, File Size: %s";
char MSG_f_str[] = ", File: <%s>";
char MSG_lnk[] = "All windows on buffer <%s> are %s";
char MSG_unlink[] = "unlinked";
char MSG_link[] = "linked";
char MSG_bad_key[] = "ERROR: bad key = ";
char MSG_esc[] = "Esc,";
char MSG_ctl_x[] = "Ctl-X,";
char MSG_ctl[] = "Ctl-";
char MSG_fn[] = "FN,";
char MSG_w_not_empty[] = "Destination buffer must be empty and modifiable";
char MSG_procing[] = "Processing at %s, Hit Ctl-G to quit.";
char MSG_edit_float[] = "ERROR: Cannot edit floating point display.";

/* in region.c */
char MSG_sv_in_b[] = "Save in buffer: ";
char MSG_sav_slf[] = "ERROR: Can't save to self!";

/* in search.c */
char MSG_sch_str[] = " Search String";
char MSG_bsrc_str[] = "Back Search String";
char MSG_rpl_str[] = "Replace String";
char MSG_pat_fnd[] = "Pattern found at %s";
char MSG_no_srch[] = "ERROR: No last search";
char MSG_fnd_at[] =
"Found at %s, (R)eplace, (S)kip, (A)ll, (O)ne, (Q)uit.";
char MSG_no_rpl[] = "No replacements done";
char MSG_1_rpl[] = "1 replacement done";
char MSG_n_rpl[] = "%s replacements done";
char MSG_srcing[] = "Searching at %s, Hit Ctl-G to quit.";
char MSG_curs[] = "%s; Curs = %s, %s Len = %s => ";
char MSG_cmp_end[] = "Compare reached the end of a buffer";
char MSG_cmp_term[] = "Compare terminated by user";
char MSG_cmp_dif[] =
"Difference is detected at the two cursor positions";
char MSG_only_2[] =
"ERROR: There must be exactly two windows displayed to use Compare";
char MSG_cmping[] = "Comparing at %s, Hit Ctl-G to quit.";

/* in spawn.c */
char MSG_shell[] = "COMSPEC";
char MSG_def_shell[] = "/command.com";
char MSG_pmpt[] = "PROMPT=[BEAV]";
char MSG_pt[] = "PROMPT";
char MSG_pme[] = "PROMPT=";

/* in symbol.c */
char MSG_byte_shift[] = "display-byte-shift";
char MSG_back_char[] = "move-back-char";
char MSG_forw_del_char[] = "delete-forw-char";
char MSG_toggle_swap[] = "display-swap-order";
char MSG_forw_char[] = "move-forw-char";
char MSG_abort[] = "abort-cmd";
char MSG_back_del_char[] = "delete-back-char";
char MSG_refresh[] = "refresh-screen";
char MSG_forw_line[] = "move-forw-line";
char MSG_back_line[] = "move-back-line";
char MSG_quote[] = "insert-literally";
char MSG_recall[] = "recall-srch-string";
char MSG_twiddle[] = "unit-twiddle";
char MSG_forw_page[] = "move-forw-page";
char MSG_kill_region[] = "delete-mark-to-cursor";
char MSG_yank[] = "yank";
char MSG_down_window[] = "move-window-down";
char MSG_ins_toggle[] = "insert-toggle";
char MSG_display_buffers[] = "buffers-display";
char MSG_exit_flush_all[] = "quit-save-all";
char MSG_set_file_name[] = "buffer-set-file-name";
char MSG_file_insert[] = "insert-file";
char MSG_buf_size_lock[] = "buffer-size-lock";
char MSG_flush_all[] = "save-all-buffers";
char MSG_up_window[] = "move-window-up";
char MSG_file_read[] = "file-read";
char MSG_file_save[] = "file-save";
char MSG_file_visit[] = "file-visit";
char MSG_file_write[] = "file-write";
char MSG_swap_dot_and_mark[] = "swap-cursor-and-mark";
char MSG_shrink_window[] = "window-shrink";
char MSG_display_position[] = "show-position";
char MSG_start_macro[] = "macro-start";
char MSG_end_macro[] = "macro-end";
char MSG_help[] = "binding-for-key";
char MSG_only_window[] = "window-single";
char MSG_del_window[] = "window-delete";
char MSG_split_window[] = "window-split";
char MSG_scr_row[] = "screen-rows";
char MSG_use_buffer[] = "change-buffer";
char MSG_spawn_cli[] = "spawn-shell";
char MSG_execute_macro[] = "macro-execute";
char MSG_goto_line[] = "move-to-byte";
char MSG_ins_unit[] = "insert-unit";
char MSG_kill_buffer[] = "kill-buffer";
char MSG_load_bindings[] = "bindings-load";
char MSG_forw_window[] = "change-window-forw";
char MSG_back_window[] = "change-window-back";
char MSG_view_file[] = "file-view";
char MSG_enlarge_window[] = "window-enlarge";
char MSG_ascii_mode[] = "display-ascii";
char MSG_binary_mode[] = "display-binary";
char MSG_n_combine[] = "n-way-combine";
char MSG_buffer_name[] = "buffer-set-name";
char MSG_decimal_mode[] = "display-decimal";
char MSG_ebcdic_mode[] = "display-ebcdic";
char MSG_float_mode[] = "display-float";
char MSG_hex_mode[] = "display-hex";
char MSG_back_del_unit[] = "delete-back-unit";
char MSG_octal_mode[] = "display-octal";
char MSG_n_split[] = "n-way-split";
char MSG_display_version[] = "show-version";
char MSG_show_save_buf[] = "show-save-buf";
char MSG_unit_size1[] = "display-bytes";
char MSG_unit_size2[] = "display-words";
char MSG_unit_size4[] = "display-double-words";
char MSG_unit_size8[] = "display-quad-words";
char MSG_reposition_window[] = "window-reposition";
char MSG_set_mark[] = "mark-set";
char MSG_goto_eob[] = "move-to-end";
char MSG_goto_bob[] = "move-to-begining";
char MSG_next_buff[] = "change-to-next-buffer";
char MSG_prev_buff[] = "change-to-prev-buffer";
char MSG_query_replace[] = "replace";
char MSG_display_bindings[] = "help";
char MSG_auto_save[] = "auto-save";
char MSG_back_unit[] = "move-back-unit";
char MSG_compare[] = "compare";
char MSG_forw_del_unit[] = "delete-forw-unit";
char MSG_forw_unit[] = "move-forw-unit";
char MSG_link_windows[] = "window-link";
char MSG_print[] = "print-mark-to-cursor";
char MSG_back_search[] = "search-back";
char MSG_forw_search[] = "search-forw";
char MSG_back_page[] = "move-back-page";
char MSG_copy_region[] = "copy-mark-to-cursor";
char MSG_extended_command[] = "extended-command";
char MSG_search_again[] = "search-again";
char MSG_bind_to_key[] = "bind-to-key";
char MSG_file_visit_split[] = "file-visit-split";
char MSG_yank_buffer[] = "yank-buffer";
char MSG_save_region[] = "save-mark-to-cursor";
char MSG_use_buffer_split[] = "move-to-buffer-split";
char MSG_no_f_tb[] =
"ERROR: Could not find <%s> in look up table\n";

/* in ttykbd.c */
char MSG_sp_key[] = "%u special keys bound\n";

/* in window.c */
char MSG_no_splt[] = "ERROR: Cannot split a %s line window";
char MSG_cnt_al_w[] = "ERROR: Cannot allocate WINDOW block";
char MSG_one_w[] = "ERROR: Only one window";
char MSG_imp_chg[] = "ERROR: Impossible change";
char MSG_scrn_rows[] = "Number of screen rows: ";

#if RUNCHK
/* in basic.c */
char ERR_bas_1[] = "ERROR: unknown r_type in basic #1";

/* in display.c */
char ERR_disp_1[] = "ERROR: unknown r_type in display #1";
char ERR_disp_2[] = "ERROR: unknown r_type in display #2";
char ERR_disp_3[] = "ERROR: row less than zero\n";
char ERR_disp_4[] = "ERROR: row greater then window size\n";
char ERR_disp_5[] = "ERROR: unknown r_type in display #3";
char ERR_disp_6[] = "ERROR: unknown r_size in display ";

/* in line.c */
char ERR_no_alloc[] =
"ERROR: new line was allocated during read pattern\n";
char ERR_db_dalloc[] =
"ERROR: line was deallocated during read pattern\n";
char ERR_lock[] = "ERROR: This is size locked, cannot insert.";
char ERR_lock_del[] = "ERROR: This is size locked, cannot delete.";
/* in random.c */
char ERR_rnd_1[] = "ERROR: unknown r_type in random #1";
char ERR_rnd_2[] = "ERROR: unknown r_size in random #2";
char ERR_rnd_3[] = "ERROR: unknown r_type in random #3";
char ERR_rnd_4[] = "ERROR: unknown r_size in random #4";
char ERR_rnd_5[] = "ERROR: unknown r_size in random #5";
char ERR_rnd_6[] = "ERROR: unknown r_size in random #6";
char ERR_rnd_7[] = "ERROR: unknown r_size in random #7";
/* in search.c */
char ERR_rdpat[] = "ERROR: bad r_type in readpattern\n";
char ERR_mask[] = "ERROR: size of mask pattern, pat=%d, mask=%d\n";
char ERR_m_cl[] = "ERROR: in ascii mode mask byte was not cleared\n";
/* in ttyio.c */
char ERR_bd_pl[] = "ERROR: bad call to putline\n";
#endif
