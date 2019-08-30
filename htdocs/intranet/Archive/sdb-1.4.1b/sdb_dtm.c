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
static char RcsId[] = "@(#)sdb_dtm.c,v 1.7 1996/04/15 17:58:06 georgev Exp";
#endif

/* sdb_dtm.c,v 1.7 1996/04/15 17:58:06 georgev Exp */

/*
  hdfDtmThang
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_dtm.h"
#include "sdb_util.h"

#ifdef HAVE_DTM
#include "netdata.h"
#include "net.h"

/*
 * This code came from the X version of Mosaic
 * 
 */

/* Creation of an input port implies done_init and done_register
   as well as done_inport.  Creation of an output port implies
   all of these (since an input port is always created prior to
   creating an output port). */
static int done_init     = 0;   /* called NetInit?? */
static int done_register = 0;   /* called NetRegisterModule?? */
static int done_outport  = 0;   /* called NetCreateOutPort?? */
static int done_inport   = 0;   /* called NetCreateInPort?? */


mo_status mo_dtm_in (char *path, lvar_st *l_vars)
{
  NetPort *inport;

  ENTER(2,"mo_dtm_in");  

  if (!done_init)
    {
      NetInit ("SDB");
      done_init = 1;
    }
  
  if (!done_register)
    {
#if 0
      NetRegisterModule
        ("Mosaic", NETCOM,
         mo_receive_com, (caddr_t) state,
         NULL, (caddr_t) 0,
         NULL, (caddr_t) 0);
#endif
      done_register = 1;
    }
  
  if (!done_inport)
    {
      inport = NetCreateInPort (path);
      done_inport = 1;
    }

#if 0
  mo_register_dtm_blip ();
#endif
  DBUG_PRINT(2,(LOGF,"mo_dtm_in: done_init %d \n", done_init));
  DBUG_PRINT(2,(LOGF,"mo_dtm_in: done_inport %d \n", done_inport));
  EXIT(2,"mo_dtm_in");  
  return mo_succeed;
}

mo_status mo_dtm_out (char *port, lvar_st *l_vars)
{
  ENTER(2,"mo_dtm_out");  
  if (!done_outport)
    {
      mo_dtm_in (":0", l_vars);

      /* Make the output port. */
      NetCreateOutPort (port);

      done_outport = 1;
    }
  DBUG_PRINT(2,(LOGF,"done_outport %d \n", done_outport));
  EXIT(2,"mo_dtm_out");   
  return mo_succeed;
}

mo_status mo_dtm_disconnect (void)
{
  if (done_init)
    if (!NetSendDisconnect (NULL, NULL, NULL)) {
      sleep(1);
      NetTryResend();
    }

  return;
}

mo_status mo_dtm_out_active_p (void)
{
  if (done_outport)
    return mo_succeed;
  else
    return mo_fail;
}


mo_status mo_dtm_poll_and_read (void)
{
  if (done_inport)
    NetClientPollAndRead ();

  return mo_succeed;
}

#if 0
mo_status mo_dtm_send_text (mo_window *win, char *url, char *text)
{
  Text *t;
  char *title;
  int rv;

  if (!mo_dtm_out_active_p ())
    return mo_fail;

  title = (char *)malloc (strlen (url) + 16);
  sprintf (title, "Mosaic: %s\0", url);

  t = (Text *)malloc (sizeof (Text));
  t->title = title;
  t->id = strdup ("Mosaic");
  t->selLeft = t->selRight = t->insertPt = 0;
  t->numReplace = t->dim = strlen (text);
  t->replaceAll = TRUE;
  t->textString = strdup (text);

  rv = NetSendText (NULL, t, FALSE, "NewText");

  return mo_succeed;
}
#endif

mo_status mo_dtm_send_image (void *data, lvar_st *l_vars)
{
  ImageInfo *img = (ImageInfo *)data;
  int rv, i;
  char palette[768];

  if (!mo_dtm_out_active_p ())
    return mo_fail;

  for (i = 0; i < 256; i++)
    {
      if (i < img->num_colors)
        {
          palette[i*3+0] = img->reds[i]   >> 8;
          palette[i*3+1] = img->greens[i] >> 8;
          palette[i*3+2] = img->blues[i]  >> 8;
        }
      else
        {
          palette[i*3+0] = i;
          palette[i*3+1] = i;
          palette[i*3+2] = i;
        }
    }

  rv = NetSendRaster8Group
    (NULL, "Mosaic Image", img->image_data,
     img->width, img->height, palette, TRUE, FALSE, NULL);

  return mo_succeed;
}

mo_status mo_dtm_send_palette (void *data, lvar_st *l_vars)
{
  ImageInfo *img = (ImageInfo *)data;
  int rv, i;
  char palette[768];

  if (!mo_dtm_out_active_p ())
    return mo_fail;

  for (i = 0; i < 256; i++)
    {
      if (i < img->num_colors)
        {
          palette[i*3+0] = img->reds[i]   >> 8;
          palette[i*3+1] = img->greens[i] >> 8;
          palette[i*3+2] = img->blues[i]  >> 8;
        }
      else
        {
          palette[i*3+0] = i;
          palette[i*3+1] = i;
          palette[i*3+2] = i;
        }
    }

  rv = NetSendPalette8
    (NULL, "Mosaic Palette", palette, NULL, FALSE, NULL);

  return mo_succeed;
}

mo_status mo_dtm_send_dataset (void *spanker, lvar_st *l_vars)
{
    Data *d = (Data *) spanker;
    int rv, i;
    char palette[768];

    if (!mo_dtm_out_active_p ())
        return mo_fail;

/*

    for (i = 0; i < 256; i++)
        {
            if (i < img->num_colors)
                {
                    palette[i*3+0] = img->reds[i]   >> 8;
                    palette[i*3+1] = img->greens[i] >> 8;
                    palette[i*3+2] = img->blues[i]  >> 8;
                }
            else
                {
                    palette[i*3+0] = i;
                    palette[i*3+1] = i;
                    palette[i*3+2] = i;
                }
        }
*/
  
#define COLLAGE_SUCKS
#ifdef COLLAGE_SUCKS
    rv = NetSendArray
        (NULL, d, TRUE, FALSE, NULL, (d->rank == 3 ? TRUE : FALSE));
#else
    rv = NetSendArray
        (NULL, d, TRUE, FALSE, NULL, TRUE);
#endif

    return mo_succeed;

}

extern ImageInfo *sdbGetImage(char *filename, char *reference, intn subsample, intn *bg, 
            lvar_st *l_vars);

extern ImageInfo *sdbGrokImage(char *filename, char *reference, int *bg, lvar_st *l_vars);
/*-------------------------------------------------------------------- 
 NAME
     
 DESCRIPTION

 RETURNS
     
--------------------------------------------------------------------*/   
int
hdfDtmThang(char *filename, 
            uint16 tag,
            uint16 ref,
            char *reference,
            lvar_st *l_vars)
{
    ImageInfo *img = NULL;
    intn  foo;
    int32 start[10]; 
    int32 end[10];
    int32 fid = FAIL; 
    int32 sds = FAIL; 
    int32 nt;
    int32 nattr; 
    int32 size;
    int32 index;
    int32 i, tmp;
    char name[512];
    mo_status status;
    Data   *d = NULL;                   /* for SDS */
    int   ret_value = SUCCEED;

    ENTER(2,"hdfDtmTHang");

    /* Check for presence of DTM output port. */
    if (!mo_dtm_out_active_p ())
      {
          ret_value = FAIL;
          goto done;
      }

    /* raseter 8 */
    if((uint16)tag == DFTAG_RIG) 
      {
          /* Using a cached image is not an option since we quantized. */
          img = sdbGetImage(filename, reference, FALSE, &foo,l_vars);
          if (img) 
            {
                mo_dtm_send_image ((void *)img,l_vars);
                /* We specify copy_internally in args to NetSendRaster8Group,
                   so we can free everything here now. */
                free (img->image_data);
                free (img->reds);
                free (img->greens);
                free (img->blues);
                free (img);
            }
          ret_value = SUCCEED;
          goto done;
      }
        
    /* Palette */
    if((uint16)tag == DFTAG_IP8) 
      {
          img = sdbGrokImage(filename, reference, NULL,l_vars);
          if(img) 
            {
                mo_dtm_send_palette ((void *)img,l_vars);
                /* We specify copy_internally in args to NetSendRaster8Group,
                   so we can free everything here now. */
                free (img->image_data);
                free (img->reds);
                free (img->greens);
                free (img->blues);
                free (img);
            }
      }

    /* Scientific Dataset */
    if((uint16)tag == DFTAG_NDG) 
      {
          DBUG_PRINT(1,(LOGF, "hdfDtmThang: this is an SDS \n"));
          if ((fid = SDstart(filename, DFACC_RDONLY)) == FAIL)
            {
                ret_value = FAIL;
                goto done;
            }

          if ((index = SDreftoindex(fid, (int32) ref)) == FAIL)
            {
                ret_value = FAIL;
                goto done;
            }

          if ((sds = SDselect(fid, index)) == FAIL)
            {
                ret_value = FAIL;
                goto done;
            }
            
          /* davet what the hell is this about ???? */
          if ((d = DataNew()) == NULL) 
            {
                ret_value = FAIL;
                goto done;
            }
          d->entity = ENT_Internal;
          d->dot = DOT_Array;
            
          /* get all basic meta-data */
          sprintf(name, "(no name)");
          if (SDgetinfo(sds, name, (int32 *)&(d->rank), (int32 *)d->dim, &nt, &nattr) == FAIL)
            {
                ret_value = FAIL;
                goto done;
            }

          DBUG_PRINT(1,(LOGF, "hdfDtmThang: rank=%d \n",d->rank ));
          if((d->rank > 3) || (d->rank < 2)) 
            {
                ret_value = SUCCEED;
                goto done;
            }
            
          switch(nt) 
            {
            case DFNT_INT8:
            case DFNT_UINT8:
                d->dost = DOST_Char;
                break;
            case DFNT_INT16:
            case DFNT_UINT16:
                d->dost = DOST_Int16;
                break;
            case DFNT_INT32:
            case DFNT_UINT32:
                d->dost = DOST_Int32;
                break;
            case DFNT_FLOAT32:
                d->dost = DOST_Float;
                break;
            case DFNT_FLOAT64:
                d->dost = DOST_Double;
                break;
            }
       
          /* set up the region we want to read */
          for(i = 0; i < d->rank; i++) 
            {
                start[i] = 0;
                end[i]  = d->dim[i];
            }
            
          /* figger out how much space */
          for(i = 0, size = 1; i < d->rank; i++)
              size *= d->dim[i];
            
          /* allocate storage to store the raw numbers */
          if ((d->data = (VOIDP) HDgetspace(size * DFKNTsize(nt))) == NULL)
            {
                ret_value = FAIL;
                goto done;
            }

          /* read that crazy data */
          if ((SDreaddata(sds, start, NULL, end, d->data)) == FAIL)
            {
                ret_value = FAIL;
                goto done;
            }

            
          /* set the name */
          if (d->label = (char *)HDmalloc(strlen(name)+1))
            {
                strcpy(d->label,name);
            }
#if 0
          SDendaccess(sds);
          SDend(fid);
#endif            
          /* swap numbers because Collage was written by a bunch of idiots */
          for(i = 0; i < ( d->rank / 2 ); i++) 
            {
                tmp = d->dim[i];
                d->dim[i] = d->dim[d->rank - i - 1];
                d->dim[d->rank - i - 1] = tmp;
            }
            
          status = mo_dtm_send_dataset(d,l_vars);
          DBUG_PRINT(1,(LOGF, "hdfDtmThang: status=%d \n",status ));
          /* free it */
          HDfreespace((void *)(d->label));
          HDfreespace((void *)(d->data));
          HDfreespace((void *)d);
      } /* end if tag */

done:

    if (ret_value == FAIL)
      {
          DBUG_PRINT(1,(LOGF, "hdfDtmThang: Failed\n"));
      }
    if (sds != FAIL)
        SDendaccess(sds);
    if (sds!= FAIL)
        SDend(fid);
            
    EXIT(2,"hdfDtmTHang");
    return ret_value;
} /* hdfDTMthang */
#endif /* HAVE_DTM */

