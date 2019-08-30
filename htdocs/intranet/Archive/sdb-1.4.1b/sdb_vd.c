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
static char RcsId[] = "@(#)sdb_vd.c,v 1.23 1996/04/15 17:58:24 georgev Exp";
#endif

/* sdb_vd.c,v 1.23 1996/04/15 17:58:24 georgev Exp */

/*
   LIST Functions
   --------------
     add_vd     - Add vdata to Vdata List
     add_field  - Adds field to Field list of Vdata
     vd_cmp     - Iterator fcn to find the Vdata element given its ref
     free_field - frees field element
     free_vd    - frees Vdata element and its field list        
     cleanup_vd - Frees Vdata List      

   Regular Functions
   -----------------
     print_vdata   - Dump information about a vdata given its ref
     do_vd         - prccess a vdata
     do_lone_vds   - process all the Lone Vdatas in file and put them in the List  
     dump_lone_vds - dump info on the Lone Vdatas in the file

   OBSOLETE
   --------
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_vd.h"
#include "show.h"
#include "sdb_util.h"
#include "glist.h"

/*---------------------------------------------------------------------
 NAME
       add_vd - Add vdata to Vdata List
 DESCRIPTION
       Add vdata to Vdata List
 RETURNS
        SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
add_vd(Generic_list *vd_list,   /* Vdata list */
       intn lone,               /* lone flag */
       int32 nelem,             /* number of record in VData */
       int32 nfields,           /* number of fields of Vdata */
       int32 interlaced,        /* interlaced flag */
       int32 size,              /* size */
       uint16 ref,              /* ref of VData */
       char *name,              /* name of Vdata */
       char *class,             /* class of Vdata */
       Generic_list *field_list, /* field List of Vdata */
       lvar_st *l_vars)
{
    tvd_st *vd_elem  = NULL;
    int    ret_value = SUCCEED;

    /* allocate space for Vdata element */
    if ((vd_elem = (tvd_st *)HDmalloc(sizeof(tvd_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_vd: failed to allocate space for vd_elem\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* allcate space for name of Vdata */
    if(name != NULL)
      {
          if ((vd_elem->name = (char *)HDmalloc(sizeof(char)*(HDstrlen(name)+1))) == NULL)
            {
                gateway_err(l_vars->hfp,"add_vd: failed to allocate space for vd_elem->name\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }
          HDstrcpy(vd_elem->name, name);
      }
    else
        vd_elem->name = NULL;

    /* allcate space for class of Vdata */
    if(class != NULL)
      {
          if ((vd_elem->class = (char *)HDmalloc(sizeof(char)*(HDstrlen(class)+1))) == NULL)
            {
                gateway_err(l_vars->hfp,"add_vd: failed to allocate space for vd_elem->class\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }
          HDstrcpy(vd_elem->class, class);
      }
    else
        vd_elem->class = NULL;

    /* fill in relevant info */
    vd_elem->lone       = lone;
    vd_elem->ref        = ref;
    vd_elem->nelem      = nelem;
    vd_elem->interlaced = interlaced;
    vd_elem->size       = size;
    vd_elem->nfields    = nfields;
    vd_elem->field_list = field_list;

    /* add to list */
    add_to_beginning(*vd_list, vd_elem);
    DBUG_PRINT(3,(LOGF,"add_vd: adding vdata,ref=%d,name=%s,class=%s,nelem=%d \n",
                  vd_elem->ref,vd_elem->name,vd_elem->class,vd_elem->nelem));

  done:
    /* failure */
    if (ret_value == FAIL)
      {
          if (vd_elem != NULL)
            {
                if(vd_elem->name != NULL)
                    HDfree(vd_elem->name);
                if(vd_elem->class != NULL)
                    HDfree(vd_elem->class);
                HDfree(vd_elem);
            }
      }
    return ret_value;
} /* add_vd() */

/*---------------------------------------------------------------------
 NAME
     add_field  - Adds field to Field list of Vdata
 DESCRIPTION
     Adds field to Field list of Vdata  
 RETURNS
     SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
add_field(Generic_list *field_list, /* field List */
          char *field,              /* field to add to list */
          lvar_st *l_vars)
{
    char *f_elem   = NULL;
    int  ret_value = SUCCEED;

    /* allocate space for field element */
    if ((f_elem = (char *)HDmalloc(sizeof(char)*(HDstrlen(field)+1))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_field: failed to allocate space for f_elem\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* copy field */
    HDstrcpy(f_elem,field);

    /* add to list */
    add_to_beginning(*field_list, f_elem);
    DBUG_PRINT(3,(LOGF,"add_field: adding field=%s \n",
                  f_elem));

  done:
    return ret_value;
} /* add_field() */

/*---------------------------------------------------------------------
 NAME
     vd_cmp  - Iterator fcn to find the Vdata element given its ref
 DESCRIPTION
     Iterator fcn to find the Vdata element given its ref  
 RETURNS
     1-> SUCCESS, 0->FAIL
-------------------------------------------------------------------------*/
int
vd_cmp(void *dptr,  /* Vdata element */
       void *args)
{
    tvd_st *vd_elem = dptr;
    uint16 *ref     = args;

    if(vd_elem->ref == *ref)
        return 1;
    else
        return 0;
}/* vd_cmp() */

/*---------------------------------------------------------------------
 NAME
     free_field - frees field element
 DESCRIPTION
     frees field element  
 RETURNS
     Nothing
-------------------------------------------------------------------------*/
void
free_field(void *dptr, /* field element */
           void *args)
{
    char *field = dptr;
    /* free structure itself */
    if(field != NULL)
        HDfree(field);
} /* free_field() */

/*---------------------------------------------------------------------
 NAME
     free_vd - frees Vdata element and its field list        
 DESCRIPTION
     frees Vdata element and its field list   
 RETURNS
     Nothing
-------------------------------------------------------------------------*/
void
free_vd(void *dptr, /* Vdata element to free */
        void *args)
{
    tvd_st *vd_elem = dptr;

    ENTER(2,"free_vd");
    if (vd_elem != NULL)
      {
          /* free elem list , can get recursive if non-NULL*/
          if (vd_elem->field_list != NULL)
            {
                DBUG_PRINT(1,(LOGF,"free_vd: destroy field_list=%d\n", 
                              num_of_objects(*(vd_elem->field_list))));
                perform_on_list(*(vd_elem->field_list),free_field,NULL);
                /* destroy element list */
                destroy_list(vd_elem->field_list);
                free(vd_elem->field_list);
            }

          /* free name and class */
          if (vd_elem->name != NULL)
              HDfree(vd_elem->name);
          if (vd_elem->class != NULL)
              HDfree(vd_elem->class);
          
          /* free structure itself */
          if(vd_elem != NULL)
              free(vd_elem);
      }
    EXIT(2,"free_vd");
} /* free_vd() */

/*---------------------------------------------------------------------
 NAME
     cleanup_vd - Frees Vdata List      
 DESCRIPTION
     Frees Vdata List  
 RETURNS
      Nothing
-------------------------------------------------------------------------*/
void
cleanup_vd(lvar_st *l_vars)
{
    ENTER(2,"cleanup_vd");
    /* now the Vdata list */
    if (l_vars->vd_list != NULL)
      { /* free elements first */
          DBUG_PRINT(1,(LOGF,"cleanup_vd: destroy vd_list=%d\n", 
                        num_of_objects(*(l_vars->vd_list))));
          perform_on_list(*(l_vars->vd_list),free_vd,NULL);        
          /* destory list itself */
          destroy_list(l_vars->vd_list);

          free(l_vars->vd_list);
          l_vars->vd_list = NULL;
      }
    EXIT(2,"cleanup_vd");
}/* cleanup_vd() */


/*---------------------------------------------------------------------
 NAME
       print_vdata - Dump information about a vdata given its ref
 DESCRIPTION
       Dump information about a vdata given its ref
 RETURNS
       SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
print_vdata(int32 fid,       /* file handle */
            int32 ref,       /* reference number of Vdata */
            lvar_st *l_vars)
{
    int i;
#if 0
    int32  vd;        /* Vdata handle */
    int32 single;     /* flag: single Vdata ? */
#endif
    int32  intr;      /* type of interlace */
    int32  sz;        /* size of Vdata */
    int32  cnt;       /* number of Vdatas */
    int32  nfields;   /* number of fields in a Vdata */
    tvd_st *vd_elem    = NULL;
    char   *field_name = NULL;
    uint16 vdref;
    int32  count;
    FILE  *h_fp = l_vars->hfp;
    int ret_value = SUCCEED;

    ENTER(2,"print_vdata");    

    /* Get number of lone Vdatas i.e. not in any Vgroups */
    if(l_vars->vd_list != NULL)
      {
          count = num_of_objects(*(l_vars->vd_list));
      }
    else
        count = 0;

    DBUG_PRINT(1,(LOGF, "print_vdata: There are %d Vdatas.\n", 
                  count));

    if(count < 1) 
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* find Vdata from Vdata List using ref */
    vdref = (uint16)ref;
    reset_to_beginning(*(l_vars->vd_list));
    vd_elem = first_that(*(l_vars->vd_list),vd_cmp,&vdref);

    if (vd_elem != NULL)
      {
          cnt     = vd_elem->nelem;
          intr    = vd_elem->interlaced;
          nfields = vd_elem->nfields;
          sz      = vd_elem->size;
          DBUG_PRINT(2,(LOGF, "print_vdata: nfields= %d", nfields));    

#ifdef LATER
          fprintf(h_fp, "Toggle which fields you want to see when you select the Vdata: <p>");
          fprintf(h_fp, "<FORM METHOD=\"POST\" ");
          fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                  l_vars->h_env->path_info);
#endif

          /* print Vdata info in a table */
          fprintf(h_fp, "<TABLE BORDER> <TR>\n");
          fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH> <TH> %s </TH>", 
                  "Vdata Name", "Class", "Number of Records");
          fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH> \n", 
                  "Number of Fields", "Size (bytes)");    
          fprintf(h_fp, "<TH> %s </TH></TR> \n", 
                  "Field Names");    

          DBUG_PRINT(1,(LOGF, "print_vdata: Vdata %s", vd_elem->name));    
          /* Vdata name and class */
          fprintf(h_fp, "<TR ALIGN=\"center\" VALIGN=\"top\" ><TD> <pre> %s </pre> </TD> <TD> <pre> %s </pre> </TD> ", 
                  vd_elem->name, (vd_elem->class[0] ? vd_elem->class : "--"));

          /* number of records and fields */
          fprintf(h_fp, "<TD> <pre> %d </pre> </TD> <TD> <pre> %d </pre> </TD> ",
                  cnt,nfields);
          /* size in bytes */
          fprintf(h_fp, "<TD> <pre> %d </pre> </TD>",
                  cnt*sz);
          /* print field List */
#ifdef LATER
          fprintf(h_fp, "<TD ROWSPAN=%d> <pre>",nfields);
#else
          fprintf(h_fp, "<TD> <pre>");
#endif
          if(vd_elem!= NULL)
            {
                reset_to_end(*(vd_elem->field_list));
                for (i = 1; i <= vd_elem->nfields; i++)
                  {
                      field_name = (char *)previous_in_list(*(vd_elem->field_list));
#ifdef LATER
                      fprintf(h_fp, "<INPUT TYPE=\"checkbox\" NAME=\"VH_FIELD\" VALUE=\"%d\" CHECKED>%s \n",
                              i,field_name);
#else
                      fprintf(h_fp, "%s \n", field_name);
#endif
                  }
            }
          fprintf(h_fp, " <pre> </TD> ");
          fprintf(h_fp, "</TR></TABLE> <p>\n");


#if 0
          single = FALSE;
          if((cnt == 1) && (nfields == 1))
              single = TRUE;

    /* if there is a single field with one value print out its values */
          if(single && l_vars->display_mode != 2) 
            {
                uint8 *tbuff = NULL;  /* buffer to hold vdata values */
                uint8 *cbuff = NULL;  /* buffer to hold converted vdata values */

                DBUG_PRINT(1,(LOGF, " Single field"));    

                /* Attempted bugfix -- added 1 -- marca, 10:52pm sep 23. */
                /* Allocate space for specified Vdata field */
                if ((tbuff = (uint8 *)HDgetspace(VFfieldisize(vd, 0) + 1)) == NULL)
                  {
                      gateway_err(h_fp,"print_vdata: unable to allocate space\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }
        
                /* Set field of VData to be accessed in next read */
                if ((VSsetfields(vd, fields)) == FAIL)
                  {
                      gateway_err(h_fp,"print_vdata: failed to set field of Vdata to be read\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }

                /* Read value specifed Vdata field */
                if ((VSread(vd, tbuff, 1, 0)) == FAIL)
                  {
                      gateway_err(h_fp,"print_vdata: failed to read Vdata field\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }

                /* Convert vdata field to string , note at tbuff is freed inside func */
                cbuff = (char *)buffer_to_string((char *)tbuff, (int32)VFfieldtype(vd, 0), 
                                                 (int32)VFfieldorder(vd, 0));

                if(cbuff) 
                  { /* Print vdata field in HTML form and then free buffer */
                      fprintf(l_vars->hfp, ": %s", cbuff);
                      DBUG_PRINT(4,(LOGF, ": %s \n", cbuff));
                      HDfreespace(cbuff);
                  }
                else
                  {
                      gateway_err(h_fp,"print_vdata: converting vdata field to string\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }
            
            } /* end if(single) */
#endif

          fprintf(h_fp, "<FORM METHOD=\"POST\" ");
          fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                  l_vars->h_env->path_info);
          fprintf(h_fp, "Toggle which fields you want to see when you select the Vdata: <p>");

          /* print out field list to select */
          if(vd_elem!= NULL)
            {
                reset_to_end(*(vd_elem->field_list));
                for (i = 1; i <= vd_elem->nfields; i++)
                  {
                      field_name = (char *)previous_in_list(*(vd_elem->field_list));
                      fprintf(h_fp, "<INPUT TYPE=\"checkbox\" NAME=\"VH_FIELD\" VALUE=\"%d\" >%s \n",
                              i,field_name);
                  }
            }
          fprintf(h_fp, "<P>");

          /* subsetting records */
          fprintf(h_fp, "Subsetting across Records: <P>\n");
          fprintf(h_fp, "<UL>");
          fprintf(h_fp, "<LI>starting record :");
          fprintf(h_fp, "<INPUT NAME=\"VH_START\" VALUE=%d> <P>\n", 
                  1); 
          fprintf(h_fp, "<LI>ending record:");          
          fprintf(h_fp, "<INPUT NAME=\"VH_END\" VALUE=%d> <P>\n", 
                  cnt);
          fprintf(h_fp, "</UL>");
          fprintf(h_fp, "<P>");

          /* submit button */
#if 0
          fprintf(h_fp, "<SELECT NAME=\"Vdata\" SIZE=1>\n");
          fprintf(h_fp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                  DFTAG_VH, ref, 0);
          fprintf(h_fp, "Vdata: %s\n", vd_elem->name);
          fprintf(h_fp, "</SELECT> <P>");
#endif
          fprintf(h_fp, "To display the vdata, press this button: ");
          fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Select Vdata\"> \n");
          fprintf(h_fp, "<INPUT TYPE=\"reset\" VALUE=\"Clear fields\"> <P>\n");
          fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"hdfref\" VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                  DFTAG_VH, ref, 0);
          fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",
                  l_vars->f_name); 
          fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                  3);
          fprintf(h_fp, "</FORM>");
      } /* if vd_elem */

  done:
    if(ret_value == FAIL)
      {
      }
    EXIT(2,"print_vdata");
    return ret_value;
} /* print_vdata */

/*----------------------------------------------------------------------------
 NAME
       do_vd - prccess a vdata
 DESCRIPTION  
       dump either info on the Vdata or the Vdata itself
 RETURNS
       SUCCEED/FAIL
-----------------------------------------------------------------------------*/
int
do_vd(char *fname,                 /* file name */
      int32 t,                     /* tag */
      int32 r,                     /* ref */
      lvar_st *l_vars)
{
    int32 i,j;
    int32 fid = FAIL;
    int32 vd_id = FAIL;
    char  sep[2];
    int   flds_indices[100];
    char   name[VSNAMELENMAX + 1];  /* Vdata name */
    char   class[VSNAMELENMAX + 1]; /* Vdata class */
    char   fields[(FIELDNAMELENMAX + 1) * VSFIELDMAX]; /* Vdata field names*/
    int32  nfields;
    int32  intr;      /* type of interlace */
    int32  sz;        /* size of Vdata */
    int32  cnt;       /* number of Vdatas */
    int x;
    uint16 vdref;
    Generic_list *field_list = NULL;
    char   *fptr = NULL;
    char   *nptr = NULL;
    FILE   *h_fp = l_vars->hfp;
    int ret_value = SUCCEED;

    ENTER(2,"do_vd");

    DBUG_PRINT(3,(LOGF,"do_vd: lets open file \n"));

    /* open file */
    if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_vd: Problem opening HDF file",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }

    /* start Vxx inteface */
    if ((Vstart(fid)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_vd: failed to start Vset interface\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }

    /* attach to Vdata */
    if ((vd_id = VSattach(fid, r, "r")) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_vd: failed to attach to Vdata\n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }

    /* initialize Vdata lists */
    if(l_vars->vd_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->vd_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_vd: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->vd_list);
      }

    /* get info on Vdata */
    if ((VSinquire(vd_id, &cnt, &intr, fields, &sz, name)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_vd: failed to get info on Vdata",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }

    /* Get class of Vdata */
    if ((VSgetclass(vd_id, class)) == FAIL)
      {
          Vdetach(vd_id); /* Dettach from Vdata */
          gateway_err(l_vars->hfp,"do_vd: failed to get class of Vdata",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }

    /* Get number of fields in Vdata */
    if ((nfields = VFnfields(vd_id)) == FAIL)
      {
          Vdetach(vd_id); /* Dettach from Vdata */
          gateway_err(l_vars->hfp,"do_vd: failed to number of fields in Vdata",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }

    /* Check if Vdata in list already */
    vdref = (uint16)r;
    if (first_that(*(l_vars->vd_list),vd_cmp,&vdref) == NULL)
      {
          /* prepare field list */
          if (nfields > 0)
            {
                /* allocate list to hold elements */
                if ((field_list = HDmalloc(sizeof(Generic_list))) == NULL)
                  {
                      Vdetach(vd_id); /* Dettach from Vdata */
                      gateway_err(l_vars->hfp,"do_vd: failed to allocate space for attribute list\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }   

                /* initialize field list of this Vdata */
                initialize_list(field_list);

                /* add fields to field list */
                fptr = fields;
                for (j = 0; j < nfields; j++)
                  {
                      nptr = HDstrchr(fptr,',');
                      if (nptr != NULL)
                        {
                            *nptr = '\0';
                            nptr++;
                        }
                      if (add_field(field_list,fptr,l_vars) == FAIL)
                        {
                            Vdetach(vd_id); /* Dettach from Vdata */
                            gateway_err(h_fp,"do_vd: failed to add vdata to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                      fptr = nptr;
                  } /* end for nfields */
            }
          else
              field_list = NULL;

          /* add Vdata to Vdata list */
          if (add_vd(l_vars->vd_list,0,cnt,nfields,intr,sz,r,name,class,
                     field_list,l_vars) == FAIL)
            {
                Vdetach(vd_id); /* Dettach from Vdata */
                gateway_err(h_fp,"do_vd: failed to add vdata to list \n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }
      }/* if Vdata not in list already */

    /* detach from Vdata */
    VSdetach(vd_id);

    /* prepare Vdata HTML */
    fprintf(l_vars->hfp, "<TITLE>Vdata:  %s</TITLE>\n", name);
    fprintf(l_vars->hfp, "<H1>Vdata: %s</H1>\n", name);

    DBUG_PRINT(3,(LOGF,"do_vd: get ready to dump Vdata\n"));

    /* Dump Vdata data 
     *  else dump vdata info */
    if (l_vars->display_mode == 3)
      { /* we are dumping the data itself */
          if ((vd_id = VSattach(fid, r, "r")) == FAIL)
            {
                gateway_err(l_vars->hfp,"do_vd: failed to attach to Vdata\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }

          for (x=0; x<100; x++)
              flds_indices[x] = -1;

          /* Debugging stuff */
          DBUG_PRINT(3,(LOGF,"do_vd: get ready to dump actual Vdata\n"));
          DBUG_PRINT(3,(LOGF,"do_vd: dumping fields ==>"));
          for (i = 0; i < l_vars->nfields; i++)
            {
                DBUG_PRINT(3,(LOGF,"%d,",l_vars->field_indices[i]));
            }
          DBUG_PRINT(3,(LOGF,"\n"));
          DBUG_PRINT(3,(LOGF,"do_vd: starting record=%d\n",l_vars->vh_start_rec));
          DBUG_PRINT(3,(LOGF,"do_vd: ending=%d\n",l_vars->vh_end_rec));
          DBUG_PRINT(3,(LOGF,"\n"));

          /* hmm.. */
          strcpy(sep, ";");

          /* Dump actual Vdata */
          if ((dumpvd(vd_id, FALSE, l_vars->hfp, sep, flds_indices, TRUE, l_vars)) == FAIL)
            {
                gateway_err(l_vars->hfp,"do_vd: to dump Vdata\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }

          /* detach from Vdata */
          VSdetach(vd_id);
      }
    else /* print info on Vdata in a Table */
        print_vdata(fid, r,l_vars);    


  done:
    /* failure */
    if (ret_value == FAIL)
      {
          cleanup_vd(l_vars);
      }

    if (fid != FAIL)
      {
          Vend(fid);
          Hclose(fid);
      }

    EXIT(2,"do_vd");
    return ret_value;
} /* do_vd() */

/*---------------------------------------------------------------------
 NAME
     do_lone_vds - process all the Lone Vdatas in file and put them in the List  
 DESCRIPTION
     process all the Lone Vdatas in file and put them in the List  
 RETURNS
     SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
do_lone_vds(char *fname,      /* file name */
            lvar_st *l_vars)
{
    int32 count;        /* Number of lone Vdatas */
    int32 vd;           /* Vdata handle */
    int32 i,j;
    int32 status;      /* flag: function return ? */
    char   name[VSNAMELENMAX + 1];  /* Vdata name */
    char   class[VSNAMELENMAX + 1]; /* Vdata class */
    char   fields[(FIELDNAMELENMAX + 1) * VSFIELDMAX]; /* Vdata field names*/
    int32  intr;      /* type of interlace */
    int32  sz;        /* size of Vdata */
    int32  cnt;       /* number of Vdatas */
    int32  nfields;
    int32  fid = -1;
    int32 *ids = NULL;        /* Vdata */
    Generic_list *field_list = NULL;
    char   *fptr = NULL;
    char   *nptr = NULL;
    FILE   *h_fp;
    int    ret_value = SUCCEED;
    
    ENTER(2,"do_lone_vds");
    h_fp = l_vars->hfp;

    /* Open file */
    if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_lone_vds: Problem opening HDF file",0,l_vars);
          ret_value = NULL;
          goto done;
      }

    /* setup to start accessing Vgroups */
    if (Vstart(fid) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_lone_vds: error with Vstart ",0,l_vars);
          ret_value = NULL;
          goto done;
      }

    /* Get number of lone Vdatas i.e. not in any Vgroups */
    if ((count = VSlone(fid, NULL, 0)) < 1)
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* initialize Vdata lists */
    if(l_vars->vd_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->vd_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_lone_vds: failed to allocate space for SDS list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->vd_list);
      }
    
    /* allocate space for array of Vdata reference numbers  */
    if ((ids = (int32 *) HDgetspace(sizeof(int32) * count)) == NULL)
      {
          ret_value = FAIL; 
          goto done;
      }


    /* Get the array containg the lone Vdata reference numbers */
    if ((status = VSlone(fid, ids, count)) != count)
      {
          ret_value = FAIL; 
          goto done;
      }

    /* For each lone Vdata, add it to list */
    for(i = 0; i < count; i++) 
      {
          if ((vd = VSattach(fid, ids[i], "r")) == FAIL)
            {
                ret_value = FAIL; 
                goto done;
            }
          
          HDmemset(fields,'\0',(FIELDNAMELENMAX + 1) * VSFIELDMAX);
          /* Inquire about the Vdata */
          if ((VSinquire(vd, &cnt, &intr, fields, &sz, name)) == FAIL)
            {
                Vdetach(vd); /* Dettach from Vdata */
                gateway_err(l_vars->hfp,"do_lone_vds: failed to get info on Vdata",0,l_vars);
                ret_value = FAIL; goto done;
            }

          /* Get class of Vdata */
          if ((VSgetclass(vd, class)) == FAIL)
            {
                Vdetach(vd); /* Dettach from Vdata */
                gateway_err(l_vars->hfp,"do_lone_vds: failed to get class of Vdata",0,l_vars);
                ret_value = FAIL; goto done;
            }

          /* Get number of fields in Vdata */
          if ((nfields = VFnfields(vd)) == FAIL)
            {
                Vdetach(vd); /* Dettach from Vdata */
                gateway_err(l_vars->hfp,"do_lone_vda: failed to number of fields in Vdata",0,l_vars);
                ret_value = FAIL; goto done;
            }

          /* prepare field list */
          if (nfields > 0)
            {
                /* allocate list to hold elements */
                if ((field_list = HDmalloc(sizeof(Generic_list))) == NULL)
                  {
                      Vdetach(vd); /* Dettach from Vdata */
                      gateway_err(l_vars->hfp,"do_lone_vdata: failed to allocate space for attribute list\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }   

                /* initialize field list of this Vdata */
                initialize_list(field_list);

                /* add fields to field list */
                fptr = fields;
                for (j = 0; j < nfields; j++)
                  {
                      nptr = HDstrchr(fptr,',');
                      if (nptr != NULL)
                        {
                            *nptr = '\0';
                            nptr++;
                        }
                      if (add_field(field_list,fptr,l_vars) == FAIL)
                        {
                            Vdetach(vd); /* Dettach from Vdata */
                            gateway_err(h_fp,"do_lone_vds: failed to add vdata to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                      fptr = nptr;
                  } /* end for nfields */
            }
          else
              field_list = NULL;

          /* add Vdata to lone Vdata list */
          if (add_vd(l_vars->vd_list,1,cnt,nfields,intr,sz,ids[i],name,class,
                     field_list,l_vars) == FAIL)
            {
                Vdetach(vd); /* Dettach from Vdata */
                gateway_err(h_fp,"do_lone_vds: failed to add vdata to list \n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* detach from Vdata */
          VSdetach(vd);
      } /* for each lone Vdata */

  done:
    if (ret_value == FAIL)
      {
          cleanup_vd(l_vars);
      }

    /* free up allocated space */
    if (ids!= NULL)
        HDfreespace((void *)ids); /* Free Vdata reference number array */

    /* clean up */
    if (fid != FAIL)
      {
          Vend(fid);
          Hclose(fid);
      }
    EXIT(2,"do_lone_vds");
    return ret_value;
} /* do_lone_vds() */

/*----------------------------------------------------------------------------
 NAME
       dump_lone_vds - dump info on the Lone Vdatas in the file
 DESCRIPTION  
       dump info on the Lone Vdatas in the file
 RETURNS
       SUCCEED/FAIL
-----------------------------------------------------------------------------*/
int
dump_lone_vds(char *fname,     /* file name */
              lvar_st *l_vars)
{
    int32 i;
    int32 count;        /* Number of lone Vdatas */
    int32  intr;      /* type of interlace */
    int32  sz;        /* size of Vdata */
    int32  cnt;       /* number of Vdatas */
    int32  nfields;
    uint16 vdref;
    char   tmp_html[1024];
    tvd_st *vd_elem = NULL;
    FILE   *h_fp;
    int    ret_value = SUCCEED;
    
    ENTER(2,"dump_lone_vds");
    /* Get number of lone Vdatas i.e. not in any Vgroups */
    if(l_vars->vd_list != NULL)
      {
          count = num_of_objects(*(l_vars->vd_list));
      }
    else
        count = 0;

    DBUG_PRINT(1,(LOGF, "dump_lone_vds: There are %d Vdatas visible at this level of the file.\n", 
                  count));

    if(count < 1) 
      {
          ret_value = SUCCEED;
          goto done;
      }
    
    if (l_vars->do_dump)
      {
          /* Create name for HTML file */
          if (l_vars->html_dir == NULL)
              sprintf(tmp_html,"%s_vd.html",fname);
          else
              sprintf(tmp_html,"%s/%s_vd.html",l_vars->html_dir,l_vars->f_name);
            
          DBUG_PRINT(1,(LOGF,"dump_lone_vds: hdf html file name %s \n", tmp_html));
          
          /* Open temproary file to write HTML description of HDF/netCDF file */
          if (!(h_fp = fopen(tmp_html, "w")))
            {
                ret_value = FAIL; goto done;
            }

          /* Write MIME header */
          if (write_html_header(h_fp, TEXT_HTML,l_vars) == FAIL)
            {
                gateway_err(h_fp,"dump_lone_vds:  writing HTML header",0,l_vars);
                ret_value = FAIL; goto done;
            }

          if(l_vars->hdf_path_r != NULL)
              fprintf(h_fp,"%s Vdata%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                      (count > 1 ? "These" : "This"),(count > 1 ? "s" : ""),
                      l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
          else
              fprintf(h_fp,"%s Vdata%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                      (count > 1 ? "These" : "This"),(count > 1 ? "s" : ""),
                      l_vars->cgi_path,l_vars->f_path_r,l_vars->f_name,l_vars->f_name);
      }
    else
        h_fp = l_vars->hfp;

    /* set up Vdata HTML stuff */
    fprintf(h_fp, "<hr>\n");
    fprintf(h_fp, "<H2>Vdatas</H2>\n");
    fprintf(h_fp, "There are %d Vdatas visible at this level of the file.\n", 
            count);
    DBUG_PRINT(1,(LOGF, "dump_lone_vds: There are %d Vdatas visible at this level of the file.\n", 
                  count));

    if(l_vars->do_dump)
        fprintf(h_fp,"<OL>\n");
    else
      {
          fprintf(h_fp, "<FORM METHOD=\"POST\" ");
          fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                  l_vars->h_env->path_info);
          fprintf(h_fp, "<SELECT NAME=\"Lone_Vdata\" SIZE=5>\n");
      }

    /* For each lone Vdata, list it  */
    reset_to_beginning(*(l_vars->vd_list));
    for(i = 0; i < count; i++) 
      {
          vd_elem = next_in_list(*(l_vars->vd_list));
          cnt = vd_elem->nelem;
          intr = vd_elem->interlaced;
          sz   = vd_elem->size;
          vdref = vd_elem->ref;
          nfields = vd_elem->nfields;

          if (l_vars->do_dump)
            {
                fprintf(h_fp,"<LI> This Vdata <B>%s</B> has <B>%d</B> element%s of size <B>%d</B> bytes, <B>%s</B> interlaced where the %s \n",
                        vd_elem->name, cnt,(cnt > 1 ? "s" : ""),sz,(intr ? "NOT" : "FULLY"),(nfields > 1?"fields are":"field is"));
            }
          else
            {
                if (i==0)
                    fprintf(h_fp, "<OPTION SELECTED VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                            DFTAG_VH, vdref, 0);
                else	
                    fprintf(h_fp, "<OPTION VALUE=\";tag=%d,ref=%d,s=%d\"> ",
                            DFTAG_VH, vdref, 0);
                fprintf(h_fp, "%s\n", vd_elem->name);
            }
      } /* end for each lone Vdata */

    if(!l_vars->do_dump)
      {
          fprintf(h_fp, "</SELECT> <P>");
          fprintf(h_fp, "To select a particular vdata, press this button: ");
          fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Select Vdata\">. <P>\n");
          fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",
                  l_vars->f_name); 
          fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                  2);
          fprintf(h_fp, "</FORM>");
      }
    else
      {
          fprintf(h_fp, "</OL>\n");
          fclose(h_fp);
      }
    
  done:
    EXIT(2,"dump_lone_vds");
    return ret_value;
} /* dump_lone_vds */
