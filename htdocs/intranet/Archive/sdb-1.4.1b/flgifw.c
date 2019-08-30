/*****************************************************************
 * flgifw.c: FBM Release 1.0 25-Feb-90 Michael Mauldin
 *
 * Modifications to GIFTORLE are Copyright (C) 1989,1990 by Michael
 * Mauldin.  Permission is granted to use this file in whole or in
 * part for any purpose, educational, recreational or commercial,
 * provided that this copyright notice is retained unchanged.
 * This software is available to all free of charge by anonymous
 * FTP and in the UUNET archives.
 *
 * CONTENTS
 *	write_gif (image, stream, mstr, mlen)
 *
 * EDITLOG
 *	LastEditDate = Mon Jun 25 00:09:49 1990 - Michael Mauldin
 *	LastFileName = /usr2/mlm/src/misc/fbm/flgifc.c
 *
 * HISTORY
 * 25-Jun-90  Michael Mauldin (mlm@cs.cmu.edu) Carnegie Mellon
 *	Package for Release 1.0
 *
 * 07-Mar-89  Michael Mauldin (mlm) at Carnegie Mellon University
 *	Beta release (version 0.9) mlm@cs.cmu.edu
 *
 * 19-Feb-89  Michael Mauldin (mlm) at Carnegie Mellon University
 *	Adapted to FBM package.
 *
 * 13-Feb-89  David Rowley (mgardi@watdcsu.waterloo.edu)
 *	GIF encoding modifications.
 *
 *	Based on: compress.c - File compression ala IEEE Computer, June 1984.
 *
 *	Spencer W. Thomas       (decvax!harpo!utah-cs!utah-gr!thomas)
 *	Jim McKie               (decvax!mcvax!jim)
 *	Steve Davies            (decvax!vax135!petsd!peora!srd)
 *	Ken Turkowski           (decvax!decwrl!turtlevax!ken)
 *	James A. Woods          (decvax!ihnp4!ames!jaw)
 *	Joe Orost               (decvax!vax135!petsd!joe)
 *
 *****************************************************************/

# include <stdio.h>
# include "fbm.h"

unsigned char *pixels = NULL;
int rowlen = 0;

int GetGIFPixel (int x, int y);

#ifndef lint
static char *fbmid =
"$FBM flgifw.c <1.0> 25-Jun-90  (C) 1989,1990 by Michael Mauldin, source \
code available free from MLM@CS.CMU.EDU and from UUNET archives$";
#endif

int 
write_gif (FBM *image, FILE *wfile)
{ register int i, bits, sum, bkg, clrs;
  unsigned char *red, *grn, *blu;
  int rint[256], gint[256], bint[256];
    
  if (image->hdr.planes > 1 || image->hdr.clrlen == 0)
  { fprintf (stderr, "write_gif can only handle mapped color images\n");
    return (0);
  }

  clrs = image->hdr.clrlen / 3;
  
  /* Calculate bits per pixel in colormap */
  for (i=clrs, bits=1; i > 2; )
  { i >>= 1; bits++; }

  if (1 << bits != clrs)
  { fprintf (stderr, "Error, number of colors %d is not a power of 2\n",
	     clrs);
    return (0);
  }
  
  if (bits < 1 || bits > 8)
  { fprintf (stderr, "Error, bits per pixel (%d) must be in range 1..8\n",
	     bits);
    return (0);
  }
  
  red = image->cm;
  grn = red + clrs;
  blu = grn + clrs;

  pixels = image->bm;
  rowlen = image->hdr.rowlen;
  
  /* Copy colormap, and find darkest pixel for background */
  { bkg = 0; sum = 1e9;

    for (i=0; i<clrs; i++)
    { rint[i] = red[i];
      gint[i] = grn[i];
      bint[i] = blu[i];

      if (red[i] + grn[i] + blu[i] < sum)
      { bkg = i; sum = red[i] + grn[i] + blu[i]; }
    }
  }
# ifdef DEBUG
  fprintf (stderr, "Writing GIF file [%dx%d], %d colors, %d bits, bkg %d\n",
	  image->hdr.cols, image->hdr.rows, clrs, bits, bkg);

  fprintf (stderr, "\n\nColormap:\n");
  for (i=0; i<clrs; i++)
  { fprintf (stderr, "%5d: <%3d, %3d, %3d>\n",
	     i, rint[i], gint[i], bint[i]);
  }
# endif

  return (GIFEncode (
	      wfile,
	      image->hdr.cols,		/* width */
	      image->hdr.rows,		/* height */
	      0,			/* No interlacing */
	      bkg,			/* Index of Backgrounf */
	      bits,			/* Bits Per pixel */
	      rint,			/* Red colormap */
	      gint,			/* Green colormap */
	      bint,			/* Blue colormap */
	      GetGIFPixel ) );		/* Get Pixel value */
}

/* Returns value of next pixel */
int
GetGIFPixel (int x, int y)
{
  return (pixels[y * rowlen + x]);
}
