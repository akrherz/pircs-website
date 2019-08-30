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
static char RcsId[] = "@(#)sdb_ri.c,v 1.18 1996/04/15 17:58:10 georgev Exp";
#endif

/* sdb_ri.c,v 1.18 1996/04/15 17:58:10 georgev Exp */

/*
   LIST Functions
   --------------
     add_rig      - Added raster to the raster List       
     rig_ann2html - Iterator fcn to print annotation of raster if any
     free_rig     - frees Raster element
     cleanup_rig  - Frees the raster Lists

   Regular Functions
   -----------------
     do_rigs   - Process the raster8 images in the file 
     dump_rigs - Print out the info for RIGSs in the file 

   OBSOLETE
   --------
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_ri.h"
#include "sdb_ann.h"
#include "sdb_util.h"
#include "sdb_pal.h"
#include "sdb_dtm.h"
#include "glist.h"

/*------------------------------------------------------------------------ 
 NAME
     add_rig - Added raster to the raster List       
 DESCRIPTION
     This adds the raster to the raster list
 RETURNS
     SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
add_rig(Generic_list *rig_list,  /* Raster List */
        uint16 ref,              /* ref of raster */
        uint16 pal_ref,          /* ref of palette if any */
        intn have_pal,           /* flag */
        int32 w,                 /* width */
        int32 h,                 /* height */
        lvar_st *l_vars)
{
    trig_st *rig_elem = NULL;
    int     ret_value = SUCCEED;

    /* allocate space for raster element */
    if ((rig_elem = (trig_st *)HDmalloc(sizeof(trig_st))) == NULL)
      {
          gateway_err(l_vars->hfp,"add_rig: failed to allocate space for rig_elem \n",0,l_vars);
          ret_value = FAIL; 
          goto done;
      }   

    /* fille in relavant info */
    rig_elem->ref      = ref;
    rig_elem->pal_ref  = pal_ref;
    rig_elem->have_pal = have_pal;
    rig_elem->width    = w;
    rig_elem->hieght   = h;

    /* add to list */
    add_to_beginning(*rig_list, rig_elem);
    DBUG_PRINT(3,(LOGF,"add_rig: adding raster,ref=%d \n",
                  rig_elem->ref));

  done:
    return ret_value;
} /* add_rig() */

/*------------------------------------------------------------------------ 
 NAME
     rig_ann2html - Iterator fcn to print annotation of raster if any
 DESCRIPTION
     Iterator fcn to print annotation of raster if any
 RETURNS
     Nothing
-------------------------------------------------------------------------*/
void
rig_ann2html(void *dptr, /* raster element */
             void *args)
{
    tan_st *rig_ann = dptr;
    lvar_st *l_vars = args;

    if (rig_ann != NULL)
      {
          if (rig_ann->ann != NULL)
              fprintf(l_vars->hfp," <pre> %s </pre>",rig_ann->ann);
          else
              fprintf(l_vars->hfp," <pre> -- </pre>");
      }
} /* rig_ann2html() */


/*------------------------------------------------------------------------ 
 NAME
     free_rig - frees Raster element
 DESCRIPTION
     Frees Raster element
 RETURNS
     Nothing
-------------------------------------------------------------------------*/
void
free_rig(void *dptr, /* raster element to free */
         void *args)
{
    trig_st *rig_elem = dptr;

    if (rig_elem != NULL)
        free(rig_elem);
} /* free_rig() */


/*------------------------------------------------------------------------ 
 NAME
     cleanup_rig - Frees the raster Lists
 DESCRIPTION
      Frees the raster Lists
 RETURNS
      Nothing
-------------------------------------------------------------------------*/
void
cleanup_rig(lvar_st *l_vars)
{
    ENTER(2,"cleanup_rig");
    /* Palette list */
    if (l_vars->rig_list != NULL)
      { /* free elements first */
          DBUG_PRINT(1,(LOGF,"cleanup_rig: destroy rig_list=%d\n", 
                        num_of_objects(*(l_vars->rig_list))));
          perform_on_list(*(l_vars->rig_list),free_rig,NULL);        
          /* destory list itself */
          destroy_list(l_vars->rig_list);

          free(l_vars->rig_list);
          l_vars->rig_list = NULL;
      }
    EXIT(2,"cleanup_rig");
} /* cleanup_rig() */


/*------------------------------------------------------------------------ 
 NAME
       do_rigs - Process the raster8 images in the file 
 DESCRIPTION
       Process the raster8 images in the file
 RETURNS
       SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
do_rigs(char *fname,     /* file name */
        lvar_st *l_vars)
{
    int32  count;   /* counter for images ? */
    int32  i;       /* loop variable */
    int32  ref;     /* reference number for image */
    int32  status;  /* flag */
    int32  w;       /* width dimension of image */
    int32  h;       /* height dimesnion of image */
    intn   ip;       /* pallete flag */
    uint16 pal_ref;
    FILE   *h_fp  = NULL;
    int ret_value = SUCCEED;

    ENTER(2,"do_rigs");

    /* get number of raster images in file */
    if ((count = DFR8nimages(fname)) < 1)
      {
          ret_value = SUCCEED; 
          goto done;
      }

    /* initialize Palette lists */
    if(l_vars->rig_list == NULL)
      {
          /* allocate list to hold atributes */
          if ((l_vars->rig_list = HDmalloc(sizeof(Generic_list))) == NULL)
            {
                gateway_err(l_vars->hfp,"do_rigs: failed to allocate space for rigs list\n",0,l_vars);
                ret_value = FAIL; 
                goto done;
            }   

          /* initialize list */
          initialize_list(l_vars->rig_list);
      }

    /* For each image in the file */
    for(i = 0; i < count; i++) 
      { /* Get width and height dimensions of image */
          if ((status = DFR8getdims(fname, &w, &h, &ip)) == FAIL)
            {
                gateway_err(h_fp,"do_rigs: failed to get dimension info on image",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* get ref of palette if any*/
          if (ip)
              DFR8getpalref(&pal_ref);
          else
              pal_ref = 0;

          /* Save last reference number */
          if ((ref = DFR8lastref()) == FAIL)
            {
                gateway_err(h_fp,"do_rigs: failed to get last ref for image",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* add rig to list */
          if (add_rig(l_vars->rig_list,ref,pal_ref,ip,w,h,l_vars) == FAIL)
            {
                gateway_err(h_fp,"do_rigs: failed to add rig to list \n",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          /* now update palette list to reflect those that do not belong
           * to any image */
          if (l_vars->pal_list != NULL && ip)
              perform_on_list(*(l_vars->pal_list),set_lone_pal, &pal_ref);
      } /* end for each raster */

  done:
    /* failure */
    if (ret_value == FAIL)
      {
          cleanup_rig(l_vars);
      }
    EXIT(2,"do_rigs");
    return ret_value;
} /* do_rigs() */

/*------------------------------------------------------------------------ 
 NAME
       dump_rigs - Print out the info for RIGSs in the file 
 DESCRIPTION
       Print out the info for RIGSs in the file 
 RETURNS
       SUCCEED/FAIL
-------------------------------------------------------------------------*/
int
dump_rigs(char *fname,    /* file name */
          lvar_st *l_vars)
{
    int32 count;   /* counter for images ? */
    int32 i,j;     /* loop variable */
    int32 ref;     /* reference number for image */
    int32 w;       /* width dimension of image */
    int32 h;       /* height dimesnion of image */
    intn  ip;      /* */
    intn  nlabels = 0;
    intn  ndescs  = 0;
    char  tmp_html[1024];
    FILE         *h_fp     = NULL;
    trig_st      *rig_elem = NULL;
    Generic_list *rig_data_label = NULL;
    Generic_list *rig_data_desc  = NULL;
    tan_st       *rig_ann        = NULL;
    int ret_value = SUCCEED;

    ENTER(2,"dump_rigs");
    /* Get number of raster 8 in file */
    if(l_vars->rig_list != NULL)
        count = num_of_objects(*(l_vars->rig_list));
    else
      {
          ret_value = SUCCEED;
          goto done;
      }

    /* how many rasters do we have */
    if (count < 1)
      {
          ret_value = SUCCEED; 
          goto done;
      }

    DBUG_PRINT(2,(LOGF,"dump_rigs: count=%d\n", count));
    if (l_vars->do_dump)
      {
          /* Create name for HTML file */
          if (l_vars->html_dir == NULL)
              sprintf(tmp_html,"%s_ri.html",fname);
          else
              sprintf(tmp_html,"%s/%s_ri.html",l_vars->html_dir,l_vars->f_name);
            
          DBUG_PRINT(1,(LOGF,"dump_rigs: hdf html file name %s \n", tmp_html));
          
          /* Open temproary file to write HTML description of HDF/netCDF file */
          if (!(h_fp = fopen(tmp_html, "w")))
            {
                ret_value = FAIL;
                goto done;
            }

          /* Write MIME header */
          if (write_html_header(h_fp, TEXT_HTML,l_vars) == FAIL)
            {
                gateway_err(h_fp,"dump_rigs: writing HTML headr",0,l_vars);
                ret_value = FAIL;
                goto done;
            }

          if(l_vars->hdf_path_r != NULL)
              fprintf(h_fp,"These raster images came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                      l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
          else
              fprintf(h_fp,"These raster images came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                      l_vars->cgi_path,l_vars->f_path_r,l_vars->f_name,l_vars->f_name);
      }
    else
        h_fp = l_vars->hfp;

    /* Print Image header stuff in HTML  */
    fprintf(h_fp, "<hr>\n");
    fprintf(h_fp, "<H2>Images</H2>\n");

    if(count == 1)
        fprintf(h_fp, "There is 1 image in this file :\n");
    else
        fprintf(h_fp, "There are %d images in this file,\n", count);
    fprintf(h_fp," They have been subsampled for display: \n" );
#if 0
    fprintf(h_fp, "<UL>\n");
#endif
    DBUG_PRINT(1,(LOGF, "There are %d images in this file :\n", count));    

    /* Prepare HTML table to hold Images */
    fprintf(h_fp, "<TABLE BORDER>\n");
    fprintf(h_fp,"<caption align=\"top\">Images</caption> \n");
    fprintf(h_fp, "<TR><TH> %s </TH> <TH> %s </TH>", 
            "Labels", "Descriptions" );
    fprintf(h_fp, "<TH> %s </TH> <TH> %s </TH>", 
            "Dimensions", "Image(subsampled)" );
    fprintf(h_fp, "<TH> %s </TH> </TR> \n", 
            "Palette");

    /* For each image in the file */
    for(i = 0; i < count; i++) 
      { 
          /* get palette from list */
          rig_elem = next_in_list(*(l_vars->rig_list));
          w   = rig_elem->width;
          h   = rig_elem->hieght;
          ip  = rig_elem->have_pal;
          ref = rig_elem->ref; /* not uint16 -> int32 */

          /* Print out image info in HTML depending on user prefrences */
          if (l_vars->do_dump)
            {
                fprintf(h_fp, "<LI> This : <A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"> image </A> has dimensions %d by %d\n", 
                        l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name, (int32)DFTAG_RIG, ref, 1, w, h);
                if(ip) 
                    fprintf(h_fp, " and also has a palette.<p>\n");
                else
                    fprintf(h_fp,".<p>\n");
            }
          else /* not dumping */
            {
                /* get rig labels and descriptions if any */
                if ((rig_data_label = get_data_labels(fname, DFTAG_RIG, ref, h_fp,l_vars)) == NULL)
                  {
                      gateway_err(h_fp,"dump_rigs: error getting data labels",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }
                nlabels = num_of_objects(*rig_data_label);

                DBUG_PRINT(1,(LOGF, "dump_rigs: found %d labels for image,tag=%d,ref=%d \n",
                              nlabels,DFTAG_RIG,ref));    

                if (nlabels > 0)
                  {
                      fprintf(h_fp,"<TR ALIGN=\"center\" VALIGN=\"center\"><TD>");
                      perform_on_list(*rig_data_label,rig_ann2html,l_vars);
                      fprintf(h_fp,"</TD>");
                  }
                else
                    fprintf(h_fp,"<TR><TD> -- </TD>");

                if ((rig_data_desc = get_data_descs(fname, DFTAG_RIG, ref, h_fp,l_vars)) == NULL)
                  {
                      gateway_err(h_fp,"dump_rigs: error getting data descs",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }
                ndescs = num_of_objects(*rig_data_desc);

                DBUG_PRINT(1,(LOGF, "dump_rigs: found %d descs for image,tag=%d,ref=%d \n",
                              ndescs,DFTAG_RIG,ref));    

                /* for descriptions dont print them out, list them */
                if (ndescs > 0)
                  {
                      fprintf(h_fp,"<TD>");
#if 0
                      perform_on_list(*rig_data_desc,rig_ann2html,l_vars);
#endif
                      fprintf(h_fp, "<UL>\n");
                      for (j =0; j < ndescs; j++)
                        {
                            rig_ann = next_in_list(*rig_data_desc);
                            if (rig_ann != NULL)
                                fprintf(h_fp,"<LI><A HREF=\"%s\"> desc%d </A></LI>",
                                        obj_href(rig_ann->ann_tag,rig_ann->ann_ref,rig_ann->index,l_vars),j);
                        }
                      fprintf(h_fp, "</UL>\n");
                      fprintf(h_fp,"</TD>");
                  }
                else
                    fprintf(h_fp,"<TD> -- </TD>");

#if 0
                fprintf(h_fp,"<LI> This image : <A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"><IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"></A> has dimensions %d by %d\n", 
                        l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                        (int32) DFTAG_RIG, ref, 0, l_vars->h_env->script_name, 
                        l_vars->h_env->path_info,l_vars->f_name,(int32)DFTAG_RIG, ref, 1, w, h);

                if(w > hdfImageSize || h > hdfImageSize)
                    fprintf(h_fp, "  (the image has been subsampled for display)");
                fprintf(h_fp, ".  ");
            
                if(ip) 
                  {
                      fprintf(h_fp, "There is also a palette associated with this image.\n");
                      fprintf(h_fp, " Here's what the palette looks like : <IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\">\n", 
                              l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name, 
                              (int32) DFTAG_IP8, rig_elem->pal_ref,0);
                  }
#endif
                /* Dimensions */
                fprintf(h_fp,"<TD> %d by %d </TD>",w,h);
                fprintf(h_fp,"<TD><A HREF=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"><IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"></A>", 
                        l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                        (int32) DFTAG_RIG, ref, 0, l_vars->h_env->script_name, 
                        l_vars->h_env->path_info,l_vars->f_name,(int32)DFTAG_RIG, ref, 1);

#ifdef HAVE_DTM
                /* DTM stuff */
                fprintf(l_vars->hfp, "<FORM METHOD=\"POST\" ");
                fprintf(l_vars->hfp, "ACTION=\"%s%s\">\n",l_vars->h_env->script_name,
                        l_vars->h_env->path_info);
                fprintf(h_fp, "<LI>DTMPORT:");
                fprintf(h_fp, "<INPUT NAME=\"dtmport\" VALUE=%s> <P>\n",":0" );
                fprintf(l_vars->hfp, "To broadcast this image over DTM, ");
                fprintf(l_vars->hfp, "press this button: ");
                fprintf(l_vars->hfp, "<INPUT TYPE=\"submit\" VALUE=\"Select image\">. <P>\n");
                fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"hdfref\" VALUE=\";tag=%d,ref=%d,s=%d\">\n",
                        DFTAG_RIG,ref,0);
                fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" ");
                fprintf(l_vars->hfp, "VALUE=\"%s\">\n", l_vars->f_name);
                fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_mode\" VALUE=\"%d\">\n",
                        2);
                fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"display_type\" VALUE=\"%d\">\n",
                        1);
                fprintf(l_vars->hfp, "</FORM>\n");

#if 0
                if (mo_dtm_out_active_p ())
                    fprintf(h_fp,"(To broadcast this image over DTM, click <A HREF=\"%s\"> here </A>)</TD>",
                            obj_href(DFTAG_RIG,ref,0,l_vars));
#endif

#else /* !HAVE_DTM */
                fprintf(h_fp,"</TD>");
#endif /* !HAVE_DTM */

                /* deal with palett if any */
                if (ip)
                  { /* yes */
                      fprintf(h_fp, "<TD> <IMG SRC=\"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d\"> </TD> </TR>", 
                              l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name, 
                              (int32) DFTAG_IP8, rig_elem->pal_ref,0);
                  }
                else
                    fprintf(h_fp,"<TD> -- </TD> </TR>");
            } /* end else not dumping */

          /* cleanup of lables/descriptons */
          if (rig_data_label != NULL)
            {
                perform_on_list(*(rig_data_label),free_an,NULL);        
                destroy_list(rig_data_label);
                HDfree(rig_data_label);
                rig_data_label = NULL;
            }
          if (rig_data_desc != NULL)
            {
                perform_on_list(*(rig_data_desc),free_an,NULL);        
                destroy_list(rig_data_desc);
                HDfree(rig_data_desc);
                rig_data_desc = NULL;
            }

      } /* for loop for each image */

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

    /* cleanup of labels/descs */
    if (rig_data_label != NULL)
      {
          perform_on_list(*(rig_data_label),free_an,NULL);        
          destroy_list(rig_data_label);
          HDfree(rig_data_label);
          rig_data_label = NULL;
      }
    if (rig_data_desc != NULL)
      {
          perform_on_list(*(rig_data_desc),free_an,NULL);        
          destroy_list(rig_data_desc);
          HDfree(rig_data_desc);
          rig_data_desc = NULL;
      }
    EXIT(2,"dump_rigs");
    return ret_value;
} /* dump_rigs */
