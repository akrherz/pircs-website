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
static char RcsId[] = "@(#)sdb_pal.c,v 1.21 1996/04/15 17:58:08 georgev Exp";
#endif

/* sdb_pal.c,v 1.21 1996/04/15 17:58:08 georgev Exp */

/*
   LIST Functions
   --------------
      add_pal      - add palette to palette list
      pal_ann2html - Iterator function to print annotation of palette
      set_lone_pal - Iterator fcn to set this is a lone palette 
      lone_pal     - Iterator fcn to indicate whether this is a lone palette  
      free_pal     - frees palette element 
      cleanup_pal  - frees palette lists

   Regular Functions
   -----------------
      do_pals   - process palettes in file and add to global list
      dump_pals -  print out the info for Palettes in the file 

   OBSOLETE
   --------
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_pal.h"
#include "sdb_ann.h"
#include "sdb_dtm.h"
#include "sdb_util.h"
#include "glist.h"

/*------------------------------------------------------------------------- 
 NAME
      add_pal - add palette to palette list
 DESCRIPTION
      This adds palette and its info to the palette List.
 RETURNS
      SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
add_pal(Generic_list *pal_list, /* palette list */
        uint16 ref,             /* ref of palette to add */
        char *pal,              /* palette itself */
        lvar_st *l_vars)
{
    tpal_st *pal_elem = NULL;
    int    ret_value = SUCCEED;

    /* allocate space for palette element */
    if ((pal_elem = (tpal_st *)HDmalloc(sizeof(tpal_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_pal: failed to allocate space for pal_elem \n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* fill in relevant info */
    pal_elem->ref = ref;
    pal_elem->lone_pal = 1; /* yes for now */

    /* copy palette */
    HDmemcpy(pal_elem->pal, pal,768);

    /*add to list */
    add_to_beginning(*pal_list, pal_elem);
    DBUG_PRINT(3,(LOGF,"add_pal: adding palette,ref=%d \n",
                  pal_elem->ref));

  done:
    return ret_value;
} /* add_pal() */

/*------------------------------------------------------------------------- 
 NAME
       pal_ann2html - Iterator function to print annotation of palette
 DESCRIPTION
       Iterator function to print annotation of palette.
 RETURNS
       Nothing
--------------------------------------------------------------------------*/
void
pal_ann2html(void *dptr, /* palette element */
             void *args)
{
    tan_st *pal_ann = dptr;
    lvar_st *l_vars = args;

    if (pal_ann != NULL)
      {
          if (pal_ann->ann != NULL)
              fprintf(l_vars->hfp," <pre> %s </pre>",pal_ann->ann);
          else
              fprintf(l_vars->hfp," <pre> -- </pre>");
      }
} /* pal_ann2html() */

/*------------------------------------------------------------------------- 
 NAME
       set_lone_pal - Iterator fcn to set this is a lone palette 
 DESCRIPTION
       Iterator fcn to set this is a lone palette given the ref of the 
       palette.
 RETURNS
       Nothing
--------------------------------------------------------------------------*/
void
set_lone_pal(void *dptr, /* palette element from list */
             void *args /* ref to match against */ )
{
    tpal_st *pal_elem = dptr;
    uint16 *ref = args;

    if (pal_elem->ref ==  *ref)
        pal_elem->lone_pal = 0; /* no */
} /* set_lone_pal() */

/*------------------------------------------------------------------------- 
 NAME
     lone_pal - Iterator fcn to indicate whether this is a lone palette  
 DESCRIPTION
     Iterator fcn to indicate whether this is a lone palette  
 RETURNS
     1->SUCCEED, 0->FAIL
--------------------------------------------------------------------------*/
int
lone_pal(void *dptr, /* palette element to inquire */
         void *args)
{
    tpal_st *pal_elem = dptr;

    if (pal_elem->lone_pal)
        return 1;
    else
        return 0;
} /* lone_pal() */

/*------------------------------------------------------------------------- 
 NAME
    free_pal  - frees palette element 
 DESCRIPTION
    Frees palette element       
 RETURNS
    Nothing
--------------------------------------------------------------------------*/
void
free_pal(void *dptr,  /* palette element to free */
         void *args)
{
    tpal_st *pal_elem = dptr;

    if (pal_elem != NULL)
        free(pal_elem);
} /* free_pal() */

/*------------------------------------------------------------------------- 
 NAME
    cleanup_pal - frees palette lists
 DESCRIPTION
    Fress up the palette lists
 RETURNS
    Nothing
--------------------------------------------------------------------------*/
void
cleanup_pal(lvar_st *l_vars)
{
    ENTER(2,"cleanup_pal");
    /* Palette list */
    if (l_vars->pal_list != NULL)
      { /* free elements first */
          DBUG_PRINT(1,(LOGF,"cleanup_pal: destroy pal_list=%d\n", 
                        num_of_objects(*(l_vars->pal_list))));
          perform_on_list(*(l_vars->pal_list),free_pal,NULL);        
          /* destory list itself */
          destroy_list(l_vars->pal_list);

          free(l_vars->pal_list);
          l_vars->pal_list = NULL;
      }
    EXIT(2,"cleanup_pal");
} /* cleanup_pal() */

/*------------------------------------------------------------------------- 
 NAME
       do_pals - process palettes in file and add to global list
 DESCRIPTION
       Process palettes in file and add to global list
 RETURNS
       SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
do_pals(char *fname,   /* file name */
        lvar_st *l_vars)
{
    int32  count;     /* number of palettes */
    int32  i;         /* loop variable */
    uint16 ref;       /* reference number of paletter */
    int32  status;    /* flag */
    char   pal[768];   /* palette */
    FILE   *h_fp = l_vars->hfp;
    int    ret_value = SUCCEED;

    ENTER(2,"do_pals");

    /* Get number of palettes in file */
    count = DFPnpals(fname);

    if(count < 1) 
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* initialize Palette lists */
    if(l_vars->pal_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->pal_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_pals: failed to allocate space for pals list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->pal_list);
      }

    /* For each palette in the file */
    for(i = 0; i < count; i++) 
      {
          if ((status = DFPgetpal(fname, pal)) == FAIL)
            {
                gateway_err(h_fp,"do_pals: failed getting palette",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* Save last reference number */
          if ((ref = DFPlastref()) < 1)
            {
                gateway_err(h_fp,"do_pals: failed getting last ref for palette",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* add palette to list */
          if (add_pal(l_vars->pal_list,ref,pal,l_vars) == FAIL)
            {
                gateway_err(h_fp,"do_pals: failed to add palette to list \n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

      } /* end for count */

  done:
    /* failure */
    if (ret_value == FAIL)
      {
          cleanup_pal(l_vars);
      }
    EXIT(2,"do_pals");
    return ret_value;
} /* do_pals */

/*------------------------------------------------------------------------- 
 NAME
       dump_pals -  print out the info for Palettes in the file 
 DESCRIPTION
       print out the info for Palettes in the file 
 RETURNS
       SUCCEED/FAIL
--------------------------------------------------------------------------*/
int
dump_pals(char *fname,    /* file name */
          lvar_st *l_vars)
{
    char  tmp_html[1024];
    FILE  *h_fp;
    int32 count;        /* number of palettes */
    int32 i,j;            /* loop variable */
    int32 ref;          /* reference number of paletter */
    char  *pal = NULL;          /* palette */
    tpal_st *pal_elem = NULL;
    tan_st  *pal_ann = NULL;
    Generic_list *lone_pals = NULL; /* list of lone palettes */
    Generic_list *pal_data_label = NULL;
    Generic_list *pal_data_desc = NULL;
    int nlabels = 0;
    int ndescs  = 0;
    int ret_value = SUCCEED;

    ENTER(2,"dump_pals");
    /* Get number of Lone palettes in file */
    if(l_vars->pal_list != NULL)
      {
          /* initialize Palette lists */
          if(lone_pals == NULL)
            {
                /* allocate list to hold atributes */
                if ((lone_pals = HDmalloc(sizeof(Generic_list))) == NULL)
                  {
                      gateway_err(l_vars->hfp,"dump_pals: failed to allocate space for pals list\n",0,l_vars);
                      ret_value = FAIL; 
                      goto done;
                  }   
            }
          *lone_pals = all_such_that(*(l_vars->pal_list),lone_pal,NULL);
          count = num_of_objects(*lone_pals);
      }
    else
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* how many lone paletes do we have */
    if(count < 1) 
      {
          ret_value = SUCCEED;
          goto done;
      }


    if (l_vars->do_dump)
      {
          /* Create name for HTML file */
          if (l_vars->html_dir == NULL)
              sprintf(tmp_html,"%s_pl.html",fname);
          else
              sprintf(tmp_html,"%s/%s_pl.html",l_vars->html_dir,l_vars->f_name);
            
          DBUG_PRINT(1,(LOGF,"dump_pals: hdf html file name %s \n", tmp_html));
          
          /* Open temproary file to write HTML description of HDF/netCDF file */
          if (!(h_fp = fopen(tmp_html, "w")))
            {
                ret_value = FAIL;
                goto done;
            }

          /* Write MIME header */
          if (write_html_header(h_fp, TEXT_HTML,l_vars) == FAIL)
            {
                gateway_err(h_fp,"dump_pals: writing HTML headr",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          if(l_vars->hdf_path_r != NULL)
              fprintf(h_fp,"%s palette%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                      (count > 1 ? "These" : "This"),(count > 1 ? "s" : ""),
                      l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
          else
              fprintf(h_fp,"%s palette%s came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                      (count > 1 ? "These" : "This"),(count > 1 ? "s" : ""),
                      l_vars->cgi_path,l_vars->f_path_r,l_vars->f_name,l_vars->f_name);
      }
    else
        h_fp = l_vars->hfp;

    /* Print Palette header stuff in HTML */
    fprintf(h_fp, "<hr>\n");
    fprintf(h_fp, "<H2>Palettes</H2>\n");

    if(count == 1)
        fprintf(h_fp, "There is 1 lone palette in this file :\n");
    else
        fprintf(h_fp, "There are %d lone  palettes in this file :\n", count);
    fprintf(h_fp, "<UL>\n");
    DBUG_PRINT(1,(LOGF, "dump_pals: There are %d lone palettes in this file :\n", count));

    /* setup HTML table to hold palettes */
    fprintf(h_fp, "<TABLE BORDER>\n");
    fprintf(h_fp,"<caption align=\"top\">Palettes</caption> \n");
    fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH>", 
            "Labels", "Descriptions" );
    fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH> </TR> \n", 
            "Dimensions", "Palette" );

    /* For each palette in the file */
    for(i = 0; i < count; i++) 
      {
          /* get palette from list */
          pal_elem = next_in_list(*lone_pals);
          pal = pal_elem->pal;
          ref = pal_elem->ref; /* not uint16 -> int32 */

          /* get palettes labels and descriptions if any */
          if ((pal_data_label = get_data_labels(fname, DFTAG_IP8, ref, h_fp,l_vars)) == NULL)
            {
                gateway_err(h_fp,"dump_pals: error getting data labels",0,l_vars);
                ret_value = FAIL;
                goto done;
            }
          nlabels = num_of_objects(*pal_data_label);

          DBUG_PRINT(1,(LOGF, "dump_pals:found %d labels for palette,tag=%d,ref=%d \n",
                        nlabels,DFTAG_IP8,ref));    

          if (nlabels > 0)
            {
                fprintf(h_fp,"<TR ALIGN=\"center\" VALIGN=\"center\"> <TD>");
                perform_on_list(*pal_data_label,pal_ann2html,l_vars);
                fprintf(h_fp,"</TD>");
            }
          else
              fprintf(h_fp,"<TR><TD> -- </TD>");

          if ((pal_data_desc = get_data_descs(fname, DFTAG_IP8, ref, h_fp,l_vars)) == NULL)
            {
                gateway_err(h_fp,"dump_pals: error getting data descs",0,l_vars);
                ret_value = FAIL;
                goto done;
            }
          ndescs = num_of_objects(*pal_data_desc);

          DBUG_PRINT(1,(LOGF, "dump_pals:found %d descs for palette,tag=%d,ref=%d \n",
                        ndescs,DFTAG_IP8,ref));    

          /* for descriptions show a list and don't print them out */
          if (ndescs > 0)
            {
                fprintf(h_fp,"<TD>");
#if 0
                perform_on_list(*pal_data_desc,pal_ann2html,l_vars);
#endif
                fprintf(h_fp, "<UL>\n");
                for (j =0; j < ndescs; j++)
                  {
                      pal_ann = next_in_list(*pal_data_desc);
                      if (pal_ann != NULL)
                          fprintf(h_fp,"<LI><A HREF=\"%s\"> desc%d </A></LI>",
                                  obj_href(pal_ann->ann_tag,pal_ann->ann_ref,pal_ann->index,l_vars),j);
                  }
                fprintf(h_fp, "</UL>\n");
                fprintf(h_fp,"</TD>");
            }
          else
              fprintf(h_fp,"<TD> -- </TD>");

          /* Dimensions */
          fprintf(h_fp,"<TD> 256 by 3 </TD>");

          /* Print palette info in HTML form */
          fprintf(h_fp, "<TD> <IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"> </TD> </TR>\n", 
                  l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name, 
                  (int32) DFTAG_IP8, ref,0);
#if 0
          if (l_vars->do_dump)
              fprintf(h_fp, "<LI> Here's the link to the <A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"> palette </A>\n", 
                      l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name, (int32) DFTAG_IP8, ref,0);
          else
              fprintf(h_fp, "<LI> Here's what the palette looks like : <IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">\n", 
                      l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name, 
                      (int32) DFTAG_IP8, ref,0);
#endif
          DBUG_PRINT(1,(LOGF, "dump_pals: Here's what the palette looks like : <IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d\"> \n", 
                        l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name, 
                        (int32) DFTAG_IP8, ref));

#ifdef HAVE_DTM
          if (mo_dtm_out_active_p ())
              fprintf (l_vars->hfp, "(To broadcast this palette over DTM, click <A HREF=\"#hdfdtm;tag=%d,ref=%d,dtmport=%s\">here</A>.)\n", 
                       (int32) DFTAG_IP8, ref,l_vars->dtm_outport);
#endif

          /* regular cleanup of labels/descs if any */
          if (pal_data_label != NULL)
            {
                perform_on_list(*(pal_data_label),free_an,NULL);        
                destroy_list(pal_data_label);
                HDfree(pal_data_label);
                pal_data_label = NULL;
            }
          if (pal_data_desc != NULL)
            {
                perform_on_list(*(pal_data_desc),free_an,NULL);        
                destroy_list(pal_data_desc);
                HDfree(pal_data_desc);
                pal_data_desc = NULL;
            }

      } /* for loop for each palette */

    fprintf(h_fp, "</TABLE>\n");

#if 0
    if (l_vars->do_dump)
        fclose(h_fp);
    else
        fprintf(l_vars->hfp, "</UL>\n");
#endif

  done:
    /* failure */
    if (ret_value == FAIL)
      {
      }

    /* regular cleanup */
    if (lone_pals != NULL)
      {
          /* destory list itself */
          destroy_list(lone_pals);
          free(lone_pals);
      }

    /* regular cleanup */
    if (pal_data_label != NULL)
      {
          perform_on_list(*(pal_data_label),free_an,NULL);        
          destroy_list(pal_data_label);
          HDfree(pal_data_label);
          pal_data_label = NULL;
      }
    if (pal_data_desc != NULL)
      {
          perform_on_list(*(pal_data_desc),free_an,NULL);        
          destroy_list(pal_data_desc);
          HDfree(pal_data_desc);
          pal_data_desc = NULL;
      }

    EXIT(2,"dump_pals");
    return ret_value;
} /* dump_pals */
