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

#ifndef SDB_ATTR_H
#define SDB_ATTR_H

/*------------------------- Prototypes -------------------------*/
IMPORT char *get_attribute(int32 id, int32 num, int32 nt, int32 count,
                           lvar_st *l_vars);
IMPORT int do_attributes(char *fname, int index, lvar_st *l_vars); 

#endif /* SDB_ATTR_H */
