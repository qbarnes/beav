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
 ************************************************************************
 */

/* In order to reduce the size of this file drastically, the large
 * comments and pseudo code was removed as well as the padding stuff
 * which is not needed for OS/2. Size went from 28k down to 8k.
 */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char *fgetlr(char *bp, int bpsize, FILE *fp);
static FILE *find_file(char *bp);
static gotcha(char *bp, char *name);
static char *decode(char *bp, char **area);
static char *do_esc(char *out, char *in);
static void process(void);

#define TRUE 1
#define FALSE 0
#define BUFSIZE 1024			/* Assumed size of external buffer */

#define NO_FILE	 -1			/* Returned if can't open file */
#define NO_ENTRY  0			/* Returned if can't find entry */
#define SUCCESS   1			/* Returned if entry found ok */
#define TRUNCATED 2			/* Returned if entry found but trunc */

# ifdef DGK
# define DEFAULT_ROOT "termcap.cnf"		/* name without path component */
  FILE *fopenp();
# endif

# define DEFAULT_FILE "termcap.dat"

char *_tcpbuf;                          /* Place to remember buffer pointer */

# define index strchr


static char *fgetlr(bp,bpsize,fp)
char *bp;
int bpsize;
FILE *fp;
{
    int numch;
    char *cp;

    if (fgets(bp,bpsize,fp) == NULL) {
	return(NULL);
    } else {
	numch = strlen(bp);
	cp = &bp[numch];
	if (*--cp == '\n') {
	    if (numch > 1 && *--cp == '\\') {
		*cp++ = '\n';
		*cp = 0;
		fgetlr(cp,bpsize-numch+1,fp);
	    }
	}
	return(bp);
    }
}


int tgetent(bp,name)
char *bp;				/* Pointer to buffer (1024 char min) */
char *name;				/* Pointer to terminal entry to find */
{
    FILE *fp;

    *bp = 0;
    _tcpbuf = bp;
    if ((fp = find_file(bp)) == NULL) {
	if (*bp != 0) {
	    return(SUCCESS);
	} else {
	    return(NO_FILE);
	}
    } else {
	while (fgetlr(bp,BUFSIZE,fp)) {
	    if (gotcha(bp,name)) {
		fclose(fp);
		if (bp[strlen(bp)-1] != '\n') {
		    return(TRUNCATED);
		} else {
		    return(SUCCESS);
		}
	    }
	}
	return(NO_ENTRY);
    }
}


static FILE *find_file(bp)
char *bp;
{
    FILE *fp;
    char *cp, *ncp;

    if ((cp = getenv("TERMCAP")) != NULL) {
	if (*cp != 0) {
	    if (*cp == '/' || *cp == '\\') {
		if ((fp = fopen(cp,"r")) != NULL) {
		    return(fp);
		}
	    } else {
		if ((ncp = getenv("TERM")) != NULL) {
		    if (strcmp(cp,ncp) == 0) {
			strcpy(bp,cp);
			return((FILE *)NULL);
		    }
		}
	    }
	}
    }
  {
    char path[128];

    _searchenv(DEFAULT_FILE, "INIT", path);
    if ( path[0] == 0 )
      _searchenv(DEFAULT_FILE, "PATH", path);
    if ( path[0] == 0 )
      _searchenv(DEFAULT_FILE, "DPATH", path);

    return(fopen(path,"r"));
  }
}


static int gotcha(bp,name)
char *bp;
char *name;
{
    char *np;
 
    if (*bp == '#') {
	return(FALSE);
    } else {
	np = name;
	while (*np == *bp && *np != 0) {np++; bp++;}
	if (*np == 0 && (*bp == 0 || *bp == '|' || *bp == ':')) {
	    return(TRUE);
	} else {
	    while (*bp != 0 && *bp != ':' && *bp != '|') {bp++;}
	    if (*bp != '|') {
		return(FALSE);
	    } else {
		return(gotcha(++bp,name));
	    }
	}
    }
}


tgetflag(id)
char *id;
{
    char *bp;

    bp = _tcpbuf;
    while ((bp = index(bp,':')) != NULL) {
	bp++;
	if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
	    if (*bp == 0 || *bp++ == ':') {
		return(TRUE);
	    } else {
		return(FALSE);
	    }
	}
    }
    return(FALSE);
}


tgetnum(id)
char *id;
{
    int value, base;
    char *bp;

    bp = _tcpbuf;
    while ((bp = index(bp,':')) != NULL) {
	bp++;
	if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
	    if (*bp != 0 && *bp++ != '#') {
		return(-1);
	    } else {
		value = 0;
		if (*bp == '0') {
		    base = 8;
		} else {
		    base = 10;
		}
		while (isdigit(*bp)) {
		    value *= base;
		    value += (*bp++ - '0');
		}
		return(value);
	    }
	}
    }
    return(-1);
}


char *tgetstr(id,area)
char *id;
char **area;
{
    char *bp;
    char *decode();

    bp = _tcpbuf;
    while ((bp = index(bp,':')) != NULL) {
	bp++;
	if (*bp++ == id[0] && *bp != 0 && *bp++ == id[1]) {
	    if (*bp != 0 && *bp++ != '=') {
		return(NULL);
	    } else {
		return(decode(bp,area));
	    }
	}
    }
    return(NULL);
}


static char *decode(bp,area)
char *bp;
char **area;
{
    char *cp, *bgn;
    char *do_esc();

    cp = *area;
    while (*bp != 0 && *bp != ':') {
	switch(*bp) {
	case '\\':
	    bp = do_esc(cp++,++bp);
	    break;
	case '^':
	    *cp++ = (char) (*++bp & 037);
	    bp++;
	    break;
	default:
	    *cp++ = *bp++;
	    break;
	}
    }
    *cp++ = 0;
    bgn = *area;
    *area = cp;
    return(bgn);
}


static char *maplist = {
    "E\033b\bf\fn\nr\rt\t"
};

char *do_esc(out,in)
char *out;
char *in;
{
    int count;
    char ch;
    char *cp;

    if (*in != 0) {
	if (isdigit(*in)) {
	    ch = 0;
	    for (count = 0; count < 3 && isdigit(*in); in++) {
		 ch <<= 3;
		 ch |= (*in - '0');
	    }
	    *out++ = ch;
	} else if ((cp = index(maplist,*in)) != NULL) {
	    *out++ = *++cp;
	    in++;
	} else {
	    *out++ = *in++;
	}
    }
    return(in);
}


#define MAXARGS 2

static char *in;		/* Internal copy of input string pointer */
static char *out;		/* Pointer to output array */
static int args[MAXARGS];	/* Maximum number of args to convert */
static int pcount;		/* Count of args processed */
static char output[64];		/* Converted string */

char *tgoto(cm,destcol,destline)
char *cm;
int destcol;
int destline;
{
    if (cm == NULL) {
	return("OOPS");
    } else {
	in = cm;
	out = output;
	args[0] = destline;
	args[1] = destcol;
	pcount = 0;
	while (*in != 0) {
	    if (*in != '%') {
		*out++ = *in++;
	    } else {
		process();
	    }
	}
        *out++ = 0;
	return(output);
    }
}


static void process()
{
    int temp;

    in++;
    switch(*in++) {
    case 'd':
	sprintf(out,"%d",args[pcount++]);
	out = &output[strlen(output)];	
	break;
    case '2':
	sprintf(out,"%02d",args[pcount++]);
	out = &output[strlen(output)];
	break;
    case '3':
	sprintf(out,"%03d",args[pcount++]);
	out = &output[strlen(output)];
	break;
    case '.':
	*out++ = (char) args[pcount++];
	break;
    case '+':
	*out++ = (char) args[pcount++] + *in++;
	break;
    case '>':
	if (args[pcount] > (int) *in++) {
	    args[pcount] += *in++;
	} else {
	    in++;
	}
	break;
    case 'r':
	temp = args[pcount];
	args[pcount] = args[pcount+1];
	args[pcount+1] = temp;
	break;
    case 'i':
	args[pcount]++;
	args[pcount+1]++;
	break;
    case '%':
	*out++ = '%';
	break;
    }
}


void tputs(cp,affcnt,outc)
char *cp;
int affcnt;
int (*outc)(int);
{
    int ptime;			/* Pad time in tenths of milliseconds */

    if (cp == NULL || *cp == 0) {
	return;
    } else {
	for (ptime = 0; isdigit(*cp); cp++) {
	    ptime *= 10;
	    ptime += (*cp - '0');
	}
	ptime *= 10;
	if (*cp == '.') {
	    cp++;
	    if (isdigit(*cp)) {
		ptime += (*cp++ - '0');
	    }
	    while (isdigit(*cp)) {cp++;}
	}
	if (*cp == '*') {
	    ptime *= affcnt;
	    cp++;
	}
	while (*cp != 0) {
	    (*outc)(*cp++);
	}
    }
}
