
#include    "def.h"

extern char ERR_ebcdic[];

/* Function definitions */

/* This table defines the translation from EBCDIC code to ASCII. */

char ebcdic_table[] =
{
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 00-07 */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 08-0F */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 10-17 */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 18-1F */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 20-27 */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 28-2F */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 30-37 */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 38-3F */
    0x20, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 40-47 */
    0x2E, 0x2E, 0x2E, 0x2E, 0x3C, 0x28, 0x2B, 0x2E,	/* 48-4F */
    0x26, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 50-57 */
    0x2E, 0x2E, 0x21, 0x24, 0x2A, 0x29, 0x3B, 0x5E,	/* 58-5F */
    0x2D, 0x2F, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 60-67 */
    0x2E, 0x2E, 0x7C, 0x2E, 0x25, 0x5F, 0x3E, 0x3F,	/* 68-6F */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x27, 0x2E, 0x2E,	/* 70-77 */
    0x2E, 0x60, 0x3A, 0x23, 0x40, 0x2C, 0x3D, 0x22,	/* 78-7F */
    0x2E, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,	/* 80-87 */
    0x68, 0x69, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 88-8F */
    0x2E, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,	/* 90-97 */
    0x71, 0x72, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* 98-9F */
    0x2E, 0x7E, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,	/* A0-A7 */
    0x79, 0x7A, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* A8-AF */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* B0-B7 */
    0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* B8-BF */
    0x7B, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,	/* C0-C7 */
    0x48, 0x49, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* C8-CF */
    0x7D, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,	/* D0-D7 */
    0x51, 0x52, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* D8-DF */
    0x5C, 0x2E, 0X53, 0x54, 0x55, 0x56, 0x57, 0x58,	/* E0-E7 */
    0x59, 0x5A, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,	/* E8-EF */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,	/* F0-F7 */
    0x38, 0x39, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E	/* F8-FF */
};

/* convert a ASCII character to an EBCDIC character */
char
to_ebcdic (ch)

    char ch;
{
    int cnt;
    char buf[NCOL], buf1[NCOL];

    for (cnt = 0; cnt < sizeof (ebcdic_table); cnt++)
    {
	if (ch == ebcdic_table[cnt])
	    return (cnt);
    }
    sprintf (buf1, ERR_ebcdic, R_BYTE_FMT (curwp));
    sprintf (buf, buf1, ch);
    writ_echo (buf);
    return (0);
}
