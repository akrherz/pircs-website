/* Program: fitsutils.c
   Description:
      The fitsutils.c contains a bunch of subroutines related to the read
      of the FITS file by using the FITSIO Fortran Library. 
 * Reference: 
      HDF-FITS Conversion Page(http://hdf.ncsa.uiuc.edu/fits/)
      The client-side FITS Browser based on JAVA(http://hdf/fits/java/)
 */
#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>
#include <varargs.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_FITS

#ifndef FITSUTILS_C
#define FITSUTILS_C
#define LF 10
#define CR 13
#define FILESIZE 255
#define NUM_FIELDS 99

/* Package include files */
#include "sdb.h"
#include "fitsutils.h"

/* need FITSIO library */
#define  FITSREADONLY  0
#include "cfitsio.h"

/* finding max & min values from datasets */
#define findMaxMin(x, dataMax, dataMin) { \
    tmp=(x);  if(tmp > dataMax) dataMax=tmp; \
    if (tmp<dataMin) dataMin=tmp;}

/* Private */
static char t_file[FILESIZE];
static int file_suffix = 0;
static char   retStr[5000];

/* Global */
char errtxt[FITS_CLEN_ERRMSG];

/*------------------------------------------------------------------------ 
 NAME
     NewTempFile - Generate the new temple file
 DESCRIPTION
     get the new temple file if possible
 RETURNS
     the pointer of the new temple file
-------------------------------------------------------------------------*/

char *NewTempFile()
{
  
    sprintf(t_file, "tmp_%d.%d", getpid(), file_suffix++);
    return(t_file);
}

/*------------------------------------------------------------------------ 
 NAME
     FatalError - print the fatal error massage  
 DESCRIPTION
      print the fatal error massage  
 RETURNS
     NONE
-------------------------------------------------------------------------*/  

void FatalError (infoMsg)
char *infoMsg;
{
    printf("Content-type: text/html%c%c",10,10);
    printf("<title>Mapping FITS Gateway Error</title>");
    printf("<h1>Mapping FITS Gateway Error</h1>");
    printf("This FITS Gateway encountered an error: \n");
    printf("%s<p>\n", infoMsg);

}

/*------------------------------------------------------------------------ 
 NAME
     space - generate blank string  
 DESCRIPTION
     generate the blank string    
 RETURNS
     the blank string
-------------------------------------------------------------------------*/  

char* space(n)
int n;
{
    int i;

    if ((n>5000) || (n<=0)) {
        retStr[0]='\0';
        return retStr;
    }
    for  (i=0; i<n; i++)
        retStr[i]=' ';
    retStr[n]='\0';

    return retStr;
}



/*------------------------------------------------------------------------ 
 NAME
     at -  get the first position of the string2 within the string1
 DESCRIPTION
     get the first position of the string2 within the string1
 RETURN
     the number if found, otherwise -1;
-------------------------------------------------------------------------*/  
int at(str1,str2)
char *str1, *str2;
{
    int  i=0, len;
    len = strlen(str2);
    if (!str1)
        return(-1);

    while (strncmp(str2,str1,strlen(str1)) != 0 )
      { *str2++;
      i++;
      if (len < i)
          return(-1);
      }

    return(i);
}

/*------------------------------------------------------------------------ 
 NAME
     CHECKFITSERROR -   check error after make a fits routine call
 DESCRIPTION
     check error after make a fits routine call     
 RETURN
     NONE
-------------------------------------------------------------------------*/   
void 
#ifdef  __STDC__
CHECKFITSERROR(char routineName[10],int status,char errTxt[FITS_CLEN_ERRMSG])
#else
CHECKFITSERROR(routineName, status, errTxt)
char routineName[10];
int status;
char errTxt[FITS_CLEN_ERRMSG];
#endif
{
    if (status != 0){
        FCGERR(status,errTxt);
        /* printf ("%s status = %d : %s\n", routineName, status, errTxt); */
        /*      Quit(-1); */
    }
}



/*------------------------------------------------------------------------ 
 NAME
     CHECKFITSERROR -  open FITS file with dersied mode
 DESCRIPTION
     open FITS file with dersied mode    
 RETURN
     return the unit number if succeed, oftherwise -1
-------------------------------------------------------------------------*/ 
int openFits(fileName, mode)
char *fileName;
int mode;
{
    
    int iounit,status=0;
    int bksize,rwstat;

    rwstat = mode;

    /*  get an unused logical unit number */
    FCGIOU(&iounit, &status);
    CHECKFITSERROR("FCGIOU", status, errtxt);

    /* open fits file */
    FCOPEN(iounit, fileName, rwstat, &bksize, &status);
    CHECKFITSERROR("FCOPEN", status, errtxt);

    /* return the result */
    if (status == 0) 
        return(iounit);
    else { 
        printf("Can't open file: %s\n", fileName);
        return(-1);
    }
}

/*------------------------------------------------------------------------ 
 NAME
     fitsnimages -  get the image number within the fits file
 DESCRIPTION
     get the image number within the fits file    
 RETURN
     return the image number if succeed, oftherwise -1
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsnimages(int unit)
#else
fitsnimages(unit)
int unit;  /* fits i/o unit number */
#endif
{
    /* variables related to fits file  */
    int imageNumber = 0;
    int status = 0;
    int hdutype; 

    char errtxt[FITS_CLEN_ERRMSG];

    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("fitsnimages",status,errtxt);
    if (hdutype != 0)  /* primary array or Image  */
        FatalError("FITS file structure is so bad!");

    while (status == 0) {

        switch (hdutype) {
	 
        case 0: /* Image */
            ++imageNumber;
            break;
        default:
            /* fits table */
            break;
        }
   
        FCMRHD(unit,1,&hdutype,&status);

    }
    
    return(imageNumber);
}
/*------------------------------------------------------------------------ 
 NAME
     fitsntable -  get the ascii table number within the fits file
 DESCRIPTION
     get the ascii table number within the fits file    
 RETURN
     return the ascii table number 
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsntables(int unit)
#else
fitsntables(unit)
int unit;  /* fits i/o unit number */
#endif
{
    /* variables related to fits file  */
    int tableNumber = 0;
    int status = 0;
    int hdutype; 

    char errtxt[FITS_CLEN_ERRMSG];

    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("fitsntables",status,errtxt);
    if (hdutype != 0)  /* primary array or Table  */
        FatalError("FITS file structure is corrupt!");

    while (status == 0) {

        switch (hdutype) {
	 
        case 1: /* Table */
            ++tableNumber;
            break;
        default:
            /* image, priary array or binary table */
            break;
        }
   
        FCMRHD(unit,1,&hdutype,&status);

    }
    
    return(tableNumber);
}

/*------------------------------------------------------------------------ 
 NAME
     fitsnbintable -  get the binary table number within the fits file
 DESCRIPTION
     get the binary table number within the fits file    
 RETURN
     return the binary table number 
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsnbintables(int unit)
#else
fitsnbintables(unit)
int unit;  /* fits i/o unit number */
#endif
{
    /* variables related to fits file  */
    int tableNumber = 0;
    int status = 0;
    int hdutype; 

    char errtxt[FITS_CLEN_ERRMSG];

    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("fitsnbintables",status,errtxt);
    if (hdutype != 0)  /* primary array or Table  */
        FatalError("FITS file structure is corrupt!");

    while (status == 0) {

        switch (hdutype) {
	 
        case 2: /* Table */
            ++tableNumber;
            break;
        default:
            /* image, priary array or binary table */
            break;
        }
   
        FCMRHD(unit,1,&hdutype,&status);

    }
    
    return(tableNumber);
}

/*------------------------------------------------------------------------ 
 NAME
     fitsInfo -  get the information of the fits primary array
                 & Image extension
 DESCRIPTION
     get the information of the fits primary array  & Image extension        
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsInfo(int unit,int img, int *rank, int *width, int *height, int *plane)
#else
fitsInfo(unit, img, rank, width, height, plane)
int unit;
int img;
int *rank;
int *width, *height;
int *plane;
#endif
{
    
    /* some of fits variable */
    int simple,extend,anyflg;
    int bitpix,naxis,naxes[15],pcount,gcount;
    int nelmt;
    
    int imgNumber=1;
    int status = 0;
    int hdutype;
    int i;

    char errtxt[FITS_CLEN_ERRMSG];

    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    status= 0;
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("FCMAHD",status,errtxt);

    if (img != 1) {

        while (status == 0) {

            /* Moving to next extension in the FITS file */
            status= 0;
            FCMRHD(unit,1,&hdutype,&status);

            switch (hdutype) {
            case 0: /* Image */
                ++imgNumber;
                break;
            default:
                break;
            }
            if (img == imgNumber) 
                break;
        }
    } /*  if (img != 1) */
    
    /* read the required primary array keywords  */
    status = 0;
    FCGHPR(unit, 99, &simple, &bitpix, &naxis, naxes, &pcount, &gcount, \
           &extend, &status);
    CHECKFITSERROR("FCGHPR", status, errtxt);
    
    /* if (naxis != 2) {  */
    if (0) {
        InfoMsg(" The FITS object is not 2-D data");
        return (-1);
    }

    if (!simple) {   /* verify TRUE */
        InfoMsg("This file is not FITS standard!");
        return(-1);
    }

    if ((naxis>999) || (naxis<0)) {
        InfoMsg("Illegal NAXIS value");
        return(-1);
    }
    
    *rank   = naxis;
    if (naxis) {
        *width = naxes[0];  /*   x  */
        *height= naxes[1];  /*   y  */
    }
    else {
        *width = 0;  /*   x  */
        *height= 0;  /*   y  */
    }
    *plane = 1;

    if (naxis > 2) {
        for (i=2; i<naxis; i++)
            *plane *= naxes[i];
    }

    return(0);
}

/*------------------------------------------------------------------------ 
 NAME
     fitsImage -  get the image data of the CHDU                
 DESCRIPTION
     get the image data of the CHDU based on the plane number       
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsImage(int unit, char *imageData , int plane)
#else
fitsImage(unit, imageData, plane)
int  unit;
char *imageData;
int  plane;
#endif
{
    
    /* some of fits variable */
    int simple,extend,anyflg;
    int bitpix,naxis,naxes[15],pcount,gcount;
    int rNaxes[15];
    int nelmt;
    int datatype;
  
    int numPlane; /* total nuber of the plane */
    int start;

    float *buffer=NULL,*buf=NULL;

    int group, datasize, i;

    /* pointer of various types */
    union {
        char      *cp;    /* Byte or char */
        short     *sp;    /* short */
        int       *ip;    /* integer */
        float32   *fp;    /* float */
        float64   *dp;    /* double */
    } databuf;

    char *ptr;

    int  status = 0;

    char errtxt[FITS_CLEN_ERRMSG];
    
    /* read the required primary array keywords  */
    status = 0;
    FCGHPR(unit, 99, &simple, &bitpix, &naxis, naxes, &pcount, &gcount, \
           &extend, &status);
    CHECKFITSERROR("FCGHPR", status, errtxt);
    
    if ( 0 ) { /* naxis != 2 */
        InfoMsg(" The FITS object is not 2-D data");
        return (-1);
    }

    if (!simple) {   /* verify TRUE */
        InfoMsg("This file is not FITS standard!");
        return(-1);
    }

    if (naxis <= 0)
        return (-1);

    numPlane = 1;
    for (i=2; i<naxis; i++)
        numPlane *= naxes[i];

    if (plane > numPlane)
        return(-1);

    for (i=0; i<naxis; i++)
        rNaxes[i] = naxes[naxis-i-1];

    for (i=0; i<naxis; i++)
        naxes[i] = rNaxes[i];

    datatype = bitpix;
    nelmt = 1;
    /* for (i=0; i<naxis; i++)
       nelmt *=  naxes[i]; */

    nelmt =  naxes[naxis-1]*naxes[naxis-2];

    buffer =  (float *)malloc(nelmt*sizeof(float));

    if (buffer == NULL)
        /* out of memory */
        return(-1);
    buf = buffer;

    datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * nelmt;
    databuf.cp = ptr = (char *)malloc(datasize);

    if (databuf.cp == NULL)
        /* out of memory */
        return(-1);

    group = 0;

    /* reset the bscale */
    status = 0;
    FCPSCL(unit,1.0, 0, &status);
    CHECKFITSERROR("FCPSCAL",status,errtxt);

    start  = naxes[naxis-1]*naxes[naxis-2];
    start *= (plane-1);
    ++start;

    switch (bitpix){
    case 8:             /* byte/char */
	
        FCGPVB(unit,group,start , nelmt, 1, (unsigned char*)databuf.cp, &anyflg, &status);
        CHECKFITSERROR("FCGPVB",status,errtxt);

        /*get image data by plane */

        for (i=0; i<nelmt; i++)
            imageData[i] = databuf.cp[i];

        break;
    case 16: {

        FCGPVI(unit,group, start, nelmt, -1, databuf.sp, &anyflg, &status);

        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.sp++;
        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], imageData);
        break;
    }
    case 32: {

        FCGPVJ(unit,group, start, nelmt, -1, databuf.ip, &anyflg, &status);
		
        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.ip++;

        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], imageData);

        break;
    }
    case -32: {
		
        FCGPVE(unit,group, start, nelmt, -1.0, databuf.fp, &anyflg, &status);
        /* fdat2cdat_float32(bitpix,naxis,naxes,databuf.fp); */
	
        for (i=0; i<nelmt; i++) 
            *buf++ = *databuf.fp++;
	
        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], imageData);

        break;
    }

    case -64: {
       
        FCGPVD(unit,group, start, nelmt, -1.0, databuf.dp, &anyflg, &status);
      
        for (i=0; i<nelmt; i++)
            *buf++ = *databuf.dp++;

        /*                    x               y                      */   
        convert2image(buffer, naxes[naxis-1], naxes[naxis-2], imageData);
	
        break;
    }
    default:
        FatalError("Illegal bitpix value");
    }
    CHECKFITSERROR("FCGPV[BIJED]",status, errtxt);
     
    FITSfree(ptr); 
    FITSfree(buffer);
  
    return(0);

}

#if 0
/*------------------------------------------------------------------------ 
 NAME
     convert2image -  convert the current dataset to the image data           
 DESCRIPTION
     convert the current dataset to the image data if possible. 
     If the range of the dataset is too small, the image can not be created.
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int convert2image(input, h,w,output)
float    *input;
int      h, w;
unsigned char     *output;
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
#endif

/*------------------------------------------------------------------------ 
 NAME
     fitsTableInfo -  get the fits acsii table information          
 DESCRIPTION
     The CHDU can be gotten based  on the given table number
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsTableInfo(int unit,int tab, int *entries, int *recNum)
#else
fitsTableInfo(unit, tab, entries, recNum)
int unit;
int tab;
int *entries, *recNum;
#endif
{
    int tabNumber=0;
    int status = 0;
    int hdutype;
    int i;

    char errtxt[FITS_CLEN_ERRMSG];

    /* variables that get the ASCII table header keywords from the CHU  */
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS]; 
    
    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
        tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS],    
        tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];
    
    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    status= 0;
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("FCMAHD",status,errtxt);
    
    while (status == 0) {

        /* Moving to next extension in the FITS file */
        status= 0;
        FCMRHD(unit,1,&hdutype,&status);

        switch (hdutype) {
        case 1: /* asciiTable */
            ++tabNumber;
            break;
        default:
            break;
        }
        if (tab == tabNumber) 
            break;
    }   
 
    /* read the required ASCII TABLE keywords  */
    status = 0;
    FCGHTB(unit, NUM_FIELDS, &rowlen, &nrows, &tfields, ttype, tbcol, \
           tform, tunit, extname, &status);

    CHECKFITSERROR("FCGHTB", status, errtxt);
    
    if (!status) {
        *entries = tfields;
        *recNum  = nrows;
    }
    else {
        *entries = 0;
        *recNum  = 0;
        return FAIL;
    } 

    return(0);
}


/*------------------------------------------------------------------------ 
 NAME
     fitsTabFldName -  get the fits acsii table field name        
 DESCRIPTION
      get the field name of the fits table
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
char *
#ifdef __STDC__
fitsTabFldName(int unit,int fieldNumber)
#else
fitsTabFldName(unit, fieldNumber)
int unit;
int fieldNumber;
#endif
{
    char errtxt[FITS_CLEN_ERRMSG];
    int  status;

    /* variables that get the ASCII table header keywords from the CHU  */
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS]; 
    
    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
        tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS],    
        tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];
  
    /* read the required ASCII TABLE keywords  */
    status = 0;
    FCGHTB(unit, NUM_FIELDS, &rowlen, &nrows, &tfields, ttype, tbcol, \
           tform, tunit, extname, &status);
    if ((status == 0)&&(fieldNumber>=1)) {
      strcpy(retStr, ttype[fieldNumber-1]);   
      return(retStr);
    }
    else
      return("");
}

/*------------------------------------------------------------------------ 
 NAME
     getTabDataType -  return the data type based on the tform[n]         
 DESCRIPTION
     get the data number type of the field
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int  getTabDatType(str)
char *str;
{
    switch(str[0]) {
    case 'A':
        return(DFNT_CHAR8);
    case 'I':
        return(DFNT_INT32);
    case 'F': case 'E':
        return(DFNT_FLOAT32);
    case 'D':
        return(DFNT_FLOAT64);
      defaults:
        InfoMsg("Wrong TFORMn in the fits file ");
        return(-1);
    }
}

/*------------------------------------------------------------------------ 
 NAME
     fitsTabWidth -  return width of the table based on the TDISP[i]  
 DESCRIPTION
     get the width of the field
 RETURN
     the width number
-------------------------------------------------------------------------*/ 
int fitsTabWidth(str)
char *str;
{

    int pos, width;
    pos = at(".", str);
    if (pos >=1 ) 
        width = atoi((char *)substr(str,2,pos-1));
    else
        if (!strcmp((char *)substr(str,1,1),"A"))
            width = atoi((char *)substr(str,2,5));
        else
            width = 10;
    return width;
}

/*------------------------------------------------------------------------ 
 NAME
     fitsTabDispFmt -  convert the Fortran display format in C       
 DESCRIPTION
     convert the Fortran display format in C      
 RETURN
     NONE
-------------------------------------------------------------------------*/ 
void fitsTabDispFmt(str)
char *str;
{
    int  pos,i, width;
    char dispStr[20];
    char head[8];
    char fmtStr[20];
 
    strcpy(fmtStr,"%");

    if (strlen(str)) {

        strcpy(head, (char *)substr(str,1,1));
  
        pos = at("E", (char *)substr(str,2,20));
        if (pos <0)
            pos = at("e", (char *)substr(str,2,20));

        if (pos >=1 ) 
            strcpy(dispStr, (char *)substr(str,2,pos-1));
        else
            strcpy(dispStr, (char *)substr(str,2,10));

        if (!strncmp(head,"d",1) || !strncmp(head,"D",1) || !strncmp(head,"E",1)|| \
            !strncmp(head,"e",1) ) {
            strcat(fmtStr, dispStr);
            strcat(fmtStr, "e");
        }
        else {
            if (!strncmp(head,"A",1) || !strncmp(head,"a",1)) {
                strcat(fmtStr, dispStr);
                strcat(fmtStr, "s");
            }
            else {
                if (!strncmp(head,"F",1) || !strncmp(head,"f",1)) {
                    strcat(fmtStr, dispStr);
                    strcat(fmtStr, "f");
                }
                else { 
                    strcat(fmtStr, dispStr);
                    strcat(fmtStr, "d");
                }
            }
        }
 
        for(i=0; i<strlen(fmtStr); i++)
            str[i] = fmtStr[i];
        str[i]='\0';
    }
}

/*------------------------------------------------------------------------ 
 NAME
     fitsTable -  convert the current table  to the HTML table           
 DESCRIPTION
     retrieve the fits data in the table , then covert it to the HTML table
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsTable(int unit,FILE *fp,lvar_st *l_vars)
#else
fitsTable(unit,fp, l_vars)
int     unit;  /* fits i/o unit number */
FILE    *fp;
lvar_st *l_vars
#endif
{
 
    /* some of fits variable */
    int simple,extend,anyflg;
    int bitpix,naxis,naxes[99],pcount,gcount;
    int group;

    int objNumber=1;
    int status = 0;
    int hdutype;
    int i, j, kk;
    
    int length, tmpLen;

    /* variables that get the ASCII table header keywords from the CHU  */   
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS], tblen[NUM_FIELDS], \
        torder[NUM_FIELDS],tdataType[NUM_FIELDS];

    int datacode, datatype, repeat, width;
    char snull[5];

    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
      tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS],     
      tdisp[NUM_FIELDS][FITS_CLEN_HDEFKWDS],   
      tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];
   
    static float64 tscale[NUM_FIELDS], tzero[NUM_FIELDS];
 
    int tbrowlen;

    /* variables that get elements from an ASCII or binary table (FTGCV.) */
    int colnum, frow, felem, nelmt;
    int order, varidat, FitsStrBufLen;
    
    char *tmpStr;
    char tmpStr1[FITS_CLEN_HDEFKWDS],tmpStr2[FITS_CLEN_HDEFKWDS];

    char *retString, *ptr;
    char errtxt[FITS_CLEN_ERRMSG];

    int  numberType, datasize;

    int  recNumLen = 5;

    /* pointer of various types  */
    union {
        char    *cp;    /* Byte or char */
        int16   *sp;    /* short */
        int     *ip;    /* integer */
        float32 *fp;    /* float */
        float64 *dp;    /* double */
    } databuf;
  
    /* define the sunseting variables */
    int  startRecord, endRecord, subTfieldNumber;      
    int subIndex[FIELDNAMELENMAX];
    
    startRecord = l_vars->ft_start_rec;
    if (startRecord < 0)
      startRecord =  1;
    endRecord   = l_vars->ft_end_rec;
            
    /* assign subIndex to respond the real field  */ 
    subTfieldNumber = l_vars->ft_nfields;
    for (i=0; i<subTfieldNumber; i++) 
      subIndex[i] = *(l_vars->ft_fld_indices)++;

    /* fprintf(fp,"<PRE>\n");  */
    
    fprintf(fp,"<TABLE BORDER>\n");

    /* read the required ASCII TABLE keywords  */
    status = 0;
    FCGHTB(unit, NUM_FIELDS, &rowlen, &nrows, &tfields, ttype, tbcol, \
	   tform, tunit, extname, &status);

    CHECKFITSERROR("FCGHTB", status, errtxt);

    /* tittle of the table  */
    if (strlen(extname)) {
      fprintf(fp, "<TR>\n");
      fprintf(fp, "<TH COLSPAN = %d> %s </TH>\n", subTfieldNumber+1, extname);
      fprintf(fp, "</TR>\n");
    }

    /* get information for each field */
    for (i=0; i<tfields; i++) {
      status = 0;
      FCGACL(unit, i+1, ttype[i], &tbcol[i], tunit[i], tform[i], \
	     &tscale[i],&tzero[i], snull, tdisp[i], &status);
      CHECKFITSERROR("FCGACL", status, errtxt);

      datatype = getTabDatType(tform[i]);
 
      if (!(strlen(tdisp[i])))
	 strcpy(tdisp[i], tform[i]);

      width    = fitsTabWidth(tdisp[i]);
      repeat   = 1;

      if (!status) {
	switch(datatype) {
	case DFNT_CHAR8:

	  tdataType[i] = DFNT_CHAR8;
	  strcpy(tdisp[i], tform[i]);
	  width    = fitsTabWidth(tdisp[i]);
	  break;

	case DFNT_UINT8:  /* unsigned char */
	  
	    tdataType[i] = DFNT_UINT8;
	 
	    /* no change for display */
	    if (!((tscale[i] == 1.0) && (tzero[i] == 0.0))) {
	        
		tdataType[i] = DFNT_FLOAT32;
		/* realValue = dataValue * tscale + tzero; */
		datatype = 82;
	    }
	  
	    break;
	case DFNT_INT16:  /* short integer */
	case DFNT_INT32:  /* integer */
	  
	    tdataType[i] = DFNT_INT32;
	 
	    /* no change for display */
	    if (!((tscale[i] == 1.0) && (tzero[i] == 0.0))) {
	     
		tdataType[i] = DFNT_FLOAT32;
		/* realValue = dataValue * tscale + tzero; */
		datatype = 82;
	    }
	  
	    break;

	case DFNT_FLOAT32:  /* float */
	case DFNT_FLOAT64:  /* double */ 
	  tdataType[i] = DFNT_FLOAT32;
	  break;
 
	defaults:
	  InfoMsg("Complex TFORMn in the fits file ");
	  tdataType[i] = DFNT_FLOAT32;
	  width     = 10;	 
	  tbcol[i]  = width;
	  torder[i] = 1;
	}

	tbcol[i]  = width * repeat + repeat;
	torder[i] = repeat;
      }
      else {
	tdataType[i] = DFNT_FLOAT32;	 
	tbcol[i]  = 10;
	torder[i] =  1;
      }
    }

    tbrowlen = 1;
    /* modify the length for each field so that I can put the data */    
    for (i=0; i<tfields; i++) {

      /* length of each field from table header */    
      length =  tbcol[i];

      /* length of the title of each field */
      tmpLen = strlen((char *)trim(ttype[i]));

      if (length <= tmpLen)
	/* expand the table */
	tblen[i] = tmpLen;
      else {

	tmpLen = (length - tmpLen);

	if (mod((length-strlen((char *)trim(ttype[i]))),2))
	  /* odd */
	  tblen[i] = length +1;
	else
	  tblen[i] = length;
      }
      tbrowlen += tblen[i];

    }
 
    retString = (char *)FITSmalloc(tbrowlen + tfields);
    ptr = tmpStr = (char *)FITSmalloc(tbrowlen + tfields); 
    if ((retString == NULL) || (tmpStr == NULL)) 
      return FAIL;
 
    retString[0] = '\0';

    /* create the tittle for this ascii table  */
    for (i=0; i<(tbrowlen + tfields); i++)
      strcat(retString, "-");
    
    /* fprintf(fp,"%s\n", retString);  */

 
    length = 0;
    tmpStr[0] = '\0';

    fprintf(fp,"<TR>\n");

    /* create the first column with record number */
    fprintf(fp,"<TH> Rec. </TH>\n");

    /* create table title for each name of the field */    
    for (i=0; i<subTfieldNumber; i++) {

      tmpStr[0] = '\0';
      j = subIndex[i]-1;
      
      #if defined(oldway) 
      /* length of each field */     
      length = tblen[i];          
      tmpLen = strlen((char *)trim(ttype[i]));
      tmpLen = (length - tmpLen)/2;

      strcat(tmpStr, (char *)space(tmpLen));     
      strcat(tmpStr, (char *)trim(ttype[i]));
      strcat(tmpStr, (char *)space(tmpLen));

      /* if (i < (tfields-1))
	strcat(tmpStr,"|"); */
      #endif
      fprintf(fp,"<TH>%s</TH>\n",ttype[j]);


    }

    fprintf(fp,"</TR>\n");

    /* fprintf(fp, "%s\n", tmpStr); */
    
    tmpStr = ptr;
    tmpStr[0] = '\0';

    fprintf(fp,"<TR>\n");

    /* create the first column for the record number */
    fprintf(fp,"<TH>%s</TH>\n",(char *)space(recNumLen));


    /* create table title for each name of the field(unit) */    
    for (i=0; i<subTfieldNumber; i++) {

      tmpStr[0] = '\0';
      j = subIndex[i] - 1;
   
      #if defined(oldway) 
      /* length of each field */
     
      length = tblen[i];

      tmpLen = strlen((char *)trim(tunit[i]));
      if (!tmpLen)  /* empty */
	strcat(tmpStr, (char *)space(length));
      else {

	tmpLen = length - tmpLen;
	
	if (tmpLen < 0) 
	  tmpLen = 0;

	if (mod(tmpLen,2))
	  /* odd */
	  strcat(tmpStr, (char *)space((tmpLen/2)+1));  	
	else
	  strcat(tmpStr, (char *)space(tmpLen/2)); 
	strcat(tmpStr, (char *)substr(tunit[i],1,length));
	strcat(tmpStr, (char *)space(tmpLen/2));
      }
  
      /* if (i < (tfields-1))
	strcat(tmpStr,"|"); */
      #endif
      fprintf(fp,"<TH>%s</TH>\n",tunit[j]);

    }

    fprintf(fp,"</TR>\n");

    /* fprintf(fp, "%s\n", tmpStr);       
    fprintf(fp, "%s\n", retString); */

    /* data processing */    
    nelmt  = nrows; /* number of elements (number of records) */   

    /* adjust tdisp */
    for (i=0; i<tfields; i++) {
    
      numberType = getTabDatType(&tform[i][0]); 
  
      switch(numberType) {
   
      case DFNT_UINT8:

	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%u");
	else
	  fitsTabDispFmt(tdisp[i]);

	if (!((tscale[i] == 1.0) && (tzero[i] == 0.0))) 
	  strcpy(tdisp[i], "%12.5f");

        break;
    
      case DFNT_INT32:

	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%d");
	else
	  fitsTabDispFmt(tdisp[i]);
        break;
      case DFNT_FLOAT32:
	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%12.5f");
	else
	  fitsTabDispFmt(tdisp[i]);
        break;
      case DFNT_FLOAT64:
	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%14.7e");
	else
	  fitsTabDispFmt(tdisp[i]);

        break;
      case DFNT_CHAR8:
	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%s");
	else
	  fitsTabDispFmt(tdisp[i]);

	break;
      default:
	strcpy(tdisp[i],"%10d");
      }
    }
 
    /* ======= subsetting ====== */
    nelmt = endRecord - startRecord + 1;

    /* ======= subsetting ====== */   
    for (kk=startRecord; kk <= endRecord; kk++) {
    
    fprintf(fp, "<TR>\n");

    tmpStr[0] = '\0';

    /* create the first column */
    sprintf(tmpStr, "%5d", kk);
    fprintf(fp,"<TD ALIGN=right> %s</TD>\n",tmpStr);


    /* read the table and move to memory buffer   */
    for (i=1; i<=subTfieldNumber; i++) {

      static char **tmpchr;
      
      tmpStr[0] = '\0';

      /* Calculate vdata size and allocate buffer space */
      colnum = subIndex[i-1];  /* colnum in table */
      frow   = kk;
      felem  = 1;
      nelmt  = 1; /* number of elements */

      numberType = getTabDatType(&tform[colnum-1][0]);

      switch(numberType) {
      case DFNT_INT32:
        datasize = sizeof(int32)*nelmt;
        break;
      case DFNT_FLOAT32:
        datasize = sizeof(float32)*nelmt;
        break;
      case DFNT_FLOAT64:
        datasize = sizeof(float64)*nelmt;
        break;
      case DFNT_CHAR8:
        FitsStrBufLen = (int)atoi((char *)substr(&tform[colnum-1][0],2,2)); 
        datasize = (FitsStrBufLen+1) * nelmt;
        break;
      }

      databuf.cp = (char *)FITSmalloc(datasize);

      group=0;

      switch (numberType){
      case DFNT_CHAR8:          /* byte/char */

        tmpchr   = (char **)malloc(nelmt*sizeof(char *));  /* keep address */
        tmpchr[0] = (char *)malloc(datasize);
        for (j=1; j<nelmt; j++)
          tmpchr[j] = tmpchr[0] + ((FitsStrBufLen+1)*j);

        status = 0;
        FCGCVS(unit,colnum,frow,felem,nelmt," " ,tmpchr[0],&anyflg, &status);
        CHECKFITSERROR("FCGCVS",status,errtxt);

        strcpy(tmpStr1, tmpchr[0]);
	break;

      case DFNT_UINT8:            /* unsigned char */
   
        /* reset the bscale */
        status = 0;
	FCTSCL(unit,i, 1.0, 0 ,&status);
        CHECKFITSERROR("FCTSCL",status,errtxt);
        status = 0;
        FCGCVB(unit,colnum, frow, felem, nelmt,1,(unsigned char *)databuf.cp, \
	       &anyflg,&status);
        CHECKFITSERROR("FCGCVB",status,errtxt);
	if (!anyflg) {
	  if (tdataType[colnum-1] == DFNT_UINT8) {
	    
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
					       (uint8)*databuf.cp));
	    
	  }
	  else { /* realData = fits DATA * bscale + bzero  */

	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
	     (float)((uint8)(*databuf.cp)*tscale[colnum-1]+tzero[colnum-1])));

	  }
	}
	else      
	  strcpy(tmpStr1, "NaN");

	break;

      case DFNT_INT32:            /* integer */
   
        /* reset the bscale */
        status = 0;
	FCTSCL(unit,i, 1.0, 0 ,&status);
        CHECKFITSERROR("FCTSCL",status,errtxt);
        status = 0;
        FCGCVJ(unit,colnum, frow, felem, nelmt,0,databuf.ip,&anyflg,&status);
        CHECKFITSERROR("FCGCVJ",status,errtxt);
	if (!anyflg) {
	  if (tdataType[colnum-1] == DFNT_INT32) {

	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],  \
					       (int)*databuf.ip));
	  }
	  else { /* realData = fits DATA * bscale + bzero  */

	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
	        (float)((*databuf.ip)*tscale[colnum-1]+tzero[colnum-1])));
	  }
	}
	else
	  strcpy(tmpStr1, "NaN");
	break;

      case DFNT_FLOAT32:          /* float */


        status = 0;
        FCGCVE(unit,colnum,frow,felem,nelmt,-1.0,databuf.fp, &anyflg, &status);
	CHECKFITSERROR("FCGCVE",status,errtxt);
	/* 
	if (!(strlen(tdisp[colnum-1])))
	  strcpy(tdisp[colnum-1],"%f");
	else
	  fitsTabDispFmt(tdisp[colnum-1]); */
	if (!anyflg)
	  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
					   (float)*databuf.fp));
	else
	  strcpy(tmpStr1, "NaN");
	break;

      case DFNT_FLOAT64:                /* double */
        status = 0;
        FCGCVD(unit,colnum,frow,felem, nelmt,-1.0,databuf.dp,&anyflg,&status);
        CHECKFITSERROR("FCGCVD",status,errtxt);
	/* 
	if (!(strlen(tdisp[colnum-1])))
	  strcpy(tdisp[colnum-1],"%E");
	else
	  fitsTabDispFmt(tdisp[colnum-1]); */
	if (!anyflg)
	  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], *databuf.dp));
	else
	  strcpy(tmpStr1, "NaN");
        break;

      default:

        /* FatalError("Complex field number type in this table\n");
	return FAIL; */

	strcpy(tmpStr1,"*");
      }

     if (status == 0) {
       /* data format processing */

       /* length of each field */
       length = tblen[colnum-1];
       strcat(tmpStr, (char *)substr(tmpStr1,1,length));        
     }
     /* 
     if (i<tfields)
       strcat(tmpStr, "|"); */

     fprintf(fp,"<TD ALIGN=right> %s</TD>\n",tmpStr);

    } /* for (i=1 ...) */

    /*  fprintf(fp,"%s\n",tmpStr); */
    fprintf(fp, "</TR>\n");
    
    } /* for (kk=1, ..)  */

    /* fprintf(fp,"%s\n",retString);
    fprintf(fp,"</PRE>\n"); */

    fprintf(fp, "</TABLE>\n");

    /* free memory  */
    FITSfree(tmpStr);
    FITSfree(retString);

    
    return SUCCEED;
}

/*------------------------------------------------------------------------ 
 NAME
     fitsBinTabInfo -  get fits binary table information         
 DESCRIPTION
     set fits table  read position and get binary table information
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsBinTabInfo(int unit,int tab, int *entries, int *recNum)
#else
fitsBinTabInfo(unit, tab, entries, recNum)
int unit;
int tab;
int *entries, *recNum;
#endif
{
    int tabNumber=0;
    int status = 0;
    int hdutype;
    int i;

    char errtxt[FITS_CLEN_ERRMSG];

    /* variables that get the Binary table header keywords from the CHU  */
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS]; 
    
    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
        tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS],    
        tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];

    /* variable length data */
    int  varidat;

    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    status= 0;
    FCMAHD(unit,1,&hdutype,&status);
    /* CHECKFITSERROR("FCMAHD",status,errtxt); */
    
    while (status == 0) {

        /* Moving to next extension in the FITS file */
        status= 0;
        FCMRHD(unit,1,&hdutype,&status);

        switch (hdutype) {
        case 2: /* asciiTable */
            ++tabNumber;
            break;
        default:
            break;
        }
        if (tab == tabNumber) 
            break;
    }   
 
    /* read the required ASCII TABLE keywords  */
    status = 0;
    FCGHBN(unit, NUM_FIELDS, &nrows, &tfields, ttype, tform, tunit, extname,&varidat, &status);

    /* CHECKFITSERROR("FCGHBN", status, errtxt); */
    
    if (!status) {
        *entries = tfields;
        *recNum  = nrows;
    }
    else {
        *entries = 0;
        *recNum  = 0;
        return FAIL;
    } 

    return(0);
}

/*------------------------------------------------------------------------ 
 NAME
     fitsBinTabFldName -  get the fits binary table field name        
 DESCRIPTION
      get the field name of the fits binary table
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
char *
#ifdef __STDC__
fitsBinTabFldName(int unit,int fieldNumber)
#else
fitsBinTabFldName(unit, fieldNumber)
int unit;
int fieldNumber;
#endif
{
    char errtxt[FITS_CLEN_ERRMSG];
    int  status;
    /* variables that get the ASCII table header keywords from the CHU  */
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS]; 
    
    /* variable length data */
    int  varidat;
    
    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
        tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS],    
        tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];
   
    /* read the required BINARY TABLE keywords  */
    status = 0;
    FCGHBN(unit, NUM_FIELDS, &nrows, &tfields, ttype, tform, tunit, 
	   extname,&varidat, &status);
    if ((status == 0)&&(fieldNumber>=1)) {
      strcpy(retStr, ttype[fieldNumber-1]);   
      return(retStr);
    }
    else
      return("");
}


/*------------------------------------------------------------------------ 
 NAME
     getBinTabDatType - return the date type based on the TFORM[i]          
 DESCRIPTION
     get the data number type of the field
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int getBinTabDatType(str)
char *str;
{

    int datacode, repeat, width;
    int status = 0;

    FCBNFM(str, &datacode, &repeat, &width, &status);

    if (!status) {
        switch(datacode) {
        case 16:
            return(DFNT_CHAR8);

        case 11:   /* byte */
            return(DFNT_UINT8);

        case 21:  /* short integer */
        case 41:  /* integer */
            return(DFNT_INT32);
        case 42:  /* float */
            return(DFNT_FLOAT32);
        case 82:
            return(DFNT_FLOAT64);

          defaults:
            InfoMsg("Complex TFORMn in the fits file ");
            return(-1);
        }
    }
    else 
        return -1;
}

/*------------------------------------------------------------------------ 
 NAME
     fitsBinTabWidth -  return width of the table based on the TDISP[i]  
 DESCRIPTION
     get the width of the field
 RETURN
     the width number
-------------------------------------------------------------------------*/ 
int fitsBinTabWidth(str)
char *str;
{

    int pos, width;
    pos = at(".", str);
    if (pos >=1 ) 
        width = atoi((char *)substr(str,2,pos-1));
    else
        if (!strcmp((char *)substr(str,1,1),"A"))
            width = atoi((char *)substr(str,2,5));
        else
            width = 10;
    return width;
}

/*------------------------------------------------------------------------ 
 NAME
     fitsBinTabDispFmt -  convert the Fortran display format in C       
 DESCRIPTION
     convert the Fortran display format in C      
 RETURN
     NONE
-------------------------------------------------------------------------*/ 
void fitsBinTabDispFmt(str)
char *str;
{
    int  pos,i, width;
    char dispStr[20];
    char head[8];
    char fmtStr[20];
 
    strcpy(fmtStr,"%");

    if (strlen(str)) {

        strcpy(head, (char *)substr(str,1,1));
  
        pos = at("E", (char *)substr(str,2,20));
        if (pos <0)
            pos = at("e", (char *)substr(str,2,20));

        if (pos >=1 ) 
            strcpy(dispStr, (char *)substr(str,2,pos-1));
        else
            strcpy(dispStr, (char *)substr(str,2,10));

        if (!strncmp(head,"d",1) || !strncmp(head,"D",1) || !strncmp(head,"E",1)|| \
            !strncmp(head,"e",1) ) {
            strcat(fmtStr, dispStr);
            strcat(fmtStr, "e");
        }
        else {
            if (!strncmp(head,"A",1) || !strncmp(head,"a",1)) {
                strcat(fmtStr, dispStr);
                strcat(fmtStr, "s");
            }
            else {
                if (!strncmp(head,"F",1) || !strncmp(head,"f",1)) {
                    strcat(fmtStr, dispStr);
                    strcat(fmtStr, "f");
                }
                else { 
                    strcat(fmtStr, dispStr);
                    strcat(fmtStr, "d");
                }
            }
        }
 
        for(i=0; i<strlen(fmtStr); i++)
            str[i] = fmtStr[i];
        str[i]='\0';
    }
}

/*------------------------------------------------------------------------ 
 NAME
     fitsBinTable -  retrieve the data in the fits binary table  and 
                     convert it to the HTML table
 DESCRIPTION
     Retrieve the data in the fits binary table  and convert it to the HTML 
     table                     
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsBinTable(int unit, FILE *fp, lvar_st *l_vars)
#else
fitsBinTable(unit, fp,l_vars)
int     unit;  /* fits i/o unit number */
FILE    *fp;
lvar_st *l_vars;
#endif
{
 
    /* some of fits variable */
    int simple,extend,anyflg;
    int bitpix,naxis,naxes[99],pcount,gcount;
    int group;

    int objectNumber=1;
    int status = 0;
    int hdutype;
    int i = 1, j=1, kk = 1;
    
    int length, tmpLen;
    /* variables that get the binary table header keywords from the CHU  */
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS], tblen[NUM_FIELDS], \
        torder[NUM_FIELDS],tdataType[NUM_FIELDS];

    int datacode, datatype, repeat, width, tnull;

    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
      tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS], 
      tdisp[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  
      tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];
    
    static float64 tscale[NUM_FIELDS], tzero[NUM_FIELDS];

    int tbrowlen;

    /* variables that get elements from an ASCII or binary table (FTGCV.) */
    int colnum, frow, felem, nelmt, ncol;
    int order, varidat, FitsStrBufLen;
    
    char *tmpStr;
    char tmpStr1[5000],tmpStr2[FITS_CLEN_HDEFKWDS];

    char *retString, *ptr;
    char errtxt[FITS_CLEN_ERRMSG];

    int  numberType, datasize;

    int  recNumLen = 5;

    /* pointer of various types  */
    union {
        char    *cp;    /* Byte or char */
        int16   *sp;    /* short */
        int     *ip;    /* integer */
        float32 *fp;    /* float */
        float64 *dp;    /* double */
    } databuf;
  

    /* define the sunseting variables */
    int  startRecord, endRecord, subTfieldNumber;      
    int subIndex[FIELDNAMELENMAX];

    /*  ===============     TEST FOR SUBSETTING  =============== */
    
    startRecord = l_vars->ft_start_rec;
    if (startRecord < 0)
      startRecord =  1;
    endRecord   = l_vars->ft_end_rec;
            
    /* assign subIndex to respond the real field  */ 
    subTfieldNumber = l_vars->ft_nfields;
    for (i=0; i<subTfieldNumber; i++) 
      subIndex[i] = *(l_vars->ft_fld_indices)++;

    /* read the required ASCII TABLE keywords  */
    status = 0;
    FCGHBN(unit, NUM_FIELDS, &nrows, &tfields, ttype, \
	   tform, tunit, extname,&varidat, &status);
    
    CHECKFITSERROR("FCGHTB", status, errtxt);

    fprintf(fp,"<TABLE BORDER>\n");

    /* tittle of the table  */
    if (strlen(extname)) {
      fprintf(fp, "<TR>\n");
      fprintf(fp, "<TH COLSPAN = %d> %s </TH>\n", subTfieldNumber + 1,extname);
      fprintf(fp, "</TR>\n");
    }

    /* get length & order for each field */
    for (i=0; i<tfields; i++) {
      status = 0;
      FCGBCL(unit, i+1, ttype[i], tunit[i], tmpStr1, &repeat, &tscale[i], \
	     &tzero[i], &tnull, tdisp[i], &status);
      CHECKFITSERROR("FCGBCL", status, errtxt);

      datatype = getBinTabDatType(tform[i]); 

      width    = fitsBinTabWidth(tdisp[i]);
   
      if (!status) {
	switch(datatype) {
	case DFNT_CHAR8:

	  tdataType[i] = DFNT_CHAR8;
	  strcpy(tdisp[i], tmpStr1);
	  width    = fitsBinTabWidth(tdisp[i]);
	  break;

	case DFNT_UINT8:  /* unsigned char */
	  
	    tdataType[i] = DFNT_UINT8;
	 
	    /* no change for display */
	    if (!((tscale[i] == 1.0) && (tzero[i] == 0.0))) {
	        
		tdataType[i] = DFNT_FLOAT32;
		/* realValue = dataValue * tscale + tzero; */
		datatype = 82;
	    }
	  
	    break;
	case DFNT_INT16:  /* short integer */
	case DFNT_INT32:  /* integer */
	  
	    tdataType[i] = DFNT_INT32;
	 
	    /* no change for display */
	    if (!((tscale[i] == 1.0) && (tzero[i] == 0.0))) {
	     
		tdataType[i] = DFNT_FLOAT32;
		/* realValue = dataValue * tscale + tzero; */
		datatype = 82;
	    }
	  
	    break;

	case DFNT_FLOAT32:  /* float */
	case DFNT_FLOAT64:  /* double */ 
	  tdataType[i] = DFNT_FLOAT32;
	  break;
 
	defaults:
	  InfoMsg("Complex TFORMn in the fits file ");
	  tdataType[i] = DFNT_FLOAT32;
	  width     = 10;	 
	  tbcol[i]  = width;
	  torder[i] = 1;
	}

	tbcol[i]  = width * repeat + repeat;
	torder[i] = repeat;
      }
      else {
	tdataType[i] = DFNT_FLOAT32;	 
	tbcol[i]  = 10;
	torder[i] =  1;
      }
    }

    tbrowlen = 1;
    /* modify the length for each field so that I can put the data */    
    for (i=0; i<tfields; i++) {

      /* length of each field from table header */
      length = tbcol[i];

      /* length of the title of each field */
      tmpLen = strlen((char *)trim(ttype[i]));

      if (length <= tmpLen)
	/* expand the table */
	tblen[i] = tmpLen;
      else {

	tmpLen = (length - tmpLen);

	if (mod((length-strlen((char *)trim(ttype[i]))),2))
	  /* odd */
	  tblen[i] = length +1;
	else
	  tblen[i] = length;
      }
      tbrowlen += tblen[i];

    }
 
    retString = (char *)FITSmalloc(tbrowlen + tfields);
    ptr = tmpStr = (char *)FITSmalloc(tbrowlen + tfields); 
    if ((retString == NULL) || (tmpStr == NULL)) 
      return FAIL;
 
    retString[0] = '\0';

    /* create the tittle for this ascii table  */
    /* for (i=0; i<(tbrowlen + tfields); i++)
      strcat(retString, "-"); */
    
    /* fprintf(fp,"%s\n", retString); */

    fprintf(fp,"<TR>\n"); 

    /* create the first column with record number */
    fprintf(fp,"<TH> Rec. </TH>\n");


    length = 0;
    tmpStr[0] = '\0';

    /* create table title for each name of the field */    
    for (i=0; i<subTfieldNumber; i++) {
      
      tmpStr[0] = '\0';
      j = subIndex[i]-1;

      #if defined(oldway) 
      /* length of each field */
      length = tblen[i];

      tmpLen = strlen((char *)trim(ttype[i]));
      tmpLen = (length - tmpLen)/2;
      
      strcat(tmpStr, (char *)space(tmpLen));     
      strcat(tmpStr, (char *)trim(ttype[i]));
      strcat(tmpStr, (char *)space(tmpLen)); 

      /* if (i < (tfields-1))
	strcat(tmpStr,"|"); */
      #endif
      fprintf(fp, "<TH> %s </TH>\n", ttype[j]);

    }

    fprintf(fp,"</TR>\n");

    /*
     fprintf(fp, "%s\n", tmpStr); */

    fprintf(fp,"<TR>\n");

    tmpStr = ptr;
    tmpStr[0] = '\0';

    /* create the first column for the record number */
    fprintf(fp,"<TH>%s</TH>\n",(char *)space(recNumLen));


    /* create table title for each name of the field(unit) */    
    for (i=0; i<subTfieldNumber; i++) {
      
      tmpStr[0] = '\0';
      j = subIndex[i]-1;

      #if defined(oldway) 
      /* length of each field */
      length = tblen[i];

      tmpLen = strlen((char *)trim(tunit[i]));
      if (!tmpLen)  /* empty */
	strcat(tmpStr, (char *)space(length));
      else {

	tmpLen = length - tmpLen;
	
	if (tmpLen < 0) 
	  tmpLen = 0;

	if (mod(tmpLen,2))
	  /* odd */
	  strcat(tmpStr, (char *)space((tmpLen/2)+1));  	
	else
	  strcat(tmpStr, (char *)space(tmpLen/2)); 
	strcat(tmpStr, (char *)substr(tunit[i],1,length));
	strcat(tmpStr, (char *)space(tmpLen/2));
      }
      /* if (i < (tfields-1))
	strcat(tmpStr,"|"); */
      #endif
      
      /* fprintf(fp, "<TH> %s </TH>\n", tmpStr); */
      fprintf(fp, "<TH> %s </TH>\n", tunit[j]);

    }

    fprintf(fp,"</TR>\n");
    
    /* 
       fprintf(fp, "%s\n", tmpStr);      
       fprintf(fp, "%s\n", retString); */

    /* data processing */
     
    nelmt  = nrows; /* number of elements (number of records) */

    /* adjust tdisp */
    for (i=0; i<tfields; i++) {
    
      numberType = getBinTabDatType(&tform[i][0]); 
  
      switch(numberType) {
   
      case DFNT_UINT8:

	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%u");
	else
	  fitsBinTabDispFmt(tdisp[i]);

	if (!((tscale[i] == 1.0) && (tzero[i] == 0.0))) 
	  strcpy(tdisp[i], "%f");

        break;
    
      case DFNT_INT32:

	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%d");
	else
	  fitsBinTabDispFmt(tdisp[i]);
        break;
      case DFNT_FLOAT32:
	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%f");
	else
	  fitsBinTabDispFmt(tdisp[i]);
        break;
      case DFNT_FLOAT64:
	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%.5e");
	else
	  fitsBinTabDispFmt(tdisp[i]);

        break;
      case DFNT_CHAR8:
	if (!(strlen(tdisp[i]))) 
	  strcpy(tdisp[i],"%10s");
	else
	  fitsBinTabDispFmt(tdisp[i]);

	break;
      default:
	strcpy(tdisp[i],"%10d");
      }
    }
  

    /* for each record */
    for (kk=startRecord; kk<=endRecord; kk++) {
   
    fprintf(fp,"<TR>\n");

    tmpStr[0] = '\0';

    /* create the first column */
    sprintf(tmpStr, "%5d", kk);
    fprintf(fp,"<TD ALIGN=right> %s</TD>\n",tmpStr);

    /* read the binary table and move to string   */
    for (i=1; i<=subTfieldNumber; i++) {

      static char **tmpchr;
 
      tmpStr[0] = '\0';
 
      /* Calculate vdata size and allocate buffer space */
      colnum = subIndex[i-1];  /* colnum in table */
      frow   = kk;
      felem  = 1;
      ncol   = torder[colnum-1];  /* number of elements */
      
      /* assume the repeat nuber is less than 6  */
      if (ncol > 3)
	nelmt = 3;
      else
	nelmt = ncol;
      
      if (nelmt) { /* non dummy */
      numberType = getBinTabDatType(&tform[colnum-1][0]); 
  
      switch(numberType) {
      case DFNT_INT32:
        datasize = sizeof(int32)*nelmt;
        break;
      case DFNT_UINT8:
        datasize = nelmt;
        break;
 
      case DFNT_FLOAT32:
        datasize = sizeof(float)*nelmt;
        break;
      case DFNT_FLOAT64:
        datasize = sizeof(float64)*nelmt;
        break;
      case DFNT_CHAR8:
	status = 0;
	FCBNFM(tform[colnum-1], &datacode, &repeat, &width, &status);

        FitsStrBufLen = width; /* TFORMn = 'A9' */
        datasize = (FitsStrBufLen+1) * nelmt;
        break;
      }

      databuf.cp = (char *)FITSmalloc(datasize);
      group=0;
     
      switch (numberType){
      case DFNT_CHAR8:          /* byte/char */

        tmpchr   = (char **)malloc(nelmt*sizeof(char *));  /* keep address */
        tmpchr[0] = (char *)malloc(datasize);
        for (j=1; j<nelmt; j++)
          tmpchr[j] = tmpchr[0] + ((FitsStrBufLen+1)*j);

        status = 0;
        FCGCVS(unit,colnum,frow,felem,nelmt," " ,tmpchr[0],&anyflg, &status);
        CHECKFITSERROR("FCGCVS",status,errtxt);

        strcpy(tmpStr1, tmpchr[0]);

	for (j=2; j<=nelmt; j++) {
	  strcat(tmpStr1, ";");
	  strcat(tmpStr1, tmpchr[j-1]);
	}

	break;

      case DFNT_UINT8:            /* unsigned char */
   
        /* reset the bscale */
        status = 0;
	FCTSCL(unit,i, 1.0, 0 ,&status);
        CHECKFITSERROR("FCTSCL",status,errtxt);
        status = 0;
        FCGCVB(unit,colnum, frow, felem, nelmt,0, \
	       (unsigned char *)databuf.cp,&anyflg,&status);
        CHECKFITSERROR("FCGCVB",status,errtxt);
	if (!anyflg) {
	  if (tdataType[colnum-1] == DFNT_UINT8) {

	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
					     (uint8)*databuf.cp));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	      
	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1], \
					       (uint8)databuf.cp[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	  }
	  else { /* realData = fits DATA * bscale + bzero  */

	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
	  (float)((uint8)(*databuf.cp)*tscale[colnum-1]+tzero[colnum-1])));
	    
	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	      
	      strcpy(tmpStr2,(char *)dispFmtDat(tdisp[colnum-1], \
	 (float)((uint8)(databuf.cp[j-1])*tscale[colnum-1]+tzero[colnum-1])));

	      strcat(tmpStr1, tmpStr2);
	    }
	  }
	}
	else 
	  strcpy(tmpStr1, "NaN");

	break;

      case DFNT_INT32:            /* integer */
   
        /* reset the bscale */
        status = 0;
	FCTSCL(unit,i, 1.0, 0 ,&status);
        CHECKFITSERROR("FCTSCL",status,errtxt);
        status = 0;
        FCGCVJ(unit,colnum, frow, felem, nelmt,-1,databuf.ip,&anyflg,&status);
        CHECKFITSERROR("FCGCVJ",status,errtxt);
	/*
	if (!(strlen(tdisp[colnum-1]))) 
	  strcpy(tdisp[colnum-1],"%d");
	else
	  fitsBinTabDispFmt(tdisp[colnum-1]); */
	if (!anyflg) {
	if (tdataType[colnum-1] == DFNT_INT32) {

	  strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1],(int)*databuf.ip));

	  for (j=2; j<=nelmt; j++) {
	    strcat(tmpStr1, ";");
	    
	    strcpy(tmpStr2,(char *)dispFmtDat(tdisp[colnum-1], \
					      (int)databuf.ip[j-1]));

	    strcat(tmpStr1, tmpStr2);
	  }
	}
	else { /* realData = fits DATA * bscale + bzero  */

	  strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
		(float)((*databuf.ip)*tscale[colnum-1]+tzero[colnum-1])));

	  for (j=2; j<=nelmt; j++) {
	    strcat(tmpStr1, ";");

	    strcpy(tmpStr2,(char *)dispFmtDat(tdisp[colnum-1],\
		(float)((databuf.ip[j-1])*tscale[colnum-1]+tzero[colnum-1])));

	    strcat(tmpStr1, tmpStr2);
	  }
	}
	}
	else
	  strcpy(tmpStr1,"NaN");
	break;

      case DFNT_FLOAT32:   {         /* float */
	
	status = 0;
        FCGCVE(unit,colnum,frow,felem,nelmt,-1.0,databuf.fp, &anyflg, &status);
	 
	CHECKFITSERROR("FCGCVE",status,errtxt);

	/* 
	if (!(strlen(tdisp[colnum-1])))
	  strcpy(tdisp[colnum-1],"%f");
	else
	  fitsBinTabDispFmt(tdisp[colnum-1]); */

	DBUG_PRINT(2,(LOGF, "Data= %d\n", databuf.fp));
	if (!anyflg) {
        strcpy(tmpStr1,(char*)dispFmtDat(tdisp[colnum-1],*databuf.fp)); 

	for (j=2; j<=nelmt; j++) {
	  strcat(tmpStr1, ";");
	  
	  strcpy(tmpStr2,(char *)dispFmtDat(tdisp[colnum-1], \
					    (float)databuf.fp[j-1]));

	  strcat(tmpStr1, tmpStr2);
	}
	}
	else
	  strcpy(tmpStr1,"NaN");

	break;
      }
      case DFNT_FLOAT64:                /* double */
        status = 0;
        FCGCVD(unit,colnum,frow,felem, nelmt,-1.0,databuf.dp,&anyflg,&status);
        CHECKFITSERROR("FCGCVD",status,errtxt);
	/* 
	if (!(strlen(tdisp[colnum-1])))
	  strcpy(tdisp[colnum-1],"%E");
	else
	  fitsBinTabDispFmt(tdisp[colnum-1]); */
	if (!anyflg) {
    	strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], *databuf.dp));
	for (j=2; j<=nelmt; j++) {
	  strcat(tmpStr1, ";");

	  strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1],databuf.dp[j-1]));
	  strcat(tmpStr1, tmpStr2);
	}
	}
	else
	  strcpy(tmpStr1, "NaN");
        break;

      default:

        /* FatalError("Complex field number type in this table\n");
	return FAIL; */

	strcpy(tmpStr1,"*");
      }


     if (status == 0) {
       /* data format processing */

       /* length of each field */
       length = tblen[colnum-1];
       
       #if defined(oldway) 

       tmpLen = strlen((char *)trim(tmpStr1));
       if (!tmpLen)  /* empty */
	 strcat(tmpStr, (char *)space(length));
       else {

	 tmpLen = length - tmpLen;
	
	 if (tmpLen < 0) 
	   tmpLen = 0;
	 
	 if (mod(tmpLen,2))
	   /* odd */
	   strcat(tmpStr, (char *)space((tmpLen/2)+1));  	
	 else
	   strcat(tmpStr, (char *)space(tmpLen/2)); 
	 strcat(tmpStr, (char *)substr(tmpStr1,1,length));
	 strcat(tmpStr, (char *)space(tmpLen/2));
       }
       #endif
       strcat(tmpStr, (char *)substr(tmpStr1,1,length)); 
     }
     /* 
      if (i<tfields)
        strcat(tmpStr, "|"); */
     
     if (torder[colnum-1] <= 3)
       fprintf(fp, "<TD ALIGN = right> %s </TD>\n", tmpStr);   
     else {
       sprintf(tmpStr2, "#row%dcol%d",kk,i);
       
       fprintf(fp, "<TD ALIGN = left> <A HREF=\"%s\" >%s ...</A> </TD>\n",tmpStr2,tmpStr);
     }
      }
    else  /* dummy */
      fprintf(fp, "<TD ALIGN = right> %s </TD>\n", "*");   

    } /* for (i=1 ...) */

    
    fprintf(fp,"</TR>\n");

    /* fprintf(fp,"%s\n",tmpStr); */

    DBUG_PRINT(2,(LOGF,"Rec. =%d\n", kk));
       
    } /* for (kk=1, ..)  */

    /* fprintf(fp,"%s\n",retString);
    fprintf(fp,"</PRE>\n"); */

    fprintf(fp, "</TABLE>\n");
    
    /* if the entries of the column contain ore than 3 items 
       ake a new table to see details ( <A NAME="#row1col3" > ... </A>*/
 
    /* for each record */
    for (kk=startRecord; kk<=endRecord; kk++) {
   
    tmpStr[0] = '\0';
    /* read the binary table and move to string   */
    for (i=1; i<=subTfieldNumber; i++) {

      static char **tmpchr;
 
      tmpStr[0] = '\0';
 
      /* Calculate vdata size and allocate buffer space */
      colnum = subIndex[i-1];  /* colnum in table */
      frow   = kk;
      felem  = 1;
      ncol   = torder[colnum-1];  /* number of elements */

      if (ncol > 200) 
	nelmt  = 200;
      else 
	nelmt  = ncol;

      /* repeat number is greater thsn 3 use hyper link to see more datas */
      if (ncol > 3) {

	fprintf(fp,"<HR>\n");
	/* create the HTML */
	sprintf(tmpStr2, "row%dcol%d",kk,i);  
	fprintf(fp, "<A NAME=\"%s\"><H3> Data within the table(Row: %d; Col: %d)</H3> </A>\n", tmpStr2, kk, i);
	
	fprintf(fp, "<TABLE BORDER>\n");
	
	numberType = getBinTabDatType(&tform[colnum-1][0]); 
  
	switch(numberType) {
	case DFNT_INT32:
	  datasize = sizeof(int32)*nelmt;
	  break;
	case DFNT_FLOAT32:
	  datasize = sizeof(float32)*nelmt;
	  break;
	case DFNT_FLOAT64:
	  datasize = sizeof(float64)*nelmt;
	  break;
	case DFNT_UINT8:
	  datasize = nelmt;
	  break;
	case DFNT_CHAR8:
	  status = 0;
	  FCBNFM(tform[colnum-1], &datacode, &repeat, &width, &status);

	  FitsStrBufLen = width; /* TFORMn = 'A9' */
	  datasize = (FitsStrBufLen+1) * nelmt;
	  break;
	}

	
	databuf.cp = (char *)FITSmalloc(datasize);
      
	group=0;

	switch (numberType){
	case DFNT_CHAR8:          /* byte/char */

	  tmpchr   = (char **)malloc(nelmt*sizeof(char *));  /* keep address */
	  tmpchr[0] = (char *)malloc(datasize);
	  for (j=1; j<nelmt; j++)
	    tmpchr[j] = tmpchr[0] + ((FitsStrBufLen+1)*j);

	  status = 0;
	  FCGCVS(unit,colnum,frow,felem,nelmt," " ,tmpchr[0],&anyflg, &status);
	  CHECKFITSERROR("FCGCVS",status,errtxt);

	  objectNumber = 1;
	  strcpy(tmpStr1, tmpchr[0]);

	  fprintf(fp,"<TR>\n");
	  fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	  for (j=2; j<=nelmt; j++) {
	    ++objectNumber;
	    if (objectNumber > 10) {
	      objectNumber = 1;
	      fprintf(fp,"</TR>\n");
	      fprintf(fp,"<TR>\n");
	    }

	    strcpy(tmpStr1, tmpchr[j-1]);
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	  }

	  fprintf(fp,"</TR>\n");

	  break;

	case DFNT_UINT8:            /* unsigned char */
   
	  /* reset the bscale */
	  status = 0;
	  FCTSCL(unit,i, 1.0, 0 ,&status);
	  CHECKFITSERROR("FCTSCL",status,errtxt);
	  status = 0;
	  FCGCVB(unit,colnum, frow, felem, nelmt,1,(unsigned char *)databuf.cp,&anyflg,&status);
	  CHECKFITSERROR("FCGCVB",status,errtxt);

	  if ((tdataType[colnum-1] == DFNT_UINT8) && (!anyflg)) {

	    objectNumber = 1;
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], (uint8)*databuf.cp));

	    fprintf(fp,"<TR>\n");
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    for (j=2; j<=nelmt; j++) {
	      ++objectNumber;
	      if (objectNumber > 10) {
		objectNumber = 1;
		fprintf(fp,"</TR>\n");
		fprintf(fp,"<TR>\n");
	      }
	      strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						 (uint8)databuf.cp[j-1]));

	      fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    }

	    fprintf(fp,"</TR>\n");

	  }
	  else { /* realData = fits DATA * bscale + bzero  */

	    objectNumber = 1;
	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1],\
			   (float)((uint8)(*databuf.cp)*tscale[colnum-1]+tzero[colnum-1])));

	    fprintf(fp,"<TR>\n");
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    for (j=2; j<=nelmt; j++) {
	      ++objectNumber;
	      if (objectNumber > 10) {
		objectNumber = 1;
		fprintf(fp,"</TR>\n");
		fprintf(fp,"<TR>\n");
	      }

	      strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
			  (float)((uint8)(databuf.cp[j-1])*tscale[colnum-1]+tzero[colnum-1])));
	     
	      fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    }

	    fprintf(fp,"</TR>\n");

	  }
	  break;
	case DFNT_INT32:            /* integer */
   
	  /* reset the bscale */
	  status = 0;
	  FCTSCL(unit,i, 1.0, 0 ,&status);
	  CHECKFITSERROR("FCTSCL",status,errtxt);
	  status = 0;
	  FCGCVJ(unit,colnum, frow, felem, nelmt,-1,databuf.ip,&anyflg,&status);
	  CHECKFITSERROR("FCGCVJ",status,errtxt);

	  if ((tdataType[colnum-1] == DFNT_INT32) && (!anyflg)) {

	    objectNumber = 1;
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], (int)*databuf.ip));

	    fprintf(fp,"<TR>\n");
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    for (j=2; j<=nelmt; j++) {
	      ++objectNumber;
	      if (objectNumber > 10) {
		objectNumber = 1;
		fprintf(fp,"</TR>\n");
		fprintf(fp,"<TR>\n");
	      }
	      strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						 (int)databuf.ip[j-1]));

	      fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    }

	    fprintf(fp,"</TR>\n");

	  }
	  else { /* realData = fits DATA * bscale + bzero  */

	    objectNumber = 1;
	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1],\
			   (float)((*databuf.ip)*tscale[colnum-1]+tzero[colnum-1])));

	    fprintf(fp,"<TR>\n");
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    for (j=2; j<=nelmt; j++) {
	      ++objectNumber;
	      if (objectNumber > 10) {
		objectNumber = 1;
		fprintf(fp,"</TR>\n");
		fprintf(fp,"<TR>\n");
	      }

	      strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
			  (float)((databuf.ip[j-1])*tscale[colnum-1]+tzero[colnum-1])));
	     
	      fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    }

	    fprintf(fp,"</TR>\n");

	  }
	  break;

	case DFNT_FLOAT32:           /* float */
	  status = 0;
	  FCGCVE(unit,colnum,frow,felem,nelmt,-1.0,databuf.fp, &anyflg, &status);
	  CHECKFITSERROR("FCGCVE",status,errtxt);

	  /* begin to generate HTML for ore datas  */
	  objectNumber = 1;
	  if (!anyflg) {
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], (float)*databuf.fp));

	    fprintf(fp,"<TR>\n");
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);
	  
	    for (j=2; j<=nelmt; j++) {
	      ++objectNumber;
	      if (objectNumber > 10) {
		objectNumber = 1;
		fprintf(fp,"</TR>\n");
		fprintf(fp,"<TR>\n");
	      }
	      strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
						(float)databuf.fp[j-1]));
	      fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);
	    }
	  }
	  break;

	case DFNT_FLOAT64:                /* double */
	  status = 0;
	  FCGCVD(unit,colnum,frow,felem,nelmt,-1.0,databuf.dp,&anyflg,&status);
	  CHECKFITSERROR("FCGCVD",status,errtxt);

	  /* begin to generate HTML for ore datas  */
	  objectNumber = 1;
	  if (!anyflg) {
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], *databuf.dp));
	    fprintf(fp,"<TR>\n");
	    fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);

	    for (j=2; j<=nelmt; j++) {
	      ++objectNumber;
	      if (objectNumber > 10) {
		objectNumber = 1;
		fprintf(fp,"</TR>\n");
		fprintf(fp,"<TR>\n");
	      }
	      strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],databuf.dp[j-1]));
	    
	      fprintf(fp,"<TD ALIGN=right>%s</TD>\n",tmpStr1);
	      
	    }
	  }
	  break;

	default:
	  /* 
	  FatalError("Complex field number type");
	  return FAIL; */
	  break;
	}

	fprintf(fp, "</TABLE><P>\n");
      } /* if (col > 3) */
    } /* for (i=2; i<tfields; i++)  */
    } /*for (kk=1;....) */


    /* free memory  */
    FITSfree(tmpStr);
    FITSfree(retString);

    return SUCCEED;
}

/*------------------------------------------------------------------------ 
 NAME
       readFits
 DESCRIPTION
       Print out the info for the FITS file 
 RETURNS
-------------------------------------------------------------------------*/
int
#ifdef __STDC__
readFits(char *fileName,lvar_st *l_vars)
#else
readFits(fileName, l_vars)
char *fileName;    /* file name */
lvar_st *l_vars;
#endif
{
    int32 count, realCount;   /* counter for images ? */
    int32 i,j;       /* loop variable */
    int32 ref;     /* reference number for image */
    int32 len;     
    int   status;  /* flag */
    int   width;       /* width dimension of image */
    int   height;       /* height dimesnion of image */
    int   rank;
    int   plane;

    /* table variables */
    int   fldNum, recNum;

    int   ip = 0;

    char  tmp_html[1024];
    char  *img_name = NULL;
    FILE  *h_fp;
    int   unit, rwstat;

    ENTER(2,"do_rigs");

    /* open fits file */
    rwstat = FITSREADONLY;
    
    if ((unit = openFits(fileName,rwstat)) == FAIL)
        return FAIL;

    /* get number of raster images in file */
    if ((count = fitsnimages(unit)) < 1)
        return SUCCEED; 
    realCount = count;
    status = fitsInfo(unit,1,&rank,&width,&height,&plane);
    if (!(width*height))
        realCount = count -1;
    
    DBUG_PRINT(2,(LOGF,"count=%d\n", count));

    if (l_vars->do_dump)  {
     
        /* Create name for HTML file */
        if (l_vars->html_dir == NULL)
            sprintf(tmp_html,"%s_fits.html",fileName);
        else
            sprintf(tmp_html,"%s/%s_fits.html",l_vars->html_dir,l_vars->f_name);
            
        DBUG_PRINT(1,(LOGF," fits html file name %s \n", tmp_html));
          
        /* Open temproary file to write HTML description of FITS file */
        if (!(h_fp = fopen(tmp_html, "w")))
            return FAIL;

        /* Write MIME header */
        if (write_html_header(h_fp, TEXT_HTML,l_vars) == FAIL) {
  
            gateway_err(h_fp,"readfits: writing HTML headr",0,l_vars);
            return FAIL;
        }

        if(l_vars->hdf_path_r != NULL)
            fprintf(h_fp,"These images came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                    l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
        else
            fprintf(h_fp,"These images came from <A HREF=\"%s%s?%s\"> %s </A><p>",
                    l_vars->cgi_path,l_vars->f_path_r,l_vars->f_name,l_vars->f_name);
    }
    else
        h_fp = l_vars->hfp;

    if (realCount >= 1) {
        fprintf(h_fp, "<HR>\n");

        /* Print Image header stuff in HTML  */
    
        fprintf(h_fp, "<H2>Images</H2>\n");    

        if(realCount == 1)
            fprintf(h_fp, "There is 1 image in this file :\n");
        else
            fprintf(h_fp, "There are %d images in this file :\n", realCount);
        fprintf(h_fp, "<UL>\n");


        /* For each image in the file */
        for(i = 0; i < count; i++) {
	
            ref = i+1;
	
            /* Get width and height dimensions of image */
            if ((status = fitsInfo(unit,i+1,&rank,&width,&height,&plane))==FAIL) {
         
                gateway_err(h_fp,"Fits file ? ",0,l_vars);
                return FAIL;
            }
	
            if (!(width*height))
                continue;

            /* Print out image info in HTML depending on user prefrences */
            if (l_vars->do_dump) {
          
                fprintf(h_fp, "<LI> This : <A HREF=\"%s%s?%s!sdbref;ref=%d,s=%d,plane=%d\"> image </A> has dimensions %d by %d\n", 
                        l_vars->cgi_path,l_vars->hdf_path_r,l_vars->f_name, ref, 1,l_vars->plane, width, height);

                if(ip) 
                    fprintf(h_fp, " and also has a palette.<p>\n");
                else
                    fprintf(h_fp,".<p>\n");
    
                if (rank>2)  { /* dimensions > 2)  */
	
                    if  (l_vars->plane<plane)
                        ++l_vars->plane;
                    fprintf(l_vars->hfp,"<h3>To see the next plane of the image, click <A HREF=\"%s%s?%s!sdbplane;plane=%d\"> here </A>", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,l_vars->plane);


                    fprintf(l_vars->hfp,"<h3>To preview all the planes winthin the image, click <A HREF=\"%s%s?%s!sdbview;ref=%d,plane=%d\"> here </A>", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,ref,plane);


                }

            }
            else { /* not dumping */

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
                    fprintf(l_vars->hfp,"<h3>To see the next plane of the image, click <A HREF=\"%s%s?%s!sdbplane;plane=%d\"> here </A> </h3>\n", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,l_vars->plane);



                    fprintf(l_vars->hfp,"<h3>To preview all the planes winthin the image, click <A HREF=\"%s%s?%s!sdbview;ref=%d,start=%d,end=%d\"> here </h3> </A> \n", l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,ref,1,plane);


                    fprintf(l_vars->hfp,"<h3>To preview the group of the planes winthin the image, please enter:</h3> \n");
		
                    fprintf(l_vars->hfp,"<FORM METHOD=\"POST\" \n");
                    fprintf(l_vars->hfp,"ACTION=\"%s%s\">\n",
                            l_vars->h_env->script_name,
                            l_vars->h_env->path_info);

                    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",l_vars->f_name);
                    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"SDB_REF\" VALUE=\"%d\">\n",ref);
                    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"SDB_PLANE\"  VALUE=\"%d\">\n",plane);
                    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"SDB_IMGFLAG\"  VALUE=\"%d\">\n",SDBVIEW);

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

            } /* end else not dumping */

	
        } /* for loop for each image */
    
        /* end of image */
        fprintf(l_vars->hfp, "</UL>\n");
  
    } /* if (count > 1)  */

    /* what about table */

    /* get number of table number in file */
    if ((count = fitsntables(unit)) >= 1) {
  
        fprintf(h_fp, "<HR>\n");

        /* Print Table header stuff in HTML  */
    
        fprintf(h_fp, "<H2>Ascii Tables</H2>\n");

        if(count == 1)
            fprintf(h_fp, "There is 1 ascii table in this file :\n");
        else
            fprintf(h_fp, "There are %d ascii table in this file :\n", count);
        fprintf(h_fp, "<UL>\n");

        /* For each table in the file */
        for(i = 0; i < count; i++) {
	
            ref = i+1;
	
            /* Get width and height dimensions of image */
            if ((status = fitsTableInfo(unit,i+1,&fldNum,&recNum))==FAIL) {
         
                gateway_err(h_fp,"Fits file ? ",0,l_vars);
                return FAIL;
            }

            /* print table info in HTML format */
            /* fprintf(h_fp,"<LI> This table has <b>%d</b> columns and  <b>%d</b> records, click <A HREF=\"%s%s?%s!fitstab;ref=%d\"> here </A> to see the contents of the table \n ",
                    fldNum, recNum,
                    l_vars->h_env->script_name,l_vars->h_env->path_info,
                    l_vars->f_name, ref);
		    */
            fprintf(h_fp,"<LI> This table has <b>%d</b> columns and  <b>%d</b> records.",
                    fldNum, recNum);
        

	    /* processing of the FITS Table subsetting  */
	    
	    fprintf(h_fp, "<FORM METHOD=\"POST\" ");
	    fprintf(h_fp, "ACTION=\"%s%s\">\n",
                            l_vars->h_env->script_name,
                            l_vars->h_env->path_info);
	    fprintf(h_fp, "Select the fields from the FITS Ascii Table <p>\n");

	    /* print out field list to select */          
	    for (j = 1; j<= fldNum;j++) {
              char field_name[25];
	      strcpy(field_name, fitsTabFldName(unit,j));	      
	      fprintf(h_fp, "<INPUT TYPE=\"checkbox\" NAME=\"FT_FIELD\" VALUE=\"%d\" CHECKED>%s \n",  j,field_name);		     
	    }
       
	    fprintf(h_fp, "<P>\n");

	    /* subsetting records */
	    fprintf(h_fp, "Select the record number: <P>\n");
	    fprintf(h_fp, "<UL>");
	    fprintf(h_fp, "<LI>starting record :");
	    fprintf(h_fp, "<INPUT NAME=\"FT_START\" VALUE=%d> <P>\n", 1); 
	    fprintf(h_fp, "<LI>ending record:");
	    fprintf(h_fp, "<INPUT NAME=\"FT_END\"   VALUE=%d> <P>\n", recNum); 
	    fprintf(h_fp, "</UL>");
	    fprintf(h_fp, "<P>");

	    /*  hidden information  */
	    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",l_vars->f_name);
	    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"FT_REF\" VALUE=\"%d\">\n",ref);
	    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"FT_TABFLAG\"  VALUE=\"%d\">\n",FITSTAB);
	  
	    fprintf(h_fp, "To display the selected fits table, press the button: ");
	    fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Fits Table\">. <P>\n");
	    fprintf(h_fp, "</FORM>");

        }
        /* end of table reading */
        fprintf(l_vars->hfp, "</UL>\n");
    }  /* ascii table */

    /* turn to binary table */
	  
    /* get number of binary tables in file */
    if ((count = fitsnbintables(unit)) >= 1) {
      
        fprintf(h_fp, "<HR>\n");

        /* Print b`Binary Table header stuff in HTML  */
    
        fprintf(h_fp, "<H2>Binary Tables</H2>\n");

        if(count == 1)
            fprintf(h_fp, "There is 1 binary table in this file :\n");
        else
            fprintf(h_fp, "There are %d binary table in this file :\n", count);
        fprintf(h_fp, "<UL>\n");

        /* For each binary table in the file */
        for(i = 0; i < count; i++) {
	
            ref = i+1;
	
            /* Get info. of table (field, record number) */
            if ((status = fitsBinTabInfo(unit,i+1,&fldNum,&recNum))==FAIL) {
         
                gateway_err(h_fp,"Fits file ? ",0,l_vars);
                return FAIL;
            }

            /* print table info in HTML format */
            /* fprintf(h_fp,"<LI> This binary table has <b>%d</b> columns and  <b>%d</b> records, click <A HREF=\"%s%s?%s!fitsbintab;ref=%d\"> here </A> to see the contents of the table \n ",
                    fldNum, recNum,
                    l_vars->h_env->script_name,l_vars->h_env->path_info,
                    l_vars->f_name, ref);
		    */
            fprintf(h_fp,"<LI> This binary table has <b>%d</b> columns and  <b>%d</b> records. ",
                    fldNum, recNum);
 
	    /* processing of the FITS Binary Table subsetting  */	    
	    fprintf(h_fp, "<FORM METHOD=\"POST\" ");
	    fprintf(h_fp, "ACTION=\"%s%s\">\n",
                            l_vars->h_env->script_name,
                            l_vars->h_env->path_info);
	    fprintf(h_fp, "Select the fields from the FITS Binary Table <p>\n");
	    /* print out field list to select */          
	    for (j = 1; j <= fldNum;j++) {
              char field_name[25];
	      strcpy(field_name, fitsBinTabFldName(unit,j));	      
	      fprintf(h_fp, "<INPUT TYPE=\"checkbox\" NAME=\"FT_FIELD\" VALUE=\"%d\" CHECKED>%s \n",  j,field_name);		     
	    }
       
	    fprintf(h_fp, "<P>\n");

	    /* subsetting records */
	    fprintf(h_fp, "Select the record number: <P>\n");
	    fprintf(h_fp, "<UL>");
	    fprintf(h_fp, "<LI>starting record :");
	    fprintf(h_fp, "<INPUT NAME=\"FT_START\" VALUE=%d> <P>\n", 1); 
	    fprintf(h_fp, "<LI>ending record:");
	    fprintf(h_fp, "<INPUT NAME=\"FT_END\"   VALUE=%d> <P>\n", recNum); 
	    fprintf(h_fp, "</UL>");
	    fprintf(h_fp, "<P>");

	    /*  hidden information  */
	    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"f_name\" VALUE=\"%s\">\n",l_vars->f_name);
	    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"FT_REF\" VALUE=\"%d\">\n",ref);
	    fprintf(l_vars->hfp, "<INPUT TYPE=\"hidden\" NAME=\"FT_TABFLAG\"  VALUE=\"%d\">\n",FITSBINTAB);
	  
	    fprintf(h_fp, "To display the selected fits table, press the button: ");
	    fprintf(h_fp, "<INPUT TYPE=\"submit\" VALUE=\"Fits Bintable\">. <P>\n");
	    fprintf(h_fp, "</FORM>");

        }
        /* end of binary table reading */
        fprintf(l_vars->hfp, "</UL>\n");
    } /* end of binary table */

    if (unit) {
        /* close the fits file & free the unit number */
        FCCLOS(unit, &status);
        FCFIOU(unit, &status);
    }

    if (l_vars->do_dump)
        fclose(h_fp);
    EXIT(2,"do_rigs");
} 


/*------------------------------------------------------------------------ 
 NAME
     fitsnobject -  get the whole objects number in the FITS file         
 DESCRIPTION
     get the whole objects number in the FITS file     
 RETURN
     the objects number
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
fitsnobject(int unit)
#else
fitsnobject(unit)
int unit;  /* fits i/o unit number */
#endif
{
    /* variables related to fits file  */
    int numberObject = 0;
    int status = 0;
    int hdutype; 

    char errtxt[FITS_CLEN_ERRMSG];

    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("fitsnobject",status,errtxt);
    if (hdutype != 0)  /* primary array or Image  */
        FatalError("FITS file structure is corrupt!");

    while (status == 0) {
        ++numberObject;
        FCMRHD(unit,1,&hdutype,&status);

    }
    
    return(numberObject);
}

/*------------------------------------------------------------------------ 
 NAME
     getDataType  -  get the data number type description          
 DESCRIPTION
     get the data number type description based on the fits keyword "BITPIX"  
 RETURN
     the description string
-------------------------------------------------------------------------*/ 
char *
#ifdef __STDC__
getDatatype(int bitPix)
#else
getDatatype(bitPix)
int  bitPix
#endif
{  
    switch(bitPix) {
    case 8:  /* char */
        sprintf(retStr, " %d-bits unsigned char\n", bitPix);
        break;
    case 16: /* short */
        sprintf(retStr, " %d-bits short integer\n", bitPix);
        break;
    case 32: /* int */
        sprintf(retStr, " %d-bits integer\n", bitPix);
        break;
    case -32: /* float */
        strcpy(retStr, " real floating point\n");
        break;

    case -64: /* double */
        strcpy(retStr, " double precision floating point\n");
        break;

    default:
        sprintf(retStr, " \n");
        break;
    }
  
    return retStr;
}

/*------------------------------------------------------------------------ 
 NAME
     getDataRange -  get the data range of the current HDU        
 DESCRIPTION
     retrieve the maximum & minimum of the fits data
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int  
#ifdef __STDC__
getDatRange(int unit, int *datFlag, float *datMin, float *datMax)
#else
getDatRange(unit, datFlag, datMin, datMax)
int   unit;
int   *datFlag;
float *datMin, *datMax;
#endif

{
    int  status = 0;
    char comment[80];

    FCGKYE(unit, "DATAMIN", datMin, comment, &status);
    if (!status) {
        FCGKYE(unit, "DATAMAX", datMax, comment, &status);
        if (!status)
            *datFlag = 1;
        else
            *datFlag = 0;
    }
    else
        *datFlag = 0;
    return SUCCEED;

}
/*------------------------------------------------------------------------ 
 NAME
     readFitsKey -  read the fits keyword       
 DESCRIPTION
     read the fits keyword
 RETURN
     the keyword value
-------------------------------------------------------------------------*/ 
char * 
#ifdef __STDC__
readFitsKey(int unit, char *keyword)
#else
readFitsKey(unit,keyword)
int  unit;
char *keyword;
#endif

{
    int  status = 0;
    char comment[80];
    char keyVal[500];

    int  i;
 
    FCGKEY(unit, keyword, keyVal, comment, &status);
    /*
      printf("Content-type: text/html%c%c",10,10);
      printf("%s=%d\n",keyword,status);
      */
    if (status!=0)
        strcpy(retStr,"");
    else {
    
        if ((!(strcmp((char *)substr(keyVal,1,1),"'")))&& \
            (!strcmp((char *)substr(keyVal,strlen(keyVal),1),"'"))) {
            strcpy(retStr, (char *)substr(keyVal,2,strlen(keyVal)-2));

        }
	else
	  strcpy(retStr, keyVal);
    }

    return retStr;
}

/*------------------------------------------------------------------------ 
 NAME
     readCoordinate -  get the coordinate description of the fits file       
 DESCRIPTION
     get the coordinate description of the fits file
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 

int
#ifdef __STDC__
readCoordinate(int unit, FILE *fp)
#else
readCoordinate(unit, fp)
int  unit;
FILE *fp;
#endif
{
    
    /* some of fits variable */
    int simple,extend,anyflg;
    int bitpix,naxis,naxes[15],pcount,gcount;
    
    int objNumber=1;
    int status = 0;
    int hdutype;
    int i, count;
    char tmpStr[100];
    char comment[79];

    float crpix[15], crval[15], cdelt[15], floatVal;
    int   flag[15];

    char  ctype[15][30];

    float hours, minutes, seconds;
    char sign, tmpc[30], units[10], crvalStr[50];

    
    /* read the required primary array keywords  */
    status = 0;
    FCGHPR(unit, 99, &simple, &bitpix, &naxis, naxes, &pcount, &gcount, \
           &extend, &status);
    CHECKFITSERROR("FCGHPR", status, errtxt);

    count = naxis;
    if (naxis > 15)
        count = 15;
    
    for (i=0; i<count; i++) {


        flag[i] = 1;
	crpix[i] = 0;
	crval[i] = 0;
	cdelt[i] = 0;
        status = 0;
        sprintf(tmpStr, "CTYPE%d",i+1);
        FCGKYS(unit, tmpStr, ctype[i], comment, &status);
       
        status = 0;
        sprintf(tmpStr, "CRPIX%d",i+1);
        FCGKYE(unit, tmpStr, &crpix[i], comment, &status); 
        if (status)
            flag[i] = 0;

        status = 0;
        sprintf(tmpStr, "CRVAL%d",i+1);

        FCGKYE(unit, tmpStr, &crval[i], comment, &status);

        if (status)
            flag[i] = 0;

        status = 0;
        sprintf(tmpStr, "CDELT%d",i+1);
        FCGKYE(unit, tmpStr, &cdelt[i], comment, &status);
        if (status)
            flag[i] = 0;

    }

    fprintf(fp, "<XMP>\n");
    fprintf(fp, "The Coordindate System information looks like: \n");

    fprintf(fp, "-----------------------------------------------------------------\n");

    fprintf(fp, "Coor. Name Pixels  Coor. Value at Ref. Point by  Increment  Units\n");
 
    for ( i=0; i<count; i++) {
        if (flag[i]) { /* valid */
            if (!strncmp(ctype[i],"RA",2)){  /* RA--- */
	  
                hours   = crval[i]/15;
                minutes = 60 * (hours - floor(hours));
                seconds= 60 * (minutes- floor(minutes));
                sprintf(crvalStr, "%d:%d:%.2f",(int)hours,(int)minutes,seconds); 
                strcpy(units, "arcsec");
            }
            else {
                if (!strncmp(ctype[i],"DEC",3)){  /* DEC--- */
                    strcpy(units, "arcsec");
                    if (crval[i]<0) {
                        sign = '-';
                        crval[i] = -crval[i];
                    }
                    else
                        sign = '+';

                    hours = crval[i];
                    minutes = 60 * (hours - (int)hours);
                    seconds= 60 * (minutes-(int)minutes);
                    if (sign == '-')
                        sprintf(crvalStr, "%c%d:%d:%.2f",sign,(int)hours,(int)minutes,seconds);
                    else	      
                        sprintf(crvalStr, "%d:%d:%.2f",(int)hours,(int)minutes,seconds);
                }
                else {
                    if (!strncmp(ctype[i],"FRE",3)){  /*FRE */
	      
                        strcpy(units, "Hz");	    
                        sprintf(crvalStr, "%.5E",crval[i]);
                    }
                    else {
                        if (!strncmp(ctype[i],"VEL",3)){  /*VEL */
		
                            strcpy(units, "Km/s");	    
                            sprintf(crvalStr, "%.5E",crval[i]);
                        }
                        else {
                            strcpy(units, "");
                            strcpy(crvalStr,"");
                        }
                    }
                }
            }
     
            /* Name */
            strcpy(tmpStr, (char *)trim(ctype[i]));
            strcat(tmpStr, (char *)space(10-strlen(tmpStr)));

            /* Pixels */
            sprintf(tmpc, "%d",naxes[i]);
            strcat(tmpStr, (char *)space(7-strlen(tmpc)));
            strcat(tmpStr, tmpc);

            /* coor. Value */
            strcat(tmpStr, (char *)space(13-strlen(crvalStr)));
            strcat(tmpStr, crvalStr);

            /* ref. point */
            sprintf(tmpc, "%.2f",crpix[i]);
            strcat(tmpStr, (char *)space(14-strlen(tmpc)));
            strcat(tmpStr, tmpc);
      
            /* cdelt */
            sprintf(tmpc, "%.5e",cdelt[i]*3600);
            strcat(tmpStr, (char *)space(14-strlen(tmpc)));
            strcat(tmpStr, tmpc);
    
            /* units */
            strcat(tmpStr, (char *)space(8-strlen(units)));
            strcat(tmpStr, units);
 
            fprintf(fp, "%s\n", tmpStr);

        } /* if (flag[i])  */

    } /* for ... */
      
    fprintf(fp, "-----------------------------------------------------------------\n");

    fprintf(fp, "</XMP>\n");

    return SUCCEED;
}

/*------------------------------------------------------------------------ 
 NAME
     getFitsDesc -  get the summary information of the FITS file          
 DESCRIPTION
      get the summary information of the FITS file(HTML format)        
 RETURN
     SUCCEED/FAIL
-------------------------------------------------------------------------*/ 
int
#ifdef __STDC__
getFitsDesc(int unit, FILE *fp)
#else
getFitsDesc(unit, fp)
int  unit;
FILE *fp;
#endif
{
    
    /* some of fits variable */
    int simple,extend,anyflg;
    int bitpix,naxis,naxes[99],pcount,gcount;
    
    int objNumber=1;
    int status = 0;
    int hdutype;
    int i;
        
    /* variables that get the ASCII table header keywords from the CHU  */
    int rowlen, nrows, tfields, tbcol[NUM_FIELDS]; 
    static char  ttype[NUM_FIELDS][FITS_CLEN_HDEFKWDS],  \
        tform[NUM_FIELDS][FITS_CLEN_HDEFKWDS], \
        tunit[NUM_FIELDS][FITS_CLEN_HDEFKWDS], extname[NUM_FIELDS];
    
    /* variables that get elements from an ASCII or binary table (FTGCV.) */
    int colnum, frow, felem, nelmt;
    int order, varidat, FitsStrBufLen;
    
    char tmpStr[80], tmpStr1[10];

    char errtxt[FITS_CLEN_ERRMSG];

    objNumber = fitsnobject(unit);
  
    if (objNumber > 1)
        fprintf(fp, "<B>There are %d objects in this FITS file</B>\n", objNumber);
    else
        fprintf(fp, "<b>There is one object in this FITS file</b>\n");
    
    fprintf(fp, "<OL>\n");

    objNumber=0;
    
    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    status= 0;
    FCMAHD(unit,1,&hdutype,&status);
    CHECKFITSERROR("FCMAHD",status,errtxt);

    while (status==0) {
        ++objNumber;
        switch (hdutype) {
        case 0: { /* Image */
            int     datFlag = 0;
            float   datMin, datMax; 

            if (objNumber == 1)
                fprintf(fp, "<LI>Primary Array\n");
            else
                fprintf(fp, "<LI>Image Extension\n");
	
            strcpy(tmpStr,readFitsKey(unit, "DATE"));
            if (strlen(tmpStr))
                fprintf(fp, "<DD><I>This Header and Data Unit(HDU) was created on <b>%s</B></I>\n", tmpStr); 

            fprintf(fp,"<UL>\n");

            /* read the required primary array keywords  */
            status = 0;
            FCGHPR(unit, 99, &simple, &bitpix, &naxis, naxes, &pcount, &gcount, \
                   &extend, &status);
            CHECKFITSERROR("FCGHPR", status, errtxt);

            strcpy(tmpStr,readFitsKey(unit, "EXTNAME"));
            fprintf(fp,"<LI>Image Name: %s\n",tmpStr );

            fprintf(fp,"<LI>Number of Axis: %d\n", naxis);
            if (naxis >0 ) {
                sprintf(tmpStr, "%d",naxes[0]);
 
                for (i=1; i<naxis; i++) {
                    sprintf(tmpStr1, "x%d", naxes[i]);
                    strcat(tmpStr, tmpStr1);
                }
                fprintf(fp,"<LI>Dimension: %s\n", tmpStr);
            }

            strcpy(tmpStr, (char *)getDatatype(bitpix)); 
            fprintf(fp,"<LI>Data Type: %s\n", tmpStr);

            getDatRange(unit, &datFlag, &datMin, &datMax);
            if (datFlag)
                fprintf(fp,"<LI>Data Ranges: [%.6e, %.6e]\n", datMin, datMax);
	  
            break;
        }
        case 1: { /* ASCII  Table */
	
            fprintf(fp, "<LI>ASCII  Table\n");
	
            strcpy(tmpStr,readFitsKey(unit, "DATE"));
            if (strlen(tmpStr))
                fprintf(fp, "<DD><I>This Header and Data Unit(HDU) was created on <b>%s</B></I>\n", tmpStr); 

            fprintf(fp,"<UL>\n");

            /* read the required ASCII TABLE keywords  */
            status = 0;
            FCGHTB(unit, NUM_FIELDS, &rowlen, &nrows, &tfields, ttype, tbcol, \
                   tform, tunit, extname, &status);

            CHECKFITSERROR("FCGHTB", status, errtxt);

            fprintf(fp,"<LI>Table Name: %s\n", extname );

            fprintf(fp,"<LI>Number of Fields: %d\n", tfields);

            fprintf(fp,"<LI>Number of Records: %d\n", nrows);
	
            break;
        }
        case 2: /* Binary Table */

            fprintf(fp, "<LI>Binary  Table\n");

            strcpy(tmpStr,readFitsKey(unit, "DATE"));
            if (strlen(tmpStr))
                fprintf(fp, "<DD><I>This Header and Data Unit(HDU) was created on <b>%s</B></I>\n", tmpStr); 

            fprintf(fp,"<UL>\n");

            /* read the required Binary TABLE keywords  */
            status = 0;
            FCGHBN(unit, NUM_FIELDS, &nrows, &tfields, ttype, \
                   tform, tunit, extname,&varidat, &status);

            CHECKFITSERROR("FCGHBN", status, errtxt);

            fprintf(fp,"<LI>Table Name: %s\n", extname );

            fprintf(fp,"<LI>Number of Fields: %d\n", tfields);

            fprintf(fp,"<LI>Number of Records: %d\n", nrows);

	
            break;
	
        default:
            continue;
        }

        strcpy(tmpStr, (char *)readFitsKey(unit, "OBJECT"));      
        fprintf(fp,"<LI>Object Name: %s\n", tmpStr);

        strcpy(tmpStr, (char *)readFitsKey(unit, "TELESCOP"));	
        fprintf(fp,"<LI>Telescope: %s\n", tmpStr);

        strcpy(tmpStr, (char *)readFitsKey(unit, "AUTHOR"));	
        fprintf(fp,"<LI>Author: %s\n", tmpStr);

        strcpy(tmpStr, (char *)readFitsKey(unit, "EQUINOX"));
        fprintf(fp,"<LI>Equinox: %s\n", tmpStr);

        strcpy(tmpStr, (char *)readFitsKey(unit, "REFERENC"));	
        fprintf(fp,"<LI>Reference: %s\n", tmpStr);
   
        fprintf(fp,"</UL>\n");

        strcpy(tmpStr, (char *)readFitsKey(unit, "CRPIX1"));
        if (strlen(tmpStr)) {
            /* get information about the coordinate ststem */
            if (!hdutype) /* not table */
                readCoordinate(unit,fp);
        }
        /* Moving to next extension in the FITS file */
        status= 0;
        FCMRHD(unit,1,&hdutype,&status);

    } /* while (status == 0) */

    fprintf(fp,"</OL>\n");

    return SUCCEED;
}
#endif /* FITSUTILS_C */
#endif /* HAVE_FITS */
