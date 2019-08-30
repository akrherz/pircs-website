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
 
/**************************************************************************
  Original code framework came from X-Mosaic.
     Some of the old code exits and should be cleaned up, especially the
  parts dealing with Vgroup/Vdatas, the Image handling code
  and the SDS and attribute handling.
     Bear in mind this whole program should be rewritten.
  This code defintely should NOT be used as an example of *good* 
  programming with HDF...prototypes should ways be thrown out:-)


  This module provides access routines to take an HDF file and return an
  HTML description of its contents.

MAIN MODULES
   sdbGetImage       - extracts selected image from file
   sdbGrokFile       - Process whole HDF/netCDF file and find objects at
                       top level of hierarchy.
   myGetGIFPixel     - returns value of next pixel(used by gif routines)
   fitsGrokHDU       - get the image from the current PHDU or IMAGE extension
   sdbGrokRef        - Process HDF/netCDF from a given point in the file
                       i.e. from the reference number. Used primarily
                       ot navigate through a forest of Vgroups.
   sdbGrokImage      - extracts subsampled image from file
  
   fitsGrokTab       - get the ascii table information
   fitsGrokBinTab    - get the binary table information
   listhead          - get summary information of the fits file
   sdsGetImage       - convert sds to image
   sdsGrokRef        - get image based on the Query string
   sdsGrokFile       - objects(sds) in the HDF
  
   filetype          - file type?
   get_env           - Gets the environment variables that might be useful to us
   do_init
   main              - process client request

****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)sdb.c,v 1.53 1996/05/17 19:42:23 xlu Exp";
#endif

/* sdb.c,v 1.53 1996/05/17 19:42:23 xlu Exp */

/* Include our stuff */
#define DMAIN_C
#include "sdb.h"
#include "sdb_util.h"
#ifdef HAVE_HDF
#include "sdb_ann.h"
#include "sdb_attr.h"
#include "sdb_cci.h"
#include "sdb_dtm.h"
#include "sdb_pal.h"
#include "sdb_ri.h"
#include "sdb_sds.h"
#include "sdb_vd.h"
#include "sdb_vg.h"
#include "sdsdump.h"
#include "show.h"

/* netCDF error toggle */
IMPORT int ncopts;

#endif /* HAVE_HDF */

/* Fits stuff */
#ifdef HAVE_FITS
#include "fitsutils.h"
#include "cfitsio.h"
#endif /* HAVE_FITS */


/*---------------------- Local variables --------------------------------*/

/* This is not good to handle multiple contexts but until the GIF
   code is retrofitted we can't handle more than one context for now */
LOCAL unsigned char *mypixels;
LOCAL int            myrowlen;


/* ------------------------- MAIN MODULES -------------------------*/

/*--------------------------------------------------------------------------
 NAME
       sdbGetImage
 DESCRIPTION
      returns an ImageInfo struct corresponding to the named reference,
      as in sdbGrokRef.  If subsample == TRUE then subsample the 
      image to fit within a hdfImageSize X hdfImageSize box
      Return the backgroud pixel's index in bg only if the file Image
 RETURNS
       return NULL on failure
--------------------------------------------------------------------------*/
ImageInfo *
sdbGetImage(char *filename,   /* HDF/netCDF file */
            char *reference,  /* reference number of image */
            intn subsample,   /* flag: TRUE - fit image within HDFImageSize^2 box */
            intn *bg,         /* background pixel index */
            lvar_st *l_vars)
{
    ImageInfo *Image = NULL;    /* Image info struct */
    int32 tag, ref;      /* tag,ref of image */
    intn  i, j;
    int   plane;                /* plane nuber */
    int   numPlane;
    int   s;
    int   rank;
    int32 status;
    int32 w, h;          /* image width,height */
    intn  isp;           /* flag: TRUE- there is a palette with image */
    uint8 pal[768];       /* pallette */
    ImageInfo *ret_value = NULL;

    ENTER(2,"sdbGetImage");
    DBUG_PRINT(2,(LOGF,"sdbGetImage: filename=%s,reference=%s \n",
                  filename,reference));

    if (l_vars->do_fits)
      {
#ifdef HAVE_FITS
          /* Get ref pair */
          if(sscanf(reference, "ref=%d,s=%d,plane=%d",&ref,&s,&plane) == 3)  {
     
              /* Get dimensions of image */
              if ((status = fitsInfo(l_vars->unit,ref, &rank, &w, &h, &numPlane)) == FAIL)
                {
                    ret_value = NULL;
                    goto done;
                }

              DBUG_PRINT(1,(LOGF," w=%d,h=%d\n", w,h));
              DBUG_PRINT(1,(LOGF," PLANE=%d\n", plane));

              /* allocate the basic structure */
              if ((Image = (ImageInfo *) malloc(sizeof(ImageInfo))) == NULL)
                {
                    ret_value = NULL;
                    goto done;
                }

              /* allocate space for the data */
              if ((Image->image_data = (unsigned char *) 
                   malloc(w * h * sizeof(unsigned char))) == NULL)
                {
                    ret_value = NULL;
                    goto done;
                }

              /* allocate the palette space */
              if ((Image->reds =   (int *)malloc(256 * sizeof(int))) == NULL)
                {
                    ret_value = NULL;
                    goto done;
                }

              if ((Image->greens = (int *)malloc(256 * sizeof(int))) == NULL)
                {
                    ret_value = NULL;
                    goto done;
                }

              if ((Image->blues =  (int *)malloc(256 * sizeof(int))) == NULL)
                {
                    ret_value = NULL;
                    goto done;
                }

              /* fill in the static fields */
              Image->ismap  = FALSE;
              Image->width  = w;
              Image->height = h;
              Image->num_colors = 256;

              DBUG_PRINT(1,(LOGF," image->width=%d\n", Image->width));
              DBUG_PRINT(1,(LOGF," image->height=%d\n", Image->height));

              /* read the image */
              if ((status = 
                   fitsImage(l_vars->unit, Image->image_data, plane)) == FAIL)
                {
                    ret_value = NULL;
                    goto done;
                }
	    
              /* set the palette */

              /* create a fake palette */
              for(i = 0; i < 256; i++)  
                {
                    Image->reds  [i] = i;
                    Image->greens[i] = i;
                    Image->blues [i] = i;
                }
              Image->color_map = NULL;

              /* reset the pallete (test only) */
              /* setPallete("image.pal",Image->reds,Image->greens,Image->blues); */

              /* Handle sub-sampling */
              if(subsample) 
                {
              
                    int max   = (h > w ? h : w);
                    int skip;   /* skip factor */
	    
                    DBUG_PRINT(1,(LOGF, "[***] max '%d'\n", max));
                    DBUG_PRINT(1,(LOGF, "[***] hdfImageSize '%d'\n", hdfImageSize));
                    DBUG_PRINT(1,(LOGF," we are subsampling\n"));

                    skip = max / hdfImageSize;

                    if((skip)&&(w/skip)&&(h/skip))
                      {
              
                          int i, j;
                          int cnt = 0;
                          unsigned char * newSpace = NULL;
		
                          if (newSpace)
                              FITSfree((unsigned char *)newSpace);

                          /* allocate space for the data */
                          if ((newSpace = (unsigned char *) \
                               malloc(max * max * sizeof(unsigned char))) == NULL)
                            {
                                ret_value = NULL;
                                goto done;
                            }

                          /* sub-sample image */
                          skip++;
                          for(j = 0; j < h; j += skip)
                              for(i = 0; i < w; i += skip) 
                                  newSpace[cnt++] = Image->image_data[i + j * w];

                          /* Free previousl allocated space */
                          FITSfree((void *)(Image->image_data));

                          /* Point to new subsampled image, 
                           * adjust height and width accordingly */
                          Image->image_data = newSpace;  
                          Image->height = h / skip;
                          Image->width  = w / skip;
                          if (w % skip) 
                              Image->width++;
                      } /* end if "skip" */

                } /* end if "subsample" */

              ret_value = Image;
          } /* end if "sscanf()" */
#endif /* HAVE_FITS */
      }
    else /* we are HDF */
      {
#ifdef HAVE_HDF
          /* Get tag/ref pair */
          if(sscanf(reference, "tag=%d,ref=%d", &tag, &ref) == 2) 
            {
                switch(tag)
                  {
                  case DFTAG_RIG:
                  case DFTAG_RI8:
#if 0
                      if((uint16)tag == DFTAG_RIG || (uint16)tag == DFTAG_RI8) 
                        {
#endif
                            /* Set read reference number */
                            if((status = DFR8readref(filename, (uint16) ref)) == FAIL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* Get dimensions of image */
                            if ((status = DFR8getdims(filename, &w, &h, &isp)) == FAIL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }
                            DBUG_PRINT(1,(LOGF," w=%d,h=%d\n", w,h));

                            /* allocate the basic structure */
                            if ((Image = (ImageInfo *) HDgetspace(sizeof(ImageInfo))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* allocate space for the data */
                            if ((Image->image_data = (unsigned char *) 
                                 HDgetspace(w * h * sizeof(unsigned char))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* allocate the palette space */
                            if ((Image->reds = (int *) HDgetspace(256 * sizeof(int))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }
                            if ((Image->greens = (int *) HDgetspace(256 * sizeof(int))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }
                            if ((Image->blues = (int *) HDgetspace(256 * sizeof(int))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* fill in the static fields */
                            Image->ismap  = FALSE;
                            Image->width  = w;
                            Image->height = h;
                            Image->num_colors = 256;

                            DBUG_PRINT(1,(LOGF," image->width=%d\n", Image->width));
                            DBUG_PRINT(1,(LOGF," image->height=%d\n", Image->height));

                            /* read the image */
                            if ((status = 
                                 DFR8getimage(filename, Image->image_data, w, h, pal)) == FAIL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* set the palette */
                            if(isp) 
                              { /* move the palette over into the fields */
                                  for(i = 0; i < 256; i++) 
                                    {
                                        Image->reds  [i] = (int)pal[i * 3]     ;
                                        Image->greens[i] = (int)pal[i * 3 + 1] ;
                                        Image->blues [i] = (int)pal[i * 3 + 2] ;
                                    }

                                  /* dont mess with palette */
                                  Image->color_map = pal;
                              } 
                            else 
                              { /* create a fake palette */
                                  for(i = 0; i < 256; i++) 
                                    {
                                        Image->reds  [i] = i;
                                        Image->greens[i] = i;
                                        Image->blues [i] = i;
                                    }
                                  /* dont mess with palette */
                                  Image->color_map = NULL;
                              }

                            /* Handle sub-sampling */
                            if(subsample) 
                              {
                                  int max   = (h > w ? h : w);
                                  int skip;   /* skip factor */

                                  DBUG_PRINT(1,(LOGF, "[***] max '%d'\n", max));
                                  DBUG_PRINT(1,(LOGF, "[***] hdfImageSize '%d'\n", hdfImageSize));
                                  DBUG_PRINT(1,(LOGF," we are subsampling\n"));

                                  skip = max / hdfImageSize;

                                  if((skip)&&(w/skip)&&(h/skip)) 
                                    { 
                                        int i, j;
                                        int cnt = 0;
                                        unsigned char * newSpace = NULL;

                                        /* allocate space for the data */
                                        if ((newSpace = (unsigned char *) 
                                             HDgetspace(max * max * sizeof(unsigned char))) 
                                            == NULL)
                                          {
                                              ret_value = NULL;
                                              goto done;
                                          }

                                        /* sub-sample image */
                                        skip++;
                                        for(j = 0; j < h; j += skip)
                                            for(i = 0; i < w; i += skip) 
                                                newSpace[cnt++] = Image->image_data[i + j * w];

                                        /* Free previousl allocated space */
                                        HDfreespace((void *)(Image->image_data));

                                        /* Point to new subsampled image, 
                                         * adjust height and width accordingly */
                                        Image->image_data = newSpace;  
                                        Image->height = h / skip;
                                        Image->width  = w / skip;
                                        if (w % skip) 
                                            Image->width++;
                                    } /* end if "skip" */
                              } /* end if "subsample" */

                            ret_value = Image;
                            goto done;
#if 0
                        } /* end if "tag=DTAG_RIG" or "tag=DFTAG_RI8 */
#endif
                      break;

                  case DFTAG_IP8:
#if 0
                      /* Check if tag matches palette */
                      if ((uint16)tag == DFTAG_IP8) 
                        { /* Set up read on 8-bit image pallete */
#endif
                            if ((status = DFPreadref(filename, (uint16) ref)) == FAIL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* allocate the basic structure */
                            if ((Image = (ImageInfo *) HDgetspace(sizeof(ImageInfo))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* allocate space for the data */
                            h = 30;    /* resonable height */
                            if ((Image->image_data = (unsigned char *) 
                                 HDgetspace(768 * h * sizeof(unsigned char))) == NULL)
                              {
                                  ret_value= NULL;
                                  goto done;
                              }

                            /* allocate the palette space */
                            if ((Image->reds = (int *) HDgetspace(256 * sizeof(int))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }
                            if ((Image->greens = (int *) HDgetspace(256 * sizeof(int))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }
                            if ((Image->blues = (int *) HDgetspace(256 * sizeof(int))) == NULL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* fill in the static fields */
                            Image->ismap = FALSE;
                            Image->width = 256;   /* all values */
                            Image->height = h;
                            Image->num_colors = 256;

                            /* read the image */
                            if ((status = DFPgetpal(filename, pal)) == FAIL)
                              {
                                  ret_value = NULL;
                                  goto done;
                              }

                            /* move the palette over into the fields */
                            for(i = 0; i < 256; i++) 
                              {
                                  Image->reds  [i] = (int)pal[i * 3]     ;
                                  Image->greens[i] = (int)pal[i * 3 + 1] ;
                                  Image->blues [i] = (int)pal[i * 3 + 2] ;

                                  /* create h rows for data region */
                                  for(j = 0; j < h; j++) 
                                      Image->image_data[i + j * 256] = i;
                              }

                            Image->color_map = pal;

                            /* at this point we return a palette */
                            ret_value = Image;                 
#if 0
                        } /* end if "tag=DFTAG_IP8" */
#endif
                      break;
                  default:
                      ret_value = NULL;
                  } /* end switch tag */

            } /* end if "sscanf()" for HDF tag/ref*/
#endif /* HAVE_HDF */
      } /* end else HDF */

  done:
    return ret_value;
    EXIT(2,"sdbGetImage");
} /* sdbGetImage */

/*---------------------------------------------------------------------------
 NAME
       sdbGrokFile
 DESCRIPTION
        The function sdbGrokFile() will take a whole file and return stuff that
        can be seen at the "top level" (i.e. all datasets, all raster images 
        and all Vgroups which are not contained within other Vgroups).
 RETURNS
----------------------------------------------------------------------------*/
char *
sdbGrokFile(char *fname, 
            entry entries[MAX_ENTRIES], 
            mime_content_type *mime_type,
            int current_entry,
            int num_entries,
            lvar_st *l_vars)
{
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *data;                       /* */
    int32 len;
    int32 fid = FAIL;                        /* File handle */
    int32 ret;
    char *ret_value = NULL;

    ENTER(2,"sdbGrokFile");
    DBUG_PRINT(3,(LOGF,"l_vars->do_netcdf=%d \n",l_vars->do_netcdf));
#ifdef HAVE_HDF
    /* make sure we don't crash on invalid open */
    ncopts = 0;
#endif /* HAVE_HDF */

    if (!l_vars->do_dump)
      {     
          /* Open temproary file to write HTML description of HDF/netCDF file */
          if (!(l_vars->hfp = fopen(tmp, "w")))
            {
                ret_value = NULL;
                goto done;
            }
        
          /* Write MIME header */
          *mime_type = TEXT_HTML;
          if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
            {
                gateway_err(l_vars->hfp,"sdbGrokFile: writing HTML header",0,l_vars);
                ret_value = NULL;
                goto done;
            }
      }

    /* are we dumping FITS or HDF/netCDF fil e */
    if (l_vars->do_fits)
      {
#ifdef HAVE_FITS
          /* read fits  file */
          readFits(fname,l_vars);
#endif
      }
    else /* else HDF or netCDF file */
      {
#ifdef HAVE_HDF
          if (!l_vars->do_netcdf)
            {
                /* open file once */
                if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokFile: Problem opening HDF file",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }


                /* process all annotations */
                if (do_anns(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokFile: error processing annotations ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
            }

          /* process SDS in file */
          if (do_sds(fname, l_vars) == FAIL)
            {
                gateway_err(l_vars->hfp,"sdbGrokFile: error processing SDS ",0,l_vars);
                ret_value = NULL;
                goto done;
            }

          if (!l_vars->do_netcdf)
            {
                /* process pals and rigs in file */
                if (do_pals(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokFile: error processing palettes ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process raster8 */
                if (do_rigs(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokFile: error processing raster8 ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process Vgroups */
                if (do_vgs(fname, l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokFile: error processing Vgroups ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process lone Vdatas */
                if (do_lone_vds(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokFile: error processing lone Vdatas ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

            } /* end if not netCDF */


          /* 
           * Now dumping time 
           */
          if(!l_vars->do_netcdf)
            {
                DBUG_PRINT(3,(LOGF,"doing annotations \n"));
                /* dump file annotations */
                dump_anns(fname,l_vars);
            }

          /* Do SDSs */
          if (!l_vars->do_dump)
            {
                DBUG_PRINT(3,(LOGF,"doing sdss \n"));
                dump_sds(fname, current_entry, entries, num_entries,l_vars);
            }

          if(!l_vars->do_netcdf)
            {
                /* Do RIGs */
                DBUG_PRINT(3,(LOGF,"doing rigs \n"));
                dump_rigs(fname,l_vars);
        
                DBUG_PRINT(3,(LOGF,"doing lone pals \n"));
                /* Do lone Pals */
                dump_pals(fname,l_vars);
        
                DBUG_PRINT(3,(LOGF,"doing vgroups \n"));
                /* Do Vgroups */
                dump_vgs(fname, l_vars);
        
                DBUG_PRINT(3,(LOGF,"doing lone Vdatas \n"));
                /* Do lone Vdatas */
                dump_lone_vds(fname,l_vars);

                if (!l_vars->do_dump)       
                  {
                      /* Display graphical representation of Vgroups/Vdatas */
                      fprintf(l_vars->hfp, "<PRE WIDTH=\"%d\">", DOC_WIDTH); 
                      ret = show_tree(fname,l_vars);
                      fprintf(l_vars->hfp, "</PRE>"); 
                  }

            } /* end if not netCDF */
#endif /* HAVE_HDF */
      }     /* end else if  HDF or netCDF */

    /* Close temporary file */
    if (!l_vars->do_dump)
        fclose(l_vars->hfp);

    ret_value = tmp;
  done:
#ifdef HAVE_HDF
    if (!l_vars->do_netcdf && !l_vars->do_fits)
      {
          /* close file */
          if (fid != NULL)
              Hclose(fid);
      }
#endif /* HAVE_HDF */
    EXIT(2,"sdbGrokFile");
    return ret_value;
} /* sdbGrokFile */

/*-------------------------------------------------------------------- 
 NAME
     myGetGIFPixel
 DESCRIPTION
     Gets value of next pixel
 RETURNS
     returns next pixel value
--------------------------------------------------------------------*/ 
int
myGetGIFPixel (int x, int y)
{
    return (mypixels[y * myrowlen + x]);
} /* myGetGIFPixel */


#ifdef HAVE_HDF
/*----------------------------------------------------------------------------
 NAME
      sdbGrokRef
 DESCRIPTION
        The function sdbGrokRef() will return a description of what can
        be "seen" from a given location in the file.  Typically, this will be 
        how users can navigate their way through a forest of Vgroups.
 RETURNS
      Returns the file name containg the HTML description or the GIF image
      if successful else NULL.
----------------------------------------------------------------------------*/
char *
sdbGrokRef(char *fname,                 /* HDF/netCDF file name */
           char *ref,                  /* Reference number to start navigation from */
           entry entries[MAX_ENTRIES], 
           mime_content_type *mime_type, 
           int current_entry, 
           int num_entries,
           lvar_st *l_vars)
{
    int32 t, r;           /* tag/ref numbers */
    int32 x, y;
    intn sub_sample;
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *gtmp = (char *)tempnam(NULL,"img");  /* Temporary file name for HTML desc */
    char *ptmp = (char *)tempnam(NULL,"pal");  /* Temporary file name for HTML desc */
    char *data;
    int32 len;
    int32 fid = FAIL;
    ImageInfo *image;
    intn bkg = 0;
    int i;
    static skip=FALSE;
    int32 vd_id;
    char   name[VSNAMELENMAX + 1];  /* Vdata name */
    char   fields[(FIELDNAMELENMAX + 1) * VSFIELDMAX]; /* Vdata field names*/
    int32  intr;      /* type of interlace */
    int32  sz;        /* size of Vdata */
    int32  cnt;       /* number of Vdatas */
    int32 flds_indices[100];
    char sep[2];
    int32 file_handle;
    int32 an_handle;
    char *ret_value = NULL;

    ENTER(2,"sdbGrokRef");
    DBUG_PRINT(1,(LOGF," ref=%s\n", ref));

    /* skip past seperator */
    if (ref[0]==';')
        ref++;

    if(sscanf(ref, "tag=%d,ref=%d,s=%d", &t, &r, &sub_sample) == 3) 
      {
          DBUG_PRINT(1,(LOGF," got tag=%d, ref=%d\n", t,r));
          l_vars->obj_tag = (uint16) t;
          l_vars->obj_ref = (uint16) r;
          l_vars->sub_s = sub_sample;

#if 0
          if (!l_vars->do_netcdf)
            {
                /* open file once */
                if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: Problem opening HDF file",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process all annotations */
                if (do_anns(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: error processing annotations ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
            }

          /* process SDS in file */
          if (do_sds(fname, l_vars) == FAIL)
            {
                gateway_err(l_vars->hfp,"hdfGrokRef: error processing SDS ",0,l_vars);
                ret_value = NULL;
                goto done;
            }

          if (!l_vars->do_netcdf)
            {
                /* process lone pals in file */
                if (do_pals(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing palettes ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process rigs in file */
                if (do_rigs(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing raster8 ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* Process Vgroups in file */
                if ((do_vgs(fname, l_vars)) == FAIL)              
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing Vgroups ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process lone Vdatas */
                if (do_lone_vds(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing lone Vdatas ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

            } /* not netCDF i.e. HDF */
#endif

          switch((uint16) t) 
            {
            case DFTAG_VG:  /* Vgroup */
                DBUG_PRINT(1,(LOGF," tag is Vgroup\n"));
                /* Open temporary file name for HTML description */
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL) 
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
              
                /* Write MIME header response */
                *mime_type = TEXT_HTML;
                if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML header",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* open file once */
                if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: Problem opening HDF file",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process all annotations */
                if (do_anns(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: error processing annotations ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process SDS in file */
                if (do_sds(fname, l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing SDS ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* Process Vgroups in file */
                if ((do_vgs(fname, l_vars)) == FAIL)              
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing Vgroups ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* Dump Vgroup in file */
                if ((dump_vg(fname, r,l_vars)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error dumping Vgroup ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                fprintf(l_vars->hfp, "<hr>\n");

                break;
            case DFTAG_VH: /* Vdata */
                DBUG_PRINT(1,(LOGF," tag is Vdata\n"));
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL) 
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* Write MIME header response */
                *mime_type = TEXT_HTML;
                if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: wring HTML headr",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* open file once */
                if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: Problem opening HDF file",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process Vdata */
                if ((do_vd(fname,t,r,l_vars)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error dumping Vdata ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                break;
            case DFTAG_NDG:
            case 6666: /* fake take for netCDF */
                DBUG_PRINT(1,(LOGF," tag is NDG\n"));

                /* Open temporary file name for HTML description */
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
              
                /* Write MIME header response */
                *mime_type = TEXT_HTML;
                if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: wring HTML headr",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                if (!l_vars->do_netcdf)
                  {
                      /* open file once */
                      if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"hdfGrokRef: Problem opening HDF file",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* process all annotations */
                      if (do_anns(fname,l_vars) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"sdbGrokRef: error processing annotations ",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }
                  }

                /* process SDS in file */
                if (do_sds(fname, l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing SDS ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* print SDS info */
                if (dump_sd(fname, r,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: unable to access dataset",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                break;
            case DFTAG_IP8:
                DBUG_PRINT(1,(LOGF," tag is raster palette\n"));

                /* Open temporary file name for HTML description */
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
              
                /* Write MIME header response */
                *mime_type = IMAGE_GIF;
                if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML header",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* open file once */
                if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: Problem opening HDF file",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process all annotations */
                if (do_anns(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: error processing annotations ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process lone pals in file */
                if (do_pals(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing palettes ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process rigs in file */
                if (do_rigs(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: error processing raster8 ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* Open temporary file name for gif description */
                if ((l_vars->gfp = fopen(ptmp, "wb+")) == NULL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: Problem opening temproray file for GIF",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
              
                /* Get Palette */
                if ((image = sdbGetImage(fname, ref, FALSE, &bkg,l_vars)) == NULL)
                  {
                      fclose(l_vars->gfp);
                      gateway_err(l_vars->hfp,"sdbGrokRef: Problem with sdbGetImage",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                DBUG_PRINT(1,(LOGF," sdbGetImage returned\n"));
                DBUG_PRINT(1,(LOGF," image->width=%d\n",image->width));
                DBUG_PRINT(1,(LOGF," image->height=%d\n",image->height));
                DBUG_PRINT(1,(LOGF," bkg=%d\n",bkg));

                mypixels = image->image_data;
                myrowlen =image->width;
                DBUG_PRINT(1,(LOGF,"calling GIFEncode \n"));

                GIFEncode(
                    l_vars->gfp,                       /* file to write to */
                    image->width,		/* width */
                    image->height,		/* height */
                    0,			/* No interlacing */
                    bkg,			/* Index of Backgrounf */
                    8,			/* Bits Per pixel */
                    image->reds,		/* Red colormap */
                    image->greens,		/* Green colormap */
                    image->blues,		/* Blue colormap */
                    myGetGIFPixel);		/* Get Pixel value */
                DBUG_PRINT(1,(LOGF,"GIFEncode returned\n"));

                /* Close temporary gif file and html file*/
                fclose(l_vars->gfp);
                fclose(l_vars->hfp);

                /* Free memory allocated in sdbGetImage */
                if (image->image_data != NULL)
                    HDfree(image->image_data);
                if (image->reds != NULL)
                    HDfree(image->reds);
                if (image->greens != NULL)
                    HDfree(image->greens);
                if (image->blues != NULL)
                    HDfree(image->blues);
                if (image != NULL)
                    HDfree(image);

                ret_value = ptmp; /* return palette file name */
                goto done;

                break;
            case DFTAG_RIG:
                DBUG_PRINT(1,(LOGF," tag is raster group\n"));

                /* Open temporary file name for HTML description */
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

#ifdef HAVE_DTM
                if (l_vars->dtm_outport)
                  {
                      /* Write MIME header response */
                      *mime_type = TEXT_HTML;
                      if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                        {
                            fclose(l_vars->gfp);
                            gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML headr",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      mo_dtm_out(l_vars->dtm_outport,l_vars);
                      mo_dtm_poll_and_read ();
                      sleep(1);
                      if ((hdfDtmThang(fname,DFTAG_RIG,r,ref,l_vars)) ==FAIL)
                        {
                            gateway_err(l_vars->hfp,"hdfGrokRef: error from hdfDtmThang",0,l_vars);
                            ret_value = NULL;
                        }
                      sleep(1);
                      mo_dtm_poll_and_read ();
                      mo_dtm_disconnect ();
                  }
                else
                  {
#endif
                      /* Open temporary file name for gif description */
                      if ((l_vars->gfp = fopen(gtmp, "wb+")) == NULL)
                        {
                            gateway_err(l_vars->hfp,"sdbGrokRef: Problem opening temproray file for GIF",1,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* Write MIME header response */
                      *mime_type = IMAGE_GIF;
                      if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                        {
                            fclose(l_vars->gfp);
                            gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML headr",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* open file once */
                      if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"hdfGrokRef: Problem opening HDF file",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* process all annotations */
                      if (do_anns(fname,l_vars) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"sdbGrokRef: error processing annotations ",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* process lone pals in file */
                      if (do_pals(fname,l_vars) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"hdfGrokRef: error processing palettes ",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* process rigs in file */
                      if (do_rigs(fname,l_vars) == FAIL)
                        {
                            gateway_err(l_vars->hfp,"hdfGrokRef: error processing raster8 ",0,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      /* Get image */
                      if ((image = sdbGetImage(fname, ref, sub_sample, &bkg,l_vars)) == NULL)
                        {
                            fclose(l_vars->gfp);
                            gateway_err(l_vars->hfp,"sdbGrokRef: Problem with sdbGetImage",1,l_vars);
                            ret_value = NULL;
                            goto done;
                        }

                      DBUG_PRINT(1,(LOGF," sdbGetImage returned\n"));
                      DBUG_PRINT(1,(LOGF," image->width=%d\n",image->width));
                      DBUG_PRINT(1,(LOGF," image->height=%d\n",image->height));
                      DBUG_PRINT(1,(LOGF," bkg=%d\n",bkg));

                      mypixels = image->image_data;
                      myrowlen = image->width;
                      DBUG_PRINT(1,(LOGF," myrowlen=%d\n",myrowlen));

                      DBUG_PRINT(1,(LOGF,"calling GIFEncode \n"));

                      GIFEncode(
                          l_vars->gfp,                       /* file to write to */
                          image->width,		/* width */
                          image->height,		/* height */
                          0,			/* No interlacing */
                          bkg,			/* Index of Backgrounf */
                          8,			/* Bits Per pixel */
                          image->reds,		/* Red colormap */
                          image->greens,		/* Green colormap */
                          image->blues,		/* Blue colormap */
                          myGetGIFPixel);		/* Get Pixel value */
                      DBUG_PRINT(1,(LOGF,"GIFEncode returned\n"));

                      /* Close temporary html file and gif file */
                      fclose(l_vars->gfp);
                      fclose(l_vars->hfp);

                      /* Free memory allocated in sdbGetImage */
                      if (image->image_data != NULL)
                          HDfree(image->image_data);
                      if (image->reds != NULL)
                          HDfree(image->reds);
                      if (image->greens != NULL)
                          HDfree(image->greens);
                      if (image->blues != NULL)
                          HDfree(image->blues);
                      if (image != NULL)
                          HDfree(image);

                      ret_value = gtmp;  /* return image file name */
                      goto done;
#ifdef HAVE_DTM
                  }
#endif

                break;
            case DFTAG_FID:
            case DFTAG_FD:
            case DFTAG_DIL:
            case DFTAG_DIA:
                DBUG_PRINT(1,(LOGF," tag is an annotation \n"));
                /* Open temporary file name for HTML description */
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
              
                /* Write MIME header response */
                *mime_type = TEXT_HTML;
                if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML headr",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
                /* open file once */
                if ((fid = Hopen(fname, DFACC_RDONLY, 0)) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"hdfGrokRef: Problem opening HDF file",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* process all annotations */
                if (do_anns(fname,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: error processing annotations ",0,l_vars);
                      ret_value = NULL;
                      goto done;
                  }

                /* print annotation */
		/* This section commented out by Douglas Fils after I could not
		locate the function tag2atype() anywhere in C or HDF  (FITS)? 
                print_ann(fid,r,t,sub_sample,tag2atype(t),l_vars->hfp,l_vars);
		*/

                /* clenup */
#if 0
                Hclose(file_handle);
#endif
                break;
            default:
                /* Open temporary file name for HTML description */
                if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
                  {
                      gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                      ret_value = NULL;
                      goto done;
                  }
              
                /* Write MIME header response */
                *mime_type = TEXT_HTML;
                if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
                  {
                      gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML header",0,l_vars);
                      ret_value =  NULL;
                      goto done;
                  }

                /* Indicate we don't handle these objects */
                fprintf(l_vars->hfp, "<H2>Sorry!</H2>\n");
                fprintf(l_vars->hfp, "Objects of type <B>%s</B> are not currently ",
                        HDgettagsname((uint16)t));
                fprintf(l_vars->hfp, "handled by the browser.<P>\n");
                break;
            } /* end switch "t" */
      } 
    else  /* not able to get tag/ref */
      {
          /* Open temporary file name for HTML description */
          if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
            {
                gateway_err(stdout,"sdbGrokRef: Problem opening temproray file",1,l_vars);
                ret_value = NULL;
                goto done;
            }
              

          /* Write MIME header response */
          *mime_type = TEXT_HTML;
          if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL)
            {
                gateway_err(l_vars->hfp,"sdbGrokRef: writing HTML header",0,l_vars);
                ret_value = NULL;
                goto done;
            }

          DBUG_PRINT(1,(LOGF," couldn't get tag/ref\n"));

          fprintf(l_vars->hfp, "<H2>Sorry, Bad Reference</H2>\n");
          fprintf(l_vars->hfp, "We're sorry, but reference <I>%s</I> is bad and we can't figure out what to do about it.<P>\n", 
                  ref);
      } /* end else not able to get tag/ref */       
   

    /* Check for funky sds handling */
    if ((ref[0]=='#')||(ref[0]=='@'))
      {
          DBUG_PRINT(1,(LOGF,"this is a funky SDS, ref=%s\n",ref));
          dump_sds(fname, current_entry, entries, num_entries,l_vars);
      }

    /* Close temporary file and return its name */
    fclose(l_vars->hfp);

    ret_value = tmp; /* html response file */

  done:
    if (ret_value == NULL)
      {
      }

    /* cleanup */
    if (!l_vars->do_netcdf)
      {
          /* close file */
          if (fid != FAIL)
              Hclose(fid);
      }

    EXIT(2,"sdbGrokRef");
    return ret_value;
} /* sdbGrokRef */

#endif /* HAVE_HDF */

/*---------------------------------------------------------------------------
 NAME
       sdbGrokImage
 DESCRIPTION
       returns an ImageInfo struct corresponding to the named reference,
       as in sdbGrokRef. Calls "sdbGetImage()" with "subsample = TRUE".
 RETURNS
       return NULL on failure
----------------------------------------------------------------------------*/
ImageInfo *
sdbGrokImage(char *filename,    /* HDF/netCDF file */
             char *reference,   /* Reference number of image */
             int *bg,           /* background pixel index */
             lvar_st *l_vars)
{
    /* Return a subsampled image */
    return (sdbGetImage(filename, reference, TRUE, bg,l_vars));
} /* sdbGrokImage */



#ifdef HAVE_FITS
/*----------------------------------------------------------------------------
 NAME
      fitsGrokHDU
 DESCRIPTION
        The function fitsGrokFITS() will return a description of what can
        be "seen" from a given location in the file. 
 RETURNS
      Returns the file name containg the HTML description or the GIF image
      if successful else NULL.
----------------------------------------------------------------------------*/
char *
fitsGrokHDU(int unit,                       /* FITS  file name */
            char *ref, 
            entry entries[MAX_ENTRIES], 
            mime_content_type *mime_type, 
            int current_entry, 
            int num_entries,
            lvar_st *l_vars)
{
    int32 t, r;           /* tag/ref numbers */
    int32 x, y;
    intn  sub_sample;
    /* Temporary file name for HTML desc */
    char *tmp = (char *)tempnam(NULL,"html"); 
    
    /* Temporary file name for HTML desc */
    char *gtmp = (char *)tempnam(NULL,"img"); 
 
    /* Temporary file name for HTML desc */
    char *ptmp = (char *)tempnam(NULL,"pal");  
    char  *data;
    int32 len, fid;
    ImageInfo *image;
    intn  bkg = 0;
    int   i;
    static skip=FALSE;
    int32 vd_id;
    int32 flds_indices[100];
    char  sep[2];
    int32 file_handle;
    int32 an_handle;

    ENTER(2,"fitsGrokReference");
    DBUG_PRINT(1,(LOGF," ref=%s\n", ref));

    if (ref[0]==';')
        ref++;
    if(sscanf(ref, "ref=%d,s=%d", &r, &sub_sample) == 2) {
      
        DBUG_PRINT(1,(LOGF," got ref=%d\n",r));     
        /* Open temporary file name for HTML description */
        if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
            return(NULL);
	
        /* Open temporary file name for gif description */
        if ((l_vars->gfp = fopen(gtmp, "wb+")) == NULL)
            return(NULL);
              
        /* Write MIME header response */
        *mime_type = IMAGE_GIF;
        if (!skip) {                
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {   
                gateway_err(l_vars->hfp," writing HTML header",0,l_vars);
                return NULL;
            }
            skip = !(skip);
        }
    
        /* Get Image */
        if ((image = sdbGetImage(l_vars->hdf_file, ref, sub_sample,&bkg,l_vars)) == NULL)
            return(NULL);

        DBUG_PRINT(1,(LOGF," sdbGetImage returned\n"));
        DBUG_PRINT(1,(LOGF," image->width=%d\n",image->width));
        DBUG_PRINT(1,(LOGF," image->height=%d\n",image->height));
        DBUG_PRINT(1,(LOGF," bkg=%d\n",bkg));

        mypixels = image->image_data;
        myrowlen = image->width;
        DBUG_PRINT(1,(LOGF,"calling GIFEncode \n"));

        GIFEncode(
            l_vars->gfp,                       /* file to write to */
            image->width,		/* width */
            image->height,		/* height */
            0,			/* No interlacing */
            bkg,			/* Index of Backgrounf */
            8,			/* Bits Per pixel */
            image->reds,		/* Red colormap */
            image->greens,		/* Green colormap */
            image->blues,		/* Blue colormap */
            myGetGIFPixel);		/* Get Pixel value */

        DBUG_PRINT(1,(LOGF,"GIFEncode returned\n"));

        /* Close temporary gif file and html file*/
        fclose(l_vars->gfp);
        fclose(l_vars->hfp);

        /* Free memory allocated in sdbGetImage */

        if (image->blues != NULL)
            FITSfree(image->blues); 

        if (image->greens != NULL)
            FITSfree(image->greens); 

        if (image->reds != NULL)
            FITSfree(image->reds); 

        if (image->image_data != NULL)
            FITSfree(image->image_data);
        if (image != NULL)
            FITSfree(image);
	    	    
        EXIT(2,"fitsGrokReference");
	/*
        FITSfree(tmp);
        FITSfree(ptmp); 
	*/
        return(gtmp); /* return image file name */
    }

    else  { /* not able to get tag/ref */
      
        /* Open temporary file name for HTML description */
        if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
            return(NULL);

        /* Write MIME header response */
        *mime_type = TEXT_HTML;
        if (!skip) {
          
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
              
                gateway_err(l_vars->hfp," writing HTML header",0,l_vars);
                return NULL;
            }

            skip = !(skip);
        }

        DBUG_PRINT(1,(LOGF," couldn't get tag/ref\n"));

        if(sscanf(ref, "fileImageHit?%d,%d", &x, &y) == 2) {
          
            char *buf;
            fprintf(l_vars->hfp, "<H2>File Image Hit</H2>\n");
            fprintf(l_vars->hfp, "We're sorry, but we really have no clue what is at location %d %d\n", x, y); 

            /* hmmm....not sure */
            buf = malloc(100);
            sprintf(buf, "#DataSet%d", 2);
        } 
        else if ((ref[0]!='#') && (ref[0]!='@') && l_vars->display_type == 0) {
          
            fprintf(l_vars->hfp, "<H2>Sorry, Bad Reference</H2>\n");
            fprintf(l_vars->hfp, "We're sorry, but reference <I>%s</I> is bad and we can't figure out what to do about it.<P>\n", 
                    ref);
        }
    } /* end else not able to get tag/ref */       
   
    DBUG_PRINT(1,(LOGF," lets see if this is an SDS, ref=%s\n",ref));
  
    /* Close temporary file and return its name */
    fclose(l_vars->hfp);
    /*
    FITSfree(gtmp);
    FITSfree(ptmp); */
    EXIT(2,"fitsGrokReference");
    return(tmp);
} /* fitsGrokReference */


/*---------------------------------------------------------------------------
 NAME
       fitsGrokImage
 DESCRIPTION
       returns an ImageInfo struct corresponding to the named reference,
       as in fitsGrokHDU. Calls "sdbGetImage()" with "subsample = TRUE".
 RETURNS
       return NULL on failure
----------------------------------------------------------------------------*/
ImageInfo *
fitsGrokImage(int unit,        /* FITS  file */
              char *reference,  /* Reference number of image */
              int *bg,          /* background pixel index */
              lvar_st *l_vars)
{
    /* Return a subsampled image */
    return (sdbGetImage(NULL, reference, 1, bg,l_vars));
} /* fitsGrokImage */



/*---------------------------------------------------------------------------
 NAME
       fitsGrokTab
 DESCRIPTION
        The function fitsGrokTab() will preview all of the info within the table

        
 RETURNS
----------------------------------------------------------------------------*/
char *
fitsGrokTab(int unit,  char *ref,
            entry entries[MAX_ENTRIES], 
            mime_content_type *mime_type,
            int current_entry,
            int num_entries,
            lvar_st *l_vars)
{
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *data;                      
    int32 len;
    int32 fid;                        /* File handle */
    int32 ret;
    int   i;
    int   fits_ext;
    int   fldNum, recNum; 

    ENTER(2,"fitsGrokTab");
     
    if (ref[0]==';')
        ref++;
  
    DBUG_PRINT(1,(LOGF,"ref = %s\n",ref )); 

    if(sscanf(ref, "ref=%d", &fits_ext) == 1) {
    
        if (!l_vars->do_dump) { 
      
            /* Open temproary file to write HTML description of FITS  file */
            if (!(l_vars->hfp = fopen(tmp, "w")))
                return(NULL);

            /* Write MIME header */
            *mime_type = TEXT_HTML;
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
          
                gateway_err(l_vars->hfp,"fitsGrokTab: writing HTML header",0,l_vars);
                return NULL;
            }
        }
    
        /* read table from fits  */
        /* Print TABLE  header stuff in HTML  */
  	fprintf(l_vars->hfp,"<HR>\n"); 
        fprintf(l_vars->hfp, "<H2>ASCII TABLE within the fits file </H2>\n");  
	/* fprintf(l_vars->hfp, "The undefined array elements will be substituted with -1.\n"); */

        /* this HDU will be read */
        fitsTableInfo(unit, fits_ext, &fldNum, &recNum);
  
        ret = fitsTable(unit,l_vars->hfp,l_vars);
    }
    else {
        return NULL;
    } 
 
    /* Close temporary file */
    if (!l_vars->do_dump)
        fclose(l_vars->hfp);
    
    EXIT(2,"fitsGrokTab");
    return(tmp);
    
} /* fitsGrokTab */


/*---------------------------------------------------------------------------
 NAME
       fitsGrokBinTab
 DESCRIPTION
        The function fitsGrokBinTab() will preview all of the info within the table

        
 RETURNS
----------------------------------------------------------------------------*/
char *
fitsGrokBinTab(int unit,  char *ref,
               entry entries[MAX_ENTRIES], 
               mime_content_type *mime_type,
               int current_entry,
               int num_entries,
               lvar_st *l_vars)
{
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *data;                      
    int32 len;
    int32 fid;                        /* File handle */
    int32 ret;
    int   i;
    int   fits_ext;
    
    int   fldNum, recNum; 

    ENTER(2,"fitsGrokBinTab");
     
    if (ref[0]==';')
        ref++;
  
    DBUG_PRINT(1,(LOGF,"ref = %s\n",ref )); 

    if(sscanf(ref, "ref=%d", &fits_ext) == 1) {
    
        if (!l_vars->do_dump) { 
      
            /* Open temproary file to write HTML description of FITS  file */
            if (!(l_vars->hfp = fopen(tmp, "w")))
                return(NULL);

            /* Write MIME header */
            *mime_type = TEXT_HTML;
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
          
                gateway_err(l_vars->hfp,"fitsGrokBinTab: writing HTML header",0,l_vars);
                return NULL;
            }
        }
    
        /* read table from fits  */
        /* Print TABLE  header stuff in HTML  */
	fprintf(l_vars->hfp,"<HR>\n");
        fprintf(l_vars->hfp, "<H2>Binary TABLE within the fits file </H2>\n");
	/* fprintf(l_vars->hfp, "The undefined array elements will be substituted with -1.\n"); */
        /* this HDU will be read */
        fitsBinTabInfo(unit, fits_ext, &fldNum, &recNum);
  
	ret = fitsBinTable(unit,l_vars->hfp,l_vars); 
        if (ret == FAIL) 
            return NULL;

    }
    else {
        return NULL;
    } 
 
    /* Close temporary file */
    if (!l_vars->do_dump)
        fclose(l_vars->hfp);
  
    EXIT(2,"fitsGrokBinTab");
    return(tmp);
    
} /* fitsGrokBinTab */

/*---------------------------------------------------------------------------
 NAME
       listhead
 DESCRIPTION
        The function listhead() will retrive the info from the FITS file

        
        RETURNS
 ----------------------------------------------------------------------------*/
char *
listhead(int unit,  
         char *ref,
         entry entries[MAX_ENTRIES], 
         mime_content_type *mime_type,
         int current_entry,
         int num_entries,
         lvar_st *l_vars, int tag)
{
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *data;                      
    int32 len;
    int32 fid;                        /* File handle */
    int32 ret;
    int   fits_ext;
    int status,bitpix,naxis,naxes[99],pcount,gcount;
    int group,fpixel,nelem,i,j,rwstat,bksize;
    int hdutyp,inull;
    int  simple,extend,anyflg;
    char errtxt[FITS_CLEN_ERRMSG];
    char card[80];
    int keywordNumber, fitsSpace;
    
    int  histFlag = 0;

    ENTER(2,"listhead");
        
    if (!l_vars->do_dump) { 
      
        /* Open temproary file to write HTML description of FITS  file */
        if (!(l_vars->hfp = fopen(tmp, "w")))
            return(NULL);

        /* Write MIME header */
        *mime_type = TEXT_HTML;
        if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
          
            gateway_err(l_vars->hfp,"listhead: writing HTML header",0,l_vars);
            return NULL;
        }
    }

    /* Print header stuff in HTML  */

    switch(tag) {
    case LISTHEAD: {

        /* inforation here */
        fprintf(l_vars->hfp, "<HR>");
        /* fprintf(l_vars->hfp, "<TITLE>FITS File Information </TITLE>\n"); */
        fprintf(l_vars->hfp, "<H1>FITS File Inforation</H1>\n");
        fprintf(l_vars->hfp, "<hr>\n");
      
        /* retrieve the information from fits file */
        getFitsDesc(unit, l_vars->hfp); 
     
        fprintf(l_vars->hfp, "<hr>\n");     
        fprintf(l_vars->hfp, "<b>More Details related to the FITS file </b>\n");
        fprintf(l_vars->hfp, "<UL>\n");  
        fprintf(l_vars->hfp, "<LI> <A HREF=\"%s%s?%s!head\"<b>FITS Header</b></A>\n",l_vars->h_env->script_name, l_vars->h_env->path_info, l_vars->f_name);

        fprintf(l_vars->hfp, "<LI> <A HREF=\"%s%s?%s!history\"<b>FITS History</b></A>\n",l_vars->h_env->script_name, l_vars->h_env->path_info, l_vars->f_name);

        fprintf(l_vars->hfp, "<LI> <A HREF=\"%s/%s\"<b>Download</b></A> the fits file \n", l_vars->h_env->path_info,l_vars->f_name);
        fprintf(l_vars->hfp, "</UL>\n");

        fprintf(l_vars->hfp,"<HR>\n");

        break;
    }

    case HEAD: {
        fprintf(l_vars->hfp, "<HR>");
        /*fprintf(l_vars->hfp, "<TITLE>FITS Header </TITLE>\n"); */
        fprintf(l_vars->hfp, "<H1>FITS File Header</H1>\n");
        fprintf(l_vars->hfp, "<hr>\n");

        status = 0;
        while (status == 0) {
      
            fprintf(l_vars->hfp, "<pre>\n");
            keywordNumber = 0;
            /* read and get the number of existing keywords in the CHU  */
            FCGHSP(unit, &keywordNumber, &fitsSpace, &status);
 
            for (i=1;i<=keywordNumber;i++) {
                status = 0;
                FCGREC(unit,i,card,&status);
                if (status == 0) 
                    fprintf(l_vars->hfp,"%s\n",card);
                else {
                    FCGERR(status,errtxt);
                    gateway_err(l_vars->hfp, "Reading FITS file error\n",1,l_vars);
                }
            } 
            fprintf(l_vars->hfp,"END\n");
       
            fprintf(l_vars->hfp, "</pre>\n");

            fprintf(l_vars->hfp,"<HR>\n");

            status = 0; 
            /*C     now move to the next extension */
            FCMRHD(unit,1,&hdutyp,&status);
            FCGERR(status,errtxt);
    
        }
  
        break;
    }
    
    case HISTORY: {

        fprintf(l_vars->hfp,"<HR>");

        fprintf(l_vars->hfp, "<H1>Histories related to the FITS file </H1>\n");
        fprintf(l_vars->hfp, "<hr>\n");

        status=0;
        while (status == 0) {

	      histFlag = 0;
	      fprintf(l_vars->hfp, "<pre>\n");
	      
	      keywordNumber = 0;
	      /* read and get the number of existing keywords in the CHU  */
	      FCGHSP(unit, &keywordNumber, &fitsSpace, &status);
	    
	      for (i=1;i<=keywordNumber;i++) {
		status = 0;
                FCGREC(unit,i,card,&status);
                if (status == 0) {
		  if (!((char *)strcmp((char *)substr(card,1,7), "HISTORY"))) {
		    histFlag = 1;
		    fprintf(l_vars->hfp,"%s\n",card);
		  }
                }
                else {
		  FCGERR(status,errtxt);
		  gateway_err(l_vars->hfp, "Reading FITS file error",1,l_vars);
                }
	      } 
	      
	      fprintf(l_vars->hfp, "</pre>\n");
	      if (histFlag)
		fprintf(l_vars->hfp,"<HR>\n");
	   
	    status = 0; 
            /*C     now move to the next extension */
            FCMRHD(unit,1,&hdutyp,&status);
            FCGERR(status,errtxt);
    
        }
  
        break;
    }
    }  /* switch */

  
    /* Close temporary file */
    if (!l_vars->do_dump)
        fclose(l_vars->hfp);
 
    EXIT(2,"listhead");
    return(tmp);
    
} /* listhead */

#endif /* HAVE_FITS */

/* =========================================================  */

/* for sds */

#ifdef HAVE_HDF
/*--------------------------------------------------------------------------
 NAME
       sdsGetImage
 DESCRIPTION
      returns an ImageInfo struct corresponding to the named ref number,
      as sdsGrokRef.  If subsample == TRUE then subsample the 
      image to fit within a hdfImageSize .
      Return the backgroud pixel's index in bg only if the file Image
 RETURNS
       return NULL on failure
--------------------------------------------------------------------------*/
ImageInfo *
sdsGetImage(char *fname, 
            char *hdu,       /* hdu number of image */
            int subsample,   /* flag: TRUE - fit image within hdfImageSize^2 box */
            int *bg,         /* background pixel index */
            lvar_st *l_vars)
{
    ImageInfo *Image = NULL;    /* Image info struct */
    int32 ref;                  /* tag,ref of image */
    int   plane;                /* plane nuber */
    int   s;
    int32 rank;          /* rankof fits data */
    intn  i, j;
    int32 status;
    int32 w, h;          /* image width,height */
    intn  isp;           /* flag: TRUE- there is a palette with image */
    uint8 pal[768];       /* pallette */
    int   numPlane = 1;
    int32 sdid, sdsid;
    char  sdsName[25];
    int32 dimSize[20];
    int32 nt, nattrs;

    ENTER(2,"sdsGetImage");
    
    /* Get ref pair */
    if(sscanf(hdu, "ref=%d,s=%d,plane=%d",&ref,&s,&plane) == 3)  {

        /* Open file for reading */
        if ((sdid = SDstart(fname, DFACC_RDONLY)) == FAIL) {      
            return NULL;
        }

        /* get index for the specified dataset by the ref. */
        if (l_vars->do_netcdf)
          {
            i = ref; /* for netCDF we use the index to indicate ref
                      * as they really don't have NDG */
          }
        else
          {
              i = SDreftoindex(sdid, ref);
              if (i == FAIL)
                  return NULL;
          }
 
        /* select dataset */
        sdsid = SDselect(sdid, i);
        if (sdsid == FAIL) {
            return NULL;
        }
     
        /* Get dimensions of image */
        if ((status=SDgetinfo(sdsid,sdsName,&rank,dimSize,&nt,&nattrs))==FAIL)
            return NULL;
	
        if (rank <2)
            return NULL;

        w = dimSize[rank-1];
        h = dimSize[rank-2];
        for (i=0; i<rank-2; i++)
            numPlane *= dimSize[i];
	

        DBUG_PRINT(1,(LOGF," w=%d,h=%d\n", w,h));
        DBUG_PRINT(1,(LOGF," PLANE=%d\n", plane));

        /* allocate the basic structure */
        if ((Image = (ImageInfo *) HDmalloc(sizeof(ImageInfo))) == NULL)
            return NULL;

        /* allocate space for the data */
        if ((Image->image_data = (unsigned char *) 
             malloc(w * h * sizeof(unsigned char))) == NULL)
            return NULL;

        /* allocate the palette space */
        if ((Image->reds =   (int *)malloc(256 * sizeof(int))) == NULL)
            return NULL;
        if ((Image->greens = (int *)malloc(256 * sizeof(int))) == NULL)
            return NULL;
        if ((Image->blues =  (int *)malloc(256 * sizeof(int))) == NULL)
            return NULL;

        /* fill in the static fields */
        Image->ismap  = FALSE;
        Image->width  = w;
        Image->height = h;
        Image->num_colors = 256;

        DBUG_PRINT(1,(LOGF," image->width=%d\n", Image->width));
        DBUG_PRINT(1,(LOGF," image->height=%d\n", Image->height));

        /* read the image */
        if ((status = 
             sdsImage(sdsid, Image->image_data, plane)) == FAIL)
            return NULL;
	    
        /* set the palette */

        /* create a fake palette */
        for(i = 0; i < 256; i++)  {
		
            Image->reds  [i] = i;
            Image->greens[i] = i;
            Image->blues [i] = i;
        }
        Image->color_map = NULL;

        /* reset the pallete (test only) */
        /* setPallete("image.pal",Image->reds,Image->greens,Image->blues); */

        /* Handle sub-sampling */
        if(subsample) {
              
            int max   = (h > w ? h : w);
            int skip;   /* skip factor */
	    
            DBUG_PRINT(1,(LOGF, "[***] max '%d'\n", max));
            DBUG_PRINT(1,(LOGF, "[***] hdfImageSize '%d'\n", hdfImageSize));
            DBUG_PRINT(1,(LOGF," we are subsampling\n"));

            skip = max / hdfImageSize;

            if((skip)&&(w/skip)&&(h/skip))  {
              
                int i, j;
                int cnt = 0;
                unsigned char * newSpace = NULL;
		
                if (newSpace)
                    HDfree((unsigned char *)newSpace);

                /* allocate space for the data */
                if ((newSpace = (unsigned char *) \
                     HDmalloc(max * max * sizeof(unsigned char))) == NULL)
                    return NULL;

                /* sub-sample image */
                skip++;
                for(j = 0; j < h; j += skip)
                    for(i = 0; i < w; i += skip) 
                        newSpace[cnt++] = Image->image_data[i + j * w];

                /* Free previousl allocated space */
                HDfree((void *)(Image->image_data));

                /* Point to new subsampled image, 
                 * adjust height and width accordingly */
                Image->image_data = newSpace;  
                Image->height = h / skip;
                Image->width  = w / skip;
                if (w % skip) 
                    Image->width++;
            } /* end if "skip" */
        } /* end if "subsample" */

        /* Terminate access to the array  dataset  */
        SDendaccess(sdsid);

        /* Terminate access to the SD interface and close the file */
        SDend(sdid);

        EXIT(2,"sdsGetImage");
        return Image;
    } /* end if "sscanf()" */
    EXIT(2,"sdsGetImage");
} /* sdsGetImage */


/*----------------------------------------------------------------------------
 NAME
      sdsGrokRef
 DESCRIPTION
        The function sdsGrokRef() will return a description of what can
        be "seen" from a given location in the file. 
 RETURNS
      Returns the file name containg the HTML description or the GIF image
      if successful else NULL.
----------------------------------------------------------------------------*/
char *
sdsGrokRef(char *fname, 
           char *ref,
           entry entries[MAX_ENTRIES], 
           mime_content_type *mime_type, 
           int current_entry, 
           int num_entries,
           lvar_st *l_vars)
{
    int32 t, r;           /* tag/ref numbers */
    int32 x, y;
    int   sub_sample;
    /* Temporary file name for HTML desc */
    char *tmp = (char *)tempnam(NULL,"html"); 
    
    /* Temporary file name for HTML desc */
    char *gtmp = (char *)tempnam(NULL,"img"); 
 
    /* Temporary file name for HTML desc */
    char *ptmp = (char *)tempnam(NULL,"pal");  

    char  *data;
    int32 len, fid;
    ImageInfo *image;
    int   bkg = 0;
    int   i;
    static skip=FALSE;
    int32 vd_id;
    int32 flds_indices[100];
    char  sep[2];
    int32 file_handle;
    int32 an_handle;

    ENTER(2,"sdsGrokRef");
    DBUG_PRINT(1,(LOGF," ref=%s\n", ref));

    if (ref[0]==';')
        ref++;
    if(sscanf(ref, "ref=%d,s=%d", &r, &sub_sample) == 2) {
      
        DBUG_PRINT(1,(LOGF," got ref=%d\n",r));     
        /* Open temporary file name for HTML description */
        if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
            return(NULL);
	
        /* Open temporary file name for gif description */
        if ((l_vars->gfp = fopen(gtmp, "wb+")) == NULL)
            return(NULL);
              
        /* Write MIME header response */
        *mime_type = IMAGE_GIF;
        if (!skip) {                
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {   
                gateway_err(l_vars->hfp," writing HTML header",0,l_vars);
                return NULL;
            }
            skip = !(skip);
        }

        /* Get Image */
        if ((image = sdsGetImage(fname,ref,sub_sample,&bkg,l_vars))==NULL) {
            /* Close temporary gif file and html file*/
            fclose(l_vars->gfp);
            fclose(l_vars->hfp);
            return NULL;
        }
        DBUG_PRINT(1,(LOGF," sdsGetImage returned\n"));
        DBUG_PRINT(1,(LOGF," image->width=%d\n",image->width));
        DBUG_PRINT(1,(LOGF," image->height=%d\n",image->height));
        DBUG_PRINT(1,(LOGF," bkg=%d\n",bkg));

        mypixels = image->image_data;
        myrowlen = image->width;
        DBUG_PRINT(1,(LOGF,"calling GIFEncode \n"));

        GIFEncode(
            l_vars->gfp,                       /* file to write to */
            image->width,		/* width */
            image->height,		/* height */
            0,			/* No interlacing */
            bkg,			/* Index of Backgrounf */
            8,			/* Bits Per pixel */
            image->reds,		/* Red colormap */
            image->greens,		/* Green colormap */
            image->blues,		/* Blue colormap */
            myGetGIFPixel);		/* Get Pixel value */

        DBUG_PRINT(1,(LOGF,"GIFEncode returned\n"));

        /* Close temporary gif file and html file*/
        fclose(l_vars->gfp);
        fclose(l_vars->hfp);

        /* Free memory allocated in sdsGetImage */

        if (image->blues != NULL)
            HDfree(image->blues); 

        if (image->greens != NULL)
            HDfree(image->greens); 

        if (image->reds != NULL)
            HDfree(image->reds); 

        if (image->image_data != NULL)
            HDfree(image->image_data);

        if (image != NULL)
            HDfree(image);
   	    
        EXIT(2,"sdsGrokRef");
	/* 
        HDfree(tmp);
        HDfree(ptmp); */

        return(gtmp); /* return image file name */
    }

    else  { /* not able to get tag/ref */
      
        /* Open temporary file name for HTML description */
        if ((l_vars->hfp = fopen(tmp, "w")) == NULL)
            return(NULL);

        /* Write MIME header response */
        *mime_type = TEXT_HTML;
        if (!skip) {
          
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
              
                gateway_err(l_vars->hfp," writing HTML header",0,l_vars);
                return NULL;
            }

            skip = !(skip);
        }

        DBUG_PRINT(1,(LOGF," couldn't get tag/ref\n"));

        if(sscanf(ref, "fileImageHit?%d,%d", &x, &y) == 2) {
          
            char *buf;
            fprintf(l_vars->hfp, "<H2>File Image Hit</H2>\n");
            fprintf(l_vars->hfp, "We're sorry, but we really have no clue what is at location %d %d\n", x, y); 

            /* hmmm....not sure */
            buf = malloc(100);
            sprintf(buf, "#DataSet%d", 2);
        } 
        else if ((ref[0]!='#') && (ref[0]!='@') && l_vars->display_type == 0) {
          
            fprintf(l_vars->hfp, "<H2>Sorry, Bad Reference</H2>\n");
            fprintf(l_vars->hfp, "We're sorry, but reference <I>%s</I> is bad and we can't figure out what to do about it.<P>\n", 
                    ref);
        }
    } /* end else not able to get tag/ref */       
   
    DBUG_PRINT(1,(LOGF," lets see if this is an SDS, ref=%s\n",ref));
  
    /* Close temporary file and return its name */
    fclose(l_vars->hfp);
   
    EXIT(2,"sdsGrokRef");
    return(tmp);
} /* sdsGrokRef */



/*---------------------------------------------------------------------------
 NAME
       sdsGrokFile
 DESCRIPTION
        The function sdsGrokFile() will just take an SDS within the HDF file 
 RETURNS
        HTML file
----------------------------------------------------------------------------*/
char *
sdsGrokFile(char *fname, 
            entry entries[MAX_ENTRIES], 
            mime_content_type *mime_type,
            int current_entry,
            int num_entries,
            lvar_st *l_vars)
{
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *data;                       /* */
    int32 len;
    int32 fid;                        /* File handle */
    int32 ret;
    int32 ref;
    int32 sdid, sdsid;
    int32 i;
    int32 status;
    int32 rank, nt, nattr;
    int32 dimSize[20];
    char  name[25];

    ENTER(2,"sdsGrokFile");
    /* make sure we don't crash on invalid open */
    ncopts = 0;

    if (!l_vars->do_dump) {
        
        /* Open temproary file to write HTML description of HDF/netCDF file */
        if (!(l_vars->hfp = fopen(tmp, "w")))
            return(NULL);

        /* Write MIME header */
        *mime_type = TEXT_HTML;
        if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
          
            gateway_err(l_vars->hfp,"sdsGrokFile: writing HTML header",0,l_vars);
            return NULL;
        }
    }

    if (!l_vars->do_fits) { /* hdf or netcdf */
      
        DBUG_PRINT(3,(LOGF,"Opening file = %s for Vxx \n",fname));
       
	/* set display mode */
	l_vars->display_mode = 2;
	
	/* Do SDSs */
	DBUG_PRINT(3,(LOGF,"sdsGrokFile: doing sds \n"));

	/* process SDS in file */
	if (do_sds(fname, l_vars) == FAIL) {
	  
	  gateway_err(l_vars->hfp,"sdsGrokFile: error processing SDS ",0,l_vars);
	  return NULL;
	}

	/* get reference number */
	if (sscanf(l_vars->hdf_ref,"ref=%d,plane=%d",&ref,&l_vars->plane)==2) {
	  
	  DBUG_PRINT(3,(LOGF,"plane=%d \n", l_vars->plane));
	  
	  /* Open file for reading */
	  if ((sdid = SDstart(fname, DFACC_RDONLY)) == FAIL) {      
	  
	    DBUG_PRINT(3,(LOGF,"sdsGrokFile: Can not do SDstart\n"));
	    gateway_err(l_vars->hfp,"sdsGrokFile: opening file ",0,l_vars);
	    return NULL;
	  }
	  
	  /* get index for the specified dataset by the ref. */
	  if (l_vars->do_netcdf) {     
            i = ref; /* for netCDF we use the index to indicate ref
                      * as they really don't have NDG */
          }
	  else {       
	    i = SDreftoindex(sdid, ref);
	    if (i == FAIL) {
	      DBUG_PRINT(3,(LOGF,"sdsGrokFile: Can not do SDreftoindex\n"));
	      return NULL;
	    }
	  }

	  /* select dataset */
	  sdsid = SDselect(sdid, i);
	  if (sdsid == FAIL) {
	    DBUG_PRINT(3,(LOGF,"sdsGrokFile: Can not do SDselect\n"));
	    gateway_err(l_vars->hfp,"do_sds: opening file ",0,l_vars);
	    return NULL;
	  }
	  
	  
	  /* get dtatset info */
	  status = SDgetinfo(sdsid, name, &rank, dimSize, &nt, &nattr);
	  if (status == FAIL) {
	    DBUG_PRINT(3,(LOGF,"sdsGrokFile: SDgetinfo fail\n"));
	    return NULL;
	  }
	  else {
	    strcpy(l_vars->hdf_ref, " ");
	    strcat(l_vars->hdf_ref, name);
	  }
	  
	  
	  /* Terminate access to the array  dataset  */
	  SDendaccess(sdsid);
	  
	  /* Terminate access to the SD interface and close the file */
	  SDend(sdid);
	  
	}
	
	
	DBUG_PRINT(3,(LOGF,"Test l_vars->hdf_ref = %s  \n",l_vars->hdf_ref));
	
	if (!l_vars->do_dump)	
	  dump_sds(fname, current_entry, entries, num_entries,l_vars);
	
	if (!l_vars->do_dump) {  
	  
	  /* Write MIME trailer */
	  *mime_type = TEXT_HTML;
	  if (write_html_trailer(l_vars->hfp, *mime_type,l_vars) == FAIL) {
	    
	    gateway_err(l_vars->hfp," writing HTML trailer",0,l_vars);
	    return NULL;
	  }
	}

    }/* sdbGrokFile */
    
    /* Close temporary file */
    if (!l_vars->do_dump)
      fclose(l_vars->hfp);
    EXIT(2,"sdsGrokFile");
    return(tmp);
}
#endif /* HAVE_HDF */

/*---------------------------------------------------------------------------
 NAME
       sdbGrokView
 DESCRIPTION
        The function sdbGrokView() will preview all of the planes within the image

        
 RETURNS
----------------------------------------------------------------------------*/
char *
sdbGrokView(char *fname,  char *ref,
            entry entries[MAX_ENTRIES], 
            mime_content_type *mime_type,
            int current_entry,
            int num_entries,
            lvar_st *l_vars)
{
    char *tmp = (char *)tempnam(NULL,"html");  /* Temporary file name for HTML desc */
    char *data;                      
    int32 len;
    int32 fid;                        /* File handle */
    int32 ret;
    int32 count, start,end;            /* plane varies */
    int   i;
    int   refNumber;


    ENTER(2,"sdbGrokView");
     
    if (ref[0]==';')
        ref++;
  
    DBUG_PRINT(1,(LOGF,"ref = %s\n",ref )); 
   
    if(sscanf(ref, "ref=%d,start=%d,end=%d",&refNumber,&start,&end) == 3) {
	
        if (!l_vars->do_dump) { 
      
            /* Open temproary file to write HTML description  */
            if (!(l_vars->hfp = fopen(tmp, "w")))
                return(NULL);

            /* Write MIME header */
            *mime_type = TEXT_HTML;
            if (write_html_header(l_vars->hfp, *mime_type,l_vars) == FAIL) {
          
                gateway_err(l_vars->hfp,"sdbGrokView: writing HTML header",0,l_vars);
                return NULL;
            }
        }

        /* read planes from file */
        /* Print Image header stuff in HTML  */
        count = end - start +1;

        fprintf(l_vars->hfp, "<H2>Preview the group of the plane within the Image</H2>\n");
    
        fprintf(l_vars->hfp, "Total Number of Planes: %d(from:%d to:%d)<p>\n",
                count,start,end);

        /* For each plane in the file */
        for(i = start; i <= end; i++) {

            /* Print out image info in HTML depending on user prefrences */
            fprintf(l_vars->hfp,"<A HREF=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=%d\"><IMG SRC=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=%d\"></A> \n", 
                    l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
                    refNumber, 0,i, l_vars->h_env->script_name, 
                    l_vars->h_env->path_info,l_vars->f_name, refNumber, 1, i);
        }
    } /* if(sscanf(ref, "ref=%d,plane=%d", &fits_ext, &count) == 2) { */
    else {
        return NULL;
    }
   
    
    /* Close temporary file */
    if (!l_vars->do_dump)
        fclose(l_vars->hfp);
  
    EXIT(2,"sdbGrokView");
    return(tmp);
    
}  



/* ====================================================== */


/*
 * return value 0 -> not recognized
 *              1 -> HDF file
 *              2 -> netCDF file
 *              3 -> Fits file
 *             -1 -> FAIL
 */
int
filetype(char *fname)
{
    char hdf_type[4]    = {0x0e, 0x03, 0x13, 0x01};
    char netcdf_type[4] = {0x43, 0x44, 0x46, 0x01};
    char fits_type[4]   = {0x53, 0x49, 0x4d, 0x50};
    char ftype[4] = {0,0,0,0};
    FILE *fhandle = NULL;
    int ret_value = 0;

    DBUG_PRINT(1,(LOGF,"filetype: opening %s\n",fname));
    if ((fhandle = fopen(fname,"r")) == NULL)
      {
          DBUG_PRINT(1,(LOGF,"filetype: failed to open file \n"));
          ret_value = -1;
          goto done;
      }
    if (fread(ftype,1,4,fhandle) == 0)
      {
          DBUG_PRINT(1,(LOGF,"filetype: failed to read 4 bytes \n"));
          ret_value = -1;
          goto done;
      }
    if (!memcmp(ftype,hdf_type,4))
      {
          DBUG_PRINT(1,(LOGF,"this is an HDF file\n"));
          ret_value = 1;
      }
    else if (!memcmp(ftype,netcdf_type,4))
      {
          DBUG_PRINT(1,(LOGF,"this is a netCDF file\n"));
          ret_value = 2;
      }
    else if (!memcmp(ftype,fits_type,4))
      {
          DBUG_PRINT(1,(LOGF,"this is a Fits file\n"));
          ret_value = 3;
      }
    else 
      {
          DBUG_PRINT(1,(LOGF,"this is an unknown file type\n"));
      }

    /* close file */
    fclose(fhandle);
  done:
    if (ret_value == -1)
      {
          if (fhandle != NULL)
              fclose(fhandle);
      }
    return ret_value;
} /* filetype() */

/*-------------------------------------------------------------------- 
 NAME
     get_env
 DESCRIPTION
      Gets the enviromental variables that might be useful to us      

 RETURNS
     
--------------------------------------------------------------------*/ 
void
get_env(hserv_st *s_env )
{
    /* Get relavant enironment variables passed to us by httpd server */
    s_env->request_method = getenv("REQUEST_METHOD");
    s_env->path_info      = getenv("PATH_INFO");
    DBUG_PRINT(1,(LOGF,"path_info=%s\n",s_env->path_info));
    s_env->path_translated = getenv("PATH_TRANSLATED");
    DBUG_PRINT(1,(LOGF,"path_translated=%s\n",s_env->path_translated));
    s_env->script_name     = getenv("SCRIPT_NAME");
    DBUG_PRINT(1,(LOGF,"script_name=%s\n",s_env->script_name));
    s_env->query_string    = getenv("QUERY_STRING");
    s_env->remote_host     = getenv("REMOTE_HOST");
    s_env->remote_addr     = getenv("REMOTE_ADDR");
    s_env->auth_type       = getenv("AUTH_TYPE");
    s_env->remote_user     = getenv("REMOTE_USER");
    s_env->content_type    = getenv("CONTENT_TYPE");
    s_env->content_length  = getenv("CONTENT_LENGTH");
    s_env->http_accept     = getenv("HTTP_ACCEPT");
    s_env->http_user_agent = getenv("HTTP_USER_AGENT");

} /* get_env() */

/*-------------------------------------------------------------------- 
 NAME
     
 DESCRIPTION
   Initialze all local varibles in global struct /  

 RETURNS
     
--------------------------------------------------------------------*/ 
void
do_init(lvar_st *l_vars)
{
    l_vars->hfp = NULL;
    l_vars->gfp = NULL;
    l_vars->brief = 0;
    l_vars->my_url = NULL;
    l_vars->hdf_path = NULL;
    l_vars->hdf_path_t = NULL;
    l_vars->f_name     = NULL;
    l_vars->f_path_r   = NULL;
    l_vars->hdf_file   = NULL;
    l_vars->hdf_file_r = NULL;
    l_vars->hdf_ref    = NULL;
#if 0
    l_vars->mypixels   = NULL;
    l_vars->myrowlen   = 0;
#endif
    l_vars->do_dump    = 0;
    l_vars->display_mode  = 1;
    l_vars->display_type  = 0;
    l_vars->cgi_path   = NULL;
    l_vars->hdf_file_name = NULL;
    l_vars->hdf_path_r     = NULL;
    l_vars->html_dir      = NULL;
    l_vars->do_cci        = 0;
    l_vars->cci_port      = -1;
    l_vars->cci_host      = NULL;
    l_vars->dtm_outport   = NULL;
    l_vars->do_fits       = 0;  
    l_vars->unit          = 0;  
    l_vars->hdu           = 0;  
    l_vars->plane         = 1;
    DBUG_PRINT(1,(LOGF,"l_vars->plane = %d\n",l_vars->plane )); 
    l_vars->do_netcdf     = 0;
    l_vars->sds_list      = NULL;
    l_vars->gattr_list    = NULL;
    l_vars->dimvar_list   = NULL;
    l_vars->regvar_list   = NULL;
    l_vars->pal_list   = NULL;
    l_vars->rig_list   = NULL;
    l_vars->an_list   = NULL;
    l_vars->flan_list   = NULL;
    l_vars->fdan_list   = NULL;
    l_vars->olan_list   = NULL;
    l_vars->odan_list   = NULL;
    l_vars->vg_list   = NULL;
    l_vars->tvg_list   = NULL;
    l_vars->ovg_list   = NULL;
    l_vars->vd_list   = NULL;
    l_vars->nfields   = 0;
    l_vars->field_indices = NULL;
    l_vars->vh_start_rec = 0;
    l_vars->vh_end_rec = 0;

    /* for fits PHDU or IMAGE and SDS */
    l_vars->sdb_ref = 0;
    l_vars->sdb_imgFlag     = 0;
    l_vars->sdb_plane       = 0;
    l_vars->sdb_start_rec   = 0;
    l_vars->sdb_end_rec     = 0;
    /* end of for fits PHDU or IMAGE and SDS */

    /* for fits table */
    l_vars->ft_tabFlag     = 0;
    l_vars->ft_ref         = 0;
    l_vars->ft_nfields     = 0;
    l_vars->ft_fld_indices = NULL;
    l_vars->ft_start_rec   = 0;
    l_vars->ft_end_rec     = 0;
    /* end of for fits table */

    l_vars->obj_tag = 0;
    l_vars->obj_ref = 0;
    l_vars->sub_s = 0;
    l_vars->sd_dim_start = NULL;
    l_vars->sd_dim_stride = NULL;
    l_vars->sd_dim_end = NULL;
    l_vars->ndims = 0;
#ifdef HAVE_CCI
    MCCIInitialize();
#endif
} /* do_init () */

/* ----------------------------------------------------------------------

			      Main Function

   ---------------------------------------------------------------------- */
/*
 * do the main thang.
 *
 * Note that there is some UGLY code that got put in previously
 * to handle SDS which should be revamped in the next revision
 *
 */

#if defined(sun) || defined(__sun__) 
/* for SUN (Call Fortran Library in C ) */
int MAIN_;
#endif

int
main(int argc, char *argv[]) 
{
    entry entries[MAX_ENTRIES];  /* array to hold query string */
    register int x;              /* loop array */
    register int m=0;
    char *html_desc;             /* HTML description of target */
    char *html_file;             /* HTML file name */
    int  target;              /* flag: Positive - HDF reference, NULL - HDF file */
    mime_content_type mime_type; /* reply type e.g. text/html */
    int no_path=FALSE;
    int current_entry=0;
    int ret;
    int c;
    int errflag = 0;
    int no_query;
    char *cl;                    /* Pointer to query string */
    int  pcl;
    char hdf_html_file[1024];
    lvar_st  l_vars;            /* Local variables */
    hserv_st myserv_env;        /* Server environemnt variables */
    int file_type = -1;
    int unit, status;


    DBUG_OPEN("/tmp/sdb2.log","a+"); 
    DBUG_PRINT(1,(LOGF,"************SDB Gateway starting******** \n")); 

    /* Initialize local variables */
    do_init(&l_vars);

    /* Process command line arguments if any..*/
    while((c = getopt(argc,argv,":dic:f:r:D:H:p:h")) != -1) 
      {
          switch(c) 
            {
            case 'd': /* dump html desc */
                l_vars.do_dump = 1;
                break;
            case 'i': /* do cci */
#ifdef HAVE_CCI
                l_vars.do_cci = 1;
#else
                gateway_err(stdout,"This version does not support CCI\n",1,&l_vars);
#endif
                break;
            case 'c': /*cgi path*/
                if ((l_vars.cgi_path = (char *)HDgetspace(HDstrlen(optarg)+1)) == NULL)
                    gateway_err(stdout,"Unable to allocte space for variable on server\n",1,&l_vars);
                strcpy(l_vars.cgi_path,optarg);
                break;
            case 'f': /* HDF file name to dump */
                if ((l_vars.hdf_file_name = (char *)HDgetspace(HDstrlen(optarg)+1)) == NULL)
                    gateway_err(stdout,"Unable to allocte space for variable on server\n",1,&l_vars);
                strcpy(l_vars.hdf_file_name,optarg);
                break;
            case 'r': /* HDF relative path to use */
                if ((l_vars.hdf_path_r = (char *)HDgetspace(HDstrlen(optarg)+1)) == NULL)
                    gateway_err(stdout,"Unable to allocte space for variable on server\n",1,&l_vars);
                strcpy(l_vars.hdf_path_r,optarg);
                break;
            case 'D': /* directory to dump html files */
                if ((l_vars.html_dir = (char *)HDgetspace(HDstrlen(optarg)+1)) == NULL)
                    gateway_err(stdout,"Unable to allocte space for variable on server\n",1,&l_vars);
                strcpy(l_vars.html_dir,optarg);
                break;
            case 'H': /* CCI host */
#ifdef HAVE_CCI
                if ((l_vars.cci_host = (char *)HDgetspace(HDstrlen(optarg)+1)) == NULL)
                    gateway_err(stdout,"Unable to allocte space for variable on server\n",1,&l_vars);
                strcpy(l_vars.cci_host,optarg);
#else
                gateway_err(stdout,"This version does not support CCI\n",1,&l_vars);
#endif
                break;
            case 'p': /* CCI host port*/
#ifdef HAVE_CCI
                l_vars.cci_port = atoi(optarg);
#else
                gateway_err(stdout,"This version does not support CCI\n",1,&l_vars);
#endif
                break;
            case 'h': /* print help & usage */
                print_strings(stdout,(char **)&hdf_usage);
                exit(0);
                break;
            case ':':  /* handle -c,-f,-r, -D, -H, -p without arguments */
                if (l_vars.do_dump)
                    fprintf(stderr, "Option -%c requires an argument\n", optopt);
                else
                    gateway_err(stdout,"Option -%c requires an arguemnt \n",1,&l_vars);
                errflag++;
                break;
            case '?': /* Print usage otherwise....*/ 
                errflag++;
                break;
            }
      } /* end while */

    /* Check for any errors reading comand line arguments */
    if (errflag)
      {
          if(l_vars.do_dump || l_vars.do_cci)
              print_strings(stdout,(char **)&hdf_usage);
          else
              gateway_err(stderr,"Wrong number of arguments, client may not support HDF.",1,&l_vars);
      }

    /* Check for proper arguements */
    if (argc < 1)
        gateway_err(stderr,"Wrong number of arguments, client may not support HDF.",1,&l_vars);

 
    if(l_vars.do_dump || l_vars.do_cci)
      {
          DBUG_PRINT(1,(LOGF,"Enabled dumping/cci \n"));
          /* Get HDF file name from entry */
          if (pull_filename(l_vars.hdf_file_name,&l_vars) == FAIL)
              gateway_err(stderr,"error getting hdf filename \n",1,&l_vars);

          /* Check relative name path to create HDF path to show in URL */
          if (strcmp(l_vars.f_path_r,"."))
              l_vars.hdf_path = (char *)mk_path_name(2,'/',l_vars.f_path_r,l_vars.f_name);
          else
              l_vars.hdf_path = l_vars.f_name;
          DBUG_PRINT(1,(LOGF,"hdf_path =%s \n", l_vars.hdf_path));

          /* in this case the translated path is set to "." to force picking up
           * correct path */
          l_vars.hdf_path_t = "."; 

          /* Create full file name with path */
          if (strcmp(l_vars.hdf_path_t,"."))
              l_vars.hdf_file = (char *)mk_path_name(2,'/',l_vars.hdf_path_t,l_vars.hdf_path);
          else
              l_vars.hdf_file = l_vars.hdf_path;
          DBUG_PRINT(1,(LOGF,"hdf_file = %s \n", l_vars.hdf_file));

      }

    /* Are we dumping info html files or we processing a request from user */
    if (!l_vars.do_dump && !l_vars.do_cci)
      { /* Processing a request from user*/

          DBUG_PRINT(1,(LOGF,"processing request from user \n"));

          /* get Server Environment variables */
          get_env(&myserv_env);
          DBUG_PRINT(1,(LOGF,"got environmnet variables \n"));
          l_vars.h_env = &myserv_env;

          /* switch on METHOD */
          if(!strcmp(myserv_env.request_method,"GET")) 
            {
                DBUG_PRINT(1,(LOGF,"METHOD is GET=%s\n",myserv_env.request_method));

                /* Get relative path */
                l_vars.hdf_path_t = myserv_env.path_translated;
                DBUG_PRINT(1,(LOGF,"hdf_path_t =%s \n", l_vars.hdf_path_t));

                current_entry = 1;

                /* Get the Query string */
                cl = getenv("QUERY_STRING");
                if (cl == NULL) 
                    gateway_err(stderr,"No HDF/netCDF/Fits file or reference number specified.\n",1,&l_vars);
                DBUG_PRINT(1,(LOGF,"query_string = %s \n", cl));

                /* Decode query string and place in "entries" array 
                 * may need to change since we use "hdfref;tag=x,ref=y" format
                 * which cause trouble with FORMs...*/
                for(x = 0; cl[0] != '\0'; x++) 
                  {
                      m = x;
                      entries[x].val = (char *)HDgetspace(128*sizeof(char));
                      entries[x].name = (char *)HDgetspace(128*sizeof(char));
                      getword(entries[x].val,cl,'&');
                      plustospace(entries[x].val);
                      unescape_url(entries[x].val);
                      getword(entries[x].name,entries[x].val,'|');
                      DBUG_PRINT(2,(LOGF,"entries[%d].name=%s \n", x,entries[x].name));
                      DBUG_PRINT(2,(LOGF,"entries[%d].val=%s \n", x,entries[x].val));
                  }

                /* Get HDF file name from entry */
                if (pull_filename(entries[0].name,&l_vars) == FAIL)
                    gateway_err(stderr,"error getting hdf filename \n",1,&l_vars);

                /* Get proper target ie. hdf ref or file name */
                target = pull_ref(entries[0].name,&l_vars);
                DBUG_PRINT(1,(LOGF,"target = %d \n", target));
                DBUG_PRINT(1,(LOGF,"myserv_env.path_info = %s \n", myserv_env.path_info));
                DBUG_PRINT(1,(LOGF,"l_vars.f_path_r = %s \n", l_vars.f_path_r));

                /* Get DTM port if specified */
                if (pull_dtmport(entries[0].name,&l_vars) != FAIL)
                  {
#if 0
                      mo_dtm_out(l_vars.dtm_outport,&l_vars);
                      mo_dtm_poll_and_read ();
#endif
                  }

                /* Check relative name path */
                if (strcmp(l_vars.f_path_r,"."))
                    l_vars.hdf_path = (char *)mk_path_name(2,'/',l_vars.f_path_r,l_vars.f_name);
                else
                    l_vars.hdf_path = strdup(l_vars.f_name);
                DBUG_PRINT(1,(LOGF,"hdf_path =%s \n", l_vars.hdf_path));

                DBUG_PRINT(1,(LOGF,"l_vars.hdf_path_t = %s \n", l_vars.hdf_path_t));
                /* Create full file name with path */
                if (strcmp(l_vars.hdf_path_t,"."))
                    l_vars.hdf_file = (char *)mk_path_name(2,'/',l_vars.hdf_path_t,l_vars.hdf_path);
                else
                    l_vars.hdf_file = strdup(l_vars.hdf_path);
                DBUG_PRINT(1,(LOGF,"hdf_file = %s \n", l_vars.hdf_file));

                /* determine the type of file */
                switch(file_type = filetype(l_vars.hdf_file))
                  {
                  case 1: /* HDF file */
                      l_vars.do_netcdf = 0;
                      l_vars.do_fits = 0;
                      break;
                  case 2: /* netCDF file */
                      l_vars.do_netcdf = 1;
                      l_vars.do_fits = 0;
                      break;
                  case 3: /* Fits file */
                      l_vars.do_netcdf = 0;
                      l_vars.do_fits = 1;
                      break;
                  case -1: /* error */
                  case 0:
                  default:
                      gateway_err(stderr,"main: error in filetype  ",0,&l_vars);
                      break;
                  }

                if (file_type == 1 || file_type == 2)
                  {
#ifdef HAVE_HDF
                      /* Act upon whether it is a HDF reference number of file name */
                      if (target == FAIL) {                
                          l_vars.hdf_ref = entries[0].name;
                          if ((html_file=(char *)sdbGrokFile(l_vars.hdf_file, entries, 
                                                             &mime_type, current_entry, 
                                                             m,&l_vars)) == NULL)
                              gateway_err(stderr,"main: sdbGrokFile ",1,&l_vars);
                      }
                      else {
                          if (target == SDBPLANE) {
                              if ((html_file=(char*)sdsGrokFile(l_vars.hdf_file,entries, 
                                                                &mime_type, current_entry, 
                                                                m,&l_vars)) == NULL)
                                  gateway_err(stderr,"main: sdsGrokFile ",1,&l_vars);
                          }
	
                          else {
                              if (target == SDBREF) {
                                  if ((html_file=(char*)sdsGrokRef(l_vars.hdf_file,
                                                                   l_vars.hdf_ref, entries, 
                                                                   &mime_type, current_entry, 
                                                                   m,&l_vars)) == NULL)  
                                      gateway_err(stderr,"main: sdsGrokRef ",1,&l_vars);

                              }
                              else {
                                  if (target == SDBVIEW) {
	      
                                      if ((html_file = (char *)sdbGrokView(l_vars.hdf_file,
                                                                           l_vars.hdf_ref,entries, 
                                                                           &mime_type,current_entry,
                                                                           m,&l_vars))
                                          == NULL) 
                                          gateway_err(stderr,"main: sdbGrokView",1,&l_vars);
                                  }

                                  else {
              
                                      if ((html_file =(char *)sdbGrokRef(l_vars.hdf_file,
                                                                         l_vars.hdf_ref,
                                                                         entries,&mime_type, 
                                                                         current_entry, m,&l_vars))
                                          == NULL)
                                          gateway_err(stderr,"main: sdbGrokFile ",1,&l_vars);
                                  }
                              }
                          }
                      }
#endif /* HAVE_HDF */
                  }
                else /* FITS */
                  {
#ifdef HAVE_FITS
                      l_vars.do_fits = 1;
                      /* Act upon whether it is a FITS reference number of file name */
                      if ((target == FAIL)||(target == SDBPLANE)) {
            
                          l_vars.hdf_ref = entries[0].name;
                          if ((html_file =(char *)sdbGrokFile(l_vars.hdf_file, entries, 
                                                              &mime_type, current_entry, 
                                                              m,&l_vars)) == NULL) 
	      
                              gateway_err(stderr,"main: sdbGrokFile ",1,&l_vars);
                      }
                      else {
                          unit = openFits(l_vars.hdf_file, 0);	  
                          l_vars.unit = unit;
                          if (target == SDBVIEW) {
	      
                              if ((html_file = (char *)sdbGrokView(l_vars.hdf_file,
                                                                   l_vars.hdf_ref,
                                                                   entries,&mime_type, 
                                                                   current_entry, m,&l_vars))
                                  == NULL) 
                                  gateway_err(stderr,"main: fitsGrokHDU ",1,&l_vars);
	      
                          }
	    
                          else {
	      
                              if (target == FITSTAB) {
		
                                  if ((html_file = (char *)fitsGrokTab(unit,
                                                                       l_vars.hdf_ref,
                                                                       entries,&mime_type, 
                                                                       current_entry, m,&l_vars))
                                      == NULL) 
                                      gateway_err(stderr,"main: fitsGrokTab ",1,&l_vars);	     
                              }
                              else {
		
                                  unit = openFits(l_vars.hdf_file, 0);	  
                                  l_vars.unit = unit;
                                  if ((target == LISTHEAD)||(target == HEAD)||(target==HISTORY)) {
		  
                                      ENTER(2,"listhead");
		  
                                      if ((html_file = (char *)listhead(unit,
                                                                        l_vars.hdf_ref,
                                                                        entries,&mime_type, 
                                                                        current_entry, m,&l_vars,target))
                                          == NULL) 
                                          gateway_err(stderr,"main: listhead ",1,&l_vars);

                                  }
		
                                  else {
                                      if (target == FITSBINTAB) {
	      
                                          if ((html_file = (char *)fitsGrokBinTab(unit,
                                                                                  l_vars.hdf_ref,
                                                                                  entries,&mime_type, 
                                                                                  current_entry, m,&l_vars))
                                              == NULL) 
                                              gateway_err(stderr,"main: fitsGrokBinTab ",1,&l_vars);	     
                                      }
		  
                                      else {
                                          if ((html_file = (char *)fitsGrokHDU(unit,
                                                                               l_vars.hdf_ref,
                                                                               entries,&mime_type, 
                                                                               current_entry, m,&l_vars))
                                              == NULL) 
                                              gateway_err(stderr,"main: fitsGrokHDU ",1,&l_vars);
                                      }
                                  }
                              }
                          }
                      }
#endif /* HAVE_FITS */
                  }
                /* send reply to client based on mime_type*/
                if (send_reply(stdout, html_file, mime_type,&l_vars) == FAIL)
                    gateway_err(stderr,"Error sending reply ",1,&l_vars);

                DBUG_PRINT(1,(LOGF,"Cleaning up... \n\n\n"));
                /* Clean up...*/
                if (l_vars.hdf_path != NULL)
                    FREE_CLEAR(l_vars.hdf_path);
                if (l_vars.hdf_file != NULL)
                    FREE_CLEAR(l_vars.hdf_file);
                if (l_vars.f_name != NULL)
                    FREE_CLEAR(l_vars.f_name);
                if (html_file != NULL)
                    free(html_file);
            }
          else if(!strcmp(myserv_env.request_method,"POST"))
            { /* We are using the POST method */
              
                DBUG_PRINT(1,(LOGF,"METHOD is POST=%s\n",myserv_env.request_method));
                if(strcmp(getenv("CONTENT_TYPE"),"application/x-www-form-urlencoded")) 
                  {
                      printf("This CGI script can only be used to decode form results. \n");
                      exit(1);
                  }
                pcl = atoi(getenv("CONTENT_LENGTH"));

                /* extract info off STDIN */
                for(x=0;pcl && (!feof(stdin));x++) 
                  {
                      m = x; /* number of entries */
                      entries[x].val = (char *)fmakeword(stdin,'&',(int *)&pcl);
                      plustospace(entries[x].val);
                      unescape_url(entries[x].val);
                      entries[x].name = (char *)makeword((char *)entries[x].val,'=');
                      DBUG_PRINT(2,(LOGF,"entries[%d].name=%s \n", x,entries[x].name));
                      DBUG_PRINT(2,(LOGF,"entries[%d].val=%s \n", x,entries[x].val));
                      /* ============ get variables ================ */
                      /* Get file name i.e. our target 
                       *  hdf_path is used for SDS's */
                      if(!strcmp(entries[x].name,"f_name") 
                         || !strcmp(entries[x].name,"hdf_path"))
                        {
                            if (pull_filename(entries[x].val,&l_vars) == FAIL)
                                gateway_err(stderr,"error getting hdf filename \n",1,&l_vars);
                        }

                      /* Get the new display mode */
                      if(!strcmp(entries[x].name,"display_mode"))
                          l_vars.display_mode = atoi(entries[x].val);

                      /* Get the new display type */
                      if(!strcmp(entries[x].name,"display_type"))
                          l_vars.display_type = atoi(entries[x].val);

                      /* get DTM port */
                      if(!strcmp(entries[x].name,"dtmport"))
                        {
                            if ((l_vars.dtm_outport = (char *)HDmalloc(sizeof(char)*(HDstrlen(entries[x].val)+1))) == NULL)
                                gateway_err(stderr,"error allocateing space for DTMport \n",1,&l_vars);
                            HDstrcpy(l_vars.dtm_outport,entries[x].val);
                        }

                      /* get tag/ref */
                      if(!strcmp(entries[x].name,"hdfref"))
                        {
                            l_vars.hdf_ref = entries[x].val;
                        }

                      /* Vdata field  names */
                      if(!strcmp(entries[x].name,"VH_FIELD"))
                        {
                            if (l_vars.field_indices == NULL)
                              {
                                  if ((l_vars.field_indices = (int *)HDmalloc(sizeof(int)*257)) == NULL)
                                      gateway_err(stderr,"error allocateing space for field indices \n",1,&l_vars);
                              }
                            l_vars.field_indices[l_vars.nfields] = atoi(entries[x].val);
                            l_vars.nfields++;
                        }

                      /* Get the Vdata start record */
                      if(!strcmp(entries[x].name,"VH_START"))
                          l_vars.vh_start_rec = atoi(entries[x].val);

                      /* Get the Vdata ending record */
                      if(!strcmp(entries[x].name,"VH_END"))
                          l_vars.vh_end_rec = atoi(entries[x].val);

                      /* SDS rank dimensions */
                      if(!strcmp(entries[x].name,"SD_RANK"))
                        {
                            if (l_vars.sd_dim_start == NULL)
                              {
                                  if ((l_vars.sd_dim_start = (int32 *)HDmalloc(sizeof(int32)*(atoi(entries[x].val)))) == NULL)
                                      gateway_err(stderr,"error allocateing space for dim start  indices \n",1,&l_vars);
                              }
                            if (l_vars.sd_dim_stride == NULL)
                              {
                                  if ((l_vars.sd_dim_stride = (int32 *)HDmalloc(sizeof(int32)*(atoi(entries[x].val)))) == NULL)
                                      gateway_err(stderr,"error allocateing space for dim stride  indices \n",1,&l_vars);
                              }
                            if (l_vars.sd_dim_end == NULL)
                              {
                                  if ((l_vars.sd_dim_end = (int32 *)HDmalloc(sizeof(int32)*(atoi(entries[x].val)))) == NULL)
                                      gateway_err(stderr,"error allocateing space for dim end  indices \n",1,&l_vars);
                              }
                        }

                      /* SDS dimensions */
                      if(!strcmp(entries[x].name,"SD_DIM_START"))
                        {
                            if (l_vars.sd_dim_start != NULL)
                                l_vars.sd_dim_start[l_vars.ndims] = atoi(entries[x].val);
                        }

                      if(!strcmp(entries[x].name,"SD_DIM_STRIDE"))
                        {
                            if (l_vars.sd_dim_stride != NULL)
                                l_vars.sd_dim_stride[l_vars.ndims] = atoi(entries[x].val);
                        }

                      if(!strcmp(entries[x].name,"SD_DIM_END"))
                        {
                            if (l_vars.sd_dim_end != NULL)
                              {
                                  l_vars.sd_dim_end[l_vars.ndims] = atoi(entries[x].val);
                                  l_vars.ndims++; /* increment to next dimension */
                              }
                        }

                      /* ==========    variables for fits table  ===========*/
                      if(!strcmp(entries[x].name,"FT_FIELD")) {
                          if (l_vars.ft_fld_indices == NULL) {
                              if ((l_vars.ft_fld_indices =                \
                                   (int *)HDmalloc(sizeof(int)*257)) == NULL)
                                  gateway_err(stderr,"error allocateing space for field indices \n",1,&l_vars);
                          }
                          l_vars.ft_fld_indices[l_vars.ft_nfields] = atoi(entries[x].val);
                          l_vars.ft_nfields++;
                      }

                      /* Get the  start record */
                      if(!strcmp(entries[x].name,"FT_START"))
                          l_vars.ft_start_rec = atoi(entries[x].val);

                      /* Get the  ending record */
                      if(!strcmp(entries[x].name,"FT_END"))
                          l_vars.ft_end_rec = atoi(entries[x].val);
		                           
                      if(!strcmp(entries[x].name,"FT_REF"))
                          l_vars.ft_ref = atoi(entries[x].val);
		   
                      if(!strcmp(entries[x].name,"FT_TABFLAG"))
                          l_vars.ft_tabFlag = atoi(entries[x].val);
                      /* end of  variables for fits table */

                      /* ===variables for sds or phdu or image extension=== */
                      /* variables for fits PHDU or IMAGE Extension */
                      if (!strcmp(entries[x].name,"SDB_PLANE"))
                          l_vars.sdb_plane = atoi(entries[x].val);
	    
                      if (!strcmp(entries[x].name,"SDB_IMGFLAG"))
                          l_vars.sdb_imgFlag = atoi(entries[x].val);

                      if (!strcmp(entries[x].name,"SDB_REF"))
                          l_vars.sdb_ref = atoi(entries[x].val);

                      if (!strcmp(entries[x].name,"SDB_START"))
                          l_vars.sdb_start_rec = atoi(entries[x].val);

                      if (!strcmp(entries[x].name,"SDB_END"))
                          l_vars.sdb_end_rec = atoi(entries[x].val);

                  } /* end for */

                /* Did we get a valid file name to work with */
                if(l_vars.f_name != NULL)
                  {
                      l_vars.hdf_path_t = myserv_env.path_translated;
                      /* Check relative name path */
                      if (strcmp(l_vars.f_path_r,"."))
                          l_vars.hdf_path = (char *)mk_path_name(2,'/',l_vars.f_path_r,l_vars.f_name);
                      else
                          l_vars.hdf_path = strdup(l_vars.f_name);
                      DBUG_PRINT(1,(LOGF,"hdf_path =%s \n", l_vars.hdf_path));

                      /* Create full file name with path */
                      if (l_vars.hdf_path_t != NULL && strcmp(l_vars.hdf_path_t,"."))
                          l_vars.hdf_file = (char *)mk_path_name(2,'/',l_vars.hdf_path_t,l_vars.hdf_path);
                      else
                          l_vars.hdf_file = strdup(l_vars.hdf_path);
                      DBUG_PRINT(1,(LOGF,"hdf_file = %s \n", l_vars.hdf_file));

                  }
                else
                    gateway_err(stderr,"error getting hdf filename \n",1,&l_vars);

		 
                /* determine the type of file */
                switch(file_type = filetype(l_vars.hdf_file))
                  {
                  case 1: /* HDF file */
                      l_vars.do_netcdf = 0;
                      l_vars.do_fits = 0;
                      break;
                  case 2: /* netCDF file */
                      l_vars.do_netcdf = 1;
                      l_vars.do_fits = 0;
                      break;
                  case 3: /* Fits file */
                      l_vars.do_netcdf = 0;
                      l_vars.do_fits = 1;
                      break;
                  case -1: /* error */
                  case 0:
                  default:
                      gateway_err(stderr,"main: error in filetype ",0,&l_vars);
                      break;
                  }

                /* ========== make the variables(subsetting) valid ==========*/
                if (l_vars.ft_tabFlag != 0) {
                    if (l_vars.ft_start_rec <= 0)
                        l_vars.ft_start_rec = 1;
                    if (l_vars.ft_end_rec <= 0)
                        l_vars.ft_end_rec = l_vars.ft_start_rec;
                    if ( l_vars.ft_end_rec <l_vars.ft_start_rec)
                        l_vars.ft_start_rec= l_vars.ft_end_rec;

                }

                /* for sds or phdu or image extension */
                if (l_vars.sdb_imgFlag != 0) {
                    if (l_vars.sdb_plane <= 0)
                        l_vars.sdb_plane = 1;
                   if (l_vars.sdb_start_rec <= 0)
                        l_vars.sdb_start_rec = 1;
                    if (l_vars.sdb_end_rec <= 0)
                        l_vars.sdb_end_rec = l_vars.sdb_start_rec;
                    if ( l_vars.sdb_end_rec <l_vars.sdb_start_rec)
                        l_vars.sdb_start_rec= l_vars.sdb_end_rec;
                    if (l_vars.sdb_end_rec > l_vars.sdb_plane)
                        l_vars.sdb_end_rec = l_vars.sdb_plane;
                }

		 
                /* It is possible that a request of browsing 
                   one or MORE objects has been submitted, and 
                   so we need a loop to display all of them. 
                   - Start of UGLY code */
                for (x = 0; x <= m; x++) 
                  { 
                      char *tp1, *tp2;

                      DBUG_PRINT(1,(LOGF,"main: for x=%d \n", x));
		   
                      if (entries[x].val==NULL)
                          continue;

                      no_query = FALSE; /* Keep track of whether any requests have been 
                                           made. */

                      DBUG_PRINT(1,(LOGF,"sdsView(31)=%d\n",l_vars.sdb_imgFlag));

                      /* get the new Query_string */
                      if ((l_vars.do_fits) &&(l_vars.sdb_imgFlag == SDBVIEW))
                        {
                            /* modify the reference nuber */
                            l_vars.hdf_ref = HDmalloc(100);
                            sprintf(l_vars.hdf_ref, "ref=%d,start=%d,end=%d",
                                    l_vars.sdb_ref, 
                                    l_vars.sdb_start_rec,
                                    l_vars.sdb_end_rec);
                        }
                      /* =========== for fits table ========= */
                      if ((l_vars.ft_tabFlag == FITSTAB)||
                          (l_vars.ft_tabFlag == FITSBINTAB)) {
			
                          /* modify the reference number */
                          l_vars.hdf_ref = HDmalloc(100);
                          sprintf(l_vars.hdf_ref, "ref=%d",l_vars.ft_ref);
                      }

                      /* ============ for sds ==========*/
                      if (l_vars.sdb_imgFlag == SDSVIEW) {
                          l_vars.hdf_ref = HDmalloc(100);
                          /* modify the reference nuber */
                          sprintf(l_vars.hdf_ref,"ref=%d,start=%d,end=%d",
                                  l_vars.sdb_ref,
                                  l_vars.sdb_start_rec,
                                  l_vars.sdb_end_rec);
                      }


                      /* An object has been selected from any of the 
                         scroll boxes, and so determine what the hell 
                         kind of object it is. */
                      if (l_vars.hdf_ref == NULL)
                        {
                            /*  HDF ref is either tag/ref pair or name of object */
                            if ((l_vars.hdf_ref = (char *)strrchr(entries[x].val, ';'))!=NULL)
                                ; /* tag ref pair */
                            else if ((l_vars.hdf_ref=(char *)strrchr(entries[x].val, '#'))!=NULL)
                                ; /* name of object */
                            else 
                              { /* pick first entry as name i.e. hdfref  */
#if 0
                                  l_vars.hdf_ref = entries[x].name; 
#endif
                                  continue;
                              }
                        }

                      DBUG_PRINT(1,(LOGF,"l_vars.hdf_ref=%s\n",l_vars.hdf_ref)); 	  

                      /* damn */
                      if (l_vars.hdf_ref==NULL) 
                        {
                            fprintf(l_vars.hfp, "<B> Object not being handled. </B>\n");
                            if (send_reply(stdout, html_file, mime_type,&l_vars)==FAIL)
                                gateway_err(stderr,"Error sending reply.",1,&l_vars); 
                            else 
                                break;
                        }

                      /* For a scientific dataset, each request goes with a form 
                         containing three fields, namely, "start", "stride", and "end". 
                         So when a request of browsing a dataset is submitted via a 
                         form, it occupies four consecutive entries in "entries[x]", 
                         i.e. "name", "start", "stride", and "end". Obviously, we only 
                         want to process the first entry and the last three entries are
                         supplemental information. And the following block skips three 
                         iterations for the last three entries. */
                      if (x>0)
                        {
                            if (((tp1=(char *)strrchr(entries[x].val, '#'))==NULL)&&
                                ((tp2=(char *)strrchr(entries[x].val, ';'))==NULL))
                                continue;
                        }


                      DBUG_PRINT(1,(LOGF,"nfield =%d \n", l_vars.ft_nfields));
                      DBUG_PRINT(1,(LOGF,"start=%d \n", l_vars.ft_start_rec));
                      DBUG_PRINT(1,(LOGF,"end=%d \n", l_vars.ft_end_rec));

                      /* are we dealing with a FITS file or HDF/netCDF */
                      if (l_vars.do_fits)
                        {  /*FITS */
#ifdef HAVE_FITS			  
                            unit = openFits(l_vars.hdf_file, 0);
                            l_vars.unit = unit;

                            DBUG_PRINT(1,(LOGF,"unit =%d \n", unit));

                            if (l_vars.ft_tabFlag == FITSTAB) {	
                                if ((html_file = (char *)fitsGrokTab(unit, l_vars.hdf_ref,entries,&mime_type,current_entry,m,&l_vars)) == NULL) 
                                    gateway_err(stderr,"main: fitsGrokTab ",1,&l_vars);	     
                            }
                            else {			      
                                if (l_vars.ft_tabFlag == FITSBINTAB) {
                                    if ((html_file = (char *)fitsGrokBinTab(unit, l_vars.hdf_ref,entries,&mime_type,current_entry,m,&l_vars)) == NULL) 
                                        gateway_err(stderr,"main: fitsGrokTab ",1,&l_vars);	     
                                }
                                else {
                                    html_file = (char *)sdbGrokView(l_vars.hdf_file, 
                                                                    l_vars.hdf_ref, 
                                                                    entries,
                                                                    &mime_type, x, m,&l_vars);
                                } /* (l_vars.ft_tabFlag == FITSBINTAB) { */
                            } /* (l_vars.ft_tabFlag == FITSTAB) */

                            if (unit) {
                                /* close the fits file & free the unit number */
                                FCCLOS(unit, &status);
                                FCFIOU(unit, &status);
                            }
#endif /* HAVE_FITS */
                        }
                      else /* HDF or netCDF file */
                        {	
#ifdef HAVE_HDF
                            if (l_vars.sdb_imgFlag == SDSVIEW) {
		  
                                html_file = (char *)sdbGrokView(l_vars.hdf_file, 
                                                                l_vars.hdf_ref, 
                                                                entries,
                                                                &mime_type, x, m,&l_vars);
                            }
                            else {			  
                                html_file = (char *)sdbGrokRef (l_vars.hdf_file, 
                                                                l_vars.hdf_ref, 
                                                                entries,
                                                                &mime_type, x, m,&l_vars);
                            }
#endif /* HAVE_HDF */
                        } /* end else HDF or netCDF file */

                      if (html_file != NULL)
                          if (send_reply(stdout, html_file, mime_type,&l_vars)==FAIL)
                              gateway_err(stderr,"Error sending reply.",1,&l_vars);

                      if (html_file != NULL)
                          free(html_file);

                      DBUG_PRINT(1,(LOGF,"main: end for x=%d \n", x));
                      break;
                  } /* for (x...); processing all the requests. */
 
#if 0
                if (no_query) 
                  {
                      printf("<H1> No selections made! </H1>");
                      exit(-1);
                  }
#endif
            }
          else /* neither METHOD of GET or POST */
              gateway_err(stderr,"This Gateway should be reference with a METHOD of GET or POST .\n",1,&l_vars);

      }
    else if (l_vars.do_dump) /* do dump */
      { /* The following is for dumping the contenst of the HDF/netCDF file
         * into a(n) HTML file(s) */
          int fstime = TRUE;
          char *pptr;

          DBUG_PRINT(1,(LOGF,"->We are dumping file contents <-\n"));
      
          /* get environmnet variables */
          get_env(&myserv_env);
          DBUG_PRINT(1,(LOGF,"got environmnet variables \n"));
          l_vars.h_env = &myserv_env;

          /* Dump the file */
          html_file =(char *)sdbGrokFile (l_vars.hdf_file, entries, &mime_type, 
                                          fstime, 1,&l_vars);
#if 0
          /* get file name prefix w/ path i.e withour '.hdf' */
          if ((pptr = (char *)path_name(l_vars.hdf_file_name,'.')) == NULL)
            {
                printf("Error getting file name prefix\n");
                exit(1);
            }
          DBUG_PRINT(1,(LOGF," file name prefix=%s \n", pptr));

          /* Create name for HTML version of file */
          sprintf(hdf_html_file,"%s.html",pptr);
          DBUG_PRINT(1,(LOGF," hdf html file name %s \n", hdf_html_file));

          /* rename temp HTML file to one with new name i.e. includes location */
          rename(html_file,hdf_html_file);
#endif
          if (l_vars.hdf_path != NULL)
              FREE_CLEAR(l_vars.hdf_path);
          if (l_vars.hdf_file != NULL)
              FREE_CLEAR(l_vars.hdf_file);
          DBUG_CLOSE(LOGF);
      } /* end else do dump */
#ifdef HAVE_CCI
    else if (l_vars.do_cci)
      {
          int fstime = TRUE;
          char *pptr;

          DBUG_PRINT(1,(LOGF,"->We are using CCI<-\n"));

          /* get environmnet variables */
          get_env(&myserv_env);
          DBUG_PRINT(1,(LOGF,"got environmnet variables \n"));
          l_vars.h_env = &myserv_env;

          if (l_vars.cci_port == -1 || l_vars.cci_host == NULL)
              gateway_err(stderr,"Wrong number of arguments, client may not support CCI.\n",1,&l_vars);
          else
              do_cci_connect(&l_vars);

          /* cheat for now */
          myserv_env.request_method = "GET";
          html_file =(char *)sdbGrokFile (l_vars.hdf_file, entries, &mime_type, 
                                          fstime, 1,&l_vars);

          /* get file name prefix w/ path i.e withour '.hdf' */
          if ((pptr = (char *)path_name(l_vars.hdf_file_name,'.')) == NULL)
            {
                printf("Error getting file name prefix\n");
                exit(1);
            }
          DBUG_PRINT(1,(LOGF," file name prefix=%s \n", pptr));

          /* Create name for HTML version of file */
          sprintf(hdf_html_file,"%s.html",pptr);
          DBUG_PRINT(1,(LOGF," hdf html file name %s \n", hdf_html_file));

          /* rename temp HTML file to one with new name i.e. includes location */
          rename(html_file,hdf_html_file);
     
          if (send_reply(stdout, hdf_html_file, mime_type,&l_vars)==FAIL) 
              gateway_err(stderr,"Error sending reply.",1,&l_vars);
      }
#endif /* HAVE_CCI */

    /* clennup */
#ifdef HAVE_HDF
    cleanup_sds(&l_vars);
    cleanup_pal(&l_vars);
    cleanup_rig(&l_vars);
    cleanup_an(&l_vars);
    cleanup_vgs(&l_vars);
    cleanup_vd(&l_vars);
#endif /* HAVE_HDF */
#ifdef HAVE_CCI
    MCCIDisconnect(l_vars.h_cci_port);
#endif
    DBUG_CLOSE(LOGF);
    exit(0);
} /* end main() */


