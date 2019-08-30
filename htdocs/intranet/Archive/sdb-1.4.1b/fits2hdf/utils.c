/* utils.c
 * Assorted utilities for the file conversion package.  
 *
 * 
 */

#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define LF 10
#define CR 13
#define FILESIZE 255


/* Package include files */
#include "fits2sds.h"
#include "cfitsio.h"

/* HDF include files */
#include "hdf.h"
#include "herr.h"

/* Make a new temporary file name.  Result only valid until the routine is
 * called again (statat(ic space).
 */

struct class classTab[] = {
    "SIMPLE",   SIMPLE,
    "BITPIX",   BITPIX,
    "NAXIS",    NAXIS,
    "BSCALE",   BSCALE,
    "BZERO",    BZERO, 
    "DATAMAX",  DATAMAX,
    "DATAMIN",  DATAMIN,
    "BUNIT",    BUNIT,
    "BLANK",    BLANK,
    "EXTNAME",  EXTNAME,
    "EXTVER",   EXTVER,
    "HISTORY",  HISTORY,
    "COMMENT",  COMMENT,

    "BSCALERR",BSCALERR,
    "BZEROERR",BZEROERR,
    "DATATYPE",DATATYPE,
    
    "END",      END
};

#define NCLASSES (sizeof classTab / sizeof(struct class) )

struct fits2sdsAttr fits2sdsAttrTab[]={

    "SIMPLE" , DFNT_UINT8,
    "EXTEND" , DFNT_UINT8,
    /*
    "BSCALE" , DFNT_FLOAT32,
    "BZERO"  , DFNT_FLOAT32,
    "DATAMAX", DFNT_FLOAT32,
    "DATAMIN", DFNT_FLOAT32,
    */
    "PCOUNT",  DFNT_INT32,
    "GCOUNT",  DFNT_INT32,

    "DATE"   , DFNT_UINT8,
    "TITLE"  , DFNT_UINT8,
    "ORIGIN" , DFNT_UINT8,
    "BLOCKED", DFNT_UINT8,
    "AUTHOR" , DFNT_UINT8,
    "REFERENC",DFNT_UINT8,
    "DATE-OBS",DFNT_UINT8,
    "TELESCOP",DFNT_UINT8,
    "INSTRUME",DFNT_UINT8,
    "OBSERVER",DFNT_UINT8,
    "OBJECT"  ,DFNT_UINT8,
    "EQUINOX" ,DFNT_FLOAT32,
    "EPOCH"   ,DFNT_FLOAT32,

    /* BUNIT can this keyword convert to sds' dataset unit? */
    /* "BUNIT"   ,DFNT_UINT8, */
    /* CTYPEn this keyword should be converted to sds' dimension unit  */
    /* "CTYPE"  ,DFNT_UINT8,     
    "CRPIX"  ,DFNT_FLOAT32,
    "CROTA"  ,DFNT_FLOAT32,
    "CRVAL"  ,DFNT_FLOAT32,
    "CDELT"  ,DFNT_FLOAT32,
    */
    /*   
    "COMMENT",DFNT_UINT8,
    "HISTORY",DFNT_UINT8,
    "EXTNAME" ,DFNT_UINT8,
    "EXTLEVEL",DFNT_INT32,
    */
    "XTENSION",DFNT_UINT8,
    "FILENAME",DFNT_UINT8,
    "EXTVER"  ,DFNT_INT32,
    
};

#define NFITS2SDSATTR (sizeof fits2sdsAttrTab / sizeof(struct fits2sdsAttr) )


static char t_file[FILESIZE];
static int file_suffix = 0;
static char tmpStr[4000];  /* assume maxium 50 rows comments */
char errtxt[FITS_CLEN_ERRMSG];


char *NewTempFile()
{
  
  sprintf(t_file, "tmp_%d.%d", getpid(), file_suffix++);
  return(t_file);
}

/* Print out the contents of a data array */

void printData(data, dataNum, dataType)
     char *data;
     int dataNum;
     int dataType;
{
    int i;   
    int bytesize;
    bytesize = (dataType < 0 ? -dataType: dataType)/8;
    
    for (i=0; i < dataNum; ){
	
	switch (dataType){
	case 8:      /* char */
	    printf(" %4o", *(char *)data);
	    break;
	case 16:    /* short integer */
	    printf(" %6d", *(int16 *)data);
	    break;
	case 32:    /* integer */
	    printf(" %6d", *(int32 *)data);
	    break;
	case -32:		/* float */
	    printf(" %7f", *(float32 *)data);
	    break;
	case -64:		/* double  */
	    printf(" %7f", *(float64 *)data);
	    break;
	}
	data += bytesize;
	if ( ++i % 8 == 0 )
	    printf("\n");
    }
    if ( i % 8 != 0)
	printf("\n");
}



/* Print an informational message */

void InfoMsg(str)
     char *str;
{
  printf("%s\n", str);
}



/* Does a given file exist? */

int FileExists(name)
     char *name;
{
  FILE *fp;

/* Check to see if the file already exists */

  fp = fopen(name, "r");
  if (fp == NULL) return(FALSE);
  return(TRUE);
}

void FileOverWriteMsg(buf, file)
     char *buf, *file;
{
  sprintf(buf, "%s\nalready exists.  Overwrite (y/n)? ", file);
}

/* Confirm overwrite */

int OverWrite(file)
  char *file;
{
    char ScratchBuf[80];
    char c = (char)NULL;

/* Check to see if the file already exists */

    if (!FileExists(file)) return(TRUE);

/* Simple dialog with user */

    FileOverWriteMsg(ScratchBuf, file);

    printf("%s", ScratchBuf);
  
    while (c != 'Y' &&c != 'y' &&c != 'n' &&c != 'N' && c != '\n')
	c = getchar();
    if (c == 'Y' || c == 'y') return(TRUE);
    return(FALSE);
}



/* trim left space of the string */

char *ltrim(str)
    char *str;
{
   
    int  i=0;
  
    while ((*str)== ' ')
	str++;
    while ((*str) != '\0') {
	
	tmpStr[i] = *str++;
	i++;
    }
    tmpStr[i] = '\0';
    return(tmpStr);
}
	

/* trim right space of the string */

char *trim(str)
    char *str;
{
   
    int  i=strlen(str);
  
    while ((str[i-1]== ' ') && (i>0))
	--i;

    tmpStr[i]='\0';
    --i;
    while (i>=0) {
	
	tmpStr[i] = str[i];
	--i;
    }

    return(tmpStr);
}

/* get substr from the string */

char *substr( str,start, stop)
    char *str;
    int  start, stop;
{
    int x ;
    int i = 0;
    strcpy(tmpStr,"");
    if (strlen(str) < start)
	{ tmpStr[0] = '\0';
	  return(tmpStr);
	}
    {
      for(x=start-1;(stop>i)  ;x++,i++){
	  if (strlen(str) < start+i)
	      break;
          tmpStr[i] = str[x];
      }
    }
    tmpStr[i] = '\0';
    return(tmpStr);
}


/* convert every character in the string to upper character  */

char* upper(str)
    char *str;
{
    int i=0;
    while ((tmpStr[i] = toupper(str[i])) != '\0')
      i++;
    return(tmpStr);
    
}

/* look at str1 to see if it is exist in str2   */

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



/*
 * newSpace
 *
 * Allocate a space with little wastage
 *
 */

#ifdef PROTOTYPE
char *newSpace(int32 size)
#else
char *newSpace(size)
    int32 size;
#endif /* PROTOTYPE */
{
    static int32 oldSize = 0;        /* must be static */
    static char *oldSpace = NULL; /* must be static */

    if (size >= oldSize) {
        if (oldSpace != NULL) HDfreespace(oldSpace);
        if ((oldSpace = (char *) HDgetspace((uint32) size)) == NULL) {
            puts("Out of memory. Abort.");
            exit(1);
        }
        oldSize = size;
    }

    return oldSpace;
}


char *num2str(num)
    int num;
{
    if (num < 0)
	num = -num;
    if (num <10)
	sprintf(tmpStr,"%1d",num);
    else 
	sprintf(tmpStr,"%2d",num);
    return(tmpStr);
}

/* print the fatal error massage */

void FatalError (infoMsg)
      char *infoMsg;
{
  fprintf(stderr, " %s\n", infoMsg);
  exit(-1); 
}


void Quit(i)

int i;
{ 
    exit(i);
}

/* find number type based on bitpix  */

int32 findNumberType(bitpix)

    int bitpix;
{
    void FatalError();
    int refBitpix;

    refBitpix = bitpix + 100;
    if (refBitpix < 0)
	refBitpix = -refBitpix;
	
    switch(refBitpix) {
    case 108:    /* char  (100+8)  */
	return (DFNT_UINT8);
    case 116:   /* short integer  */
	return (DFNT_INT16);
    case 132:  /* integer  */
	return (DFNT_INT32);
    case 68:  /* float  */
	return(DFNT_FLOAT32);
    case 36:  /* double  */
	return(DFNT_FLOAT64);
    default:
	InfoMsg("Worng bitpix value in FITS file ");
		
    }
}




/* check error after make a fits routine call */
void CHECKFITSERROR(routineName, status, errTxt) 
    char routineName[10];
    int status;
    char errTxt[FITS_CLEN_ERRMSG];
{
    if (status != 0){ 
        FCGERR(status,errTxt);
        printf ("%s status = %d : %s\n", routineName, status, errTxt);
	/*	Quit(-1); */
    }
}

/* open FITS file with dersied mode  */

int32 openFits(fileName, mode)
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

/* open & initialize a new empty FITS file  */

int32 createFits(fileName, mode)
    char *fileName;
    int mode;
{
    
    int iounit,status=0;
    int bksize,rwstat;

    rwstat = mode;

    /*  get an unused logical unit number */
    FCGIOU(&iounit, &status);
    CHECKFITSERROR("FCGIOU", status, errtxt);

     /* create  fits file */
    bksize = 2800;
    FCINIT(iounit, fileName, bksize, &status);
    CHECKFITSERROR("FCINIT", status, errtxt);

    /* return the result */
    if (status == 0) 
	return(iounit);
    else { 
	printf("Can't create file: %s\n", fileName);
	return(-1);
    }
}
   

/* write attribute to sds */
void writeSdsAttr(id, keyword, numberType, value)
    int32 id, numberType;
    char *keyword;
    char *value;
{
    int charLen ;

    if (strlen((char *)ltrim(keyword))) {  /* make sure keyword is not empty  */
    if (numberType == DFNT_UINT8) {

        strcpy(value,(char *)ltrim(value));
	if (strcmp(substr(value,strlen(value),1),"/")==0)
	  strcpy(value,substr(value,1,strlen(value)-1));
	strcpy(value, (char *)trim(value));

	if ((strcmp(substr(value,1,1),"'")==0) && (strcmp(substr(value,strlen(value),1),"'")==0))
	  strcpy(value,substr(value,2,strlen(value)-2));

	charLen = strlen(value);
	if (charLen)
	  SDsetattr(id, keyword, DFNT_CHAR8, charLen, value); 
    }
    else 
	switch(numberType) {
	case DFNT_INT16:
	{int16 attrVal16;
	attrVal16 = (int16) atoi(value);
	SDsetattr(id, keyword, numberType, 1, &attrVal16);
	}
	break;
	case DFNT_INT32:
	{int32 attrVal32;
	attrVal32 = (int32) atoi(value);
	SDsetattr(id, keyword, numberType, 1, &attrVal32);
	}
	break;	
	case DFNT_FLOAT32:
	{float32 attrFVal32;
	attrFVal32 = (float32) atof(value);
	SDsetattr(id, keyword, numberType, 1, &attrFVal32);
	}
	break;
	case DFNT_FLOAT64:
	{float64 attrFVal64;
	attrFVal64 = (float64) atof(value);
	SDsetattr(id, keyword, numberType, 1, &attrFVal64);
	}
	break;
	}
    } /*  if (strlen(ltrim(keyword)))   */
}


/* write attribute's comment  to sds */
void writeSdsAttrComm(id, keyword, numberType, value)
    int32 id, numberType;
    char *keyword;
    char *value;
{
    int charLen ,tmpLen;
    char commKey[20], commVal[60];
    
    tmpLen = at("/ ",value);
    if (tmpLen>=0) {
      tmpLen += 2;     
      strcpy(commVal,substr(value,tmpLen,50));


      strcpy(commKey,(char *)trim(keyword));
      strcat(commKey,"_COMMENT");
      strcpy(commVal, (char *)ltrim(commVal));
      strcpy(commVal, commVal);
      charLen = strlen(commVal);
      if (charLen) 
	SDsetattr(id, commKey, DFNT_CHAR8, charLen, commVal); 
    }
}


/* get next record's keyword */	
char *getKey(unit, keyno)
    int unit, keyno;
{
   
    char value[24];
    char comment[60];
    int  status = 0;
    char card[80];

    FCGREC(unit, keyno, card, &status);
    strcpy(tmpStr, substr(card,1,8));

    if (status == 0) 
	return(tmpStr);
    else
	return('\0');
}



/* get fits' keyword attribute in the sds */

int32 locFitsAttrKey(attrKeyword)
    char attrKeyword[8];
{
    int i,arrayNum;
    int tmpLen;
    arrayNum = NFITS2SDSATTR;
    for (i=0; i<arrayNum; i++) {
	tmpLen = strlen(fits2sdsAttrTab[i].fitsKeyword);
	if (strncmp(attrKeyword, fits2sdsAttrTab[i].fitsKeyword,tmpLen) == 0)
	    return (fits2sdsAttrTab[i].sdsNumberType);
    }
    return(-1);
}

/* return keyword index in the classTab  */

int32 locateKeywordIndex(priKeyword)
    char priKeyword[8];
{
    int i,arrayNum;
    int tmpLen;
    arrayNum = NCLASSES;
    for (i=0; i< arrayNum; i++) {
	tmpLen = strlen(classTab[i].word);
	if (strncmp((classTab[i].word), priKeyword,tmpLen) == 0)
	    return(classTab[i].value);
    }
    return(-1);
}


/* subroutine of SDsetrange */
void writeSDrange(sdsid, bitpix, max, min)
    int32 sdsid;
    int   bitpix;
    float32 *max,*min;
{
    int32 numtype;
    int16 max16,min16,dataint16[2];
    int32 max32,min32,dataint32[2];
    float64 max64,min64,datafloat64[2];
    float32 datafloat32[2];
    
    numtype = findNumberType(bitpix);
    switch(numtype) {
    case DFNT_INT16:
	max16 = (dataint16[1] = (int16)(*max));
	min16 = (dataint16[0] = (int16)(*min));
	/* SDsetattr(sdsid,"valid_range",numtype,2,&dataint16[0]); */
	SDsetrange(sdsid, &max16, &min16);
	break;

    case DFNT_INT32:
	dataint32[1] = (max32 =(int32)(*max));
	dataint32[0] = (min32 =(int32)(*min));
	/* SDsetrange(sdsid,&max32,&min32); */
	SDsetattr(sdsid,"valid_range",numtype,2,dataint32);
	break;

    case DFNT_FLOAT32:
	datafloat32[1] = *max;
	datafloat32[0] = *min;
	/* SDsetrange(sdsid,max,min); */
	SDsetattr(sdsid,"valid_range",numtype,2,datafloat32);
	break;
    case DFNT_FLOAT64:
	datafloat64[1] = (max64 = (float64)(*max));
	datafloat64[0] = (min64 = (float64)(*min));
	/* SDsetrange(sdsid,&max64,&min64); */
	SDsetattr(sdsid,"valid_range",numtype,2,datafloat64);
	break;
    }
}

/* subroutine of SDsetfillvalue */
void writeSDfillval(sdsid, bitpix, val)
    int32 sdsid;
    int   bitpix;
    float32 val;
{
    int32 numtype;
    int16 val16;
    int32 val32;
    float64 val64;
    
    
    numtype = findNumberType(bitpix);
    switch(numtype) {
    case DFNT_INT16:
	val16 = (int16)val;
	SDsetfillvalue(sdsid, &val16);

	break;

    case DFNT_INT32:
	val32 = (int32)val;
	SDsetfillvalue(sdsid, &val32);

	break;

    }
}


/* get the fits keyword's value type based on value (string) */
int32 getKeyType(strval)
    char *strval;
{
    int i;
    char tmpstr[80];

    strcpy(tmpstr,ltrim(strval));    
    if (strcmp(substr(tmpstr,strlen(tmpstr),1),"/")==0)
      strcpy(tmpstr,substr(tmpstr,1,strlen(tmpstr)-1));
    strcpy(tmpstr, trim(tmpstr));

    if (strlen(tmpstr) == 0)
      return(DFNT_UINT8);
   
    /* fits keyword is String  */
    if (!(strcmp(substr(tmpstr,1,1), "'")) && (!strcmp(substr(tmpstr,strlen(tmpstr),1),"'")))  
	return(DFNT_UINT8);

    /* double precise number --->>> change to float */
    if (((at("E+",tmpstr)>=0) || (at("E-",tmpstr)>=0)) && (at(".",tmpstr)>=0))
	return(DFNT_FLOAT32);

    /* float number */
    if (at(".",tmpstr)>=0)
	return(DFNT_FLOAT32);

    /* integer number */
    
    i = (int)atoi(substr(tmpstr,1,1));
    if (i)    /* integer */
	return(DFNT_INT32);
    else
	return(DFNT_INT16); /*  0  */
}


/* get BSCALE factor so as to convert float to int , I hope not to lose precision  */
int32 oldgetBscaleFactor(bscaleVal)
    float32 bscaleVal;
{
    int32 i=1;
    float32 val;
    val = ((bscaleVal < 0) ? (-bscaleVal):bscaleVal);

    if (bscaleVal == 0)
	return(1);

    while (1) { /* loop */
	if ((val > 1) || (i > 1000)) {
	    return(i);
}
	else {
	    val = val * 10;
	    i = i * 10;
	}
    }
}

	    

/* get BSCALE factor so as to convert float to int , I hope not to lose precision  */
int32 getBscaleFactor(bitpix,databuffer,datanum)
    int32 bitpix;
    float *databuffer;
    int   datanum;
{
    int32 j,i=1;
    float32 val;
    float32 max, min;

    min = max = *databuffer;

    /* find maxinum & minium  of data  */
    for (j=1;j<=datanum;j++) {
	val = *databuffer++;
	/* val = (val < 0) ? (-val) : val; */
	if (val>max)
	    max = val;
	if (val<min)
	    min = val;
    }
    
    /* range of the dataset  */
    val = max - min;

    if ( val > 100)  /* data distribution is enough for make a image  */
	return(1);

    /* get absolute value of minium  */
    min = (min < 0) ? (-min) : min;
    
    if (min==0)
	/* assume minium is 0.00001 */
	min = 0.00001;

    /* if min < 1  */
    while(1) {

	if (min<1) {
	    i = i * 10;
	    min = min * i;
	}
	else
	    break;
    }

    /* decide factor based on bitpix */
    max = max * i;
    if (bitpix == 32) {  /* 32 bit */
	while(1) {
	    if (max>1000000000)
		i = i/10;
	    else
		break;
	}
    }
    else { /* 16 bit */
	while (1) {
	    if (max > 30000)
		i = i/10;
	    else
		break;
	}
    }

    /* if range of dataset too short and manification factor is one
       then adjust it */
    if ((val<=1) && (i==1)) {
	while (val<=100) {
	    i = i*10;
	    val = val * i;
	}
    }
    
    /* return bscaleFactor */
    if (i > 0)
	return(i);
    else 
	return(1);
}
	    


void findMaxMin8(databuffer,datanum,max,min)
    uint8 *databuffer;
    int   datanum;
    float32 *max,*min;

{
    int32 j;
    uint8 val;
    uint8 max8,min8;
    max8 =min8=*databuffer;

    /* find maxinum & minium  of data  */
    for (j=1;j<=datanum;j++) {
	val = *databuffer++;
	if (val>max8)
	    max8 = val;
	if (val<min8)
	    min8 = val;
    }

    *max = max8;
    *min = min8;

}

void findMaxMin16(databuffer,datanum,max,min)
    int16 *databuffer;
    int   datanum;
    float32 *max,*min;

{
    int32 j;
    int16 val;
    int16 max16,min16;
    max16=min16=*databuffer;

    /* find maxinum & minium  of data  */
    for (j=1;j<=datanum;j++) {
	val = *databuffer++;
	if (val>max16)
	    max16 = val;
	if (val<min16)
	    min16 = val;
    }

    *max = max16;
    *min = min16;

}



void findMaxMin32(databuffer,datanum,max,min)
    int32 *databuffer;
    int   datanum;
    float32 *max,*min;

{
    int32 j;
    int32 val;
    int32 max32,min32;
    max32=min32=*databuffer;

    /* find maxinum & minium  of data  */
    for (j=1;j<=datanum;j++) {
	val = *databuffer++;
	if (val>max32)
	    max32 = val;
	if (val<min32)
	    min32 = val;
    }

    *max = max32;
    *min = min32;

}




/* reverse order of lines in image  */
/* convert fits data to hdf form */
void fdat2cdat_uint8(bitpix,rank,dim,data)
    int32  bitpix, rank;
    int32 *dim;
    uint8 *data;
{
    int   dataNum=1, i, index;
    int   x=0,y=0,z=0,f=0;
    int   datasize ;
    uint8 *sptr;  /* point to one row fits data  */
    uint8  *dataPtr, *ptr;

    
    if (rank>4) 
	InfoMsg("FITS data dimensions is greater than 4 !");
    else {
	for (i=0; i<rank; i++)
	    dataNum *= dim[i];
	
	datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * dataNum;

	dataPtr= ptr = (uint8 *) malloc(dataNum);
	if (rank == 2) {/* two dimension */
	    for (y = dim[0] -1; y>=0; y--) { /* colunm */
		sptr = (uint8 *)&data[y*dim[1]];
		for (x=0; x<dim[1]; x++, ptr++)  /* rows  */
		    *ptr = (uint8) *sptr++;
	    }
	}
	
	if (rank == 3) { /* three dimension data */
	    for (z = 0; z < dim[0]; z++) {
		for (y = dim[1] -1; y>=0; y--) { /* colunm */
		  index = (z*dim[1] + y) * dim[2];
		  sptr = (uint8 *)&data[index];
		  for (x=0; x<dim[2]; x++, ptr++)  /* rows  */
		    *ptr = (uint8) *sptr++;
		}
	    }
	}
	
	
	if (rank == 4) { /* four dimensions data */
	    for (f = 0; f < dim[0]; f++) {
		for (z = 0; z < dim[1]; z++) {
		    for (y = dim[2] -1; y>=0; y--) { /* colunm */
		      index = ((f*dim[1]+z)*dim[2]+y)*dim[3];
		      sptr = (uint8 *)&data[index];
		      for (x=0; x<dim[3]; x++, ptr++)  /* rows  */
			*ptr = (uint8) *sptr++;
		    }
		}	   
	    }
	}
	for (i=0; i<= dataNum; i++)
	    *data++ = (uint8) *dataPtr++;
	
    }
}
    


/* convert fits data to hdf form */
void fdat2cdat_int16(bitpix, rank,dim,data)
    int32  bitpix, rank;
    int32 *dim;
    int16 *data;
{
    int   dataNum=1, i, index;
    int   x=0,y=0,z=0,f=0;
    int   datasize ;
    int16 *sptr;  /* point to one row fits data  */
    int16  *dataPtr, *ptr;

    if (rank>4) 
	InfoMsg("FITS data dimensions is greater than 4 !");
    else {
	for (i=0; i<rank; i++)
	    dataNum *= dim[i];
	
	datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * dataNum;

	dataPtr= ptr = (int16 *) malloc(datasize);
	if (rank == 2) {/* two dimension */
	    for (y = dim[0] -1; y>=0; y--) { /* colunm */
	      index =  y * dim[1];
	      sptr = (int16 *)&data[index];
	      for (x=0; x<dim[1]; x++, ptr++)  /* rows  */
		*ptr = (int16) *sptr++;
	    }
	}
	
	if (rank == 3) { /* three dimension data */
	    for (z = 0; z < dim[0]; z++) {
		for (y = dim[1] -1; y>=0; y--) { /* colunm */
		  index = (z*dim[1] + y) * dim[2];
		  sptr = (int16 *)&data[index];
		  for (x=0; x<dim[2]; x++, ptr++)  /* rows  */
		    *ptr = (int16) *sptr++;
		}
	    }
	}
	
	
	if (rank == 4) { /* four dimensions data */
	    for (f = 0; f < dim[0]; f++) {
		for (z = 0; z < dim[1]; z++) {
		    for (y = dim[2] -1; y>=0; y--) { /* colunm */
		      index = ((f*dim[1]+z)*dim[2]+y)*dim[3];
		      sptr = (int16 *)&data[index];
		      for (x=0; x<dim[3]; x++, ptr++)  /* rows  */
			*ptr = (int16) *sptr++;
		    }
		}	   
	    }
	}
	for (i=0; i<= dataNum; i++)
	    *data++ = (int16) *dataPtr++;
	
   }
}
    




/* convert fits data to hdf form */
void fdat2cdat_int32(bitpix, rank,dim,data)
    int32  bitpix, rank;
    int32 *dim;
    int32 *data;
{
    int   dataNum=1, i, index;
    int   x=0,y=0,z=0,f=0;
    int   datasize ;
    int32 *sptr;  /* point to one row fits data  */

    int32  *dataPtr, *ptr;

    if (rank>4) 
	InfoMsg("FITS data dimensions is greater than 4 !");
    else {
	for (i=0; i<rank; i++)
	    dataNum *= dim[i];
	
	datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * dataNum;

	dataPtr= ptr = (int32 *) malloc(datasize);
	if (rank == 2) {/* two dimension */
	    for (y = dim[0] -1; y>=0; y--) { /* colunm */
		sptr = (int32 *)&data[y*dim[1]];
		for (x=0; x<dim[1]; x++, ptr++)  /* rows  */
		    *ptr = (int32) *sptr++;
	    }
	}
	
	if (rank == 3) { /* three dimension data */
	    for (z = 0; z < dim[0]; z++) {
		for (y = dim[1] -1; y>=0; y--) { /* colunm */
		  index = (z*dim[1] + y) * dim[2];
		  sptr = (int32 *)&data[index];
		  for (x=0; x<dim[2]; x++, ptr++)  /* rows  */
		    *ptr = (int32) *sptr++;
		}
	    }
	}
	
	
	if (rank == 4) { /* four dimensions data */
	    for (f = 0; f < dim[0]; f++) {
		for (z = 0; z < dim[1]; z++) {
		    for (y = dim[2] -1; y>=0; y--) { /* colunm */
		      index = ((f*dim[1]+z)*dim[2]+y)*dim[3];
		      sptr = (int32 *)&data[index];
		      for (x=0; x<dim[3]; x++, ptr++)  /* rows  */
			*ptr = (int32) *sptr++;
		    }
		}	   
	    }
	}
	
	for (i=0; i<= dataNum; i++)
	    *data++ = (int32 ) *dataPtr++;
     }
}
    




/* convert fits data to hdf form */
void fdat2cdat_float32(bitpix, rank,dim,data)
    int32  bitpix, rank;
    int32 *dim;
    float32 *data;
{
    int   dataNum=1, i, index;
    int   x=0,y=0,z=0,f=0;
    int   datasize ;
    float32 *sptr;  /* point to one row fits data  */

    float32  *dataPtr, *ptr;

    if (rank>4) 
	InfoMsg("FITS data dimensions is greater than 4 !");
    else {
	for (i=0; i<rank; i++)
	    dataNum *= dim[i];
	
	datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * dataNum;

	dataPtr= ptr = (float32 *) malloc(datasize);
	if (rank == 2) {/* two dimension */
	    for (y = dim[0] -1; y>=0; y--) { /* colunm */
		sptr = (float32 *)&data[y*dim[1]];
		for (x=0; x<dim[1]; x++, ptr++)  /* rows  */
		    *ptr = (float32) *sptr++;
	    }
	}
	
	if (rank == 3) { /* three dimension data */
	    for (z = 0; z < dim[0]; z++) {
		for (y = dim[1] -1; y>=0; y--) { /* colunm */
		  index = (z*dim[1] + y) * dim[2];
		    sptr = (float32 *)&data[index];
		    for (x=0; x<dim[2]; x++, ptr++)  /* rows  */
			*ptr = (float32) *sptr++;
		}
	    }
	}
	
	
	if (rank == 4) { /* four dimensions data */
	    for (f = 0; f < dim[0]; f++) {
		for (z = 0; z < dim[1]; z++) {
		    for (y = dim[2] -1; y>=0; y--) { /* colunm */
		      index = ((f*dim[1]+z)*dim[2]+y)*dim[3];
		      sptr = (float32 *)&data[index];
		      for (x=0; x<dim[3]; x++, ptr++)  /* rows  */
			*ptr = (float32) *sptr++;
		    }
		}	   
	    }
	}
	for (i=0; i<= dataNum; i++)
	    *data++ = (float32 ) *dataPtr++;
 	
    }
}
    

/* reverse order of lines in image  */
/* convert fits data to hdf form */
void fdat2cdat_float64(bitpix, rank,dim,data)
    int32  bitpix, rank;
    int32 *dim;
    float64 *data;
{
    int   dataNum=1, i, index;
    int   x=0,y=0,z=0,f=0;
    int   datasize ;
    float64 *sptr;  /* point to one row fits data  */

    float64  *dataPtr, *ptr;

    if (rank>4) 
	InfoMsg("FITS data dimensions is greater than 4 !");
    else {
	for (i=0; i<rank; i++)
	    dataNum *= dim[i];
	
	datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * dataNum;

	dataPtr= ptr = (float64 *) malloc(datasize);
	if (rank == 2) {/* two dimension */
	    for (y = dim[0] -1; y>=0; y--) { /* colunm */
		sptr = (float64 *)&data[y*dim[1]];
		for (x=0; x<dim[1]; x++, ptr++)  /* rows  */
		    *ptr = (float64) *sptr++;
	    }
	}
	
	if (rank == 3) { /* three dimension data */
	    for (z = 0; z < dim[0]; z++) {
		for (y = dim[1] -1; y>=0; y--) { /* colunm */
		  index = (z*dim[1] + y) * dim[2];
		  sptr = (float64 *)&data[index];
		  for (x=0; x<dim[2]; x++, ptr++)  /* rows  */
		    *ptr = (float64) *sptr++;
		}
	    }
	}
	
	
	if (rank == 4) { /* four dimensions data */
	    for (f = 0; f < dim[0]; f++) {
		for (z = 0; z < dim[1]; z++) {
		    for (y = dim[2] -1; y>=0; y--) { /* colunm */
		      index = ((f*dim[1]+z)*dim[2]+y)*dim[3];
		      sptr = (float64 *)&data[index];
		      for (x=0; x<dim[3]; x++, ptr++)  /* rows  */
			*ptr = (float64) *sptr++;
		    }
		}	   
	    }
	}

	for (i=0; i<= dataNum; i++)
	    *data++ = (float64) *dataPtr++;
	
    }
}
    

void writeDataStrs(id,labFlag,labVal,unitFlag,unitVal,fmtFlag,fmtVal,coordVal)
int32 id,labFlag,unitFlag,fmtFlag;
char  *labVal,*unitVal,*fmtVal,*coordVal;
{
  if (labFlag) { /* set sds data label  string attribute  */
    if ((unitFlag)&&(fmtFlag))
      SDsetdatastrs(id,labVal ,unitVal ,fmtVal,coordVal);
    else {
      if (unitFlag)
	SDsetdatastrs(id,labVal ,unitVal ,NULL,coordVal);
      else {
	if (fmtFlag)
	  SDsetdatastrs(id,labVal ,NULL,fmtVal,coordVal);
	else   
	  SDsetdatastrs(id,labVal ,NULL,NULL,coordVal);
      }
    }
  }
  else {
    if ((unitFlag)&&(fmtFlag))
      SDsetdatastrs(id,NULL ,unitVal ,fmtVal,coordVal);
    else {
      if (unitFlag)
	SDsetdatastrs(id,NULL ,unitVal ,NULL,coordVal);
      else {
	if (fmtFlag)
	  SDsetdatastrs(id,NULL ,NULL,fmtVal,coordVal);
	else   
	  SDsetdatastrs(id,NULL ,NULL,NULL,coordVal);
      }
    }
  }
}

void writeDimStrs(id,labFlag,labVal,unitFlag,unitVal,fmtFlag,fmtVal)
int32 id,labFlag,unitFlag,fmtFlag;
char  *labVal,*unitVal,*fmtVal;
{
  if (labFlag) { /* set sds data label  string attribute  */
    if ((unitFlag)&&(fmtFlag))
      SDsetdimstrs(id,labVal ,unitVal ,fmtVal);
    else {
      if (unitFlag)
	SDsetdimstrs(id,labVal ,unitVal ,NULL);
      else {
	if (fmtFlag)
	  SDsetdimstrs(id,labVal ,NULL,fmtVal);
	else   
	  SDsetdimstrs(id,labVal ,NULL,NULL);
      }
    }
  }
  else {
    if ((unitFlag)&&(fmtFlag))
      SDsetdimstrs(id,NULL ,unitVal ,fmtVal);
    else {
      if (unitFlag)
	SDsetdimstrs(id,NULL ,unitVal ,NULL);
      else {
	if (fmtFlag)
	  SDsetdimstrs(id,NULL ,NULL,fmtVal);
      }
    }
  }
}


/* convert fits card to generic attributes of sds  */
int writeSdsGenericAttr(sdsid,unit,fitsrecnum)
int32 sdsid,unit,fitsrecnum;
{
  
  int  status, j;
  char card[80],value[24];
  char sdsAttrName[24];
  int  sdsAttrNumType;
  int  count;   /* number of values of SDS attribute */
  
  for (j=1 ; j<=3; j++) {
    status = 0;
    FCGREC(unit,j+fitsrecnum, card, &status);
    CHECKFITSERROR("FCGREC", status, errtxt);

    strcpy(value,substr(card,11,20));

    switch(j) {
    case 1:
      strcpy(sdsAttrName, value);
      break;
    case 2:
      sdsAttrNumType = atoi(value);
      break;
    case 3:
      count = atoi(value);
      break;
    }
  }

  switch(sdsAttrNumType) {
  case DFNT_UINT8: { /* char  */
    uint8 *ptr,*buffer;
    ptr=buffer = (uint8 *)newSpace(count);
    for (j=4;j<count+4; j++) {
      
      status = 0;
      FCGREC(unit,j+fitsrecnum, card, &status);
      CHECKFITSERROR("FCGREC", status, errtxt);

      strcpy(value,substr(card,11,20));

      *ptr++ = (uint8)atoi(value);
    }
    SDsetattr(sdsid, sdsAttrName, DFNT_UINT8, count, buffer);
    
  }
  break;

  case DFNT_INT16: { /* 16-bits integer */
    int16 *ptr,*buffer;
    ptr=buffer = (int16 *)newSpace(count*sizeof(int16));
    for (j=4;j<count+4; j++) {
      
      status = 0;
      FCGREC(unit,j+fitsrecnum, card, &status);
      CHECKFITSERROR("FCGREC", status, errtxt);

      strcpy(value,substr(card,11,20));

      *ptr++ = (int16)atoi(value);
    }
    SDsetattr(sdsid, sdsAttrName, sdsAttrNumType, count, buffer);
  }
  break;
  
  case DFNT_INT32: { /* 32-bits integer */
    int32 *ptr,*buffer;
    ptr=buffer = (int32 *)newSpace(count*sizeof(int32));
    for (j=4;j<count+4; j++) {
      
      status = 0;
      FCGREC(unit,j+fitsrecnum, card, &status);
      CHECKFITSERROR("FCGREC", status, errtxt);

      strcpy(value,substr(card,11,20));

      *ptr++ = (int32)atoi(value);
    }
    SDsetattr(sdsid, sdsAttrName, sdsAttrNumType, count, buffer);
  }
  break;

  case DFNT_FLOAT32: { /* 32-bits float */
    float32 *ptr,*buffer;
    ptr=buffer = (float32 *)newSpace(count*sizeof(float32));
    for (j=4;j<count+4; j++) {
      
      status = 0;
      FCGREC(unit,j+fitsrecnum, card, &status);
      CHECKFITSERROR("FCGREC", status, errtxt);

      strcpy(value,substr(card,11,20));

      *ptr++ = (float32)atof(value);
    }
    SDsetattr(sdsid, sdsAttrName, sdsAttrNumType, count, buffer);
  }
  break;

  case DFNT_FLOAT64: { /* 64-bits double float */
    float64 *ptr,*buffer;
    ptr=buffer = (float64 *)newSpace(count*sizeof(float64));
    for (j=4;j<count+4; j++) {
      
      status = 0;
      FCGREC(unit,j+fitsrecnum, card, &status);
      CHECKFITSERROR("FCGREC", status, errtxt);

      strcpy(value,substr(card,11,20));

      *ptr++ = (float64)atof(value);
    }
    SDsetattr(sdsid, sdsAttrName, sdsAttrNumType, count, buffer);
  }
  break;
  default:
    InfoMsg("Unknown number type of SDS attributes");
  }
  
 

  return(fitsrecnum+3+count);
}


