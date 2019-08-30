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

#ifndef SDB_VD_H
#define SDB_VD_H

typedef struct tvd_t {
    int32   nelem;               /* number of elements in Vdata */
    int32   nfields;             /* number of fields in Vdata */
    int32   interlaced;          /* interlaced flag */
    uint16  ref;                 /* Vdata ref number */
    int32   size;                /* size */
    Generic_list *field_list;    /* field List */
    char    *name;               /* Vdata name */
    char    *class;              /* Vdata class */
    intn    lone:1;                /* flag */
} tvd_st, *tvd_stp;

/*------------------------- Prototypes -------------------------*/
IMPORT int print_vdata(int32 fid, int32 ref, lvar_st *l_vars);
IMPORT int do_vd(char *fname, int32 t, int32 r, lvar_st *l_vars);
IMPORT int dump_lone_vds(char *fname, lvar_st *l_vars);
IMPORT int do_lone_vds(char *fname, lvar_st *l_vars);
IMPORT void cleanup_vd(lvar_st *l_vars);
#endif /* SDB_VD_H */
