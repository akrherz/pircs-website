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

#ifndef SHOW_H
#define SHOW_H

IMPORT int32 dumpvd(int32 vd, int data_only, FILE *fp, char separater[2], 
                    int flds_indices[100], int dumpallfields, lvar_st *l_vars);

#endif /* SHOW_H */
