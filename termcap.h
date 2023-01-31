/************************************************************************
 *									*
 *			Copyright (c) 1982, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released for public	*
 *	distribution for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************/

extern int tgetent(char *bp, char *name);
extern int tgetflag(char *id);
extern int tgetnum(char *id);
extern char *tgetstr(char *id, char **area);

extern char *tgoto(char *cm, int destcol, int destline);
extern void tputs(char *cp, int affcnt, int (*outc)(int));
