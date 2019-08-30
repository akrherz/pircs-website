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

#ifndef SDB_UTIL_H
#define SDB_UTIL_H

#ifndef findMaxMin
/* finding max & min values from datasets */
#define findMaxMin(x, dataMax, dataMin) { \
    tmp=(x);  if(tmp > dataMax) dataMax=tmp; \
    if (tmp<dataMin) dataMin=tmp;}

#endif
#include <varargs.h>
/*------------------------- Prototypes -------------------------*/
IMPORT void InfoMsg(char *str);
IMPORT char *trim(char *str);
IMPORT int  mod(int a,int b);
IMPORT char *substr(char *str, int start, int stop);
IMPORT char * dispFmtDat();

IMPORT void print_strings(FILE *output, char **str);
IMPORT void gateway_err(FILE *fout, char *msg, int fatal, lvar_st *l_vars);
IMPORT int  write_html_header(FILE *fhandle, mime_content_type mc_type, 
                              lvar_st *l_vars);
IMPORT int  write_html_trailer(FILE *fhandle, mime_content_type mc_type, 
                               lvar_st *l_vars);
IMPORT int  recieve_request(FILE *shandle, char *request, lvar_st *l_vars);
IMPORT int  send_reply(FILE *shandle, char *fname, mime_content_type mc_type,
                       lvar_st *l_vars);
IMPORT int  pull_filename(char *target, lvar_st *l_vars);
IMPORT int  pull_ref (char *target, lvar_st *l_vars);
IMPORT char * get_type(int32 nt);
IMPORT char * buffer_to_string(char * tbuff, int32 nt, int32 count);
IMPORT int32 string_length(char *name);
IMPORT char * get_atype(ann_type atype);
IMPORT char *file_href(lvar_st *l_vars);
IMPORT char *obj_href(uint16 tag, uint16 ref, int sampling, lvar_st *l_vars);
#endif /* SDB_UTIL_H */
