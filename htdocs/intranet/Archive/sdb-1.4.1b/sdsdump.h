/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * COPYING file.                                                            *
 *                                                                          *
 ****************************************************************************/

#ifndef SDSDUMP_H
#define SDSDUMP_H

IMPORT int32 sdsdumpfull(int32 sds_id, int32 rank, int32 dimsizes[], int32 nt, 
                         intn indent, FILE *fp, int32 *start, int32 *stride, 
                         int32 *edge, int32 *left, lvar_st *l_vars); 

extern int32 fmtchar(VOIDP x, FILE * ofp);
extern int32 fmtuchar8(VOIDP x, FILE * ofp);
extern int32 fmtbyte(unsigned char *x, FILE * ofp);
extern int32 fmtint(VOIDP x, FILE * ofp);
extern int32 fmtshort(VOIDP x, FILE * ofp);
extern int32 fmtint8(VOIDP x, FILE * ofp);
extern int32 fmtuint8(VOIDP x, FILE * ofp);
extern int32 fmtint16(VOIDP x, FILE * ofp);
extern int32 fmtuint16(VOIDP x, FILE * ofp);
extern int32 fmtint32(VOIDP x, FILE * ofp);
extern int32 fmtuint32(VOIDP x, FILE * ofp);
extern int32 fmtfloat32(VOIDP x, FILE * ofp);
extern int32 fmtfloat64(VOIDP x, FILE * ofp);

#endif /* SDSDUMP_H */
