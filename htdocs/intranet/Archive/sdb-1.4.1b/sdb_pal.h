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

#ifndef SDB_PAL_H
#define SDB_PAL_H

/* palette list elements */
typedef struct tpal_t {
    uint16  ref;         /* ref of palette */
    intn    lone_pal:1;  /* flag */
    char    pal[768];    /* palette itself */
} tpal_st, *tpal_stp;

/*------------------------- Prototypes -------------------------*/
IMPORT int do_pals(char *fname, lvar_st *l_vars);
IMPORT int dump_pals(char *fname, lvar_st *l_vars);
IMPORT void set_lone_pal(void *dptr, void *args);
IMPORT void cleanup_pal(lvar_st *l_vars);
#endif /* SDB_PAL_H */
