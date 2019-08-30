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
static char RcsId[] = "@(#)sdb_img.c,v 1.5 1996/04/15 17:58:07 georgev Exp";
#endif

/* sdb_img.c,v 1.5 1996/04/15 17:58:07 georgev Exp */

/*
   convert2image - convert the current 2D dataset to image data   
 */

/* Include our stuff */
#include "sdb.h"
#include "sdb_img.h"
#include "sdb_util.h"


/*------------------------------------------------------------------------ 
 NAME
     convert2image -  convert the current dataset to the image data           
 DESCRIPTION
     convert the current dataset to the image data if possible. 
     If the range of the dataset is too small, the image can not be created.
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int 
convert2image(float *input, 
              int h,
              int w, 
              unsigned char *output)
{ 
    unsigned char    *image=output;
    int     i;
    float   tmp, *buffer1,*buffer2;
    float   spread ;

    float   dataMax,dataMin;
    
    int jj;

    buffer2 = (float *)input;
    buffer1 = buffer2;
   
    dataMin = *input;
    dataMax = *input;

    for (i=0; i<w*h; buffer1++, i++) 
        findMaxMin(*buffer1, dataMax, dataMin);

    spread = (dataMax == dataMin) ? 1. : 255./((float32) dataMax -(float32) dataMin);
  
    if (spread > 255./2.1e9) { /* is max-min too big for an int ? */

        for (i=0; i< (w * h); buffer2++,i++,image++) 
            *image = (uint8)((*buffer2 - dataMin)*spread);
    }
    else {
	
        for (i=0; i< (w * h); buffer2++,i++,image++) 
            *image = 0; 
    }

    return(0);
}
