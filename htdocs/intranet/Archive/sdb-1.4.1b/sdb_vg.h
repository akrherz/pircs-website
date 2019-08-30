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

#ifndef SDB_VG_H
#define SDB_VG_H
#include "glist.h"

typedef struct telem_t {
  uint16  tag;          /* tag of element in Vgroup */
  uint16  ref;          /* ref of element in Vgroup */
  char    *name;        /* name of element in Vgroup */
} telem_st, *telem_stp;

typedef struct tvg_t {
  intn   nelem;                  /* number of elements in Vgroup */
  int32  vref;                   /* Vgroup ref number */
  char   *name;                  /* Vgroup name */
  char   *class;                 /* Vgroup class */
  Generic_list *elem_list;       /* element List */
  struct tvg_t  *parent;         /* parent of this Vgroup */
  intn   toplevel:1;             /* flag */
} tvg_st, *tvg_stp;

/*------------------------- Prototypes -------------------------*/
IMPORT void cleanup_vg(lvar_st *l_vars);
IMPORT int do_vgs(char *fname, lvar_st *l_vars);
IMPORT int dump_vg(char *fname, uint16 ref, lvar_st *l_vars);
IMPORT int dump_vgs(char *fname, lvar_st *l_vars);
#endif /* SDB_VG_H */
