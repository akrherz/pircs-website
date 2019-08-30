/******************************************************************** 
** fits2sds.h --  Header file to be included in fits2sds.c utils.c 
********************************************************************/

/* Prevent multiple inclusion of this file  */ 
#ifndef fits2sds_h
#define fits2sds_h

#include "hdf.h"

#define CARDLENGTH         80
#define BLOCKLENGTH      2880
#define MAXKEYWORDCOL       8
#define MAXDATACOL         30
#define NOTaNUMBER 0x7FFFFFFF     
/* #define TRUE  1 */
#define FALSE 0
#define NULL  0

/* file access mode  */
#define FITSREADONLY  0
#define FITSREADWRITE 1

#if defined(HP9000)
#define TRUEVAL -1
#else
#define TRUEVAL 1
#endif

typedef int boolean;

/* tokens and their classes*/

#define ILLEGAL      0      /* indicates illegal token */
#define SIMPLE       10
#define BITPIX       11
#define NAXIS        12
#define BSCALE       13
#define BZERO        14
#define DATAMAX      15
#define DATAMIN      16
#define BUNIT        17
#define COMMENT      18
#define HISTORY      19
#define EXTNAME      20
#define EXTVER       21

#define BSCALERR     22
#define BZEROERR     23
#define DATATYPE     24

#define BLANK        90
#define END         100

/* Table of keywords, together with their token values */

struct class {
       char word[8];
       int  value;
};


/* define TOKEN struct, to hold info about a single token */

typedef struct {
      int  class;         /* e.g. SIMPLE, BITPIX, NAXIS, DIMSIZE, etc. */
      int  subclass;      /* currently only 'n' from NAXISn            */
      int  int_val;       /* integer value assigned, if any            */
      float float_val;    /* float value assigned, if any              */
      char str_val[CARDLENGTH+1]; /* string value assigned, if any     */
      char comment[CARDLENGTH+1]; /* comment                           */
} TOKEN;


struct fits2sdsAttr{
    char fitsKeyword[8];
    int  sdsNumberType;
} ;


#endif
