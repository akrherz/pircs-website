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

#ifndef SDB_SDS_H
#define SDB_SDS_H

#include "glist.h"

/* Variable list elements */
typedef struct tsds_t {
    char  *name;            /* variable naem */
    int   var_type;         /* coordinate variable = 1, regular variable = 2 */
    int32 rank;             /* rank of variable */
    int32 nt;               /* number type of variabe */
    int32 dimsizes[5];      /* array of dimsizes - NOTE hard coded limit */
    int32 nattrs;           /* number of attributes */
    int   index;            /* index of variable  in file */
    int   lone:1;           /* i.e not in a Vgroup */
    int32 ndg_ref;          /* NDG ref of SDS */
    uint16 tag;             /* DFTAG_NDG except for netCDF  */
    Generic_list *tatrlist; /* attribute list for this variable */
} tsds_st, *tsds_stp;

/* attribute list elements */
typedef struct tatr_t {
    char *name;        /* name of attriubre */
    char *value_str;   /* value of attribue */
} tatr_st, *tatr_stp;

/*------------------------- Prototypes -------------------------*/
IMPORT void sd_set_lone(void *dptr, void *args);
IMPORT int ndg_cmp(void *dptr, void *args);
IMPORT int dump_sds(char *fname, int current_entry, entry entries[MAX_ENTRIES], 
                    int num_entries,lvar_st *l_vars);
IMPORT int dump_sd(char *fname, int32 sdref, lvar_st *l_vars);
IMPORT int do_sds(char *fname, lvar_st *l_vars);
IMPORT void cleanup_sds(lvar_st *l_vars);
#endif /* SDB_SDS_H */
