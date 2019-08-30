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
static char RcsId[] = "@(#)sdb_ann.c,v 1.17 1996/05/09 20:58:09 georgev Exp";
#endif

/* sdb_ann.c,v 1.17 1996/05/09 20:58:09 georgev Exp */

/*
   LIST Functions
   --------------
   add_an     - add annotation of specified type to annotation list
   an1_cmp    - Iterator function to compare the annotations
   an_cmp     - Iterator function to compare the annotations       
   free_an    - Iterator to free annotation element       
   cleanup_an - cleans up annotation lists

   Regular Functions
   -----------------
   get_data_labels - gets a List of data labels for tag/ref       
   get_data_descs  - gets a List of data descriptions for tag/ref              
   init_ann_html   - initialize file to dump annotation info to
   print_ann       - dump info about the selected annotation
   do_anns         - process all anotation in file
   dump_anns       - dump info about file labels/descs

   OBSOLETE
   --------
   print_desc      - print label/description info for object
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_ann.h"
#include "sdb_util.h"
#include "glist.h"

/*====================== List Functions ================================*/

/*----------------------------------------------------------------------- 
 NAME
       add_an - add annotation of specified type to annotation list
 DESCRIPTION
       Adds annotations to the annotation list.
 RETURNS
      SUCCEED/FAIL
-----------------------------------------------------------------------*/
int
add_an(Generic_list *an_list, /* list to add this annotation to */
       intn index,            /* index position of this annotation in file */
       ann_type atype,        /* annotation type */
       uint16 ref,            /* ref of annotation */
       uint16 tag,            /* tag of annotation */
       char *ann,             /* annotation itself */
       lvar_st *l_vars)
{
    tan_st *an_elem  = NULL;
    int    ret_value = SUCCEED;

    /* allocate space for annotation element */
    if ((an_elem = (tan_st *)HDmalloc(sizeof(tan_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_an: failed to allocate space for an_elem \n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* fill in the appropriate info */
    an_elem->atype = atype;
    an_elem->index = index;
    an_elem->ann_ref = ref;
    an_elem->ann_tag = tag;
    an_elem->ann = ann;

    /* add to the list */
    add_to_beginning(*an_list, an_elem);
    DBUG_PRINT(3,(LOGF,"add_an: adding annotation,index=%d, objref=%d, objtag=%d \n",
                  an_elem->index,an_elem->ann_ref, an_elem->ann_tag));

  done:
    return ret_value;
} /* add_an() */

/*----------------------------------------------------------------------- 
 NAME
       an1_cmp - Iterator function to compare the annotations
 DESCRIPTION
       Iterator function to compare the give annotation with ones
       in the annotation list using annotation type, tag and ref.
 RETURNS
       1-> SUCCESS, 0->FAIL
-----------------------------------------------------------------------*/
int
an1_cmp(void *dptr, /* annotation element in list */
        void *args /* annotation element passed in to compare against */)
{
    tan_st *an_elem = dptr;
    tan_st *an2_elem = args;

    /* satisfy annotation type, object tag and ref */
    if(an_elem->atype == an2_elem->atype 
       && an_elem->ann_ref == an2_elem->ann_ref
       && an_elem->ann_tag == an2_elem->ann_tag)
        return 1;
    else
        return 0;
} /* an1_cmp() */

/*----------------------------------------------------------------------- 
 NAME
       an_cmp - Iterator function to compare the annotations       
 DESCRIPTION
        Iterator function to compare the give annotation with ones
       in the annotation list using the annotation type.
 RETURNS
       1-> SUCCESS, 0->FAIL
-----------------------------------------------------------------------*/
int
an_cmp(void *dptr, 
       void *args /* annotation type */)
{
    tan_st *an_elem = dptr;
    ann_type *atype = args;

    /* satisfy annotation type */
    if(an_elem->atype == *atype)
        return 1;
    else
        return 0;
} /* an_cmp() */

/*----------------------------------------------------------------------- 
 NAME
      free_an - Iterator to free annotation element       
 DESCRIPTION
      frees annotation element and components
 RETURNS
      Nothing
-----------------------------------------------------------------------*/
void
free_an(void *dptr, /* annotation element to free */
        void *args)
{
    tan_st *an_elem = dptr;

    if (an_elem != NULL)
      { /* free annotation itself */
          if(an_elem->ann != NULL)
              free(an_elem->ann);
          free(an_elem); /* free element */
      }
} /* free_an() */

/*----------------------------------------------------------------------- 
 NAME
      cleanup_an - cleans up annotation lists
 DESCRIPTION
      Frees all the annotation lists and their elements
 RETURNS
      Nothing
-----------------------------------------------------------------------*/
void
cleanup_an(lvar_st *l_vars)
{
    ENTER(2,"cleanup_an");

    /* object label annotation  list */
    if (l_vars->olan_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_an: destroy olan_list=%d\n", 
                        num_of_objects(*(l_vars->olan_list))));
          /* destory list itself */
          destroy_list(l_vars->olan_list);
          free(l_vars->olan_list);
          l_vars->olan_list = NULL;
      }

    /* object description annotation  list */
    if (l_vars->odan_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_an: destroy odan_list=%d\n", 
                        num_of_objects(*(l_vars->odan_list))));
          /* destory list itself */
          destroy_list(l_vars->odan_list);
          free(l_vars->odan_list);
          l_vars->odan_list = NULL;
      }

    /* file label annototation list */
    if (l_vars->flan_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_an: destroy flan_list=%d\n", 
                        num_of_objects(*(l_vars->flan_list))));
          /* destory list itself */
          destroy_list(l_vars->flan_list);
          free(l_vars->flan_list);
          l_vars->flan_list = NULL;
      }

    /* file description annototation list */
    if (l_vars->fdan_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_an: destroy fdan_list=%d\n", 
                        num_of_objects(*(l_vars->fdan_list))));
          /* destory list itself */
          destroy_list(l_vars->fdan_list);
          free(l_vars->fdan_list);
          l_vars->fdan_list = NULL;
      }

    /* all annotations  list */
    if (l_vars->an_list != NULL)
      { 
          DBUG_PRINT(1,(LOGF,"cleanup_an: destroy an_list=%d\n", 
                        num_of_objects(*(l_vars->an_list))));
          /* free elements first */
          perform_on_list(*(l_vars->an_list),free_an,NULL);        
          /* destory list itself */
          destroy_list(l_vars->an_list);
          free(l_vars->an_list);
          l_vars->an_list = NULL;
      }
    EXIT(2,"cleanup_an");
} /* cleanup_an() */

/*===================== Regular Functions ==============================*/

/*----------------------------------------------------------------------- 
 NAME
      get_data_labels - gets a List of data labels for tag/ref       
 DESCRIPTION
      This returns a new Generic List of data labels for the
      given tag/ref i.e. HDF object
 RETURNS
      Generic_list if Successfull and NULL otherwise.
-----------------------------------------------------------------------*/
Generic_list *
get_data_labels(char *fname,     /* file name */
                uint16 tag,      /* tag of object */
                uint16 ref,      /* ref of object */
                FILE *fout,      /* html output file? */
                lvar_st *l_vars)
{
    int    i;
    int32  ret;
    int32  ann_len;
    int32  file_handle = FAIL; 
    int32  an_handle   = FAIL;
    int32  *dlabels    = NULL;
    char   *ann_label  = NULL;
    intn   num_dlabels;
    uint16 atag;
    uint16 aref;
    Generic_list *dlabel_list = NULL;
    Generic_list *ret_value   = NULL;

    ENTER(2,"get_data_labels");

    /* open file and start Anotation handling */
    if((file_handle = Hopen(fname, DFACC_READ, 0)) == FAIL)
      {
          gateway_err(fout,"get_data_labels: opening file",1,l_vars);
          ret_value = NULL;
          goto done;
      }

    if ((an_handle = ANstart(file_handle)) == FAIL)
      {
          gateway_err(fout,"get_data_labels: Starting annotation interface",0,l_vars);
          ret_value =  NULL;
          goto done;
      }

    /* initialize data Annotation lists */
    if(dlabel_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((dlabel_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"get_data_labels: failed to allocate space for ann list\n",0,l_vars);
                ret_value = NULL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(dlabel_list);
      }

    /* Get number of data labels with this tag/ref */
    num_dlabels = ret = ANnumann(an_handle, AN_DATA_LABEL, tag, ref);

    /* If we don't have any labels or descriptions we are done */
    if (num_dlabels == 0)
      {
          ret_value = dlabel_list; /* empty list */
          goto done;
      }
    else /* we have some data labels */
      { 
          /* allocate space for list of label annotation id's with this tag/ref */
          if ((dlabels = (int32 *)HDmalloc(num_dlabels * sizeof(int32))) == NULL)
            {
                gateway_err(fout,"get_data_labels: failed to allocate space to hold data label ids\n",0,l_vars);
                ret_value =  NULL;
                goto done;
            }

          /* get list of label annotations id's with this tag/ref */
          if (ANannlist(an_handle, AN_DATA_LABEL, tag, ref, dlabels) != num_dlabels)
            {
                gateway_err(fout,"get_data_labels: getting number of data labels for tag/ref \n",0,l_vars); 
                ret_value = NULL;
                goto done;
            }

          DBUG_PRINT(1,(LOGF,"get_data_labels: num_dlabels=%d\n", num_dlabels));

          /* loop through label list */
          for (i = 0; i < num_dlabels; i++)
            {
                if ((ann_len = ANannlen(dlabels[i])) == FAIL)
                  {
                      gateway_err(fout,"get_data_labels: getting data label length\n",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
        
                /* allocate space for label */
                if (ann_label == NULL)
                  {
                      if ((ann_label = (char *)HDmalloc((ann_len+1)*sizeof(char))) == NULL)
                        {
                            gateway_err(fout,"get_data_labels: failed to allocate space to hold data label \n",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }
                      HDmemset(ann_label,'\0', ann_len+1);
                  }
      
                /* read label */
                if (ANreadann(dlabels[i], ann_label, ann_len+1) == FAIL)
                  {
                      gateway_err(fout,"get_data_labels: reading data label \n",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* get tag ref */
                if (ANid2tagref(dlabels[i],&atag,&aref) == FAIL)
                  {
                      gateway_err(fout,"get_data_labels: getting tag/ref for annotation",0,l_vars); 
                      ret_value = NULL;
                      goto done;
                  }

                /* end access */
                ANendaccess(dlabels[i]);

                /* add label annotation to list */
                if (add_an(dlabel_list,i,AN_DATA_LABEL,aref,atag,ann_label,l_vars) == FAIL)
                  {
                      gateway_err(fout,"get_data_labels: failed to add annotation to list \n",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
#if 0
                /* Print annotation in HTML form */
                if (i == 0)
                    fprintf(fout, "<P> This %s was given the label%s : <B>%s</B> \n", 
                            name, (num_dlabels == 1 ? "":"s"),ann_label);
                else
                    fprintf(fout, ", <B>%s</B> \n", ann_label);
                HDfree(ann_label);
#endif

                ann_label = NULL;
            } /* end for labels */

          /* return value is the new list of data labels */
          ret_value = dlabel_list; 

      } /* end else nolabels > 0 */

  done:
    /* Failure */
    if (ret_value == NULL)
      {  /* cleanup */
          if (dlabel_list != NULL)
            {
                perform_on_list(*(dlabel_list),free_an,NULL);        
                /* destory list itself */
                destroy_list(dlabel_list);

                HDfree(dlabel_list);
            }
      }

    /* cleanup */
    if (dlabels != NULL)
        HDfree(dlabels);
    if(an_handle != FAIL)
        ANend(an_handle);
    if(file_handle != FAIL)
        Hclose(file_handle);

    EXIT(2,"get_data_labels");
    return ret_value;
} /* get_data_labels() */

/*----------------------------------------------------------------------- 
 NAME
      get_data_descs - gets a List of data descriptions for tag/ref              
 DESCRIPTION
      This returns a new Generic List of data descriptions for the
      given tag/ref i.e. HDF object       
 RETURNS
      Generic_list if Successfull and NULL otherwise.
-----------------------------------------------------------------------*/
Generic_list *
get_data_descs(char *fname,     /* file name */
               uint16 tag,      /* tag of object */
               uint16 ref,      /* ref of object */
               FILE *fout,      /* html output file? */
               lvar_st *l_vars)
{
    int    i;
    int32  ret;
    int32  ann_len;
    int32  file_handle = FAIL; 
    int32  an_handle   = FAIL;
    int32  *ddescs     = NULL;
    char   *ann_desc   = NULL;
    intn   num_ddescs;
    uint16 atag;
    uint16 aref;
    Generic_list *ddesc_list = NULL;
    Generic_list *ret_value  = NULL;

    ENTER(2,"get_data_descs");

    /* open file and start Anotation handling */
    if((file_handle = Hopen(fname, DFACC_READ, 0)) == FAIL)
      {
          gateway_err(fout,"get_data_descs: opening file",1,l_vars);
          ret_value = NULL;
          goto done;
      }

    if ((an_handle = ANstart(file_handle)) == FAIL)
      {
          gateway_err(fout,"get_data_descs: Starting annotation interface",0,l_vars);
          ret_value =  NULL;
          goto done;
      }

    /* initialize data Annotation lists */
    if(ddesc_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((ddesc_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"get_data_descs: failed to allocate space for ann list\n",0,l_vars);
                ret_value = NULL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(ddesc_list);
      }

    /* Get number of data labels with this tag/ref */
    num_ddescs = ret = ANnumann(an_handle, AN_DATA_DESC, tag, ref);

    /* If we don't have any labels or descriptions we are done */
    if (num_ddescs == 0)
      {
          ret_value = ddesc_list; /* empty list */
          goto done;
      }
    else /* we have data descriptions */
      { 
          /* allocate space for list of descs annotation id's with this tag/ref */
          if ((ddescs = (int32 *)HDmalloc(num_ddescs * sizeof(int32))) == NULL)
            {
                gateway_err(fout,"get_data_descs: failed to allocate space to hold data desc ids\n",0,l_vars);
                ret_value =  NULL;
                goto done;
            }

          /* get list of desc annotations id's with this tag/ref */
          if (ANannlist(an_handle, AN_DATA_DESC, tag, ref, ddescs) != num_ddescs)
            {
                gateway_err(fout,"get_data_descs: getting number of data descs for tag/ref \n",0,l_vars); 
                ret_value = NULL;
                goto done;
            }
          DBUG_PRINT(1,(LOGF,"get_data_descs: num_ddescs=%d\n", num_ddescs));

          /* loop through desc list */
          for (i = 0; i < num_ddescs; i++)
            {
                if ((ann_len = ANannlen(ddescs[i])) == FAIL)
                  {
                      gateway_err(fout,"get_data_descs: getting data label length\n",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
        
                /* allocate space for desc */
                if (ann_desc == NULL)
                  {
                      if ((ann_desc = (char *)HDmalloc((ann_len+1)*sizeof(char))) == NULL)
                        {
                            gateway_err(fout,"get_data_descs: failed to allocate space to hold data desc \n",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }
                      HDmemset(ann_desc,'\0', ann_len+1);
                  }
      
                /* read description */
                if (ANreadann(ddescs[i], ann_desc, ann_len+1) == FAIL)
                  {
                      gateway_err(fout,"get_data_descs: reading data desc \n",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* get tag ref */
                if (ANid2tagref(ddescs[i],&atag,&aref) == FAIL)
                  {
                      gateway_err(fout,"get_data_descls: getting tag/ref for annotation",0,l_vars); 
                      ret_value = NULL;
                      goto done;
                  }

                /* end access */
                ANendaccess(ddescs[i]);

                /* add desc annotation to list */
                if (add_an(ddesc_list,i,AN_DATA_DESC,aref,atag,ann_desc,l_vars) == FAIL)
                  {
                      gateway_err(fout,"get_data_descs: failed to add annotation to list \n",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* Print annotation in HTML form */
                ANendaccess(ddescs[i]);

#if 0
                /* Print annotation in HTML form */
                if (i == 0)
                    fprintf(fout, "<P> %s description%s : <pre> %s </pre>\n", 
                            name, (num_ddescs == 1 ? "" : "s"), ann_desc);
                else
                    fprintf(fout, " <pre>%s</pre> <P> \n", ann_desc);
                HDfree(ann_desc);
#endif
                ann_desc = NULL;
            } /* end for descs */

          /* return list of descriptions */
          ret_value = ddesc_list; 

      } /* end else nodescs > 0 */

  done:
    /* Failure */
    if (ret_value == NULL)
      {
          if (ddesc_list != NULL)
            {
                perform_on_list(*(ddesc_list),free_an,NULL);        
                /* destory list itself */
                destroy_list(ddesc_list);

                HDfree(ddesc_list);
            }
      }

    /* cleanup */
    if (ddescs != NULL)
        HDfree(ddescs);
    if(an_handle != FAIL)
        ANend(an_handle);
    if(file_handle != FAIL)
        Hclose(file_handle);

    EXIT(2,"get_data_descs");
    return ret_value;
} /* get_data_descs() */

/*----------------------------------------------------------------------- 
 NAME
       init_ann_html
 DESCRIPTION
       intitialize file to dump annotation info to. Used when
       dumping annotations to files.
 RETURNS
-----------------------------------------------------------------------*/
FILE *
init_ann_html(char *fname, 
              ann_type atype, 
              int32 nanns,
              lvar_st *l_vars)
{
    FILE *hout = NULL;
    char tmp_atype[10];
    char tmp_html[1024];
    FILE *ret_value = NULL;

    switch((int32)atype)
      {
      case AN_DATA_LABEL:
          strcpy(tmp_atype,"dl.html");
          break;
      case AN_DATA_DESC:
          strcpy(tmp_atype,"dd.html");
          break;
      case AN_FILE_LABEL:
          strcpy(tmp_atype,"fl.html");
          break;
      case AN_FILE_DESC:
          strcpy(tmp_atype,"fd.html");
          break;
      default:
          HE_REPORT_RETURN("Bad annotation type for this call",NULL);
      }

    /* Create name for HTML file */
    if (l_vars->html_dir == NULL)
        sprintf(tmp_html,"%s_%s",fname,tmp_atype);
    else
        sprintf(tmp_html,"%s/%s_%s",l_vars->html_dir,l_vars->f_name,tmp_atype);
            
    DBUG_PRINT(1,(LOGF," hdf html file name %s \n", tmp_html));
          
    /* Open temproary file to write HTML description of HDF/netCDF file */
    if (!(hout = fopen(tmp_html, "w")))
      {
          ret_value = NULL;
          goto done;
      }
  
    /* Write MIME header */
    if (write_html_header(hout, TEXT_HTML,l_vars) == FAIL)
      {
          gateway_err(hout,"init_ann_html: writing HTML headr",0,l_vars);
          ret_value = NULL;
          goto done;
      }

    if(l_vars->hdf_path_r != NULL)
        fprintf(hout,"%s Annotation%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                (nanns > 1 ? "These" : "This"),(nanns > 1 ? "s" : ""),
                l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
    else
        fprintf(hout,"%s Annotation%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                (nanns > 1 ? "These" : "This"),(nanns > 1 ? "s" : ""),
                l_vars->cgi_path,l_vars->f_path_r,l_vars->f_name,l_vars->f_name);

    /* set return value */
    ret_value = hout;

  done:
    if (ret_value == NULL)
      {
      }
  
    return ret_value;
} /* init_ann_html() */

/*----------------------------------------------------------------------- 
 NAME
       print_ann  - prints annotation out
 DESCRIPTION
       dump info about the selected annotation given tag/ref
 RETURNS
       SUCCEED/FAIL
-----------------------------------------------------------------------*/
int
print_ann(int32 fhandle,   /* file handle */
          int32 a_ref, 
          int32 a_tag,
          intn aindx, 
          ann_type atype, 
          FILE *fout,
          lvar_st *l_vars)
{
    int32  ret;
    int32  an_handle = FAIL;
    int32  ann_handle = FAIL;
    int32  ann_len;
    char   *ann = NULL;
    char   errtmp[80];
    tan_st *an_elem = NULL;
    tan_st an2_elem;
    uint16 aref = a_ref; 
    uint16 atag = a_tag; 
    intn   ann_indx = -1; 
    int    ret_value = SUCCEED;

    ENTER(2,"print_ann");
    DBUG_PRINT(1,(LOGF, "print_ann: aindex=%d.\n", aindx)); 
    DBUG_PRINT(1,(LOGF, "print_ann: atag=%d, aref=%d.\n", atag, aref)); 
    /* find correct annotaion */
    an2_elem.atype = atype;
    an2_elem.ann_ref = aref;
    an2_elem.ann_tag = atag;
    reset_to_beginning(*(l_vars->an_list));
    an_elem = first_that(*(l_vars->an_list),an1_cmp,&an2_elem);

    if (an_elem != NULL)
      {
#if 0
          ann_indx = aindx;
#endif
          ann_indx = an_elem->index; /* set index */

          /* start Anotation interface */
          if ((an_handle = ANstart(fhandle)) == FAIL)
            {
                gateway_err(l_vars->hfp,"sdbGrokRef: starting Annotations",0,l_vars);
                ret_value = FAIL;
                goto done;
            }
                
          /* Select annotation to print */
          if ((ann_handle = ANselect(an_handle, ann_indx, atype)) == FAIL)
            {
                sprintf(errtmp,"print_ann: selecting %s from file \n",get_atype(atype));
                gateway_err(fout,errtmp,0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* get length */
          if ((ann_len = ANannlen(ann_handle)) == FAIL)
            {
                sprintf(errtmp,"print_ann: getting %s's length from file \n",get_atype(atype));
                gateway_err(fout,errtmp,0,l_vars);
                ret_value = FAIL;
                goto done;
            }
  
          /* allocate space for annotation */
          if (ann == NULL)
            {
                if ((ann = (char *)HDgetspace((ann_len+1)*sizeof(char))) 
                    == NULL)
                  {
                      sprintf(errtmp,"print_ann: allocate space(%d) for %s\n ",
                              (ann_len+1)*sizeof(char),get_atype(atype));
                      gateway_err(fout,errtmp,0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }
                HDmemset(ann,'\0', ann_len+1);
            }
      
          /* read annotation */
          if ((ret = ANreadann(ann_handle, ann, ann_len+1)) == FAIL)
            {
                sprintf(errtmp,"print_ann: reading %s\n ",get_atype(atype));
                gateway_err(fout,errtmp,0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* end access to annotation */
          if ((ret = ANendaccess(ann_handle)) == FAIL)
            {
                sprintf(errtmp,"print_ann: ending access for annotation %s",get_atype(atype));
                gateway_err(fout,errtmp,0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* Print annotation depending on type */
          switch (atype)
            {
            case AN_FILE_LABEL:
                if(l_vars->do_dump)
                    fprintf(fout, "<li> <B>%s</B><P>\n", ann);
                else
                    fprintf(fout, "The file label is <B>%s</B><P>\n", ann);
                break;
            case AN_DATA_LABEL:
                if(l_vars->do_dump)
                    fprintf(fout, "<li> <B>%s</B><P>\n", ann);
                else
                    fprintf(fout, "The data label is <B>%s</B><P>\n", ann);
                break;
            case AN_FILE_DESC:
                if(l_vars->do_dump)
                    fprintf(fout, "<li> <pre>%s</pre><P>\n", ann);
                else
                    fprintf(fout, "The File Description is<p> <pre>%s</pre><P>\n", ann);
                break;
            case AN_DATA_DESC:
                if(l_vars->do_dump)
                    fprintf(fout, "<li> <pre>%s</pre><P>\n", ann);
                else
                    fprintf(fout, "The Data Description is<p> <pre>%s</pre><P>\n", ann);
                break;
            default:
                break;
            }

      } /* if an_elm */
    else
      {
          DBUG_PRINT(1,(LOGF, "print_ann: did not find ann\n")); 
      }

#ifdef AN_DEBUG
    printf("found ann_len=%d, ann=%s\n", strlen(ann),ann);
#endif

  done:
    if (ret_value == FAIL)
      {
      }

    /* free up space for annotation */
    if (ann != NULL)
        HDfreespace(ann);

    if (an_handle != FAIL)
        ANend(an_handle);
    EXIT(2,"print_ann");
    return ret_value;
} /* print_ann() */

/*----------------------------------------------------------------------- 
 NAME
     do_anns - process all anotation in file
 DESCRIPTION
     This proceses all annotatons in the file and puts them into
     global lists.
 RETURNS
     SUCCEED/FAIL
-----------------------------------------------------------------------*/
int
do_anns(char *fname,     /* file name */
        lvar_st *l_vars)
{
    intn   i;
    FILE   *h_fp = NULL;
    int32  ret;
    int32  file_handle = FAIL;
    int32  an_handle   = FAIL;
    int32  nflabs, nfdescs, nolabs, nodescs;
    uint16 atag;
    uint16 aref;
    ann_type atype;
    int      ret_value = SUCCEED;

    ENTER(2,"do_anns");
    h_fp = l_vars->hfp;

    /* open file again */
    if ((ret = file_handle = Hopen(fname, DFACC_READ,0)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_anns: handling Annotations",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

    /* start Annotation inteface */
    if ((ret = an_handle = ANstart(file_handle)) == FAIL)
      {
          gateway_err(l_vars->hfp,"do_anns: handling Annotations",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

    /* Get Info On Annotations In File */
    if ((ret = ANfileinfo(an_handle, &nflabs, &nfdescs, &nolabs, &nodescs)) == FAIL)
      {
          gateway_err(l_vars->hfp,"Error getting Annotations info from file ",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

    DBUG_PRINT(3,(LOGF,"There Are Nflabs=%d, Nfdescs=%d, Nolabs=%d, Nodescs=%d \n",nflabs,
                  nfdescs, nolabs, nodescs));

    /* do we have any annotations */
    if (nflabs > 0 || nfdescs > 0 || nolabs > 0 || nodescs > 0)
      {
          /* initialize Annotation lists */
          if(l_vars->an_list == NULL)
            {
                /* allocate list to hold atributes */
                if ((l_vars->an_list = HDmalloc(sizeof(Generic_list))) == NULL)
                  {
                      gateway_err(l_vars->hfp,"do_anns: failed to allocate space for ann list\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }   

                /* initialize list */
                initialize_list(l_vars->an_list);
            }

          /* Lets deal with File Labels */
          if (nflabs > 0)
            {
                DBUG_PRINT(3,(LOGF,"doing file labels \n"));
                for (i = 0; i < nflabs; i++)
                  { /* get tag/ref file label */
                      if (ANget_tagref(an_handle,i,AN_FILE_LABEL,&atag,&aref) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: getting tag/ref for annotation",0,l_vars); 
                            ret_value = FAIL;
                            goto done;
                        }

                      /* add annotation to list */
                      if (add_an(l_vars->an_list,i,AN_FILE_LABEL,aref,atag,NULL,l_vars) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: failed to add annotation to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                  }
            }

          /* Lets handle File Descriptions */
          if (nfdescs > 0)
            {
                DBUG_PRINT(3,(LOGF,"doing file descs \n"));
                for (i = 0; i < nfdescs; i++)
                  { /* get tag/ref file desc */
                      if (ANget_tagref(an_handle,i,AN_FILE_DESC,&atag,&aref) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: getting tag/ref for annotation",0,l_vars); 
                            ret_value = FAIL;
                            goto done;
                        }
                      /* add annotation to list */
                      if (add_an(l_vars->an_list,i,AN_FILE_DESC,aref,atag,NULL,l_vars) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: failed to add annotation to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                  }
            }

          /* deal with data labels */
          if (nolabs > 0 )
            {
                DBUG_PRINT(3,(LOGF,"doing data labels \n"));
                for (i = 0; i < nolabs; i++)
                  { /* get tag/ref data label */
                      if (ANget_tagref(an_handle,i,AN_DATA_LABEL,&atag,&aref) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: getting tag/ref for annotation",0,l_vars); 
                            ret_value = FAIL;
                            goto done;
                        }
                      /* add annotation to list */
                      if (add_an(l_vars->an_list,i,AN_DATA_LABEL,aref,atag,NULL,l_vars) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: failed to add annotation to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                  }
            }

          /* deal with data descriptions */
          if (nodescs > 0 )
            {
                DBUG_PRINT(3,(LOGF,"doing data descs \n"));
                for (i = 0; i < nodescs; i++)
                  { /* get tag/ref data descritpion */
                      if (ANget_tagref(an_handle,i,AN_DATA_DESC,&atag,&aref) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: getting tag/ref for annotation",0,l_vars); 
                            ret_value = FAIL;
                            goto done;
                        }
                      /* add annotation to list */
                      if (add_an(l_vars->an_list,i,AN_DATA_DESC,aref,atag,NULL,l_vars) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: failed to add annotation to list \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                  }
            }

          /* 
           * now create sepearte lists for each annotation type 
           *  i.e file labels/descs data labels/desc
           */

          /* create file label list */
          if (nflabs > 0 )
            {
                /* get list of file annotations */
                if(l_vars->flan_list == NULL)
                  {
                      /* allocate list to hold annotations */
                      if ((l_vars->flan_list = HDmalloc(sizeof(Generic_list))) == NULL)
                        {
                            gateway_err(l_vars->hfp,"do_ann: failed to allocate space for flann list\n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   
                  }
                atype = AN_FILE_LABEL;
                *(l_vars->flan_list) = all_such_that(*(l_vars->an_list),an_cmp,&atype);
            }

          /* create file description list */
          if (nfdescs > 0)
            {
                /* get list of file annotations */
                if(l_vars->fdan_list == NULL)
                  {
                      /* allocate list to hold annotations */
                      if ((l_vars->fdan_list = HDmalloc(sizeof(Generic_list))) == NULL)
                        {
                            gateway_err(l_vars->hfp,"do_ann: failed to allocate space for fdann list\n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   
                  }
                atype = AN_FILE_DESC;
                *(l_vars->fdan_list) = all_such_that(*(l_vars->an_list),an_cmp,&atype);
            }

          /* craete date label list */
          if (nolabs > 0 )
            {
                /* get list of data annotations */
                if(l_vars->olan_list == NULL)
                  {
                      /* allocate list to hold atributes */
                      if ((l_vars->olan_list = HDmalloc(sizeof(Generic_list))) == NULL)
                        {
                            gateway_err(l_vars->hfp,"do_anns: failed to allocate space for olann list\n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   
                  }
                atype = AN_DATA_LABEL;
                *(l_vars->olan_list) = all_such_that(*(l_vars->an_list),an_cmp,&atype);
            }

          /* care data description list */
          if ( nodescs > 0)
            {
                /* get list of data annotations */
                if(l_vars->odan_list == NULL)
                  {
                      /* allocate list to hold atributes */
                      if ((l_vars->odan_list = HDmalloc(sizeof(Generic_list))) == NULL)
                        {
                            gateway_err(l_vars->hfp,"do_anns: failed to allocate space for odann list\n",0,l_vars);
                            ret_value = FAIL; 
                            goto done;
                        }   
                  }
                atype = AN_DATA_DESC;
                *(l_vars->odan_list) = all_such_that(*(l_vars->an_list),an_cmp,&atype);
            }

      } /* end if nflabs, nfdescs, nolabs, nodescs */


  done:
    if (ret_value == FAIL)
      { /* failure cleanup */
          cleanup_an(l_vars);
      }

    /* clean up */
    if (file_handle != FAIL)
        Hclose(file_handle);
    if (an_handle != FAIL)
        ANend(an_handle);

    EXIT(2,"do_anns");
    return ret_value;
} /* do_anns() */

/*----------------------------------------------------------------------- 
 NAME
       dump_anns
 DESCRIPTION
       Print out info about file labels and descriptions
 RETURNS
       SUCCEED/FAIL
-----------------------------------------------------------------------*/
int
dump_anns(char *fname,     /* file name */
          lvar_st *l_vars)
{
    intn    i;
    FILE   *h_fp = NULL;
    int32   an_handle;
    int32   nflabs, nfdescs, nolabs, nodescs;
    uint16  atag;
    uint16  aref;
    tan_st  *an_elem = NULL;
    int ret_value = SUCCEED;

    ENTER(2,"dump_anns");
    /* set number of annotations of each type */
    if(l_vars->flan_list != NULL)
        nflabs = num_of_objects(*(l_vars->flan_list));
    else
        nflabs = 0;
    if(l_vars->fdan_list != NULL)
        nfdescs = num_of_objects(*(l_vars->fdan_list));
    else
        nfdescs = 0;
    if(l_vars->olan_list != NULL)
        nolabs = num_of_objects(*(l_vars->olan_list));
    else
        nolabs = 0;
    if(l_vars->odan_list != NULL)
        nodescs = num_of_objects(*(l_vars->odan_list));
    else
        nodescs = 0;

    DBUG_PRINT(3,(LOGF,"dump_anns: There Are Nflabs=%d, Nfdescs=%d, Nolabs=%d, Nodescs=%d \n",nflabs,
                  nfdescs, nolabs, nodescs));

    /* Lets deal with File Labels */
    if (nflabs > 0)
      {
          DBUG_PRINT(3,(LOGF,"dump_anns: doing file labels \n"));
          if (l_vars->do_dump)
            { /* Create HTML file for file Labels */
                if ((h_fp = init_ann_html(fname,AN_FILE_LABEL,nflabs,l_vars)) == NULL)
                  {
                      gateway_err(l_vars->hfp,"dump_anns: Error creating HTML file for annotations",
                                  0,l_vars); 
                      ret_value = FAIL;
                      goto done;
                  }
            }
          else
              h_fp = l_vars->hfp;
      
          /* setup HTML for file labels */
          if (nflabs != 1)
              fprintf(h_fp, "This file has the following file label%s: %s", 
                      (nflabs == 1 ? "": "s"),(l_vars->do_dump ? "<ol>":"" ));

          if (nflabs > 1 && l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                        l_vars->h_env->path_info);
                fprintf(h_fp, "<SELECT NAME=\"Ann\" SIZE=%d>\n",(nflabs>1?3:2));
            }

          if(l_vars->do_dump)
            {
#if 0
                /* read file labels */
                for (i = 0; i < nflabs; i++)
                  { /* select file label */
                      print_ann(an_handle,i,AN_FILE_LABEL,h_fp,l_vars);
                  }
#endif
            }
          else if(nflabs > 1 && l_vars->display_mode == 1)
            {
                DBUG_PRINT(3,(LOGF,"dump_anns: an_handle=%d, nflabs=%d\n",an_handle,nflabs));
                for (i = 0; i < nflabs; i++)
                  { 
                      /* get annotation from list */
                      an_elem = next_in_list(*(l_vars->flan_list));
                      atag = an_elem->ann_tag;
                      aref = an_elem->ann_ref;
                      if(i== 0)
                          fprintf(h_fp,"<OPTION SELECTED VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> File Label %d \n",
                                  atag,aref,i,i);
                      else
                          fprintf(h_fp,"<OPTION VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> File Label %d \n",
                                  atag,aref,i,i);
                  }
                fprintf(h_fp, "</SELECT><P>");
            }
          else if(nflabs == 1 && l_vars->display_mode == 1)
            {
#if 0
                print_ann(an_handle,0,AN_FILE_LABEL,h_fp,l_vars);
#endif
            }

          if (nflabs > 1 && l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "To select a particular annotation, press this button: ");
                fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Select Ann\">. <P>\n");
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\"> \n",
                        l_vars->f_name);
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                        2);
                fprintf(h_fp, "</FORM>\n");
            }

          if (l_vars->do_dump)
            {
                fprintf(h_fp,"</ol>");
                fclose(h_fp);
            }
      } /* end if nflabs */

    /* Lets handle File Descriptions */
    if (nfdescs > 0)
      {
          DBUG_PRINT(3,(LOGF,"dump_anns: doing file descs \n"));
          if (l_vars->do_dump)
            { /* create HTML file fore file descriptions */
                if ((h_fp = init_ann_html(fname,AN_FILE_DESC,nfdescs,l_vars)) == NULL)
                  {
                      gateway_err(l_vars->hfp,"dump_anns: creating HTML file for annotations",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }
            }
          else
              h_fp = l_vars->hfp;

          fprintf(h_fp, "This file has the following file description%s: %s", 
                  (nfdescs == 1 ? "": "s"),(l_vars->do_dump ? "<ol>":"" ));

          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                        l_vars->h_env->path_info);
                fprintf(h_fp, "<SELECT NAME=\"Ann\" SIZE=%d>\n",(nfdescs>1?3:2));
            }

          if(l_vars->do_dump)
            {
#if 0
                /* read file descriptions */
                for (i = 0; i < nfdescs; i++)
                  { /* select file description */
                      print_ann(an_handle,i,AN_FILE_DESC,h_fp,l_vars);
                  } /* end for nfdescs */
#endif
            }
          else if(l_vars->display_mode == 1)
            {
                for (i = 0; i < nfdescs; i++)
                  { 
                      /* get annotation from list */
                      an_elem = next_in_list(*(l_vars->fdan_list));
                      atag = an_elem->ann_tag;
                      aref = an_elem->ann_ref;
                      if (i==0)
                          fprintf(h_fp,"<OPTION SELECTED VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> File Description %d \n",
                                  atag,aref,i,i);
                      else
                          fprintf(h_fp,"<OPTION VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> File Description %d \n",
                                  atag,aref,i,i);
                  }
                fprintf(h_fp, "</SELECT><P>");
            }

          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "To select a particular annotation, press this button: ");
                fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Select Ann\">. <P>\n");
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\"> \n",
                        l_vars->f_name);
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                        2);
                fprintf(h_fp, "</FORM>\n");
            }

          if (l_vars->do_dump)
            {
                fprintf(h_fp,"</ol>");
                fclose(h_fp);
            }
      } /* end if nfdescs */

#if 0
    /* Not used currently. We only print them out when
     * we are dumping only since we print this info out only
     * we are accessing each object i.e. annotation goes w/ object */
    if (nolabs > 0 && l_vars->do_dump)
      {
          DBUG_PRINT(3,(LOGF,"doing data labels \n"));
          if (l_vars->do_dump)
            { /* Create HTML file for Data labels */
                if ((h_fp = init_ann_html(fname,AN_DATA_LABEL,nolabs,l_vars)) == NULL)
                  {
                      gateway_err(l_vars->hfp,"do_anns: creating HTML file for annotations",0,l_vars); 
                      return FAIL;
                  }
            }
          else
              h_fp = l_vars->hfp; 

          fprintf(h_fp, "This file has the following data label%s: <ol>", 
                  (nolabs == 1 ? "": "s"));

          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                        l_vars->h_env->path_info);
                fprintf(h_fp, "<SELECT NAME=\"Ann\" SIZE=3>\n");
            }

          if(l_vars->do_dump)
            {
#if 0
                /* read data labels */
                for (i = 0; i < nolabs; i++)
                  { /* select data label */
                      print_ann(an_handle,i,AN_DATA_LABEL,h_fp,l_vars);
                  } /* end for nolabs */
#endif
            }
          else if(l_vars->display_mode == 1)
            {
                for (i = 0; i < nolabs; i++)
                  { 
#if 0
                      /* select data label */
                      if (ANget_tagref(an_handle,i,AN_DATA_LABEL,&atag,&aref) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: getting tag/ref for annotation",0,l_vars); 
                            return FAIL;
                        }
#endif
                      if(i==0)
                          fprintf(h_fp,"<OPTION SELECTED VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> Data Label %d \n",
                                  atag,aref,i,i);
                      else
                          fprintf(h_fp,"<OPTION VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> Data Label %d \n",
                                  atag,aref,i,i);
                  }
                fprintf(h_fp, "</SELECT><P>");
            }

          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "To select a particular annotation, press this button: ");
                fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Select Ann\">. <P>\n");
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\"> \n",
                        l_vars->f_name);
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                        2);
                fprintf(h_fp, "</FORM>\n");
            }

          if (l_vars->do_dump)
            {
                fprintf(h_fp,"</ol>");
                fclose(h_fp);
            }
      } /* end if nolabs */


    /* When we process Data descriptions we only print them out when
     * we are dumping only since we print this info out only
     * we are accessing each object i.e. annotation goes w/ object */
    if (nodescs > 0 && l_vars->do_dump)
      {
          DBUG_PRINT(3,(LOGF,"doing data descs \n"));
          if (l_vars->do_dump)
            { /* Create HTML file for data descriptions */
                if ((h_fp = init_ann_html(fname,AN_DATA_DESC,nodescs,l_vars)) == NULL)
                  {
                      gateway_err(h_fp,"do_anns: creating HTML file for annotations",0,l_vars); 
                      return FAIL;
                  }
            }
          else
              h_fp = l_vars->hfp;

          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "<FORM METHOD=\"POST\" ");
                fprintf(h_fp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                        l_vars->h_env->path_info);
                fprintf(h_fp, "<SELECT NAME=\"Ann\" SIZE=3>\n");
            }

          fprintf(h_fp, "This file has the following data description%s: <ol>", 
                  (nodescs == 1 ? "": "s"));

          if(l_vars->do_dump)
            {
#if 0
                /* read data descriptions */
                for (i = 0; i < nodescs; i++)
                  { /* select data description */
                      print_ann(an_handle,i,AN_DATA_DESC,h_fp,l_vars);
                  } /* end for nodescs */
#endif
            }
          else if(l_vars->display_mode == 1)
            {
                for (i = 0; i < nodescs; i++)
                  { 
#if 0
                      /* select data descritpion */
                      if (ANget_tagref(an_handle,i,AN_DATA_DESC,&atag,&aref) == FAIL)
                        {
                            gateway_err(h_fp,"do_anns: getting tag/ref for annotation",0,l_vars); 
                            return FAIL;
                        }
#endif
                      if (i==0)
                          fprintf(h_fp,"<OPTION SELECTED VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> Data Description %d \n",
                                  atag,aref,i,i);
                      else
                          fprintf(h_fp,"<OPTION VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> Data Description %d \n",
                                  atag,aref,i,i);
                  }
                fprintf(h_fp, "</SELECT><P>");
            }

          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                fprintf(h_fp, "To select a particular annotation, press this button: ");
                fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Select Ann\">. <P>\n");
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\"> \n",
                        l_vars->f_name);
                fprintf(h_fp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                        2);
                fprintf(h_fp, "</FORM>\n");
            }

          if (l_vars->do_dump)
            {
                fprintf(h_fp,"</ol>");
                fclose(h_fp);
            }
      } /* end if nodescs */

    /* Clean up */
    if (ann_label != NULL)
        HDfreespace(ann_label);
    if (ann_desc != NULL)
        HDfreespace(ann_desc);

    /* close file */
    ANend(an_handle);
    Hclose(file_handle);

#endif /* commented out data label/desc part */

  done:
    if (ret_value == FAIL)
      {
      }
    EXIT(2,"dump_anns");
    return ret_value;
} /* dump_anns() */


/*------------------------   NOT USED   ---------------------------
 NAME
       print_desc
 DESCRIPTIOn
        Print out labels and descriptions for this item
 RETURNS
       SUCCEED/FAIL
------------------------------------------------------------------*/
int
print_desc(char *fname, 
           uint16 tag, 
           uint16 ref, 
           char *name,      /* text description of object */
           FILE *fout,
           lvar_st *l_vars)
{
    int   i;
    int32 ret;
    int32 file_handle = FAIL; 
    int32 an_handle   = FAIL;
    int32 *dlabels = NULL;
    int32 *ddescs  = NULL;
    int32 ann_len;
    char  *ann_label = NULL;
    char  *ann_desc  = NULL;
    uint16 atag, aref;
    intn  num_dlabels;
    intn  num_ddescs;
    int ret_value = SUCCEED;

    ENTER(2,"print_desc");
    DBUG_PRINT(1,(LOGF, "print_desc : display_mode=%d \n", l_vars->display_mode));
    DBUG_PRINT(1,(LOGF, "print_desc : fname=%s\n", fname));
    DBUG_PRINT(1,(LOGF, "print_desc : name=%s\n", name));

    if((file_handle = Hopen(fname, DFACC_READ, 0)) == FAIL)
      {
          gateway_err(fout,"print_desc: opening file",1,l_vars);
          ret_value = FAIL;
          goto done;
      }

    if ((an_handle = ANstart(file_handle)) == FAIL)
      {
          gateway_err(fout,"print_desc: Starting annotation interface",0,l_vars);
          ret_value =  FAIL;
          goto done;
      }

    /* Get number of data labels with this tag/ref */
    num_dlabels = ret = ANnumann(an_handle, AN_DATA_LABEL, tag, ref);

    /* Get number of data descriptions with this tag/ref */
    num_ddescs = ret = ANnumann(an_handle, AN_DATA_DESC, tag, ref);

    /* If we don't have any labels or descriptions we are done */
    if (num_dlabels == 0 && num_ddescs == 0)
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* allocate space for list of label annotation id's with this tag/ref */
    if (num_dlabels != 0)
      {
          if ((dlabels = (int32 *)HDmalloc(num_dlabels * sizeof(int32))) == NULL)
            {
                gateway_err(fout,"print_desc: failed to allocate space to hold data label ids\n",0,l_vars);
                ret_value =  FAIL;
                goto done;
            }
          /* get list of label annotations id's with this tag/ref */
          if (ANannlist(an_handle, AN_DATA_LABEL, tag, ref, dlabels) != num_dlabels)
            {
                gateway_err(fout,"print_desc: getting number of data labels for tag/ref \n",0,l_vars); 
                ret_value = FAIL;
                goto done;
            }
          DBUG_PRINT(1,(LOGF,"print_desc: num_dlabels=%d\n", num_dlabels));
      }

    /* allocate space for list of description annotation id's with this tag/ref */
    if (num_ddescs != 0)
      {
          if ((ddescs = (int32 *)HDmalloc(num_ddescs * sizeof(int32))) == NULL)
            {
                gateway_err(fout,"print_desc: failed to allocate space to hold data descs ids\n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* get list of description annotations id's with this tag/ref */
          if (ANannlist(an_handle, AN_DATA_DESC, tag, ref, ddescs) != num_ddescs)
            {
                gateway_err(fout,"print_desc: getting number of data descriptions for tag/ref \n",0,l_vars); 
                ret_value = FAIL;
                goto done;
            }
          DBUG_PRINT(1,(LOGF,"print_desc: num_ddescs=%d\n", num_ddescs));
      }

    /* loop through label list */
    for (i = 0; i < num_dlabels; i++)
      {
          if ((ann_len = ANannlen(dlabels[i])) == FAIL)
            {
                gateway_err(fout,"print_desc: getting data label length\n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }
        
          /* allocate space for label */
          if (ann_label == NULL)
            {
                if ((ann_label = (char *)HDmalloc((ann_len+1)*sizeof(char))) == NULL)
                  {
                      gateway_err(fout,"print_desc: failed to allocate space to hold data label \n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }
                HDmemset(ann_label,'\0', ann_len+1);
            }
      
          /* read label */
          if (ANreadann(dlabels[i], ann_label, ann_len+1) == FAIL)
            {
                gateway_err(fout,"print_desc: reading data label \n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          ANendaccess(dlabels[i]);

          /* Print annotation in HTML form */
          if (i == 0)
              fprintf(fout, "<P> This %s was given the label%s : <B>%s</B> \n", 
                      name, (num_dlabels == 1 ? "":"s"),ann_label);
          else
              fprintf(fout, ", <B>%s</B> \n", ann_label);

          HDfree(ann_label);
          ann_label = NULL;
      } /* end for labels */

    if (num_ddescs > 0 && l_vars->display_mode == 1 && !l_vars->do_dump) 
      {
          fprintf(fout, "<FORM METHOD=\"POST\" ");
          fprintf(fout, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                  l_vars->h_env->path_info);
          fprintf(fout, "<SELECT NAME=\"Ann\" SIZE=%d>\n",(num_ddescs>1?3:2));
      }

    /* loop through desc list */
    for (i = 0; i < num_ddescs; i++)
      {
          if (l_vars->display_mode == 1 && !l_vars->do_dump) 
            {
                if (ANid2tagref(ddescs[i],&atag,&aref) == FAIL)
                  {
                      gateway_err(fout,"print_desc: getting tag/ref for annotation",0,l_vars); 
                      ret_value = FAIL;
                      goto done;
                  }

                if(i== 0)
                    fprintf(fout,"<OPTION SELECTED VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> Data description %d \n",
                            atag,aref,i,i);
                else
                    fprintf(fout,"<OPTION VALUE=\"hdfref;tag=%d,ref=%d,s=%d\"> Data description %d \n",
                            atag,aref,i,i);
            }
          else
            {
                /* get desc length */
                if ((ann_len = ANannlen(ddescs[i])) == FAIL)
                  {
                      gateway_err(fout,"print_desc: getting data description length\n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                /* allocate space for descritpion */
                if (ann_desc == NULL)
                  {
                      if ((ann_desc = (char *)HDmalloc((ann_len+1)*sizeof(char))) == NULL)
                        {
                            gateway_err(fout,"print_desc: failed to allocate space to hold data desc \n",0,l_vars);
                            ret_value = FAIL;
                            goto done;
                        }
                      HDmemset(ann_desc,'\0', ann_len+1);
                  }

                /* read description */
                if (ANreadann(ddescs[i], ann_desc, ann_len+1) == FAIL)
                  {
                      gateway_err(fout,"print_desc: reading data description \n",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                ANendaccess(ddescs[i]);

                /* Print annotation in HTML form */
                if (i == 0)
                    fprintf(fout, "<P> %s description%s : <pre> %s </pre>\n", 
                            name, (num_ddescs == 1 ? "" : "s"), ann_desc);
                else
                    fprintf(fout, " <pre>%s</pre> <P> \n", ann_desc);

                HDfree(ann_desc);
                ann_desc = NULL;
            }
      } /* end for descs */
    
    if (num_ddescs > 0 && l_vars->display_mode == 1 && !l_vars->do_dump) 
      {
          fprintf(fout, "</SELECT><P>");
          fprintf(fout, "To select a particular annotation, press this button: ");
          fprintf(fout, "<INPUT TYPE=\"submit\" VALUE=\"Select Ann\">. <P>\n");
          fprintf(fout, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\"> \n",
                  l_vars->f_name);
          fprintf(fout, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                  2);
          fprintf(fout, "</FORM>\n");
      }


  done:
    if (ret_value == FAIL)
      {
      }

    /* free space */
    if(dlabels != NULL)
        HDfree(dlabels);
    if(ddescs != NULL)
        HDfree(ddescs);
    if (ann_label != NULL)
        HDfree(ann_label);
    if (ann_desc != NULL)
        HDfree(ann_desc);

    /* close file */
    if(an_handle != FAIL)
        ANend(an_handle);
    if(file_handle != FAIL)
        Hclose(file_handle);

    EXIT(2,"print_desc");
    return ret_value;
} /* print_desc */

