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
static char RcsId[] = "@(#)sdsdump.c,v 1.10 1996/04/15 17:58:29 georgev Exp";
#endif

#include <math.h>
#include "sdb.h"
#include "sdsdump.h"

/* printing functions copied from vshow.c and used by sdsdumpfull(). 
*/
static int32 cn = 0;

#ifdef __STDC__
int32 fmtbyte(unsigned char *x, FILE *ofp)
#else
int32 fmtbyte(x, ofp)
unsigned char *x;
FILE *ofp;
#endif
{
    cn += fprintf(ofp,"%02x ",*x);
    return SUCCEED;
}

#ifdef __STDC__
int32 fmtint8(VOIDP x, FILE *ofp)
#else
int32 fmtint8(x, ofp)
VOIDP x;
FILE *ofp;
#endif
{
    cn += fprintf(ofp, "%d", (int) *((signed char *)x));
    return SUCCEED;
}

#ifdef __STDC__
int32 fmtuint8(VOIDP x, FILE *ofp)
#else
int32 fmtuint8(x, ofp)
#endif
{
    cn += fprintf(ofp, "%u", (unsigned) *((unsigned char *)x));
    return SUCCEED;
}

#ifdef __STDC__
int32 fmtint16(VOIDP x, FILE *ofp)
#else
int32 fmtint16(x, ofp)
#endif
{
  short s = 0;
  HDmemcpy(&s, x, sizeof(int16));
  cn += fprintf(ofp, "%d", (int)s);
  return SUCCEED;
}

#ifdef __STDC__
int32 fmtuint16(VOIDP x, FILE *ofp)
#else
int32 fmtuint16(x, ofp)
#endif
{
  unsigned short s = 0;
  HDmemcpy(&s, x, sizeof(int16));
  cn += fprintf(ofp, "%u", (unsigned)s);
  return SUCCEED;
}

#ifdef __STDC__
int32 fmtchar(VOIDP x, FILE *ofp)
#else
int32 fmtchar(x, ofp)
#endif
{
    if (isprint(*(unsigned char *)x))   
      {
        putc(*((char *)x), ofp);
        cn++;
      }
    else   
      {
        putc('\\', ofp);
        cn++;
        cn += fprintf(ofp, "%03o", *((uchar8 *)x));
      }
    return SUCCEED;
}

#ifdef __STDC__
int32 fmtuchar8(VOIDP x, FILE *ofp)
#else
int32 fmtuchar8(x, ofp)
#endif
{
    cn++;
    putc('\\', ofp);
    cn++;
    fprintf(ofp, "%o", *((uchar8 *)x));
    return SUCCEED;
}

#ifdef __STDC__
int32 fmtint(VOIDP x, FILE *ofp)
#else
int32 fmtint(x, ofp)
#endif
{
  int i = 0;
  HDmemcpy(&i, x, sizeof(int32));
  cn += fprintf(ofp, "%d", (int)i);
  return SUCCEED;
}

#ifdef __STDC__
int32 fmtfloat32(VOIDP x, FILE *ofp)
#else
int32 fmtfloat32(x, ofp)
#endif
{
  float32 f = 0, epsi = 1.0e-20;
  HDmemcpy(&f, x, sizeof(float32));
  if (fabs(f - FILL_FLOAT) <= epsi)
    cn += fprintf(ofp, "FloatInf ");
  else 
    cn += fprintf(ofp, "%f", f);
  return SUCCEED;
}

#ifdef __STDC__
int32 fmtint32(VOIDP x, FILE *ofp)
#else
int32 fmtint32( x, ofp)
#endif
{
  long l = 0;
  HDmemcpy(&l, x, sizeof(int32));
  cn += fprintf(ofp, "%ld", (long)l);
  return SUCCEED;
}

#ifdef __STDC__
int32 fmtuint32(VOIDP x, FILE *ofp)
#else
int32 fmtuint32(x, ofp)
#endif
{
  uint32 l = 0;
  HDmemcpy(&l, x, sizeof(int32));
  cn += fprintf(ofp, "%lu", (unsigned long)l);
  return SUCCEED;
}

#ifdef __STDC__
int32 fmtshort(VOIDP x, FILE *ofp)
#else
int32 fmtshort(x, ofp)
#endif
{
  short s = 0;
  HDmemcpy(&s, x, sizeof(int16));
  cn += fprintf(ofp, "%d", s);
  return SUCCEED;
}

#define FLOAT64_EPSILON ((float64)1.0e-20)
#ifdef __STDC__
int32 fmtfloat64(VOIDP x, FILE *ofp)
#else
int32 fmtfloat64(x, ofp)
#endif
{
  double d = 0, epsi = 1.0e-20;

  HDmemcpy(&d, x, sizeof(float64));
  if (fabs(d - FILL_DOUBLE) <= FLOAT64_EPSILON)
    cn += fprintf(ofp, "DoubleInf ");
  else 
    cn += fprintf(ofp, "%f", d);
  return SUCCEED;
}

#ifdef __STDC__
int32 dumpfull(int32 nt,int32 cnt,VOIDP databuf,intn indent,FILE *ofp)
#else
int32 dumpfull(nt, cnt, databuf, indent, ofp)
#endif
{
    intn i; 
    VOIDP b; 
    int32 (*fmtfunct)()=NULL;
    int32 off;
    
    switch (nt)  
      {
       case DFNT_CHAR:
            fmtfunct = fmtchar;
            break;
       case DFNT_UCHAR:
            fmtfunct = fmtuchar8;
            break;
       case DFNT_UINT8:
            fmtfunct = fmtuint8;
            break;
       case DFNT_INT8:
            fmtfunct = fmtint8;
            break;
       case DFNT_UINT16:
            fmtfunct = fmtuint16;
            break;
       case DFNT_INT16:
            fmtfunct = fmtint16;
            break;
       case DFNT_UINT32:
            fmtfunct = fmtuint32;
            break;
       case DFNT_INT32:
            fmtfunct = fmtint32;
            break;
       case DFNT_FLOAT32:
            fmtfunct = fmtfloat32;
            break;
       case DFNT_FLOAT64:
            fmtfunct = fmtfloat64;
            break;
       default:
            fprintf(ofp, "HDP does not support type [%i] \n", (int)nt);
            break;
      }
    b = databuf;
    off = DFKNTsize(nt | DFNT_NATIVE);
    cn = indent;
    for (i=0; i<cnt; i++)   
      {
        fmtfunct(b, ofp);
        b = (char *)b+ off;
        if (nt != DFNT_CHAR)   
          {
            putc(' ', ofp);
            cn++;
          }
        if (cn > 65)   
          {
            putc('\n', ofp);
            for (cn=0; cn<indent; cn++) putc(' ', ofp);
          }
      }
    return SUCCEED;
}	    


#ifdef __STDC__
int32 sdsdumpfull(int32 sds_id, int32 rank, int32 dimsizes[], int32 nt, 
                  intn indent, FILE *fp, int32 *start, int32 *stride, 
		  int32 *edge, int32 *left, lvar_st *l_vars) 
#else
int32 sdsdumpfull(sds_id, rank, dimsizes[], nt, indent, fp, start, stride, 
		  edge, left, l_vars) 
int32 sds_id;
int32 rank;
int32 dimsizes[];
int32 nt;
intn indent;
FILE *fp;
int32 *start;
int32 *stride;
int32 *edge;
int32 *left;
lvar_st *l_vars;
#endif
{   
    /* "rank" is the number of dimensions and 
       "dimsizes[i]" is size of dimension "i".*/

    int32 j,i, ret;
    VOIDP buf; 
    int32 numtype, eltsz, read_nelts, num_elts, temp;
    /* int32 *left, *start, *edge; */
    int32 done;    /* number of rows we have done */

    if (indent>65)  
      { /* This block is probably not necessary. */
#if 0
         printf("Bad indentation %i\n", indent);
         exit(1);
#endif
         gateway_err(l_vars->hfp,"sds_dumpfull: Bad indentation ",0,l_vars);
         return FAIL;
      } 
  
    /* Compute the number of the bytes for each value. */
    numtype = nt & DFNT_MASK;
    eltsz = DFKNTsize(numtype | DFNT_NATIVE); 

    /* read_nelts = dimsizes[rank-1]; */
    read_nelts = left[rank-1];
    if ((buf=(VOIDP)HDgetspace(read_nelts*eltsz)) == NULL)
      {
         gateway_err(l_vars->hfp,"sds_dumpfull: failed to allocate space ",0,l_vars);
         return FAIL;
      }

#if 0
    for (i=0; i<rank; i++)   
      { /* In the first (rank-1) dimensions, only one unit
	 is to be read in, and in the last dimension, all
	  the elements are to be read in at once. */
        start[i] = 0; 
        left[i] = dimsizes[i]; 
        edge[i] = 1; /* Only one element from each (except the last one) dimension
			is read in each time. */
      }
    edge[rank-1] = dimsizes[rank-1]; 
#endif  

    if ((temp=(read_nelts%stride[rank-1]))!=0)
       num_elts = read_nelts/stride[rank-1] + 1;
    else
       num_elts = read_nelts/stride[rank-1];

    if (rank==1) 
      {   /* If there is only one dimension, then dump the data
	     and the job is done. */
        if ((ret = SDreaddata(sds_id, start, stride, edge, buf)) == FAIL)
          {
            gateway_err(l_vars->hfp,"sds_dumpfull: failed to read SDS ",0,l_vars);
            return FAIL;
          }
 
        if ((ret = dumpfull(numtype, num_elts, buf, indent, fp)) == FAIL)
         {
           gateway_err(l_vars->hfp,"sds_dumpfull: failed to dump data ",0,l_vars);
           return FAIL;
         }
 
        fprintf(fp,"\n"); 
        cn=0;
        done = 1;
     }

   done = 0; 
    while (!done && (rank-2)>=0)  
      {  /* Dump a row each time modify left[] accordingly. */
        /* In each execution of "SDreaddata", only one unit (starting from
	   "start[j]) from each of the first (rank-1) dimensions is read in.
	   and all the elements in the last dimension are all read in at 
	   once. */
        if ((ret = SDreaddata(sds_id, start, stride, edge, buf)) == FAIL) 
          {
            gateway_err(l_vars->hfp,"sds_dumpfull: failed to read SDS ",0,l_vars);
            return FAIL;
          }

        
	if ((ret = dumpfull(numtype, num_elts, buf, indent, fp)) == FAIL)
          {
            gateway_err(l_vars->hfp,"sds_dumpfull: failed to dump data ",0,l_vars);
            return FAIL;
          }
 
	fprintf(fp,"<P>");
        for (cn=0; cn<indent; cn++) 
          fprintf(fp, " ");
	
        for (j = rank-2; j >= 0; j--)  
          { /* Examine each dimension. */
	    /* Don't have to consider the last dimension since all the 
               elements in that dimension are to be read in at once. */
            left[j] = left[j] - stride[j];
            if (left[j] > 0) 
              { /* Stay in the same dimension. */
                /* We have "--left[j]" because the first call
                   to SDreaddata already dealt with the first
                   unit of each of the first (rank-1) 
                   dimensions. */
                /*start[j]++; */ /* Advance to the next unit of the current 
                  dimension. */

                start[j] = start[j] + stride[j];
/*
                  if ((temp=(read_nelts%stride[j]))!=0)
                     num_elts = read_nelts/stride[j] + 1;
                  else
                     num_elts = read_nelts/stride[j];
 */                 
                break;
              }
            else     
              {              /* borrow one from j-1 dimension  */
                left[j] = dimsizes[j];
                start[j] = 0;
                if (j==0) 
                  done=1; /* At this point, all the units in the last dimension
                             have been displayed. So, just stop. */
                if (j==rank-2)  
                  {
                    fprintf(fp, "<P>");
                    for (cn=0; cn<indent; cn++) putc(' ', fp);
                  }
              }
          }    /* for j */
      }  /* while   */

#if 0
    /* why is this freed here Eric? */
    HDfreespace((VOIDP)start);
    HDfreespace((VOIDP)left);
    HDfreespace((VOIDP)edge);
#endif
    HDfreespace((VOIDP)buf);
    fprintf(fp, "\n");
    
    /* printf("Stupid!!!!!!"); exit(-1); */
   
    return SUCCEED;
} /* sdsdumpfull */
/* ------------------------------------- */

