/*
* The module devines the format of the screen display.
*/

#include	"def.h"


extern char hex_str[];
extern char hex_l_str[];
extern char octal_str[];
extern char octal_l_str[];
extern char decimal_str[];
extern char decimal_l_str[];
#if	FLOAT_DISP
extern char float_str[];
#endif
extern char char_str[];


/* These structures contain the format for the displayed line */


#define FC 13
#define FS 0

uchar ascii_s_posn[] =
{
    FS + 0, FS + 1, FS + 2, FS + 3, FS + 4, FS + 5, FS + 6, FS + 7, FS + 8, FS + 9,
    FS + 10, FS + 11, FS + 12, FS + 13, FS + 14, FS + 15, FS + 16, FS + 17, FS + 18, FS + 19,
    FS + 20, FS + 21, FS + 22, FS + 23, FS + 24, FS + 25, FS + 26, FS + 27, FS + 28, FS + 29,
    FS + 30, FS + 31, FS + 32, FS + 33, FS + 34, FS + 35, FS + 36, FS + 37, FS + 38, FS + 39,
    FS + 40, FS + 41, FS + 42, FS + 43, FS + 44, FS + 45, FS + 46, FS + 47, FS + 48, FS + 49,
    FS + 50, FS + 51, FS + 52, FS + 53, FS + 54, FS + 55, FS + 56, FS + 57, FS + 58, FS + 59,
    FS + 60, FS + 61, FS + 62, FS + 63, FS + 64, FS + 65, FS + 66, FS + 67, FS + 68, FS + 69,
    FS + 70, FS + 71, FS + 72, FS + 73, FS + 74, FS + 75, FS + 76, FS + 77, FS + 78, FS + 79,
};

ROW_FMT text_fmt =
{
    TEXT, BYTES, 128, 128, 128, 1, 1, FALSE, char_str, hex_l_str, hex_str,
    ascii_s_posn, 0};

ROW_FMT ascii_fmt =
{
    ASCII, BYTES, 64, 64, 64, 1, 1, FALSE, char_str, hex_l_str, hex_str,
    &ascii_s_posn[FC], 0};

ROW_FMT ascii_s_fmt =
{
    ASCII, BYTES, 32, 32, 1, 1, 1, FALSE, char_str, hex_l_str, hex_str,
    ascii_s_posn, 0};

ROW_FMT ebcdic_fmt =
{
    EBCDIC, BYTES, 64, 64, 64, 1, 1, FALSE, char_str, hex_l_str, hex_str,
    &ascii_s_posn[FC], 0};

ROW_FMT ebcdic_s_fmt =
{
    EBCDIC, BYTES, 32, 32, 1, 1, 1, FALSE, char_str, hex_l_str, hex_str,
    ascii_s_posn, 0};

uchar octal_8_posn[] =
{
    FC, FC + 4, FC + 8, FC + 12, FC + 16, FC + 20,
    FC + 24, FC + 28, FC + 33, FC + 37, FC + 41,
    FC + 45, FC + 49, FC + 53, FC + 57, FC + 61
};

ROW_FMT octal_8_fmt =
{
    OCTAL, BYTES, 16, 16, 16, 1, 3, TRUE, octal_str, octal_l_str, octal_str,
    octal_8_posn, 0};

uchar octal_s_8_posn[] =
{
    FS, FS + 4, FS + 8, FS + 12, FS + 16, FS + 20,
    FS + 24, FS + 28, FS + 32, FS + 36, FS + 40,
    FS + 44, FS + 48, FS + 52, FS + 56, FS + 60,
    FS + 64, FS + 68, FS + 72, FS + 76, FS + 80
};
ROW_FMT octal_s_8_fmt =
{
    OCTAL, BYTES, 8, 8, 1, 1, 3, TRUE, octal_str, octal_l_str, octal_str,
    octal_s_8_posn, 0};

uchar octal_16_posn[] =
{
    FC, FC + 7, FC + 14, FC + 21, FC + 29, FC + 36, FC + 43, FC + 50
};

ROW_FMT octal_16_fmt =
{
    OCTAL, WORDS, 8, 16, 16, 2, 6, TRUE, octal_str, octal_l_str, octal_str,
    octal_16_posn, 0};

uchar octal_s_16_posn[] =
{
    FS, FS + 7, FS + 14, FS + 21, FS + 28, FS + 35,
    FS + 42, FS + 49, FS + 56, FS + 63, FS + 70, FS + 77
};
ROW_FMT octal_s_16_fmt =
{
    OCTAL, WORDS, 4, 8, 2, 2, 6, TRUE, octal_str, octal_l_str, octal_str,
    octal_s_16_posn, 0};

uchar octal_32_posn[] =
{
    FC, FC + 12, FC + 25, FC + 37
};

ROW_FMT octal_32_fmt =
{
 OCTAL, DWORDS, 4, 16, 16, 4, 11, TRUE, octal_l_str, octal_l_str, octal_str,
    octal_32_posn, 0};

uchar octal_s_32_posn[] =
{
    FS, FS + 12, FS + 24, FS + 36, FS + 48, FS + 60, FS + 72
};
ROW_FMT octal_s_32_fmt =
{
    OCTAL, DWORDS, 2, 8, 4, 4, 11, TRUE, octal_l_str, octal_l_str, octal_str,
    octal_s_32_posn,};

ROW_FMT decimal_8_fmt =
{
    DECIMAL, BYTES, 16, 16, 16, 1, 3, TRUE, decimal_str, decimal_l_str, decimal_str,
    octal_8_posn, 0};

ROW_FMT decimal_s_8_fmt =
{
DECIMAL, BYTES, 8, 8, 1, 1, 3, TRUE, decimal_str, decimal_l_str, decimal_str,
    octal_s_8_posn, 0};

uchar decimal_16_posn[] =
{
    FC, FC + 6, FC + 12, FC + 18, FC + 25,
    FC + 31, FC + 37, FC + 43
};

ROW_FMT decimal_16_fmt =
{
    DECIMAL, WORDS, 8, 16, 16, 2, 5, TRUE, decimal_str, decimal_l_str, decimal_str,
    decimal_16_posn, 0};

uchar decimal_s_16_posn[] =
{
    FS, FS + 6, FS + 12, FS + 18, FS + 24,
    FS + 30, FS + 36, FS + 42, FS + 48,
    FS + 54, FS + 60, FS + 66, FS + 72, FS + 78
};
ROW_FMT decimal_s_16_fmt =
{
DECIMAL, WORDS, 4, 8, 2, 2, 5, TRUE, decimal_str, decimal_l_str, decimal_str,
    decimal_s_16_posn, 0};

uchar decimal_32_posn[] =
{
    FC, FC + 11, FC + 23, FC + 34
};

ROW_FMT decimal_32_fmt =
{
    DECIMAL, DWORDS, 4, 16, 16, 4, 10, TRUE, decimal_l_str, decimal_l_str, decimal_str,
    decimal_32_posn, 0};

uchar decimal_s_32_posn[] =
{
    FS, FS + 11, FS + 22, FS + 33, FS + 44, FS + 55, FS + 66, FS + 77
};
ROW_FMT decimal_s_32_fmt =
{
    DECIMAL, DWORDS, 4, 16, 4, 4, 12, TRUE, decimal_l_str, decimal_l_str, decimal_str,
    decimal_s_32_posn, 0};

#if	FLOAT_DISP
uchar float_32_posn[] =
{
    FC, FC + 16, FC + 32, FC + 48
};

ROW_FMT float_32_fmt =
{
    FLOAT, DWORDS, 4, 16, 4, 4, 12, TRUE, float_str, decimal_l_str, decimal_str,
    float_32_posn, 0};

uchar float_s_32_posn[] =
{
    FS, FS + 17
};
ROW_FMT float_s_32_fmt =
{
    FLOAT, DWORDS, 4, 16, 4, 4, 14, TRUE, float_str, decimal_l_str, decimal_str,
    float_s_32_posn, 0};

uchar float_64_posn[] =
{
    FC, FC + 32
};

ROW_FMT float_64_fmt =
{
    FLOAT, DOUBLES, 2, 16, 16, 8, 24, TRUE, float_str, decimal_l_str, decimal_str,
    float_64_posn, 0};

uchar float_s_64_posn[] =
{
    FS, FS + 30
};
ROW_FMT float_s_64_fmt =
{
FLOAT, DOUBLES, 2, 16, 4, 8, 22, TRUE, float_str, decimal_l_str, decimal_str,
    float_s_64_posn, 0};
#endif

uchar hex_8_posn[] =
{
    FC, FC + 3, FC + 6, FC + 9, FC + 12, FC + 15,
    FC + 18, FC + 21, FC + 25, FC + 28, FC + 31,
    FC + 34, FC + 37, FC + 40, FC + 43, FC + 46,
    FC + 50, FC + 51, FC + 52, FC + 53, FC + 54,
    FC + 55, FC + 56, FC + 57, FC + 58, FC + 59,
    FC + 60, FC + 61, FC + 62, FC + 63, FC + 64,
    FC + 65
};

ROW_FMT hex_8_fmt =
{
    HEX, BYTES, 16, 16, 16, 1, 2, TRUE, hex_str, hex_l_str, hex_str,
    hex_8_posn, 0};


uchar hex_s_8_posn[] =
{
    FS, FS + 3, FS + 6, FS + 9, FS + 12, FS + 15,
    FS + 18, FS + 21, FS + 24, FS + 27, FS + 30,
    FS + 33, FS + 36, FS + 39, FS + 42, FS + 45,
    FS + 48, FS + 51, FS + 54, FS + 57, FS + 60,
    FS + 63, FS + 66, FS + 69, FS + 72, FS + 75,
    FS + 78, FS + 80, FS + 80, FS + 80, FS + 80,
    FS + 80
};
ROW_FMT hex_s_8_fmt =
{
    HEX, BYTES, 8, 8, 1, 1, 2, TRUE, hex_str, hex_l_str, hex_str,
    hex_s_8_posn, 0};

uchar hex_16_posn[] =
{
    FC, FC + 5, FC + 10, FC + 15, FC + 21, FC + 26, FC + 31, FC + 36
};

ROW_FMT hex_16_fmt =
{
    HEX, WORDS, 8, 16, 16, 2, 4, TRUE, hex_str, hex_l_str, hex_str,
    hex_16_posn, 0};

uchar hex_s_16_posn[] =
{
    FS, FS + 5, FS + 10, FS + 15, FS + 20, FS + 25,
    FS + 30, FS + 35, FS + 40, FS + 45, FS + 50,
    FS + 55, FS + 60, FS + 65, FS + 70, FS + 75
};
ROW_FMT hex_s_16_fmt =
{
    HEX, WORDS, 8, 16, 2, 2, 4, TRUE, hex_str, hex_l_str, hex_str,
    hex_s_16_posn, 0};

uchar hex_32_posn[] =
{
    FC, FC + 9, FC + 19, FC + 28
};

ROW_FMT hex_32_fmt =
{
    HEX, DWORDS, 4, 16, 16, 4, 8, TRUE, hex_l_str, hex_l_str, hex_str,
    hex_32_posn, 0};

uchar hex_s_32_posn[] =
{
    FS, FS + 9, FS + 18, FS + 27, FS + 36, FS + 45, FS + 54, FS + 63, FS + 72
};
ROW_FMT hex_s_32_fmt =
{
    HEX, DWORDS, 4, 16, 4, 4, 8, TRUE, hex_l_str, hex_l_str, hex_str,
    hex_s_32_posn, 0};

ROW_FMT binary_8_fmt =
{
    BINARY, BYTES, 4, 4, 4, 1, 8, FALSE, hex_str, hex_l_str, hex_str,
    hex_32_posn, 0};		/* use the hex position array */

ROW_FMT binary_s_8_fmt =
{
    BINARY, BYTES, 4, 4, 1, 1, 8, FALSE, hex_str, hex_l_str, hex_str,
    hex_s_32_posn, 0};		/* use the hex position array */

uchar binary_16_posn[] =
{
    FC, FC + 17, FC + 34, FC + 51
};

ROW_FMT binary_16_fmt =
{
    BINARY, WORDS, 4, 8, 8, 2, 16, FALSE, hex_str, hex_l_str, hex_str,
    binary_16_posn, 0};

uchar binary_s_16_posn[] =
{
    FS, FS + 17, FS + 34, FS + 51, FS + 68
};
ROW_FMT binary_s_16_fmt =
{
    BINARY, WORDS, 2, 4, 2, 2, 16, FALSE, hex_str, hex_l_str, hex_str,
    binary_s_16_posn, 0};

uchar binary_32_posn[] =
{
    FC, FC + 33
};

ROW_FMT binary_32_fmt =
{
    BINARY, DWORDS, 2, 8, 8, 4, 32, FALSE, hex_l_str, hex_l_str, hex_str,
    binary_32_posn, 0};

uchar binary_s_32_posn[] =
{
    FS, FS + 33
};
ROW_FMT binary_s_32_fmt =
{
    BINARY, DWORDS, 1, 4, 4, 4, 32, FALSE, hex_l_str, hex_l_str, hex_str,
    binary_s_32_posn, 0};

/* I must do this because C does not allow forward initialization of
	structures */
void
init_fmt ()
{
    text_fmt.r_srch_fmt = &text_fmt;
    ascii_fmt.r_srch_fmt = &ascii_s_fmt;
    ascii_s_fmt.r_srch_fmt = &ascii_fmt;
    ebcdic_fmt.r_srch_fmt = &ebcdic_s_fmt;
    ebcdic_s_fmt.r_srch_fmt = &ebcdic_fmt;
    octal_8_fmt.r_srch_fmt = &octal_s_8_fmt;
    octal_s_8_fmt.r_srch_fmt = &octal_8_fmt;
    octal_16_fmt.r_srch_fmt = &octal_s_16_fmt;
    octal_s_16_fmt.r_srch_fmt = &octal_16_fmt;
    octal_32_fmt.r_srch_fmt = &octal_s_32_fmt;
    octal_s_32_fmt.r_srch_fmt = &octal_32_fmt;
    decimal_8_fmt.r_srch_fmt = &decimal_s_8_fmt;
    decimal_s_8_fmt.r_srch_fmt = &decimal_8_fmt;
    decimal_16_fmt.r_srch_fmt = &decimal_s_16_fmt;
    decimal_s_16_fmt.r_srch_fmt = &decimal_16_fmt;
    decimal_32_fmt.r_srch_fmt = &decimal_s_32_fmt;
    decimal_s_32_fmt.r_srch_fmt = &decimal_32_fmt;
#if	FLOAT_DISP
    float_32_fmt.r_srch_fmt = &float_s_32_fmt;
    float_s_32_fmt.r_srch_fmt = &float_32_fmt;
    float_64_fmt.r_srch_fmt = &float_s_64_fmt;
    float_s_64_fmt.r_srch_fmt = &float_64_fmt;
#endif
    hex_8_fmt.r_srch_fmt = &hex_s_8_fmt;
    hex_s_8_fmt.r_srch_fmt = &hex_8_fmt;
    hex_16_fmt.r_srch_fmt = &hex_s_16_fmt;
    hex_s_16_fmt.r_srch_fmt = &hex_16_fmt;
    hex_32_fmt.r_srch_fmt = &hex_s_32_fmt;
    hex_s_32_fmt.r_srch_fmt = &hex_32_fmt;
    binary_8_fmt.r_srch_fmt = &binary_s_8_fmt;
    binary_s_8_fmt.r_srch_fmt = &binary_8_fmt;
    binary_16_fmt.r_srch_fmt = &binary_s_16_fmt;
    binary_s_16_fmt.r_srch_fmt = &binary_16_fmt;
    binary_32_fmt.r_srch_fmt = &binary_s_32_fmt;
    binary_s_32_fmt.r_srch_fmt = &binary_32_fmt;
}
