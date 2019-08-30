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

#ifndef SDB_DTM_H
#define SDB_DTM_H
#ifdef HAVE_DTM

typedef enum
{
  mo_fail = 0, mo_succeed
} mo_status;

#include "dtm/dtm.h"

/*------------------------- Prototypes -------------------------*/
IMPORT mo_status mo_dtm_out (char *port, lvar_st *l_vars);
IMPORT mo_status mo_dtm_disconnect (void);
IMPORT mo_status mo_dtm_out_active_p (void);
IMPORT mo_status mo_dtm_poll_and_read (void);
IMPORT mo_status mo_dtm_send_image (void *data, lvar_st *l_vars);
IMPORT mo_status mo_dtm_send_palette (void *data, lvar_st *l_vars);
IMPORT mo_status mo_dtm_send_dataset (void *spanker, lvar_st *l_vars);
IMPORT int hdfDtmThang(char *filename, uint16 tag, uint16 ref, 
                       char *reference, lvar_st *l_vars);

#endif /* HAVE_DTM */
#endif /* SDB_DTM_H */
