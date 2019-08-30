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

/**********************************************************************
 * myutil.h - Debugging macros and other useful stuff...
 *
 * Usage: Generally as the debugging level gets higher the more 
 *        information gets printed out, that is of course if you
 *        set the debugging levels for each macro in ascending order
 *        for more debugging. The debuging level can be set either
 *        at compile time using the macro "-DDEBUG=<debuggin level>"
 *        added to the "CFLAGS" make variable in a makefile or
 *        on the compile line.
 *        Another way to set the debuggin level is at run time
 *        by setting the environment variable "DBUG".
 *        e.g. setenv DBUG=2.
 *
 *        NOTE: Using either of the two ways one must define the
 *              macro "DEBUG" at compile time to switch on debugging
 *              on. Defining "-DDEBUG" turns debbuging on and sets
 *              the default level to 1.
 * 
 * NOTE: In order to get the file handle and main debugging level in other 
 *       source files you need to define "DMAIN_C" before this header file 
 *       in the same source file were the "main()" routine lies. This all
 *       because we are using macros intead of real funtions....oh well
 *
 * E.g.
 * *****************************foo.c********************************
 * #define DMAIN_C
 * #include "myutil.h"
 *
 * ........
 * void foo (...)
 * {
 *  int y;
 *  ENTER(3,"pull_guts_from_hdfref");
 *  ..............
 *  DBUG_PRINT(2,(LOGF,"variable y =%d \n", y));
 *  ..............
 *  EXIT(3,"pull_guts_from_hdfref");
 * }
 * int main(int argc, char *argv[])
 * {
 *  ......
 *  DBUG_OPEN("/tmp/hdf.log","w+");
 *  ......
 *  DBUG_PRINT(1,(LOGF,"variable x =%s \n", xstring));
 *  ......
 *  DBUG_CLOSE(LOGF);
 *  exit(0);
 * }
 **********************************************************************/

#ifndef MYUTIL_H
#define MYUTIL_H

/*
 * Macros used for variable and function scoping in code.....
 */
#ifndef EXPORT
#define   EXPORT
#define   IMPORT    extern
#define   LOCAL     static
#endif

/* Some useful macros */
#ifndef MY_MAX
#define MY_MAX(a,b) ((a > b)? a : b)
#ifndef MY_MIN
#define MY_MIN(a,b) ((a < b)? a : b)
#endif
#endif

#ifdef HAVE_HDF
#include "hdf.h"
#define FREE_CLEAR(ptr) { if(ptr != NULL) HDfree(ptr); ptr = NULL; }
#define FREE_CLEAR_SET(ptr, value) { if(ptr != NULL) HDfree(ptr); ptr = value; }
#else
#define FREE_CLEAR(ptr) { if(ptr != NULL) free(ptr); ptr = NULL; }
#define FREE_CLEAR_SET(ptr, value) { if(ptr != NULL) free(ptr); ptr = value; }
#endif

#ifdef DEBUG
/* Error logging macros */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifdef DMAIN_C      /* Need to define this in the file where "main()" 
                      resides before this header file */
EXPORT  FILE  *LOGF = NULL;    /* export the file pointer */
EXPORT int   debug_level = 0;  /* export the current debugging level */

#else /* Not main.c or equivalant */

IMPORT  FILE  *LOGF;
IMPORT  int   debug_level;
#if defined HAVE_PROTOTYPES || defined __STDC__
IMPORT  char *mk_compound_str(int nstr, ...);
IMPORT  char *mk_path_name(int nstr, char seperator, ...);
IMPORT  char *base_name(char *path_name, char seperator);
IMPORT  char *path_name(char *path_name, char seperator);
IMPORT  char *path_name_destr(char *path_name, char seperator);
#else /* !HAVE_PROTOTYPES */
IMPORT  char *mk_compound_str(nstr,va_alist);
IMPORT  char *mk_path_name(nstr, seperator, va_alist);
IMPORT  char *base_name(path_name, seperator);
IMPORT  char *path_name(path_name, seperator);
IMPORT  char *path_name_destr(path_name, seperator);
#endif /* !HAVE_PROTOTYPES */

#endif /* Not main.c or equivalant */

/* Enter and exit funcitions for procedures 
   Modify to your hearts content............*/
#if 0
#if !(defined sgi) && !(defined hpux) && (defined(__LINE_) || (defined __FILE__)) 
#define ENTER(dlevel,fcn)   \
                     do { \
    fprintf(LOGF,"\n>>>>>Entering function %s at %s in file %s \n", \
                                 fcn,__LINE__, __FILE__); \
                     } while (0)

#define EXIT(dlevel,fcn)   \
                     do {  \
    fprintf(LOGF,"<<<<<Exiting function %s at %s in file %s \n\n", \
                                       fcn,__LINE__, __FILE__); \
                     } while (0)
#else 
#endif
#endif

#define ENTER(dlevel,fcn)   \
    do {  fprintf(LOGF,"\n>>>>>Entering function %s: \n", fcn); } while(0)

#define EXIT(dlevel,fcn)   \
   do {   fprintf(LOGF,"<<<<<Exiting function %s \n\n", fcn); } while(0)



/* The following two macros should usually only be used once in a 
   program preferably in "main()" */
#define DBUG_CLOSE(logf)    fclose(LOGF)
#define DBUG_OPEN(logf,pm)  \
                   do { char *dstring; \
                         debug_level = ((dstring = getenv("DBUG"))!=NULL)\
                                        ? atoi(dstring) : 0; \
                         if (debug_level == 0 && DEBUG > 1) \
                            debug_level = DEBUG; \
                         if (debug_level > 0) { \
                            if ((LOGF = fopen(logf, pm)) == NULL) \
                               { exit(-1);} \
                         } \
                     } while (0)

/* This is main debugging macro used to print error messages. */
#define DBUG_PRINT(dlevel,printargs)  \
                   do { \
                        if (dlevel <= debug_level && LOGF != NULL){ \
                          fprintf printargs ; \
                          fflush(LOGF); \
                        } \
                      } while (0)
#else /* Not DEBUG */
#define ENTER(dlevel,fcn)     
#define EXIT(dlevel,fcn)   
#define DBUG_OPEN(logf,pm)  
#define DBUG_CLOSE(logf)    
#define DBUG_PRINT(dlevel,printargs)  
#endif /* Not DEBUG */

#endif /* MYUTIL_H */
