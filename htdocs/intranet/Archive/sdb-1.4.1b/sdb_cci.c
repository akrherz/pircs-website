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
#ifdef RCSID
static char RcsId[] = "@(#)sdb_cci.c,v 1.4 1996/04/15 17:58:00 georgev Exp";
#endif

/* sdb_cci.c,v 1.4 1996/04/15 17:58:00 georgev Exp */

/*
   do_cci_connect    - Initiates connection to CCI host
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_cci.h"
#include "sdb_util.h"

/*-------------------------------------------------------------------- 
 NAME
     do_cci_conect
 DESCRIPTION
     Initiates connection to CCI host

 RETURNS
     return 0 on success, 1 or 2 on failure 
--------------------------------------------------------------------*/ 
#ifdef HAVE_CCI
int 
#ifdef __STDC__
do_cci_connect(lvar_st *l_vars)  
#else
do_cci_connect(l_vars)  
lvar_st l_vars;
#endif
{
  DBUG_PRINT(2,(LOGF,"CCI HDF: Connecting to %s at port %d\n", l_vars->cci_host,
	   l_vars->cci_port));

  l_vars->h_cci_port=(MCCIPort) MCCIConnect(l_vars->cci_host,l_vars->cci_port, NULL, NULL);
  if (!l_vars->h_cci_port)
    {
      printf("\nCCI: Error, could not connect to machine %s at port %d.\n",
	     l_vars->cci_host, l_vars->cci_port);
    }
} /* do_cci_connect() */
#endif
