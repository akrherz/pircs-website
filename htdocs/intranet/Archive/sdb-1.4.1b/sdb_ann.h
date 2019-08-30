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

#ifndef SDB_ANN_H
#define SDB_ANN_H

#include "glist.h"

/* annotation list elements */
typedef struct tan_t {
    intn      index;    /* index of annotation in file */
    ann_type  atype;    /* type of annotation */
    uint16    ann_ref;  /* ref of annotation */
    uint16    ann_tag;  /* tag of annotation */
    char      *ann;     /* annotation itself */
} tan_st, *tan_stp;

/*------------------------- Prototypes -------------------------*/
IMPORT int an1_cmp(void *dptr, void *args);

IMPORT Generic_list * get_data_labels(char *fname, uint16 tag, uint16 ref, FILE *fout,
                      lvar_st *l_vars);
IMPORT Generic_list * get_data_descs(char *fname, uint16 tag, uint16 ref, FILE *fout,
                      lvar_st *l_vars);
IMPORT int do_anns(char *fname, lvar_st *l_vars);

IMPORT int print_desc(char *fname, uint16 tag, uint16 ref, char *name, FILE *fout,
                      lvar_st *l_vars);

IMPORT FILE * init_ann_html(char *fname, ann_type atype, int32 nanns,
                            lvar_st *l_vars);
IMPORT int print_ann(int32 fhandle, int32 a_ref, int32 a_tag, intn ann_indx, ann_type atype, FILE *fout,
                    lvar_st *l_vars);
IMPORT int dump_anns(char *fname, lvar_st *l_vars);
IMPORT void cleanup_an(lvar_st *l_vars);
IMPORT void free_an(void *dptr, void *args);
#endif /* SDB_ANN_H */
