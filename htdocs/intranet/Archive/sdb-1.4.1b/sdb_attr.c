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
static char RcsId[] = "@(#)sdb_attr.c,v 1.11 1996/04/15 17:57:53 georgev Exp";
#endif

/* sdb_attr.c,v 1.11 1996/04/15 17:57:53 georgev Exp */

/*
   get_attribute     - retrieve attribute of object as a string
   do_attributes     - process attributes for an object
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_attr.h"
#include "sdb_util.h"

/*-----------------------------------------------------------
 NAME
       get_attribute
 DESCRIPTION
       Gets the corresponding attribute from the object as
       a string.
 RETURNS
        return a string of the contents of an attribute if
        successful and NULL otherwise. 
        NOTE: we return allocated memory that should be 
        freed at some point.
-----------------------------------------------------------*/
char *
get_attribute(int32 id, 
              int32 num, 
              int32 nt, 
              int32 count,
              lvar_st *l_vars)
{
    char *tbuff = NULL;     /* buffer to return attribute */
    int32 dsize;     /* size of number type */
    int32 status;    /* flag */
    char *ret_value = NULL;

    ENTER(2,"get_attribute");
    DBUG_PRINT(3,(LOGF,"get_attribute: number type = %d \n", nt));

    /* Get size of number type */
    if ((dsize = DFKNTsize(nt)) < 1)
      {
        gateway_err(l_vars->hfp,"get_attribute: failed to get size of HDF Number type\n",0,l_vars);
        ret_value = NULL;
        goto done;
      }

    /* Allocate space for attribute */
    if ((tbuff = HDgetspace(dsize * (count + 1))) == NULL)
      {
        gateway_err(l_vars->hfp,"get_attribute: failed to allocate space \n",0,l_vars);
        ret_value = NULL;
        goto done;
      }

    /* Get attribute */
    if ((status = SDreadattr(id, num, tbuff)) == FAIL)
      {
        gateway_err(l_vars->hfp,"get_attribute: failed to read attribute \n",0,l_vars);
        ret_value = NULL;
        goto done;
      }
#if 0
    DBUG_PRINT(3,(LOGF," tbuff = %s \n", (char *)tbuff));
#endif

    /* return converted buffer i.e. binary -> ascii */
    ret_value = buffer_to_string(tbuff, nt, count);

  done:
    EXIT(2,"get_attribute");
    return ret_value;
} /* get_attribute() */

/*----------------------------------------------------------------------
 NAME
       do_attributes
 DESCRIPTION  
       If there are a lot of attributes we need to do them in a separate window
        That's what this function is for.
 RETURNS
-----------------------------------------------------------------------*/
int
do_attributes(char *fname, 
              int index, 
              lvar_st *l_vars) 
{
    char  name[MAX_NC_NAME];
    int32 nattrs;              /* Number of attributes */
    int32 nt;                  /* Number type */
    int32 dims[MAX_VAR_DIMS];  /* array for dimensions */
    int32 rank;                /* rank off data set */
    int32 status;              /* flag */
    int32 fid = FAIL;          /* file handle */
    int32 sds;                 /* data set handle */
    int32 count;
    intn j;
    int ret_value = SUCCEED;

    ENTER(2,"do_attributes");

    /* Open file for reading */
    if ((fid = SDstart(fname, DFACC_RDONLY)) == FAIL)
      {
        gateway_err(l_vars->hfp,"do_attributes: failed to start SDS interface handling on file\n",0,l_vars);
        ret_value = FAIL;
        goto done;
      }

    /* Select Data set */
    if ((sds = SDselect(fid, index)) == FAIL)
      {
        gateway_err(l_vars->hfp,"do_attributes: failed to select SDS \n",0,l_vars);
        ret_value = FAIL;
        goto done;
      }

    /* Get data set info - rank, dims, num of attributes */
    if ((status = SDgetinfo(sds, name, &rank, dims, &nt, &nattrs)) == FAIL)
      {
        gateway_err(l_vars->hfp,"do_attributes: failed to get info on  SDS \n",0,l_vars);
        ret_value = FAIL;
        goto done;
      }

    /* Lets handle the attributes */
    if(nattrs) 
      {
        fprintf(l_vars->hfp, "Dataset <B>%s</B> has the following attributes :\n", name);
        DBUG_PRINT(1,(LOGF, "do_attributes: Dataset %s has the following attributes :\n", name));

        fprintf(l_vars->hfp, "<UL>\n");

        /* For each attribute */
        for(j = 0; j < nattrs; j++) 
          {
            char *valstr = NULL; /* attribute string */

            /* Get attribute info */
            if ((status = SDattrinfo(sds, j, name, &nt, &count)) == FAIL)
              {
                gateway_err(l_vars->hfp,"do_attributes: failed to get attribute info on  SDS \n",0,l_vars);
                ret_value = FAIL;
                goto done;
              }

            /* Covert attribute info into string */
            if ((valstr = get_attribute(sds, j, nt, count,l_vars)) == NULL)
              {
                gateway_err(l_vars->hfp,"do_attributes: failed to convert attribute into a string\n",0,l_vars);
                continue;
              }
              
            fprintf(l_vars->hfp, "<LI> Attribute <i>%s</i> has the value : <pre>%s</pre>", name, valstr);
            DBUG_PRINT(1,(LOGF, "%s : %s \n", name, valstr));

            /* Free space allocated in "get_attribute" */
            if (valstr != NULL)
              {
                 HDfreespace((void *)valstr);
                 valstr = NULL;
              }
          } /* end for loop */

        fprintf(l_vars->hfp, "</UL>\n");

      } /* end if(nattrs) */
    else
       fprintf(l_vars->hfp, "Dataset <B>%s</B> doesn't have attributes.<P>\n", name);

    /* last entry */
    fprintf(l_vars->hfp, "</UL>\n");

done:

    if (fid != FAIL)
        SDend(fid);

    EXIT(2,"do_attributes");
    return ret_value;
} /* do_attributes */

