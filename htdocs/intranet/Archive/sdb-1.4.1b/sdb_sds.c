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
static char RcsId[] = "@(#)sdb_sds.c,v 1.40 1996/05/09 20:58:11 georgev Exp";
#endif

/* sdb_sds.c,v 1.40 1996/05/09 20:58:11 georgev Exp */

/*
   LIST Functions
   --------------
     add_sds     - Add SDS to List
     add_attr    - add attriubte to SDS attibute list
     sd_set_lone - Iterator fcn to set this SDS as a Lone SDS
     attr_cmp    - Iterator fcn to compare againse Attrbutes in list given name 
     print_attr  - Debuging, Iterator to print attribute
     sd_ann2html - Iterator fcn to print annotation of SDS
     attr2html   - Iterator fcn to print attriubte of  SDS
     print_sds   - Debuging, Iterator to print SDS info
     var_cmp     - Iterator fcn to find SDS element given name of SDS
     ndg_cmp     - Iterator fcn to find SDS given NDG ref
     is_regvar   - Iterator fcn to indicate whether this is a regular variable
     is_dimvar   - Iterator fcn to indicate whether this is a dimension variable
     is_lone     - Iterator fcn to indicate whether this is a Lone SDS
     free_attr   - frees attribute element 
     free_sds    - Iterator fcn frees SDS element and its attribute list
     cleanup_sds - frees SDS list

   Regular Functions
   -----------------
     readSds  -  Get ready to convert current SDS to the image if possible
     sdsImage - convert current SDS to the image if possible
     do_sds   -  This process SDS and puts them into lists 
     dump_sd  - dump info on SDS given ref of NDG
     dump_sds - Print out the info for data sets in the file. 

   OBSOLETE
   --------
 */

/* Include our stuff */
#ifdef HAVE_DTM
#include <unistd.h>
#endif
#include "sdb.h"
#include "sdb_sds.h"
#include "sdb_attr.h"
#include "sdb_ann.h"
#include "sdb_util.h"
#include "sdb_dtm.h"
#include "sdb_img.h"
#ifdef HAVE_FITS
#include "fitsutils.h"
#endif
#include "sdsdump.h"
#include "glist.h"

/*--------------------------------------------------------------------
 NAME
      add_sds - Add SDS to List
 DESCRIPTION
      Adds the SDS to the list along with relevant info.
 RETURNS
      SUCCEED/FAIL
---------------------------------------------------------------------*/
int
add_sds(Generic_list *sds_list,  /* SDS List */
        char *name,              /* name of SDS */
        int var_type,            /* dimension/regular variable */
        int32 rank,              /* rank */
        int32 *dimsizes,         /* dimension array */
        int32 nattrs,            /* number of attributes */
        int32 nt,                /* number type */
        int index,               /* index of this SDS in file */
        int32 ndg_ref,           /* ref of NDG for this SDS */
        uint16 tag,              /* tag, should be DFTAG_NDG except for netCDF*/
        Generic_list *attr_list, /* attribute LIST */
        lvar_st *l_vars)
{
    int i;
    tsds_st *sds_elem = NULL;
    int     ret_value = SUCCEED;

    /* allocate space for SDS element */
    if ((sds_elem = (tsds_st *)HDmalloc(sizeof(tsds_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_sds: failed to allocate space for sds_elem\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* allocate space for SDS name */
    if ((sds_elem->name = (char *)HDmalloc(sizeof(char)*(HDstrlen(name)+1))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_sds: failed to allocate space for sds_elem->name\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   
    HDstrcpy(sds_elem->name, name); /* copy name over */

    /* fill in othe relevant info */
    sds_elem->rank    = rank;
    sds_elem->nt      = nt;
    sds_elem->index   = index;
    sds_elem->ndg_ref = ndg_ref;
    sds_elem->tag     = tag;
    sds_elem->lone    = 1; /* yes lone is default */

    /* we assume dimesinos are < 5 for now 
     *  you could allocat this but I'm lazy */
    for (i = 0; i < rank && rank < 5; i++)
        sds_elem->dimsizes[i] = dimsizes[i];

    sds_elem->var_type = var_type;
    sds_elem->nattrs   = nattrs;
    sds_elem->tatrlist = attr_list;

    /* add element to SDS list */
    add_to_beginning(*sds_list, sds_elem);
    DBUG_PRINT(3,(LOGF,"add_sds: adding sds,name=%s \n",
                  sds_elem->name));

  done:
    return ret_value;
}/* add_sds() */

/*--------------------------------------------------------------------
 NAME
     add_attr  - add attriubte to SDS attibute list
 DESCRIPTION
     Add attriubte to SDS attibute list
 RETURNS
     SUCCEED/FAIL
---------------------------------------------------------------------*/
int
add_attr(Generic_list *attr_list, /* attribute list of SDS */
         char *name,              /* name of attribute */
         char *value_str,         /* value of attribue */
         lvar_st *l_vars)
{
    tatr_st *attr_elem = NULL;
    int     ret_value = SUCCEED;

    /* allocate space for attribute element */
    if ((attr_elem = (tatr_st *)HDmalloc(sizeof(tatr_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_attr: failed to allocate space for attr_elem \n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* allocate space for name of attribute */
    if ((attr_elem->name = (char *)HDmalloc(sizeof(char)*(HDstrlen(name)+1))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_attr: failed to allocate space for attr_elem->name\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   
    HDstrcpy(attr_elem->name, name); /* copy name */

    /* allocate space for value of attribute */
    if ((attr_elem->value_str = (char *)HDmalloc(sizeof(char)*(HDstrlen(value_str)+1))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_attr: failed to allocate space attr_elem->value_str\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   
    HDstrcpy(attr_elem->value_str, value_str); /* copy value */

    /* add to attibute list */
    add_to_beginning(*attr_list, attr_elem);
    DBUG_PRINT(3,(LOGF,"add_attr: adding attribute,name=%s \n",
                  attr_elem->name));

  done:
    /* failure */
    if (ret_value == FAIL)
      {
          if (attr_elem != NULL)
            {
                if (attr_elem->name != NULL)
                    HDfree(attr_elem->name);
                if (attr_elem->value_str != NULL)
                    HDfree(attr_elem->value_str);

                HDfree(attr_elem);
            }
      }

    return ret_value;
} /* add_attr() */

/* ------------------------------------------------------------------------
 NAME
     sd_set_lone - Iterator fcn to set this SDS as a Lone SDS
 DESCRIPTION
      Iterator fcn to set this SDS as a Lone SDS given the NDG ref.
 RETURNS 
      Nothing
--------------------------------------------------------------------------*/
void
sd_set_lone(void *dptr, /* SDS element */
            void *args  /* NDG ref of SDS */)
{
    tsds_st *sds_elem = dptr;
    int32 *ref = args;

    if(sds_elem->ndg_ref == *ref)
        sds_elem->lone = 0;
}/* sd_set_lone */

/*--------------------------------------------------------------------
 NAME
     attr_cmp - Iterator fcn to compare againse Attrbutes in list given name 
 DESCRIPTION
     Iterator fcn to compare againse Attributes in list given name of 
     Attrbute to find.
 RETURNS
     1->SUCCEESS, 0->FAIL
---------------------------------------------------------------------*/
int
attr_cmp(void *dptr, /* Attriute  element */
         void *args  /* name of Attribute to find */)
{
    tatr_st *attr_elem = dptr;
    char *name = args;

    if(!HDstrcmp(attr_elem->name,name))
        return 1;
    else
        return 0;
} /* attr_cmp() */

/*--------------------------------------------------------------------
 NAME
     print_attr - Debuging, Iterator to print attribute
 DESCRIPTION
     Debuging, Iterator to print attribute
 RETURNS
     Nothing;
---------------------------------------------------------------------*/
void
print_attr(void *dptr, 
           void *args)
{
    tatr_st *attr_elem = dptr;

    DBUG_PRINT(3,(LOGF,"print_attr: attribute->name=%s, value=%s \n",
                  attr_elem->name,attr_elem->value_str));
} /* print_attr() */

/*--------------------------------------------------------------------
 NAME
     sd_ann2html - Iterator fcn to print annotation of SDS
 DESCRIPTION
     Iterator fcn to print annotation of SDS
 RETURNS
     Nothing
---------------------------------------------------------------------*/
void
sd_ann2html(void *dptr, /* SDS element */
            void *args)
{
    tan_st *sd_ann = dptr;
    lvar_st *l_vars = args;

    if (sd_ann != NULL)
      {
          if (sd_ann->ann != NULL)
              fprintf(l_vars->hfp," <pre> %s </pre>",sd_ann->ann);
          else
              fprintf(l_vars->hfp," <pre> -- </pre>");
      }
} /* sd_ann2html() */

/*--------------------------------------------------------------------
 NAME
     attr2html - Iterator fcn to print attriubte of  SDS
 DESCRIPTION
     Iterator fcn to print attriubte of  SDS
 RETURNS
     Nothing
---------------------------------------------------------------------*/
void
attr2html(void *dptr, /* attribute element */
          void *args)
{
    tatr_st *attr_elem = dptr;
    lvar_st *lvars = args;

    fprintf(lvars->hfp, "<TR ALIGN=\"left\" VALIGN=\"top\"><TD> %s </TD> <TD> <pre>%s</pre> </TD></TR> \n", 
            attr_elem->name, attr_elem->value_str);
} /* attr2html() */

/*--------------------------------------------------------------------
 NAME
     print_sds - Debuging, Iterator to print SDS info
 DESCRIPTION
     Debuging, Iterator to print SDS info
 RETURNS
     Nothing
---------------------------------------------------------------------*/
void
print_sds(void *dptr, /* SDS elment */
          void *args)
{
    int i;
    tsds_st *sds_elem = dptr;

    DBUG_PRINT(3,(LOGF,"print_sds: sds->name=%s, rank=%d, var_type=%d ",
                  sds_elem->name,sds_elem->rank, sds_elem->var_type));
    DBUG_PRINT(3,(LOGF,", nattrs=%d ",
                  sds_elem->nattrs));
    DBUG_PRINT(3,(LOGF,"dimsizes[ "));
    for(i = 0; i < sds_elem->rank; i++) 
      {
          if (i == 0)
              DBUG_PRINT(3,(LOGF, "%d", sds_elem->dimsizes[i]));
          else
              DBUG_PRINT(3,(LOGF, ", %d", sds_elem->dimsizes[i]));
      } /* for loop */
    DBUG_PRINT(3,(LOGF,"]\n"));

    if(sds_elem->tatrlist != NULL)
      {
          perform_on_list(*sds_elem->tatrlist,print_attr,NULL);
      }
} /* print_sds() */

/*--------------------------------------------------------------------
 NAME
     var_cmp - Iterator fcn to find SDS element given name of SDS
 DESCRIPTION
     Iterator to find SDS element given name of SDS
 RETURNS
     1->SUCCESS, 0->FAIL
---------------------------------------------------------------------*/
int
var_cmp(void *dptr, /* SDS element */
        void *args  /* name of SDS to find */)
{
    tsds_st *sds_elem = dptr;
    char    *name     = args;

    if(!HDstrcmp(sds_elem->name,name))
        return 1;
    else
        return 0;
} /* var_cmp() */

/*--------------------------------------------------------------------
 NAME
      ndg_cmp  - Iterator fcn to find SDS given NDG ref
 DESCRIPTION
      Iterator fcn to find SDS given NDG ref
 RETURNS
     1->SUCCESS, 0->FAIL
---------------------------------------------------------------------*/
int
ndg_cmp(void *dptr, /* SDS element */
        void *args  /* NDG ref to find */)
{
    tsds_st *sds_elem = dptr;
    int32   *ndg_ref  = args;

    if(sds_elem->ndg_ref == *ndg_ref)
        return 1;
    else
        return 0;
}/* ndg_cmp() */

/*--------------------------------------------------------------------
 NAME
      is_regvar - Iterator fcn to indicate whether this is a regular variable
 DESCRIPTION
      Iterator fcn to indicate whether this is a regular variable
 RETURNS
      1-> SUCCESS, 0->FAIL
---------------------------------------------------------------------*/
int
is_regvar(void *dptr, /* SDS element */
          void *args)
{
    tsds_st *sds_elem = dptr;

    if(!sds_elem->var_type)
        return 1;
    else
        return 0;
} /* is_reg_var() */

/*--------------------------------------------------------------------
 NAME
     is_dimvar - Iterator fcn to indicate whether this is a dimension variable
 DESCRIPTION
     Iterator fcn to indicate whether this is a dimension variable
 RETURNS
     1->SUCCESS, 0->FAIL
---------------------------------------------------------------------*/
int
is_dimvar(void *dptr, /* SDS element */
          void *args)
{
    tsds_st *sds_elem = dptr;

    if(sds_elem->var_type)
        return 1;
    else
        return 0;
} /* is_dimvar() */

/*--------------------------------------------------------------------
 NAME
     is_lone - Iterator fcn to indicate whether this is a Lone SDS
 DESCRIPTION
     Iterator fcn to indicate whether this is a Lone SDS
 RETURNS
     1->SUCCESS, 0->FAIL
---------------------------------------------------------------------*/
int
is_lone(void *dptr, /* SDS element */ 
        void *args)
{
    tsds_st *sds_elem = dptr;

    if(sds_elem->lone)
        return 1;
    else
        return 0;
} /* is_lone() */

/*--------------------------------------------------------------------
 NAME
     free_attr - frees attribute element 
 DESCRIPTION
     frees attribute element 
 RETURNS
     Nothing
---------------------------------------------------------------------*/
void
free_attr(void *dptr, /* attribute element to free */
          void *args)
{
    tatr_st *attr_elem = dptr;

    if (attr_elem != NULL)
      {
          if(attr_elem->name != NULL)
              free(attr_elem->name);       /* name */
          if(attr_elem->value_str != NULL)
              free(attr_elem->value_str);  /* value string */
          free(attr_elem); /* structure itself */
      }
} /* free_attr() */

/*--------------------------------------------------------------------
 NAME
      free_sds - Iterator fcn frees SDS element and its attribute list
 DESCRIPTION
      Iterator fcn frees SDS element and its attribute list
 RETURNS
      Nothing
---------------------------------------------------------------------*/
void
free_sds(void *dptr,  /* SDS element to free */
         void *args)
{
    tsds_st *sds_elem = dptr;
    
    if (sds_elem != NULL)
      {
          /* free allocated spce from name */
          if (sds_elem->name != NULL)
              free(sds_elem->name);
            
          /* free attribute list */
          if (sds_elem->tatrlist != NULL)
            {
                perform_on_list(*(sds_elem->tatrlist),free_attr,NULL);
                destroy_list(sds_elem->tatrlist);
                free(sds_elem->tatrlist);
            }

          /* free structure itself */
          free(sds_elem);
      }
} /* free_sds() */

/*--------------------------------------------------------------------
 NAME
     cleanup_sds - frees SDS list
 DESCRIPTION
     frees SDS list
 RETURNS
     Nothing
---------------------------------------------------------------------*/
void
cleanup_sds(lvar_st *l_vars)
{
    ENTER(2,"cleanup_sds");
    /* destroy dimension variable list first */
    if(l_vars->dimvar_list != NULL)
      {
          DBUG_PRINT(1,(LOGF,"cleanup_sds: destroy dimvar_list=%d\n", 
                        num_of_objects(*(l_vars->dimvar_list))));
          destroy_list(l_vars->dimvar_list);
          free(l_vars->dimvar_list);
          l_vars->dimvar_list = NULL;
      }

    /* next the regular variable list */
    if(l_vars->regvar_list != NULL)
      {
          DBUG_PRINT(1,(LOGF,"cleanup_sds: destroy regvar_list=%d\n", 
                        num_of_objects(*(l_vars->regvar_list))));
          destroy_list(l_vars->regvar_list);
          free(l_vars->regvar_list);
          l_vars->regvar_list = NULL;
      }

    /* now the SDS list */
    if (l_vars->sds_list != NULL)
      { /* free elements first */
          DBUG_PRINT(1,(LOGF,"cleanup_sds: destroy sds_list=%d\n", 
                        num_of_objects(*(l_vars->sds_list))));
          perform_on_list(*(l_vars->sds_list),free_sds,NULL);        
          /* destory list itself */
          destroy_list(l_vars->sds_list);

          free(l_vars->sds_list);
          l_vars->sds_list = NULL;
      }

    /* now the global attribute list */
    if (l_vars->gattr_list != NULL)
      { /* free elements first */
          DBUG_PRINT(1,(LOGF,"cleanup_sds: destroy gattr_list=%d\n", 
                        num_of_objects(*(l_vars->gattr_list))));
          perform_on_list(*(l_vars->gattr_list),free_attr,NULL);        
          /* destory list itself */
          destroy_list(l_vars->gattr_list);

          free(l_vars->gattr_list);
          l_vars->gattr_list = NULL;
      }
    EXIT(2,"cleanup_sds");
}/* cleanup_sds() */


/*------------------------------------------------------------------------ 
 NAME
       readSds -  Get ready to convert current SDS to the image if possible
 DESCRIPTION
       Get ready to convert current SDS to the image if possible
 RETURNS
       SUCCEED if conversion is finished otherwise FAIL
-------------------------------------------------------------------------*/
int
readSds(char *fileName,  /* file name */
        int32 ref,       /* NDG ref of SDS */
        lvar_st *l_vars)
{
    int32 i;       /* loop variable */
    int   width;       /* width dimension of image */
    int   height;       /* height dimesnion of image */
    int   rank;
    int   plane;
    int   ip = 0;  /* pallete flag  */
    tsds_st *sds_elem=NULL;
    FILE  *h_fp;
    int   ret_value = SUCCEED;
 
    ENTER(2,"readSds");
    /* get file identifier */
    h_fp = l_vars->hfp;
 
    /* get ready to read SDS from HDF file */
    DBUG_PRINT(1,(LOGF,"hdf_ref = %s \n",l_vars->hdf_ref ));
    DBUG_PRINT(1,(LOGF,"ref = %d \n",ref ));

    /* l_vars->sds_list hold the generic information of the current SDS */
    /* find dataset matchine hdf_ref */
    sds_elem = first_that(*(l_vars->sds_list),ndg_cmp,&ref);

    if (sds_elem == NULL) 
      {
          DBUG_PRINT(1,(LOGF, "Not found DataSet->ref %d\n", ref));
          ret_value = FAIL;
          goto done;
      }

    rank = sds_elem->rank;
    if ((rank < 2) || (rank>3))
      {
          ret_value = FAIL;
          goto done;
      }

    width = sds_elem->dimsizes[rank-1];
    height= sds_elem->dimsizes[rank-2];

    if ((width*height)<= 0)
      {
          ret_value = FAIL;
          goto done;
      }
  
    plane =1;
    for (i=0; i<(rank-2); i++)
        plane *= sds_elem->dimsizes[i];

    DBUG_PRINT(1,(LOGF,"rank = %d \n",rank ));
    DBUG_PRINT(1,(LOGF,"width = %d \n",width ));
    DBUG_PRINT(1,(LOGF,"height = %d \n",height ));
    DBUG_PRINT(1,(LOGF,"plane = %d \n",plane ));
 
    DBUG_PRINT(1,(LOGF,"do_fits = %d \n",l_vars->do_fits ));

    fprintf(h_fp, "<HR>\n");

    /* Print Image header stuff in HTML  */
    fprintf(h_fp, "<H2>Images from the current SDS(%s)</H2>\n",sds_elem->name); 

    fprintf(h_fp, "<UL>\n");

    /* Print out image info. in HTML  */

    if ((rank>2) && (plane>1)) /* using first plane of the image */
        fprintf(h_fp,"<LI> This image has <b>%d</b> planes. This <A HREF=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=%d\"><IMG SRC=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=%d\"> </A> is the plane %d of the image with dimensions %d by %d\n",
                plane, 
                l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                ref, 0,l_vars->plane,l_vars->h_env->script_name, 
                l_vars->h_env->path_info,l_vars->f_name, ref, 1,l_vars->plane, \
                l_vars->plane, width, height);

    else
        fprintf(h_fp,"<LI> This image  <A HREF=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=1\"><IMG SRC=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=1\"></A> has dimensions %d by %d\n", 
                l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                ref, 0, l_vars->h_env->script_name, 
                l_vars->h_env->path_info,l_vars->f_name, ref, 1, width, height);

    if(width > hdfImageSize || height > hdfImageSize)
        fprintf(h_fp, "  (the image has been subsampled for display)");
    fprintf(h_fp, ".  ");
            
    if(ip) 
        fprintf(h_fp, "There is also a palette associated with this image.\n");

    
    if ((rank>2)&&(plane>1))  { /* dimensions > 2)  */
		
        if (l_vars->plane<plane)
            ++l_vars->plane;
    
        /*
          fprintf(l_vars->hfp,"<LI>To see the next plane of the image, <b>click <A HREF=\"%s%s?%s!sdbplane;plane=%d\"> here </A> </b>\n", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,l_vars->plane);
  
          */

        fprintf(l_vars->hfp,"<LI>To see the next plane of the image, <b>click <A HREF=\"%s%s?%s!sdbplane;ref=%d,plane=%d\"> here </A> </b>\n", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,ref,l_vars->plane);


        fprintf(l_vars->hfp,"<LI>To preview all the planes winthin the image, <b>click <A HREF=\"%s%s?%s!sdbview;ref=%d,start=%d,end=%d\"> here </b> </A> \n", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,ref,1,plane);


        fprintf(l_vars->hfp,"<LI>To preview the group of the planes winthin the image, please enter: \n");
		
        fprintf(l_vars->hfp,"<FORM METHOD=\"POST\" \n");
        fprintf(l_vars->hfp,"ACTION=\"%s%s\">\n",
                l_vars->h_env->script_name,
                l_vars->h_env->path_info);

        fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"SDB_IMGFLAG\" VALUE=\"%d\">\n",SDSVIEW);

        fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",l_vars->f_name);
        fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"SDB_REF\" VALUE=\"%d\">\n",ref);
        fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"SDB_PLANE\"  VALUE=\"%d\">\n",plane);

        fprintf(l_vars->hfp, "<UL>");
	
        fprintf(l_vars->hfp, "<LI>Starting plane:");
        fprintf(l_vars->hfp, "<INPUT NAME=\"SDB_START\" VALUE=%d>\n", 1);
		
        fprintf(l_vars->hfp, "<LI>Ending   plane:");
        fprintf(l_vars->hfp, "<INPUT NAME=\"SDB_END\" VALUE=%d>\n", plane);
	          
        fprintf(l_vars->hfp, "</UL>");
	
        fprintf(l_vars->hfp, "To preview the planes, press the submit button:\n");
        fprintf(l_vars->hfp, "<INPUT TYPE=\"submit\" VALUE=\"Submit\">\n");	
        fprintf(l_vars->hfp, "<INPUT TYPE=\"reset\" VALUE=\"Reset\">\n");

        fprintf(l_vars->hfp, "</FORM>\n");

    }
    
    /* end of image */
    fprintf(l_vars->hfp, "</UL>\n");

  done:

    EXIT(2,"readSds()");
    return ret_value;
} /* readSds() */


/*------------------------------------------------------------------------ 
 NAME
       sdsImage - convert current SDS to the image if possible
 DESCRIPTION
       convert current SDS to the image if possible
 RETURNS
       SUCCEED if conversion is finished otherwise FAIL
-------------------------------------------------------------------------*/
int
sdsImage(int32 sdsid, 
         char *imageData , 
         int plane)
{
    /* some of sds variable */
    int32 naxis,naxes[15];
    int32 nelmt;
    int32 datatype;
    int32 numPlane; /* total number of the plane */
    int32 start[5];
    int32 stride[5];
    int32 edge[5];
    float *buffer=NULL,*buf=NULL;
    int   datasize, i;
    char  sdsName[25];
    int32 nattrs;
    /* pointer of various types */
    union {
      char      *cp;    /* Byte or char */
      short     *sp;    /* short */
      uint16    *usp;   /* unsigned short */
      int       *ip;    /* integer */
      int32     *uip;   /* unsigned int */
      float32   *fp;    /* float */
      float64   *dp;    /* double */
    } databuf;
    char *ptr;

    int  status = 0;

    ENTER(2,"sdsImage");
    
    /* read the required primary array keywords  */
    status = SDgetinfo(sdsid, sdsName, &naxis, naxes, &datatype, &nattrs);

    if (status == FAIL)
        return FAIL;

    if ((naxis <= 1) || (naxis >=4))
        return (FAIL);

    numPlane = 1;
    for (i=0; i<(naxis-2); i++)
        numPlane *= naxes[i];

    if (plane > numPlane)
        return(FAIL);
    
    /*          w          *       h    */
    nelmt =  naxes[naxis-1]*naxes[naxis-2];

    buffer =  (float *)HDmalloc(nelmt*sizeof(float));

    if (buffer == NULL)
        /* out of memory */
        return(FAIL);

    buf = buffer;

    DBUG_PRINT(1,(LOGF, "Data Number Type: %d\n", datatype));

    switch(datatype) {
   
    case DFNT_UCHAR:
    case DFNT_CHAR:   /* char */
    case DFNT_INT8:   /* signed integer */
    case DFNT_UINT8:  /* unsigned char  */
        datasize = nelmt * sizeof(char);
        break;
    case DFNT_UINT16: /* unsigned short */
    case DFNT_INT16:  /* short */
        datasize = nelmt * sizeof(int16);
        break;
    case DFNT_UINT32:
    case DFNT_INT32:  /* integer */
        datasize = nelmt * sizeof(int32);
        break;
    case DFNT_FLOAT32:  /* float */
        datasize = nelmt * sizeof(float);
        break;
    case DFNT_FLOAT64:  /* double */
        datasize = nelmt * sizeof(double);
        break;
    default: 
        DBUG_PRINT(1,(LOGF, "unsupported Data Number Type: %d\n", datatype));
        return(FAIL);
    }

    databuf.cp = ptr = (char *)HDmalloc(datasize);

    if (databuf.cp == NULL)
        /* out of memory */
        return(FAIL);

    if (naxis == 3) {  /* 3-d image */
        start[0] = (plane-1);
        start[1] = 0;
        start[2] = 0;

        stride[0] = 1;
        stride[1] = 1;
        stride[2] = 1;

        edge[0] = 1;                /* z(length) */
        edge[1] = naxes[naxis-2];   /* y(length) */
        edge[2] = naxes[naxis-1];   /* x(length) */
    }
    else {      
        start[0] = 0;
        start[1] = 0;
  
        stride[0] = 1;
        stride[1] = 1;
 
        edge[0] = naxes[naxis-2];   /* y(length) */
        edge[1] = naxes[naxis-1];   /* x(length) */
    }


    DBUG_PRINT(1,(LOGF, "plane= %d\n", plane));
 
    status = SDreaddata(sdsid, start, NULL, edge, databuf.cp);

    DBUG_PRINT(1,(LOGF, "status = %d\n", status));

    if (status == FAIL)
        return FAIL;

    switch (datatype){
 
    case DFNT_CHAR:
    case DFNT_INT8: {

        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.cp++;
        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);
        break;
    } 

    case DFNT_UCHAR:             /* byte/char */
    case DFNT_UINT8:
        /*get image data by plane */

        for (i=0; i<nelmt; i++)
            imageData[i] = databuf.cp[i];

        break;

    case DFNT_INT16: {  /* short */

        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.sp++;
        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);
        break;
    } 
    case DFNT_UINT16: {  /* unsigned short */

        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.usp++;
        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);
        break;
    }
    case DFNT_INT32: {
		
        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.ip++;

        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);

        break;
    }
    case DFNT_UINT32: {
		
        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.uip++;

        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);

        break;
    }
 
    case DFNT_FLOAT32: {
	
        for (i=0; i<nelmt; i++) 
            *buf++ = *databuf.fp++;
	
        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);

        break;
    }
 
    case DFNT_FLOAT64: { /* double */
     
        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.dp++;

        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], (uint8 *)imageData);
	
        break;
    }
    default:
        DBUG_PRINT(1,(LOGF, "Unknown data number type: %d\n", datatype));
        return FAIL;
    }
     
    HDfree(ptr); 
    HDfree(buffer);

    EXIT(2,"sdsImage");  
    return(SUCCEED);
} /* sdsImage() */

/*--------------------------------------------------------------------
 NAME
     do_sds -  This process SDS and puts them into lists 
 DESCRIPTION
     this process SDS and puts them into lists 
 RETURNS
     SUCCEED/FAIL
---------------------------------------------------------------------*/
int
do_sds(char *fname, 
       lvar_st *l_vars)
{
    int32 i, j;         /* loop variables */
    int32 fid = FAIL;   /* file handle */
    int32 sds = FAIL;   /* Data set handle */
    int32 dsets;        /* number of data sets */
    int32 ngattr;       /* number of global attributes */
    int32 ndattr;       /* number of attriubtes of data set */
    int32 status;       /* flag */
    char  name[512];    /* sds name */
    int32 nt;           /* Number type */
    int32 dimsizes[50]; /* array for dimension sizes, fixed at 50 */
    int32 rank;         /* rank of data set */
    int32 count;        /* number of attributes */
    int32 ndg_ref;      /* ref of NDG */
    uint16 sd_tag = DFTAG_NDG;      /* DFTAG_NDG except for netCDF */
    FILE *h_fp   = l_vars->hfp;
    Generic_list  *tsds_attrlist = NULL; /* pointer to attribute list */
    int           ret_value = SUCCEED;

    ENTER(2,"do_sds");
    DBUG_PRINT(1,(LOGF," l_vars->display_mode=%d\n",l_vars->display_mode)); 
    DBUG_PRINT(3,(LOGF," before l_vars->hdf_ref=%s\n",l_vars->hdf_ref)); 

    /* initialize SDS and global attribute lists */
    if(l_vars->sds_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->sds_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_sds: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->sds_list);
      }
    if(l_vars->gattr_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->gattr_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_sds: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->gattr_list);
      }

    DBUG_PRINT(1,(LOGF," fname=%s\n",fname)); 

    /* Open file for reading */
    if ((fid = SDstart(fname, DFACC_RDONLY)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_sds: opening file ",0,l_vars);
          ret_value =  FAIL;
          goto done;
      }

    /* Find number of datasets and global attributes */
    if ((status = SDfileinfo(fid, &dsets, &ngattr)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_sds: getting file info",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

    DBUG_PRINT(1,(LOGF,"do_sds: dsets=%d\n",dsets)); 
    DBUG_PRINT(1,(LOGF,"do_sds:  ngattr=%d\n",ngattr)); 

    /* Check on the number of datasets and global attriubtes we found */
    if(dsets + ngattr < 1) 
      {
          DBUG_PRINT(1,(LOGF,"do_sds:  nothing to process for sds\n")); 
          ret_value = SUCCEED;
          goto done;
      }

    /* if we have datasets process them */
    if (dsets)
      {
          /* process each dataset */
          for (i=0; i < dsets; i++) 
            {
                int dimvar;

                /* select variable */
                if ((sds=SDselect(fid, i))==FAIL)
                  {
                      gateway_err(h_fp,"do_sds: failed to select dataset \n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                /* Is is this a dimension variable */
                dimvar=(SDiscoordvar(sds))?1:0;

                if (l_vars->do_netcdf)
                  {
                      ndg_ref = i;    /* we use the index for netCDF */
                      sd_tag  = 6666; /* fake tag for netCDF */
                  }
                else
                  {
                      /* ref of NDG */
                      if ((ndg_ref=SDidtoref(sds))==FAIL)
                        {
                            gateway_err(h_fp,"do_sds: failed to get ndg ref of dataset \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                  }
                
                /* Get info about variable like name, rank, dimension sizes,
                 * number type and number of attributes */
                if((status = SDgetinfo(sds, name, &rank, dimsizes, &nt, &ndattr)) == FAIL)
                  {
                      SDendaccess(sds);
                      gateway_err(h_fp,"do_sds: getting dataset info \n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                DBUG_PRINT(1,(LOGF, "do_sds: variable(ndg=%d) %d %s rank %d \n", 
                              ndg_ref,i, name, rank));

                /* deal with variable attributes */
                if (ndattr)
                  {
                      DBUG_PRINT(1,(LOGF, "        variable has %d attributes\n",ndattr));
                      /* allocate list to hold atributes */
                      if ((tsds_attrlist = HDmalloc(sizeof(Generic_list))) == NULL)
                        {
                            SDendaccess(sds);
                            gateway_err(l_vars->hfp,"do_sds: failed to allocate space for attribute list\n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   

                      /* initialize attriube list of this SDS */
                      initialize_list(tsds_attrlist);

                      /* for each attribute of dataset */
                      for(j = 0; j < ndattr; j++) 
                        {
                            char *val_str = NULL; /* value of attribute */
                            char t_name[512];
                            int32 t_nt;
                            int32 acount;

                            /* Get attribute info */
                            if ((status = SDattrinfo(sds, j, t_name, &t_nt, &acount)) == FAIL)
                              {
                                  SDendaccess(sds);
                                  gateway_err(h_fp,"do_sds: failed to get attribute info for SDS \n",0,l_vars);
                                  ret_value = FAIL;
                                  goto done;
                              }

                            /* Convert attribue object into a string */
                            if ((val_str = get_attribute(sds, j, t_nt, acount,l_vars)) == NULL)
                              {
                                  gateway_err(h_fp,"do_sds: failed to convert attribute into a string\n",0,l_vars);
                                  continue;
                              }

                            DBUG_PRINT(1,(LOGF, "do_sds:   %s : %s \n", t_name, val_str));

                            /* add attribute to attribute list */
                            if (add_attr(tsds_attrlist,t_name,val_str,l_vars) == FAIL)
                              {
                                  SDendaccess(sds);
                                  gateway_err(h_fp,"do_sds: failed to add attribute to list \n",0,l_vars);
                                  ret_value = FAIL;
                                  goto done;
                              }

                            /* We free space allocated in "get_attribute" */
                            if(val_str != NULL)
                                HDfreespace((void *)val_str);
                        } /* for naddtr */
                  }
                else /* no attributes */
                    tsds_attrlist = NULL;

                /* add sds w/ attribute list to SDS list */
                if (add_sds(l_vars->sds_list,name,dimvar,rank,dimsizes,ndattr,nt,i,
                            ndg_ref,sd_tag,tsds_attrlist,l_vars) == FAIL)
                  {
                      SDendaccess(sds);
                      gateway_err(h_fp,"do_sds: failed to add sds to list \n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                DBUG_PRINT(1,(LOGF, "do_sds: added variable %s to list \n", name));
                /* end acces to variable */
                SDendaccess(sds);

            } /* end for dsets */
          DBUG_PRINT(1,(LOGF, "do_sds: done adding variables to list \n"));
      } /* end if dsets */

    /* process global attributes if any */
    if(ngattr)
      {
          DBUG_PRINT(1,(LOGF, "do_sds: adding global attributes to list \n"));
          /* for each global attribute */
          for(i = 0; i < ngattr; i++) 
            {
                char *valstr; /*attribute string */

                /* Get attribute info */
                if ((status = SDattrinfo(fid, i, name, &nt, &count)) == FAIL)
                  {
                      gateway_err(h_fp,"do_sds: failed to get global attributes",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                /* Covert attribute object into a string */
                if ((valstr = get_attribute(fid, i, nt, count,l_vars)) == NULL)
                  {
                      gateway_err(h_fp,"do_sds: failed to convert global attribute into a string\n",0,l_vars);
                      continue;
                  }
                DBUG_PRINT(1,(LOGF, "      %s : %s \n", name, valstr));

                /* add attribute to attribute list */
                if (add_attr(l_vars->gattr_list,name,valstr,l_vars) == FAIL)
                  {
                      gateway_err(h_fp,"do_sds: failed to add attribute to global list \n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                /* We free space allocated in "get_attribute" */
                if(valstr != NULL)
                    HDfreespace((void *)valstr);
            } /* end for loop for each global attribute */
      } /* end if ngattr */

#if 0
    /* Debugging */
    if(num_of_objects(*(l_vars->sds_list)))
        perform_on_list(*(l_vars->sds_list),print_sds,NULL);

    if(num_of_objects(*(l_vars->gattr_list)))
        perform_on_list(*(l_vars->gattr_list),print_attr,NULL);
#endif

    /* get list of dimension variables */
    if(l_vars->dimvar_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->dimvar_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_sds: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   
      }
    *(l_vars->dimvar_list) = all_such_that(*(l_vars->sds_list),is_dimvar,NULL);

    /* get list of regular variables */
    if(l_vars->regvar_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->regvar_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_sds: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   
      }
    *(l_vars->regvar_list) = all_such_that(*(l_vars->sds_list),is_regvar,NULL);

  done:
    /* failure */
    if (ret_value == FAIL)
      {
          cleanup_sds(l_vars);
      }

    /* cleanup */
    if (fid != FAIL)
        SDend(fid);

    EXIT(2,"do_sds");
    return ret_value;
} /* do_sds() */

/*--------------------------------------------------------------------
 NAME
      dump_sd - dump info on SDS given ref of NDG
 DESCRIPTION
      dump info on SDS given ref of NDG
 RETURNS
      SUCCEED/FAIL
---------------------------------------------------------------------*/
int
dump_sd(char *fname,     /* name of file */
        int32 sdref,     /* ref of ndg of SDS */
        lvar_st *l_vars)
{
    int i,j;
    int32 dsets;    /* number of data sets */
    int32 ngattr;   /* number of global attributes */
    int32 fid = -1;
    int32 sds = -1;
    int32 display_mode =1;
    int ndimvars = 0;
    int nregvars = 0;
    tsds_st      *sds_elem = NULL;
    Generic_list *sd_data_label = NULL;
    Generic_list *sd_data_desc = NULL;
    tan_st       *sd_ann = NULL;
    int    nlabels, ndescs;
    uint16 ref;
    int    dsize;
    FILE   *h_fp = l_vars->hfp;
    int    ret_value = SUCCEED;

    ENTER(2,"dump_sd");
    DBUG_PRINT(1,(LOGF,"dump_sd: l_vars->display_mode=%d\n",l_vars->display_mode)); 
    DBUG_PRINT(3,(LOGF,"dump_sd: before l_vars->hdf_ref=%s\n",l_vars->hdf_ref)); 

    /* We key of the display_mode */
    display_mode = l_vars->display_mode;
    l_vars->hdf_ref++;
    DBUG_PRINT(3,(LOGF,"dump_sds: before l_vars->hdf_ref=%s\n",l_vars->hdf_ref)); 

    /* number of SDS and global attributes */
    if (l_vars->sds_list != NULL)
        dsets = num_of_objects(*(l_vars->sds_list));
    else
        dsets = 0;
    DBUG_PRINT(1,(LOGF, "dump_sd: there are %d data sets \n", dsets));
    if (l_vars->gattr_list != NULL)
        ngattr = num_of_objects(*(l_vars->gattr_list));
    else
        ngattr = 0;
    DBUG_PRINT(1,(LOGF, "dump_sd: there are %d global attriubes \n", ngattr));

    /* count home many dimension and regualr variables there are */
    if (l_vars->dimvar_list != NULL)
        ndimvars = num_of_objects(*(l_vars->dimvar_list));
    else
        ndimvars = 0;
    DBUG_PRINT(1,(LOGF, "dump_sd: there are %d dimension variables \n", ndimvars));
    if (l_vars->regvar_list != NULL)
        nregvars = num_of_objects(*(l_vars->regvar_list));
    else
        nregvars = 0;
    DBUG_PRINT(1,(LOGF, "dump_sd: there are %d regular variables \n", nregvars));

    DBUG_PRINT(1,(LOGF," fname=%s\n",fname)); 

    /* find correct sds */
    sds_elem = first_that(*(l_vars->sds_list),ndg_cmp,&sdref);

    if (sds_elem != NULL)
      {
          DBUG_PRINT(1,(LOGF, "dump_sd: found DataSet->ndg_ref %d\n", sdref));
#ifdef HAVE_DTM
          if (l_vars->dtm_outport)
            {
                mo_dtm_out(l_vars->dtm_outport,l_vars);
                mo_dtm_poll_and_read ();
                sleep(1);
                if ((hdfDtmThang(fname,DFTAG_NDG,sdref,l_vars->hdf_ref,l_vars)) ==FAIL)
                  {
                      gateway_err(h_fp,"dump_sd: error from hdfDtmThang",0,l_vars);
                      ret_value = FAIL;
                  }
                sleep(1);
                mo_dtm_poll_and_read ();
                sleep(1);
                mo_dtm_disconnect ();
            }
          else
            {
#endif /* !HAVE_DTM */
                /* are we subsetting */
                if (!l_vars->sub_s)
                  { /* not subsetting dataset */
                      /* prepare HTML table for SDS info */
                      /* Print name and rank of dataset in HTML */
                      fprintf(h_fp, "<TABLE BORDER>\n");
                      fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH> <TH> %s </TH>", 
                              "Data Set Name", "Rank", "Dimensions");
                      fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH></TR> \n", 
                              "Number Type", "Size in bytes(binary)");
                      fprintf(h_fp, "<TR ALIGN=\"center\" VALIGN=\"center\" ><TD> <pre> %s </pre> </TD> <TD> <pre> %d </pre> </TD> <TD> <pre>", 
                              sds_elem->name, sds_elem->rank);

                      /* Print each dimension size in HTML*/
                      dsize = 1;
                      fprintf(h_fp, "[");
                      for(j = 0; j < sds_elem->rank; j++) 
                        {
                            if (j == 0)
                                fprintf(h_fp, "%d", sds_elem->dimsizes[j]);
                            else
                                fprintf(h_fp, ", %d", sds_elem->dimsizes[j]);
                            dsize = dsize * sds_elem->dimsizes[j];
                        } /* for loop */
                      fprintf(h_fp, "]");
                      fprintf(h_fp, "</pre> </TD> <TD> <pre> %s </pre> </TD>", get_type(sds_elem->nt));
               
                      DBUG_PRINT(1,(LOGF, "dump_sd:dsize=%d.\n", dsize)); 
                      DBUG_PRINT(1,(LOGF, "dump_sd: DFKNTsize(nt)=%d.\n", 
                                    DFKNTsize(sds_elem->nt | DFNT_NATIVE))); 
                      /* print number type */
                      fprintf(h_fp, "<TD> <pre> %d </pre> </TD></TR>", (DFKNTsize(sds_elem->nt)*dsize));
                      /* enf of SDS table info */
                      fprintf(h_fp, "</TABLE> <p>\n");

                      /* set ndg ref */
                      ref = (uint16)sdref;

                      /* netCDF files dont' have annotations */
                      if (!l_vars->do_netcdf)
                        {
                            /* prepare Table for labesl/descs */
                            fprintf(h_fp, "<TABLE BORDER><TR>\n");
                            fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH></TR> \n", 
                                    "Data Set Labels", "Data Set Descriptions");

                            /* get data labels and descriptions if any */
                            if ((sd_data_label = get_data_labels(fname, DFTAG_NDG, ref, h_fp,l_vars)) == NULL)
                              {
                                  gateway_err(h_fp,"dump_sd: error getting data labels",0,l_vars);
                                  ret_value = FAIL;
                                  goto done;
                              }
                            nlabels = num_of_objects(*sd_data_label);

                            DBUG_PRINT(1,(LOGF, "dump_sd: found %d labels for data,tag=%d,ref=%d \n",
                                          nlabels,DFTAG_NDG,ref));    

                            if (nlabels > 0)
                              {
                                  fprintf(h_fp,"<TR ALIGN=\"center\" VALIGN=\"center\"><TD>");
                                  perform_on_list(*sd_data_label,sd_ann2html,l_vars);
                                  fprintf(h_fp,"</TD>");
                              }
                            else
                                fprintf(h_fp,"<TR><TD> -- </TD>");

                            if ((sd_data_desc = get_data_descs(fname, DFTAG_NDG, ref, h_fp,l_vars)) == NULL)
                              {
                                  gateway_err(h_fp,"dump_sd: error getting data descs",0,l_vars);
                                  ret_value = FAIL;
                                  goto done;
                              }
                            ndescs = num_of_objects(*sd_data_desc);

                            DBUG_PRINT(1,(LOGF, "dump_sd: found %d descs for NDG,tag=%d,ref=%d \n",
                                          ndescs,DFTAG_NDG,ref));    

                            /* for descs show links instead of descs themselves */
                            if (ndescs > 0)
                              {
                                  fprintf(h_fp,"<TD>");
#if 0
                                  perform_on_list(*sd_data_desc,sd_ann2html,l_vars);
#endif
                                  fprintf(h_fp, "<UL>\n");
                                  for (j =0; j < ndescs; j++)
                                    {
                                        sd_ann = next_in_list(*sd_data_desc);
                                        if (sd_ann != NULL)
                                            fprintf(h_fp,"<LI><A HREF=\"%s\"> desc%d </A></LI>",
                                                    obj_href(sd_ann->ann_tag,sd_ann->ann_ref,sd_ann->index,l_vars),j);
                                    }
                                  fprintf(h_fp, "</UL>\n");
                                  fprintf(h_fp,"</TD>");
                              }
                            else
                                fprintf(h_fp,"<TD> -- </TD>");

                            /* end of label/descs table */
                            fprintf(h_fp, "</TR></TABLE> <p>\n");

                        } /* if not netCDF */

                      /* deal with attributes */
                      if (sds_elem->nattrs)
                        {
                            reset_to_beginning(*(sds_elem->tatrlist));
                            fprintf(h_fp, "<TABLE BORDER>\n");
                            fprintf(h_fp,"<caption align=\"top\"> Attributes</caption> \n");
                            fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH></TR> \n", 
                                    "Attribute Name", "Attribute Value");
                            perform_on_list(*(sds_elem->tatrlist),attr2html,l_vars);
                            fprintf(h_fp, "</TABLE>\n");
                        } /* end if nattrs */


                      /* ====================SDS to image===================*/
                      /* take a sds to be as am image here?  
                         fprintf(h_fp, "<HR>");
                         fprintf(h_fp, "Insert the source code for generating the image from the SDS ");
                         */      
                      readSds(fname, sdref, l_vars);
                      fprintf(h_fp, "<HR>");	      
                      /* ================================================== */

                      /* now the rest which is the FORM for subsetting 
                       *  the SDS */
                      if (sds_elem->rank > 0)
                        {
                      fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                      fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                              l_vars->h_env->path_info);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"SD_RANK\" ");
                      fprintf(h_fp, "VALUE=\"%d\">\n", sds_elem->rank);
                      fprintf(h_fp, "<PRE WIDTH=\"%d\">", DOC_WIDTH);
            
                      for (j=0; j < sds_elem->rank; j++) 
                        {
                            fprintf(h_fp, "Dimension %d: <P>\n", j);
                            fprintf(h_fp, "<UL>");
                            fprintf(h_fp, "<LI>starting location :");
                            fprintf(h_fp, "<INPUT NAME=\"SD_DIM_START\" VALUE=%d> <P>\n", 
                                    0); 
                            fprintf(h_fp, "<LI>number of units to skip:");
                            fprintf(h_fp, "<INPUT NAME=\"SD_DIM_STRIDE\" VALUE=%d> <P>\n", 
                                    1);
                            fprintf(h_fp, "<LI>ending location :");
                            fprintf(h_fp, "<INPUT NAME=\"SD_DIM_END\" VALUE=%d> <P>\n", 
                                    sds_elem->dimsizes[j]);
                            fprintf(h_fp, "</UL>");
                        }
                      fprintf(h_fp, "</PRE>");
#ifdef HAVE_DTM
                      /* DTM stuff */
                      if (l_vars->dtm_outport && ((sds_elem->rank == 2) || (sds_elem->rank == 3)))
                          fprintf(h_fp,"(To broadcast this dataset over DTM, click <A HREF=\"%s\"> here </A>)<p>",
                                  obj_href(DFTAG_NDG,ref,0,l_vars));
                      else
                        {
                            fprintf(h_fp, "<LI>DTMPORT:");
                            fprintf(h_fp, "<INPUT NAME=\"dtmport\" VALUE=%s> <P>\n",":0" );
                        }
#endif
                      fprintf(h_fp, "To see the data, press this button: \n");
                      fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Browse Data\">.\n");
                      fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"hdfref\" VALUE=\";tag=%d,ref=%d,s=%d\">\n",
                              DFTAG_NDG,sds_elem->ndg_ref,1);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" ");
                      fprintf(h_fp, "VALUE=\"%s\">\n", l_vars->f_name);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                              3);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_type\" VALUE=\"%d\">\n",
                              1);
                      fprintf(h_fp, "</FORM>");
                      fprintf(h_fp, "<em> Please make a note of the size of browse");
                      fprintf(h_fp, "data that is being requested.\n");
                      fprintf(h_fp, "An ascii dump usually can run 3-5 times ");
                      fprintf(h_fp, "larger than the binary size.\n </em>");
                      fprintf(h_fp, "<hr>\n");
                        }
                      else if (sds_elem->rank == 0 && l_vars->do_netcdf)
                        {
                            fprintf(h_fp, "This is probably a netCDF scalar variable\n");
                            fprintf(h_fp, "which we are not yet able to dump \n");
                        }
                  }
                else /* we are subsetting */
                  {
                      fprintf(h_fp, "<B>Dataset %s</B>:-<P>\n", sds_elem->name);
                      if (sds_elem->nattrs)
                        {
#if 0
                            fprintf(h_fp, "It has the following attributes :\n");
                            DBUG_PRINT(1,(LOGF, "Attributes : \n"));
#endif
                            reset_to_beginning(*(sds_elem->tatrlist));
                            fprintf(h_fp, "<TABLE BORDER>\n");
                            fprintf(h_fp,"<caption align=\"top\"> Attributes</caption> \n");
                            fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH></TR> \n", 
                                    "Attribute Name", "Attribute Value");
                            perform_on_list(*(sds_elem->tatrlist),attr2html,l_vars);
                            fprintf(h_fp, "</TABLE>\n");
                        }
                      else
                          fprintf(h_fp, "It has no attributes.<P>\n");

                      /* Code for subsetting an SDS */
                      if (sds_elem->rank > 0)
                        {
                            int32 *start = NULL;
                            int32 *stride = NULL;
                            int32 *edge = NULL;
                            int32 *left = NULL;
                            int ret, r, end;

                            /* Open file for reading */
                            if ((fid = SDstart(fname, DFACC_RDONLY)) == FAIL)
                              {
                                  gateway_err(l_vars->hfp,"dump_sds: opening file ",0,l_vars);
                                  ret_value =  FAIL;
                                  goto done;
                              }

                            /* Select dataset "i" */
                            if ((sds = SDselect(fid, sds_elem->index)) == FAIL)
                              {
                                  SDend(fid);
                                  gateway_err(h_fp,"dump_sds: selecting dataset \n",0,l_vars);
                                  ret_value =  FAIL;
                                  goto done;
                              }

                            DBUG_PRINT(1,(LOGF, "dump_sds: selected dataset \n"));
                            fprintf(h_fp, "Data :<P>\n");
	    
                            start  = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                            stride = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                            edge   = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                            left   = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                            if (start == NULL || stride == NULL || edge == NULL || left == NULL)
                              {
                                  SDendaccess(sds);
                                  SDend(fid);
                                  gateway_err(h_fp,"dump_sds: failed to allocate space \n",0,l_vars);
                                  ret_value = FAIL;
                                  goto done;
                              }

                            /* Get subsampling values along each dimension */
                            for (r = 0; r < sds_elem->rank; r++)   
                              {
                                  DBUG_PRINT(1,(LOGF, "dump_sds: sd_dim_start[%d]=%d \n",
                                                r,l_vars->sd_dim_start[r]));
                                  DBUG_PRINT(1,(LOGF, "dump_sds: sd_dim_stride[%d]=%d \n",
                                                r,l_vars->sd_dim_stride[r]));
                                  DBUG_PRINT(1,(LOGF, "dump_sds: sd_dim_end[%d]=%d \n",
                                                r,l_vars->sd_dim_end[r]));
                                  start[r] = l_vars->sd_dim_start[r];
                                  if ((stride[r] = l_vars->sd_dim_stride[r])==0)
                                      stride[r] = 1;
                                  if ((end = l_vars->sd_dim_end[r])==0)
                                      left[r] = sds_elem->dimsizes[r] - start[r];
                                  else
                                      left[r] = end - start[r];
                                  edge[r] = 1;
                              }
                            edge[sds_elem->rank-1] = left[sds_elem->rank-1];

                            /* dump subset of SDS */
                            ret = sdsdumpfull(sds, sds_elem->rank, sds_elem->dimsizes, sds_elem->nt, 16, h_fp, 
                                              start, stride, edge, left, l_vars); 
                            if (ret == FAIL)
                              {
                                  SDendaccess(sds);
                                  SDend(fid);
                                  gateway_err(h_fp,"dump_sds: failed to dump sds \n",0,l_vars);
                                  ret_value = FAIL;
                                  goto done;
                              }

                            fprintf(h_fp, "<P>\n");
                             
                            /* free up space */
                            HDfreespace(start);
                            HDfreespace(stride);
                            HDfreespace(edge);
                            HDfreespace(left);
                            DBUG_PRINT(1,(LOGF, "dump_sds: done dumping dataset \n"));

                            SDendaccess(sds);
                            SDend(fid);
                        } /* end sds_elem->rank */
                  }
#ifdef HAVE_DTM
            }
#endif
      }

  done:
    if (ret_value == FAIL)
      {
      }

    /* cleanup labels and descs for SDS */
    if (sd_data_label != NULL)
      {
          perform_on_list(*(sd_data_label),free_an,NULL);        
          destroy_list(sd_data_label);
          HDfree(sd_data_label);
          sd_data_label = NULL;
      }
    if (sd_data_desc != NULL)
      {
          perform_on_list(*(sd_data_desc),free_an,NULL);        
          destroy_list(sd_data_desc);
          HDfree(sd_data_desc);
          sd_data_desc = NULL;
      }

    EXIT(2,"dump_sd");
    return ret_value;
} /* dump_sd() */

/*--------------------------------------------------------------------
 NAME
       dump_sds - Print out the info for data sets in the file. 
 DESCRIPTION
       Print out the info for data sets in the file. 
       Still nees work on a rewrite to work better with dump_sd()
 RETURNS
       SUCCEED/FAIL
---------------------------------------------------------------------*/
int
dump_sds(char *fname,                /* file name */
         int current_entry,      
         entry entries[MAX_ENTRIES], 
         int num_entries,
         lvar_st *l_vars)
{
    int32 fid;      /* file handle */
    int32 sds;      /* Data set handle */
    int32 dsets;    /* number of data sets */
    int32 ldsets;   /* lone SDS */
    int32 ngattr;   /* number of global attributes */
    int32 i, j;     /* loop variables */
    int32 display_mode=1; /* There are three display modes:
                             (1) display the scroll boxes containing the 
                             names of scientific datasets in a file,
                             (2) display a form with three fields, namely,
                             "start", "stride", and "end",	
                             (3) display data in the dataset. */
    char *temp;
    FILE *h_fp = l_vars->hfp;
    int ndimvars = 0;
    int nldimvars = 0;
    int nregvars = 0;
    int nlregvars = 0;
    Generic_list  lone_sds;      /* lone SDS list */
    Generic_list  lone_dimvars;  /* lone dimension variable list */
    Generic_list  lone_regvars;  /* lone regular variable list */
    int  ret_value = SUCCEED;

    ENTER(2,"dump_sds");
    DBUG_PRINT(1,(LOGF,"dump_sds: l_vars->display_mode=%d\n",l_vars->display_mode)); 
    DBUG_PRINT(3,(LOGF,"dump_sds: before l_vars->hdf_ref=%s\n",l_vars->hdf_ref)); 

    /* We key of the display_mode */
    display_mode = l_vars->display_mode;
    l_vars->hdf_ref++;
    if(display_mode == 3)
      { /* adjust so that we can easily extract hdf ref */
          if ((temp = (char *)strrchr(l_vars->hdf_ref, ';')) != NULL)   
            {
                temp[0] = '\0';
                temp++;
            }
      }

    /* number of SDS and global attributes */
    if (l_vars->sds_list != NULL)
        dsets = num_of_objects(*(l_vars->sds_list));
    else
        dsets = 0;
    DBUG_PRINT(1,(LOGF, "dump_sds: there are %d data sets \n", dsets));

    /* find lone SDS, lone dimvars, lone regvars */
    if (dsets > 0)
      {
          lone_sds = all_such_that(*(l_vars->sds_list),is_lone,NULL);
          ldsets = num_of_objects(lone_sds);
          DBUG_PRINT(1,(LOGF, "dump_sds: there are %d Lone Data sets \n", ldsets));
      }
    else
        ldsets = 0;

    /* global attributes */
    if (l_vars->gattr_list != NULL)
        ngattr = num_of_objects(*(l_vars->gattr_list));
    else
        ngattr = 0;
    DBUG_PRINT(1,(LOGF, "dump_sds: there are %d global attriubes \n", ngattr));

    /* count home many dimension and regualr variables there are */
    if (l_vars->dimvar_list != NULL)
        ndimvars = num_of_objects(*(l_vars->dimvar_list));
    else
        ndimvars = 0;
    DBUG_PRINT(1,(LOGF, "dump_sds: there are %d dimension variables \n", ndimvars));

    if (ndimvars > 0)
      {
          lone_dimvars = all_such_that(*(l_vars->dimvar_list),is_lone,NULL);
          nldimvars = num_of_objects(lone_dimvars);
          DBUG_PRINT(1,(LOGF, "dump_sds: there are %d Lone dimension variables \n", nldimvars));
      }
    else
        nldimvars = 0;

    if (l_vars->regvar_list != NULL)
        nregvars = num_of_objects(*(l_vars->regvar_list));
    else
        nregvars = 0;
    DBUG_PRINT(1,(LOGF, "dump_sds: there are %d regular variables \n", nregvars));

    if (nregvars > 0)
      {
          lone_regvars = all_such_that(*(l_vars->regvar_list),is_lone,NULL);
          nlregvars = num_of_objects(lone_regvars);
          DBUG_PRINT(1,(LOGF, "dump_sds: there are Lone %d regular variables \n", nlregvars));
      }
    else
        nldimvars = 0;

    DBUG_PRINT(1,(LOGF,"dump_sds: fname=%s\n",fname)); 
    DBUG_PRINT(1,(LOGF,"dump_sds: display_mode=%d\n",display_mode)); 
    DBUG_PRINT(3,(LOGF,"dump_sds: after l_vars->hdf_ref=%s\n",l_vars->hdf_ref)); 

    /*
     * Now to process the objects and generate HTML
     */

    /* Create HTML tile header for data sets */
    if (!l_vars->do_dump)
      {
          if (display_mode==1 && (dsets || ngattr))
            {
                fprintf(l_vars->hfp, "<hr>\n");
                fprintf(l_vars->hfp, "<H2>Scientific Datasets</H2>\n");

                /* Create HTML descritpion of number of data sets and attributes */
                fprintf(l_vars->hfp, "There %s %d Lone dataset%s and %d global attribute%s in this file.<P>\n", 
                        (ldsets == 1 ? "is" :"are"),ldsets, (ldsets == 1 ? "" : "s"), 
                        ngattr, (ngattr == 1 ? "" : "s"));
                DBUG_PRINT(1,(LOGF, "dump_sds: There are %d Lone dataset%s and %d global attribute%s in this file. \n", 
                              ldsets, (ldsets == 1 ? "" : "s"), 
                              ngattr, (ngattr == 1 ? "" : "s")));
            }
      } /* !l_vars->do_dump */

    /* Lets deal with datasets first */
    if(dsets) 
      {
          if (display_mode==1 && !l_vars->do_dump)
            {
                /* This is where we deal with Lone SDS */
                if (ldsets)
                  {
                      /* lets deal with dimension variables first */
                      if (nldimvars) 
                        {
                            int selected = FALSE;
                            tsds_st *dim_elem = NULL;

                            h_fp = l_vars->hfp;

                            fprintf(l_vars->hfp, "Dimension Variables :\n");
                            fprintf(l_vars->hfp, "<FORM METHOD=\"POST\" ");
                            fprintf(l_vars->hfp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                                    l_vars->h_env->path_info);
                            fprintf(l_vars->hfp, "<SELECT NAME=\"hdfref\" SIZE=%d>\n",
                                    (dsets > 1? 5:2));

                            /* For each dimension variable  */
                            for(i = 0; i < nldimvars; i++) 
                              {
                                  dim_elem = next_in_list(lone_dimvars);
                                  if (!selected) 
                                    {
                                        fprintf(h_fp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> %s\n", 
                                                DFTAG_NDG,dim_elem->ndg_ref,0,dim_elem->name);
                                        selected = TRUE;
                                    }
                                  else
                                      fprintf(h_fp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> %s",DFTAG_NDG,dim_elem->ndg_ref,0 ,dim_elem->name);

                              } /* end for each dimension variable */

                            fprintf(l_vars->hfp, "</SELECT><P>");
                            fprintf(l_vars->hfp, "To select a particular dimension variable, ");
                            fprintf(l_vars->hfp, "press this button: ");
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"submit\" VALUE=\"Select Dataset\">. <P>\n");
#if 0
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"hdfref\" VALUE=\";tag=%d,ref=%d,s=%d\">\n",
                                    DFTAG_NDG,sds_elem->ndg_ref,0);
#endif
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" ");
                            fprintf(l_vars->hfp, "VALUE=\"%s\">\n", l_vars->f_name);
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                                    2);
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_type\" VALUE=\"%d\">\n",
                                    1);
                            fprintf(l_vars->hfp, "</FORM>\n");

                        } /* end if ndimvars */

                      /* deal with regular variables */
                      if (nlregvars) 
                        {
                            int selected = FALSE;
                            tsds_st *reg_elem = NULL;

                            h_fp = l_vars->hfp;

                            fprintf(l_vars->hfp, "Variables :\n");
                            fprintf(l_vars->hfp, "<FORM METHOD=\"POST\" ");
                            fprintf(l_vars->hfp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                                    l_vars->h_env->path_info);

                            fprintf(l_vars->hfp, "<SELECT NAME=\"hdfref\" SIZE=%d>\n",
                                    (dsets > 1? 5:2));

                            /* For each regular variable  */
                            for(i = 0; i < nlregvars; i++) 
                              {
                                  reg_elem = next_in_list(lone_regvars);
                                  if (!selected) 
                                    {
                                        fprintf(h_fp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> %s\n", 
                                                DFTAG_NDG,reg_elem->ndg_ref,0,reg_elem->name);
                                        selected = TRUE;
                                    }
                                  else
                                      fprintf(h_fp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> %s", 
                                              DFTAG_NDG,reg_elem->ndg_ref,0,reg_elem->name);

                              } /* end for each regular variable */

                            fprintf(l_vars->hfp, "</SELECT><P>");
#ifdef HAVE_DTM
                            fprintf(h_fp, "<LI>DTMPORT:");
                            fprintf(h_fp, "<INPUT NAME=\"dtmport\" VALUE=%s> <P>\n",":0" );
#endif
                            fprintf(l_vars->hfp, "To select a particular variable, ");
                            fprintf(l_vars->hfp, "press this button: ");
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"submit\" VALUE=\"Select Dataset\">. <P>\n");
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" ");
                            fprintf(l_vars->hfp, "VALUE=\"%s\">\n", l_vars->f_name);
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                                    2);
                            fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_type\" VALUE=\"%d\">\n",
                                    1);
                            fprintf(l_vars->hfp, "</FORM>\n");

                        } /* end if nregvars */
                  } /* if ldsets */

            } /* end if display_mode == 1 */
          else if (display_mode==2 && !l_vars->do_dump) 
            {
                tsds_st *sds_elem = NULL;
                int dsize;
                Generic_list *sd_data_label = NULL;
                Generic_list *sd_data_desc = NULL;
                int nlabels, ndescs;
                uint16 ref;
                tan_st *sd_ann = NULL;

                h_fp = l_vars->hfp;

                /* find dataset matchine hdf_ref */
                sds_elem = first_that(*(l_vars->sds_list),var_cmp,l_vars->hdf_ref);

                if (sds_elem != NULL)
                  {
                      DBUG_PRINT(1,(LOGF, "dump_sds: found DataSet->hdf_ref %s\n", l_vars->hdf_ref));
                      /* Print name and rank of dataset in HTML */
                      fprintf(h_fp, "<TABLE BORDER>\n");
                      fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH> <TH> %s </TH>", 
                              "Data Set Name", "Rank", "Dimensions");
                      fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH></TR> \n", 
                              "Number Type", "Size in bytes(binary)");
#if 0
                      fprintf(h_fp, "<A NAME=\"DataSet%d\"><B>%s</B></A> has rank %d with dimensions [", 
                              sds_elem->index, sds_elem->name, sds_elem->rank);
#endif
                      fprintf(h_fp, "<TR ALIGN=\"center\" VALIGN=\"center\"><TD> <pre> %s </pre> </TD> <TD> <pre> %d </pre> </TD> <TD> <pre>", 
                              sds_elem->name, sds_elem->rank);
                      /* Print each dimension size in HTML*/
                      dsize = 1;
                      if (display_mode==2 || l_vars->do_dump) 
                        {
                            fprintf(h_fp, "[");
                            for(j = 0; j < sds_elem->rank; j++) 
                              {
                                  if (j == 0)
                                      fprintf(h_fp, "%d", sds_elem->dimsizes[j]);
                                  else
                                      fprintf(h_fp, ", %d", sds_elem->dimsizes[j]);
                                  dsize = dsize * sds_elem->dimsizes[j];
                              } /* for loop */
                            fprintf(h_fp, "]");
                        }
                      fprintf(h_fp, "</pre> </TD> <TD> <pre> %s </pre> </TD>", get_type(sds_elem->nt));
               
                      DBUG_PRINT(1,(LOGF, "dump_sds:dsize=%d.\n", dsize)); 
                      DBUG_PRINT(1,(LOGF, "dump_sds: DFKNTsize(nt)=%d.\n", 
                                    DFKNTsize(sds_elem->nt | DFNT_NATIVE))); 
#if 0
                      fprintf(h_fp, ".  The dataset is composed of %s.\n", get_type(sds_elem->nt));
                      DBUG_PRINT(1,(LOGF, ".  The dataset is composed of %s.\n", get_type(sds_elem->nt))); 

                      fprintf(h_fp,"(%d bytes binary)\n",(DFKNTsize(sds_elem->nt)*dsize));
#endif
                      fprintf(h_fp, "<TD> <pre> %d </pre> </TD></TR>", (DFKNTsize(sds_elem->nt)*dsize));
                      fprintf(h_fp, "</TABLE> <p>\n");

                      /* set ndg ref */
                      ref = (uint16)sds_elem->ndg_ref;

                      /* make sure we are not netCDF */
                      if (!l_vars->do_netcdf)
                        {
                      fprintf(h_fp, "<TABLE BORDER><TR>\n");
                      fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH></TR> \n", 
                              "Data Set Labels", "Data Set Descriptions");
                      /* get data labels and descriptions if any */
                      if ((sd_data_label = get_data_labels(fname, DFTAG_NDG, ref, h_fp,l_vars)) == NULL)
                        {
                            gateway_err(h_fp,"dump_sds: error getting data labels",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                      nlabels = num_of_objects(*sd_data_label);

                      DBUG_PRINT(1,(LOGF, "dump_sds: found %d labels for data,tag=%d,ref=%d \n",
                                    nlabels,DFTAG_NDG,ref));    

                      if (nlabels > 0)
                        {
                            fprintf(h_fp,"<TR ALIGN=\"center\" VALIGN=\"center\"> <TD>");
                            perform_on_list(*sd_data_label,sd_ann2html,l_vars);
                            fprintf(h_fp,"</TD>");
                        }
                      else
                          fprintf(h_fp,"<TR><TD> -- </TD>");

                      if ((sd_data_desc = get_data_descs(fname, DFTAG_NDG, ref, h_fp,l_vars)) == NULL)
                        {
                            gateway_err(h_fp,"dump_sds: error getting data descs",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                      ndescs = num_of_objects(*sd_data_desc);

                      DBUG_PRINT(1,(LOGF, "dump_sds: found %d descs for NDG,tag=%d,ref=%d \n",
                                    ndescs,DFTAG_NDG,ref));    

                      if (ndescs > 0)
                        {
                            fprintf(h_fp,"<TD>");
#if 0
                            perform_on_list(*sd_data_desc,sd_ann2html,l_vars);
#endif
                            for (j =0; j < ndescs; j++)
                              {
                                  sd_ann = next_in_list(*sd_data_desc);
                                  if (sd_ann != NULL)
                                      fprintf(h_fp,"<LI><A HREF=\"%s\"> desc%d </A></LI>",
                                              obj_href(sd_ann->ann_tag,sd_ann->ann_ref,sd_ann->index,l_vars),j);
                              }
                            fprintf(h_fp, "</UL>\n");
                            fprintf(h_fp,"</TD>");
                        }
                      else
                          fprintf(h_fp,"<TD> -- </TD>");

                      fprintf(h_fp, "</TR></TABLE> <p>\n");

                        } /* end if not netCDF */

#ifdef HAVE_DTM
                      if (l_vars->dtm_outport && ((sds_elem->rank == 2) || (sds_elem->rank == 3)))
                          fprintf(h_fp,"(To broadcast this dataset over DTM, click <A HREF=\"%s\"> here </A>)<p>",
                                  obj_href(DFTAG_NDG,ref,0,l_vars));
#if 0
                      fprintf (l_vars->hfp, "(To broadcast this dataset over DTM, click <A HREF=\"#hdfdtm;tag=%d,ref=%d,dtmport=%s\">here</A>.)<p>\n", 
                               (int32) DFTAG_NDG, ref,l_vars->dtm_outport);
#endif
#endif
                      /* deal with attributes */
                      if (sds_elem->nattrs)
                        {
#if 0
                            fprintf(h_fp, "It has the following attributes :\n");
#endif
                            reset_to_beginning(*(sds_elem->tatrlist));
                            fprintf(h_fp, "<TABLE BORDER>\n");
                            fprintf(h_fp,"<caption align=\"top\"> Attributes</caption> \n");
                            fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH></TR> \n", 
                                    "Attribute Name", "Attribute Value");
                            perform_on_list(*(sds_elem->tatrlist),attr2html,l_vars);
                            fprintf(h_fp, "</TABLE>\n");
                        } /* end if nattrs */

                      /* ====================== SDS to Image ====================*/
                      /* take a sds to be as am image here? 
                      fprintf(h_fp, "<HR>");
                      fprintf(h_fp, "Insert the source code for generating the image from the SDS ");
		      */
                      readSds(fname, sds_elem->ndg_ref, l_vars);
                      fprintf(h_fp, "<HR>");	      
                      /* ================================================== */
             
                      /* now the rest */
                      fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                      fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                              l_vars->h_env->path_info);
                      fprintf(h_fp, "<PRE WIDTH=\"%d\">", DOC_WIDTH);
            
                      for (j=0; j < sds_elem->rank; j++) 
                        {
                            fprintf(h_fp, "Dimension %d: <P>\n", j);
                            fprintf(h_fp, "<UL>");
                            fprintf(h_fp, "<LI>starting location :");
                            fprintf(h_fp, "<INPUT NAME=\"@%s;dim%d,start\" VALUE=%d> <P>\n", 
                                    l_vars->hdf_ref, j, 0); /*"hdf_ref" now contains the name of
                                                              the requested SD. */
                            fprintf(h_fp, "<LI>number of units to skip:");
                            fprintf(h_fp, "<INPUT NAME=\"@%s;dim%d,stride\" VALUE=%d> <P>\n", 
                                    l_vars->hdf_ref, j, 1);
                            fprintf(h_fp, "<LI>ending location :");
                            fprintf(h_fp, "<INPUT NAME=\"@%s;dim%d,end\" VALUE=%d> <P>\n", 
                                    l_vars->hdf_ref, j, sds_elem->dimsizes[j]);
                            fprintf(h_fp, "</UL>");
                        }
                      fprintf(h_fp, "</PRE>");
                      fprintf(h_fp, "To see the data, press this button: \n");
                      fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Browse Data\">. \n");
                      fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"hdfref\" VALUE=\";tag=%d,ref=%d,s=%d\">\n",
                              DFTAG_NDG,sds_elem->ndg_ref,1);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" ");
                      fprintf(h_fp, "VALUE=\"%s\">\n", l_vars->f_name);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                              3);
                      fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_type\" VALUE=\"%d\">\n",
                              1);
                      fprintf(h_fp, "</FORM>");
                      fprintf(h_fp, "<em> Please make a note of the size of browse");
                      fprintf(h_fp, "data that is being requested.\n");
                      fprintf(h_fp, "An ascii dump usually can run 3-5 times ");
                      fprintf(h_fp, "larger than the binary size.\n </em>");
                      fprintf(h_fp, "<hr>\n");

                      /* cleanup */
                      if (sd_data_label != NULL)
                        {
                            perform_on_list(*(sd_data_label),free_an,NULL);        
                            destroy_list(sd_data_label);
                            HDfree(sd_data_label);
                            sd_data_label = NULL;
                        }
                      if (sd_data_desc != NULL)
                        {
                            perform_on_list(*(sd_data_desc),free_an,NULL);        
                            destroy_list(sd_data_desc);
                            HDfree(sd_data_desc);
                            sd_data_desc = NULL;
                        }

                  }
                else
                  {
                      DBUG_PRINT(1,(LOGF, "dump_sds: failed to find DataSet->hdf_ref %s\n", l_vars->hdf_ref));
                      goto done;
                  }
            } /* end display_mode == 2 */
          else if (display_mode==3 && !l_vars->do_dump) 
            {
                tsds_st *sds_elem = NULL;

                h_fp = l_vars->hfp;

                /* find dataset matchine hdf_ref */
                sds_elem = first_that(*(l_vars->sds_list),var_cmp,l_vars->hdf_ref);

                if (sds_elem != NULL)
                  {
                      DBUG_PRINT(1,(LOGF, "dump_sds: found DataSet->hdf_ref %s\n", l_vars->hdf_ref));

#ifdef HAVE_DTM
                      if (l_vars->dtm_outport)
                        {
                            mo_dtm_out(l_vars->dtm_outport,l_vars);
                            mo_dtm_poll_and_read ();
                            sleep(1);
                            if ((hdfDtmThang(fname,DFTAG_NDG,sds_elem->ndg_ref,l_vars->hdf_ref,l_vars)) ==FAIL)
                              {
                                  gateway_err(h_fp,"dump_sds: error from hdfDtmThang",0,l_vars);
                                  ret_value = FAIL;
                              }
                            sleep(1);
                            mo_dtm_poll_and_read ();
                            mo_dtm_disconnect ();
                        }
                      else
                        {
#endif
                            fprintf(h_fp, "<B>Dataset %s</B>:-<P>\n", sds_elem->name);
                            if (sds_elem->nattrs)
                              {
#if 0
                                  fprintf(h_fp, "It has the following attributes :\n");
                                  DBUG_PRINT(1,(LOGF, "Attributes : \n"));
#endif
                                  reset_to_beginning(*(sds_elem->tatrlist));
                                  fprintf(h_fp, "<TABLE BORDER>\n");
                                  fprintf(h_fp,"<caption align=\"top\"> Attributes</caption> \n");
                                  fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH></TR> \n", 
                                          "Attribute Name", "Attribute Value");
                                  perform_on_list(*(sds_elem->tatrlist),attr2html,l_vars);
                                  fprintf(h_fp, "</TABLE>\n");
                              }
                            else
                                fprintf(h_fp, "It has no attributes.<P>\n");

                            /* Code for subsetting an SDS */
                            if (sds_elem->rank > 0)
                              {
                                  int32 *start, *stride, *edge, *left;
                                  int ret, r, end;

                                  /* Open file for reading */
                                  if ((fid = SDstart(fname, DFACC_RDONLY)) == FAIL)
                                    {
                                        gateway_err(l_vars->hfp,"dump_sds: opening file ",0,l_vars);
                                        ret_value =  FAIL;
                                        goto done;
                                    }

                                  /* Select dataset "i" */
                                  if ((sds = SDselect(fid, sds_elem->index)) == FAIL)
                                    {
                                        SDend(fid);
                                        gateway_err(h_fp,"dump_sds: selecting dataset \n",0,l_vars);
                                        ret_value =  FAIL;
                                        goto done;
                                    }

                                  DBUG_PRINT(1,(LOGF, "dump_sds: selected dataset \n"));
                                  fprintf(h_fp, "Data :<P>\n");
	    
                                  start  = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                                  stride = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                                  edge   = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                                  left   = (int32 *)HDgetspace(sds_elem->rank * sizeof(int32));
                                  if (start == NULL || stride == NULL || edge == NULL || left == NULL)
                                    {
                                        SDendaccess(sds);
                                        SDend(fid);
                                        gateway_err(h_fp,"dump_sds: failed to allocate space \n",0,l_vars);
                                        ret_value = FAIL;
                                        goto done;
                                    }

                                  /* Get subsampling values along each dimension */
                                  for (r = 0; r < sds_elem->rank; r++)   
                                    {
                                        start[r] = atoi(entries[r*3].val);
                                        if ((stride[r] = atoi(entries[r*3+1].val))==0)
                                            stride[r] = 1;
                                        if ((end = atoi(entries[r*3+2].val))==0)
                                            left[r] = sds_elem->dimsizes[r] - start[r];
                                        else
                                            left[r] = end - start[r];
                                        edge[r] = 1;
                                    }
                                  edge[sds_elem->rank-1] = left[sds_elem->rank-1];

                                  /* dump subset of SDS */
                                  ret = sdsdumpfull(sds, sds_elem->rank, sds_elem->dimsizes, sds_elem->nt, 16, h_fp, 
                                                    start, stride, edge, left, l_vars); 
                                  if (ret == FAIL)
                                    {
                                        SDendaccess(sds);
                                        SDend(fid);
                                        gateway_err(h_fp,"dump_sds: failed to dump sds \n",0,l_vars);
                                        ret_value = FAIL;
                                        goto done;
                                    }

                                  fprintf(h_fp, "<P>\n");
                             
                                  /* free up space */
                                  HDfreespace(start);
                                  HDfreespace(stride);
                                  HDfreespace(edge);
                                  HDfreespace(left);
                                  DBUG_PRINT(1,(LOGF, "dump_sds: done dumping dataset \n"));

                                  SDendaccess(sds);
                                  SDend(fid);
                              } /* end sds_elem->rank */
#ifdef HAVE_DTM
                        }
#endif
                  }
                else
                  {
                      DBUG_PRINT(1,(LOGF, "dump_sds:failed to find DataSet->hdf_ref %s\n", l_vars->hdf_ref));
                      goto done;
                  }
            } /* end display_mode == 3 */

      } /* if(dsets) */

    /* 
     * Now Deal with global attributes 
     */
    if (display_mode==1 || l_vars->do_dump)  
      {
          if (ngattr) 
            {
                tatr_st *spattr_elem = NULL;
#if 0
                tatr_st *attr_elem = NULL;
#endif

                h_fp = l_vars->hfp;

#if 0
                fprintf(h_fp, "Global attributes :\n");
                DBUG_PRINT(1,(LOGF, "Global attributes :\n"));
#endif
                /* check for special global attribute "file description" */
                spattr_elem = first_that(*(l_vars->gattr_list),
                                         attr_cmp,"file description");

                if (spattr_elem != NULL)
                  {
                      fprintf(h_fp, "<UL>\n");
                      fprintf(h_fp, "<LI> Attribute <i>%s</i> has the value : <pre>%s</pre> \n", 
                              spattr_elem->name, spattr_elem->value_str);
                      fprintf(h_fp, "</UL>\n");
                  }

                /* print global attriubes in a Table */
                reset_to_beginning(*(l_vars->gattr_list));
                fprintf(h_fp, "<TABLE BORDER>\n");
                fprintf(h_fp,"<caption align=\"top\">Global Attributes</caption> \n");
                fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH></TR> \n", 
                        "Attribute Name", "Attribute Value");
                perform_on_list(*(l_vars->gattr_list),attr2html,l_vars);
                fprintf(h_fp, "</TABLE>\n");
#if 0
                /* now deal with other elements */
                if (ngattr > 1 && !l_vars->do_dump) 
                  {
                      fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                      fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                              l_vars->h_env->path_info);
                      fprintf(h_fp, "<SELECT NAME=\"Attributes\" SIZE=%d>\n",
                              (ngattr > 1? 5:2));
                  }
                else
                    fprintf(h_fp, "<UL>\n");

                /* for each attribute of dataset */
                reset_to_beginning(*(l_vars->gattr_list));
                for(j = 0; j < ngattr; j++) 
                  {
                      attr_elem = next_in_list(*(l_vars->gattr_list));
                      if (attr_elem != spattr_elem)
                        {
                            if (ngattr > 1 && !l_vars->do_dump)               
                                fprintf(h_fp, "<OPTION> %s = %s", 
                                        attr_elem->name,attr_elem->value_str);
                            else
                                fprintf(h_fp, "<LI> Attribute <i>%s</i> has the value : <pre>%s</pre> \n", 
                                        attr_elem->name, attr_elem->value_str);
                            DBUG_PRINT(1,(LOGF, "   %s : %s \n", 
                                          attr_elem->name, attr_elem->value_str));
                        }
                  } /* end for ngattr */

                if (ngattr > 1 && !l_vars->do_dump)               
                  {
                      fprintf(h_fp, "</SELECT><P>");
                      fprintf(h_fp, "</FORM>\n");
                  }
                else
                    fprintf(h_fp, "</UL>\n");
#endif

            } /* end if (ngattr) */
      } /* end if display_mode == 1*/


  done:
    if (ret_value == FAIL)
      {
      }

    /* destory tmp lone sds list */
    if (dsets)
        destroy_list(&lone_sds);
    if (ndimvars)
        destroy_list(&lone_dimvars);
    if (nregvars)
        destroy_list(&lone_regvars);

    /* normal cleanup */

    EXIT(2,"dump_sds");
    return ret_value;
} /* dump_sds */

