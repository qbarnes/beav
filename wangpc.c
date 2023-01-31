#include "def.h"
#if MSDOS
#include "dos.h"

typedef struct SCREENINFO
{
    unsigned char state;
    unsigned char scanoff;
    unsigned short bufseg;
    unsigned char colors;
    unsigned char row;
    unsigned char col;
    unsigned char attr;
    unsigned char auxmod;
    unsigned char auxmod2;
} SCREENINFO;

typedef struct SYSCONFIG
{
    unsigned short version;
    unsigned short memsize;
    unsigned short reserved[2];
    unsigned short screen_count;
    unsigned short screens[4];
} SYSCONFIG;

#define SENDCHAR 6
#define SENDLINE 0x0d
#define BIOS 0x88
#define GETSYSCON 1

bool wang_pc = FALSE;
bool ibm_pc = FALSE;
bool mem_map = FALSE;

void
is_wang ()
{
    union REGS inregs, outregs;
    struct SREGS segregs;
    unsigned char *memptr;
    unsigned char c;
    int i;
    static char wang_id[] =
    {
	"WANG"};
    static char ret_str[6];
    char *chr_ptr;

    chr_ptr = ret_str;
    /* test for Wang PC */
    memptr = (unsigned char *) 0xFC003FC2L;
    wang_pc = TRUE;
    for (i = 0; (i < 4 && wang_pc); i++)
    {
	if (*memptr != wang_id[i])
	    wang_pc = FALSE;
	memptr++;
    }

    if (wang_pc)
    {
	mem_map = TRUE;
	ret_str[0] = 0xFF;	/* set to known value */
	mem_map = TRUE;
	inregs.h.al = 0x02;
	inregs.h.ah = 0x44;
	inregs.x.bx = 0;
	inregs.x.cx = 1;
	inregs.x.dx = FP_OFF (chr_ptr);
	segregs.ds = FP_SEG (chr_ptr);

	int86x (0x21, &inregs, &outregs, &segregs);
	if (ret_str[0] == 0x11)
	{
	    ibm_pc = TRUE;
	    return;
	}
	ibm_pc = FALSE;
	return;
    }

    /* Must be an IBM or clone */
    memptr = (unsigned char *) 0xF000FFFEL;
    c = *memptr;
    switch (c)
    {
    case 0xFC:			/* IBM AT or clone */
    case 0xFD:			/* IBM PC Jr */
    case 0xFE:			/* IBM XT or clone */
    case 0xFF:			/* IBM PC or clone */
	mem_map = TRUE;
	ibm_pc = TRUE;
	return;
    }
}

int
getsysconfig (outregs, segregs)
    union REGS *outregs;
    struct SREGS *segregs;
{
    union REGS inregs;

    inregs.h.al = GETSYSCON;

    int86x (BIOS, &inregs, outregs, segregs);
}

char
getscreenstate ()
{
    struct SREGS segregs;
    union REGS outregs;

    struct SYSCONFIG *config;
    struct SCREENINFO *screeninfo;
    unsigned short *shortptr;
    unsigned int screen_count;

    getsysconfig (&outregs, &segregs);

    /* set pointer to force register info into a long pointer. */
    shortptr = (unsigned short *) &config;

    /* Offset is first, it comes back in BX */
    *shortptr = (unsigned short) outregs.x.bx;
    shortptr++;

    /* segment is in ES */
    *shortptr = (unsigned short) segregs.es;

    /* Now, the config pointer should be set to the config table. */
    /*  printf("Version = %04x \n",config->version);
  printf("Memsize = %04x \n",config->memsize);
  printf("Screens = %04x \n",config->screen_count);
  */
    screen_count = config->screen_count;
    while (screen_count)
    {
	shortptr = (unsigned short *) &screeninfo;
	*shortptr = (unsigned short) config->screens[screen_count - 1];
	shortptr++;
	*shortptr = (unsigned short) segregs.es;
	if (screeninfo->state & 0x80)
	    break;
	screen_count--;
    }
    return (screeninfo->state);
}

#endif
