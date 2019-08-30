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

#ifndef SDB_CCI_H
#define SDB_CCI_H

/*------------------------- Prototypes -------------------------*/
#ifdef HAVE_CCI
IMPORT int do_cci_connect(lvar_st *l_vars);
#endif /* HAVE_CCI */
#endif /* SDB_CCI_H */
