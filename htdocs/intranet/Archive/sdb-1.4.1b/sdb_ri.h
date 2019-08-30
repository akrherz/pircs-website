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

#ifndef SDB_RI_H
#define SDB_RI_H


/* rig list elements */
typedef struct trig_t {
    uint16 ref;       /* ref of image */
    int32  width;     /* width of image */
    int32  hieght;    /* height of image */
    uint16 pal_ref;   /* ref of palette if any */
    intn   have_pal:1;  /* flag */
} trig_st, *trig_stp;

/*------------------------- Prototypes -------------------------*/
IMPORT int do_rigs(char *fname,lvar_st *l_vars);
IMPORT int dump_rigs(char *fname,lvar_st *l_vars);
IMPORT void cleanup_rig(lvar_st *l_vars);
#endif /* SDB_RI_H */
