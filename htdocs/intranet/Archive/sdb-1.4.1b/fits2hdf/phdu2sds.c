
/* program:  phdu2sds.c
 * function: Convert fits' Primary array  to sds file
 *
 *
 *
 */

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

/* Package include files */
#include "fits2sds.h"
#include "extern.h"
#include "cfitsio.h"

/* HDF include files */
#include "hdf.h"
#include "herr.h"

int phdu2sds(iounit, sd_id, sds_name)
    int iounit,sd_id;
    char *sds_name;
{
    /* some of fits variable */
    int simple,extend,anyflg;
    int numberKeys, numberSpace;
    int bitpix,naxis,naxes[15],hdfNaxes[15], pcount,gcount;
    int nelmt;
    int group,fpixel;
    char keyword[10];
    char value[70], comment[60];
    char sdsname[80];

    /* some of sds variable */
    int32	sds_id, dim_id;
    int32	numberType, uncalDataType=DFNT_FLOAT32;
    int32	start[15];
    char        datasetName[12];


    /* pointer of various types */
    union {
      char	*cp;	/* Byte or char */
      short     *sp;	/* short */
      int	*ip;	/* integer */
      float32	*fp;	/* float */
      float64	*dp;	/* double */
    } databuf;
    
    float32  *databuf_def, tmpVal;

    int datasize;	/* in bytes */

    int i, j;
    int nbytes;
    int status;
    
    uint8 *ptr,*sptr,*dataPtr;
    int keywordIndex;
    int tmpLen;

    float32 bscaleVal,bzeroVal,bscalerrVal,bzeroerrVal ;
    float64 sdsBscaleVal,sdsBzeroVal, sdsBscalerrVal,sdsBzeroerrVal ;
    int32 range[2];
    int16 fitsBitspix[1];
    int32 dim_num;
    static float64 dimScale[5000];

    char nextKey[8];
    char card[80];
    int commentCount = 0, blankCount = 0; /*  */
    int historyCount = 0;
    float32 fillVal;
    float32 dataMax, dataMin;
    float32 crpixVal[5], crotaVal[5], crvalVal[5], cdeltVal[5];
    int     crpixFlag[5], crotaFlag[5], crvalFlag[5], cdeltFlag[5];
    char    labelVal[5][24], unitVal[5][24], formatVal[5][24];
    int     labelFlag[5], unitFlag[5], formatFlag[5];

    char    sdslabelVal[24],sdsunitVal[24],sdsformatVal[24],sdscoordsysVal[24];
    int     sdslabelFlag,sdsunitFlag, sdsformatFlag,sdscoordsysFlag;


    int refPoint;
    float refPointVal;


    char commentAttr[12],commentVal[3000];
    char historyAttr[12];
    char naxisComment[16];
    
    int  datamaxFlag=0, dataminFlag=0;
    int  bscaleFlag,bzeroFlag;
    int32 bscaleFactor=1;

    char errtxt[FITS_CLEN_ERRMSG];

    bscaleFlag = 0;
    bzeroFlag  = 0;

    bscaleVal=1.0;
    bzeroVal=bscalerrVal=bzeroerrVal=0.0 ;
    sdslabelFlag=sdsunitFlag=sdsformatFlag=sdscoordsysFlag=0;
    for (i=0; i<5; i++) {
      crpixVal[i]  = 0;
      crpixFlag[i] = 0;

      crotaVal[i]  = 0;
      crotaFlag[i] = 0;

      crvalVal[i]  = 0;
      crvalFlag[i] = 0;

      cdeltVal[i]  = 0;
      cdeltFlag[i] = 0;


      labelFlag[i]=unitFlag[i]= formatFlag[i]=0;
    }

    for (i=0; i<5000; i++)
      dimScale[i] = 0.0;
     

    strcpy(sdsname,sds_name);

    /* read the required primary array keywords  */
    status = 0;
    FCGHPR(iounit, 99, &simple, &bitpix, &naxis, naxes, &pcount, &gcount,&extend, &status);
    CHECKFITSERROR("FCGHPR", status, errtxt);
    if (status != 0) 
	InfoMsg( "Fits file Error" );

    if (!simple) {   /* verify TRUE */
      InfoMsg("This file is not FITS standard!");
      return(-1);
    }
    if ((naxis>999) || (naxis<0)) {
      InfoMsg("Illegal NAXIS value");
      return(-1);
    }
    
    if (at("IMAGE",sds_name)>0) { /* IMAGE */
      /* modified sds_name */
      status = 0;
      FCGKYS(iounit, "EXTNAME", value,comment , &status);
      if (status == 0) {
	/* strcpy(value, substr(card,11,20)); */
	strcpy(sdsname, value);
      }
      status = 0;
      FCGKYS(iounit, "EXTVER", value,comment , &status);
      if (status == 0) {
	if (strlen(trim(value))) { /* EXTVER is exist in fits file */
	  strcpy (value,   ltrim(value));
	  strcpy (value,   trim(value));
	  strcat (sdsname, "_");
	  strcat (sdsname, value);
	}
      }

      if (pcount != 0) {
	InfoMsg("FITS: PCOUNT <> 0");
	return(-1);
      }
      if (gcount != 1) {
	InfoMsg("FITS: GCOUNT <> 1");
	return(-1);
      }
    }

    /* get numberType from bitpix  */
    numberType = findNumberType(bitpix); 

    /* reverse naxes */
    for (i=0; i<naxis; i++) 
	hdfNaxes[i] = naxes[naxis-i-1];
    for (i=0; i<naxis; i++) 
	naxes[i] = hdfNaxes[i];

    /* setup the HDF uncalibration data number type */
    if (numberType == DFNT_FLOAT64)
      uncalDataType = DFNT_FLOAT64;

    /* create sds file based on geting info */
    sds_id = SDcreate(sd_id, sdsname, numberType, naxis, hdfNaxes);
   

    /* get all of card image  */
    /* Determine the number of keywords in the header  */
    status = 0;
    FCGHSP(iounit, &numberKeys, &numberSpace, &status);
 
    for (i=1; i<=numberKeys; i++) {
	
	/* get each 80-chars keywords records  */
	FCGREC(iounit, i, card, &status);
	
	/* get slash(/)'s position */
	tmpLen = at("/ ",card);
	if (tmpLen>=0)
	  tmpLen -= 11;
	else
	  tmpLen = 70;

	/* FCGKYN(iounit, i, keyword, value, comment, status); */
	strcpy(keyword, substr(card,1,8));

	/* locate the keyword in the fits2sdsAttrTab */
	numberType = locFitsAttrKey(keyword);
	
	if (numberType > 0) {   /* write sds attribute based on numbertype  */
	    if (numberType == DFNT_UINT8) {

		if (tmpLen == 70) { /* such as ORIGIN TITLE comment  */
		    strcpy(comment,substr(card,10,72));
		    writeSdsAttr(sds_id, keyword,  numberType, comment); 
		}
		else {	/* such as SIMPLE ...  */	
		    strcpy(value, substr(card,11,tmpLen));
		    writeSdsAttr(sds_id, keyword,  numberType, value); 

		    /* write keyword's comment  */
		    writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);

		}
	    }
	    else {  /* such as BSCALE ... */
		    strcpy(value, substr(card,11,tmpLen));
		    writeSdsAttr(sds_id, keyword,  numberType, value); 

		    /* write keyword's comment  */
		    writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);

	    }
	}
	else { 
	    keywordIndex = locateKeywordIndex(keyword);
	    switch(keywordIndex) {
	    case BSCALE:	
	        /* find BSCALE or BZERO  */
		bscaleFlag = 1;
		strcpy(value, substr(card,11,tmpLen));
		bscaleVal = (float)atof(value);

		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;
	    case BSCALERR:	
	        /* find BSCALERR   */
		strcpy(value, substr(card,11,tmpLen));
		bscalerrVal = (float)atof(value);

		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;

	    case BZERO:
	        bzeroFlag = 1;
		strcpy(value, substr(card,11,tmpLen));
		bzeroVal = (float)atof(value);
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;

	    case BZEROERR:
		strcpy(value, substr(card,11,tmpLen));
		bzeroerrVal = (float)atof(value);
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;

	    case DATATYPE:
		strcpy(value, substr(card,11,tmpLen));
		uncalDataType = (int32)atoi(value);
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;


	    case DATAMAX:
	        strcpy(value, substr(card,11,tmpLen));
		datamaxFlag = 1;       /* DATAMAX is exist  */
		dataMax     = (float32)atof(value);
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);

		break;

	    case DATAMIN:
		strcpy(value, substr(card,11,tmpLen));
		dataminFlag = 1;       /* DATAMIN is exist  */
		dataMin     = (float32)atof(value);
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);

		break;
	    case BUNIT: 
	        /* BUNIT shall contain a character string described  
		   the physical units */
	        strcpy(value, substr(card,11,tmpLen));

		strcpy(value,ltrim(value));
		strcpy(value, trim(value));
	
		if ((strcmp(substr(value,1,1),"'")==0) && \
		    (strcmp(substr(value,strlen(value),1),"'")==0))
		  strcpy(value,substr(value,2,strlen(value)-2));

		sdsunitFlag = 1;
		strcpy(sdsunitVal, value);

	        /* attaches a label,unit,format & coordinate system attr to an sds */
	        /* SDsetdatastrs(sds_id, NULL, value, NULL, NULL); */

	        /* write keyword's comment  */
	        writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);

		break;

	    case BLANK:
		/* check bitpix value is positive */
		strcpy(value, substr(card,11,tmpLen));
		fillVal = atof(value);
		if ((bitpix == 16) || (bitpix == 32)) {
		  writeSDfillval(sds_id, bitpix, fillVal);
		  /* write keyword's comment  */
		  writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		}
		break;

	    case EXTNAME: 
		strcpy(value, substr(card,11,tmpLen));
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;

	    case EXTVER: 
		strcpy(value, substr(card,11,tmpLen));
		/* write keyword's comment  */
		writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		break;

	    case COMMENT: 
		/* get next keyword  */
		strcpy( commentVal, substr(card, 9,70));
		/* when comment begin with SDS attributes the consecutive 
		   rows should be generic attributes of SDS */
		if (at("SDS attributes", commentVal) >= 0) {
		  i = writeSdsGenericAttr(sds_id,iounit, i);
		  break;
		}

		commentCount = commentCount + 1;

		/* skip blank record */
		if (numberKeys >= (i+1))
		    strcpy(nextKey, getKey(iounit, i+1));

		j=0;  /* control 12 rows blank commentVal   */		
		while ((strncmp("COMMENT",nextKey,7) == 0)&& \
		       (numberKeys>=(i+1))){
			i++;
			status = 0;
			FCGREC(iounit,i,card,&status);
			CHECKFITSERROR("FCGREC", status, errtxt);

			if (at("SDS attributes", card) >= 0) {
			  i--;
			  break;
			}
			strcat(commentVal,"\n");

			strcat(commentVal, substr(card,9,70));
			if ((numberKeys >= (i+1)) && ( j <= 10)) {
			    j++;
			    strcpy(nextKey, getKey(iounit, i+1));
			}
			else
			    break;

		}

		/* get comment attribute name such as COMMENT1 ... */
		strcpy(commentAttr, "COMMENT");		
		strcat(commentAttr, num2str(commentCount));
		writeSdsAttr(sds_id, commentAttr,DFNT_UINT8, commentVal);
		break;

	    case HISTORY: 
		++commentCount ;
		/* get next keyword  */
		strcpy( commentVal, substr(card, 9,70));

		/* skip blank record */
		if (numberKeys >= (i+1))
		    strcpy(nextKey, getKey(iounit, i+1));

		j=0;  /* control 12 rows blank commentVal   */		
		while ((strncmp("HISTORY",nextKey,7) == 0)&& \
		       (numberKeys>=(i+1))){
			i++;
			status = 0;
			FCGREC(iounit,i,card,&status);
			CHECKFITSERROR("FCGREC", status, errtxt);

			strcat(commentVal,"\n");

			strcat(commentVal, substr(card,9,70));
			if ((numberKeys >= (i+1)) && ( j <= 10)) {
			    j++;
			    strcpy(nextKey, getKey(iounit, i+1));
			}
			else
			    break;

		}
		/* get comment attribute name such as COMMENT1 ... */
		strcpy(historyAttr, "HISTORY");
		strcat(historyAttr, num2str(commentCount));
		writeSdsAttr(sds_id, historyAttr,DFNT_UINT8, commentVal);
		break;
	    default:
	        if (!(strlen((ltrim(card)))))  /* card is empty */
		  break;

		/* fits keyword CRPIXn  */
		if (strcmp("CRPIX", substr(keyword,1,5))==0){		    
		    strcpy(value, substr(card,11,tmpLen));
		    j = naxis - (int)atoi(substr(keyword,6,1));  /* crpix1 --> dim2  */
		    if (j == naxis) {
			InfoMsg("Maybe Wrong FITS keyword CRPIX");
		    }
		    else {
		        crpixFlag[j] = 1;

			/* get the identifier for the desired dimension  */
			dim_id = SDgetdimid(sds_id,j);
			/* when CRPIX1 = 1 dim[] should be 0 */
			crpixVal[j] = (float) atof(value) - 1; 
			/* writeSdsAttr(dim_id, keyword, DFNT_FLOAT32, value);
			 */
			/* write keyword's comment  */
			writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);
		    }
		    break;
		}

		/* fits keyword CROTAn  */
		if (strcmp("CROTA", substr(keyword,1,5))==0){
		    strcpy(value, substr(card,11,tmpLen));
		    j =naxis - (int)atoi(substr(keyword,6,1));
		    if (j == naxis) { 
			InfoMsg("Maybe Wrong FITS keyword CROTA");
		    }
		    else {
		      crotaFlag[j] = 1;
		      
		      /* get the identifier for the desired dimension  */
		      dim_id = SDgetdimid(sds_id,j);
		      crotaVal[j] = (float)atof(value); 

		      /* write CROTAn */
		      writeSdsAttr(dim_id, keyword, DFNT_FLOAT32, value);
		      /* write keyword's comment  */
		      writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);
		    }
		    break;
		}

		/* fits keyword CRVALn  */
		if (strcmp("CRVAL", substr(keyword,1,5))==0){		    
		    strcpy(value, substr(card,11,tmpLen));
		    j =naxis - (int)atoi(substr(keyword,6,1));
		    if (j == naxis) { 
			InfoMsg("Maybe Wrong FITS keyword CRVAL ");
		    }
		    else { 
		      crvalFlag[j] = 1;
		  
		      /* get the identifier for the desired dimension  */
		      dim_id = SDgetdimid(sds_id,j);

		      crvalVal[j] = (float)atof(value); 

		      /* writeSdsAttr(dim_id, keyword, DFNT_FLOAT32, value);
		       */
		      /* write keyword's comment  */
		      writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);
		      }
		    break;
		}
		
		/* fits keyword CDELTn  */
		if (strcmp("CDELT", substr(keyword,1,5))==0){		    
		    strcpy(value, substr(card,11,tmpLen));
		    j =naxis - (int)atoi(substr(keyword,6,1));
		    if (j == naxis) {
			InfoMsg("Maybe Wrong FITS keyword CDELT");
		    }
		    else {
		        cdeltFlag[j] = 1;

			/* get the identifier for the desired dimension  */
			dim_id = SDgetdimid(sds_id,j);

			cdeltVal[j] = (float)atof(value); 
			/* writeSdsAttr(dim_id, keyword, DFNT_FLOAT32, value);
			 */
			/* write keyword's comment  */
			writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);
		    }
		    break;
		}

		/* fits keyword CTYPEn  */
		if (strcmp("CTYPE", substr(keyword,1,5))==0){		    
		    strcpy(value, substr(card,11,tmpLen));
		    /* attaches a label,unit,format & coordinate system attr to an sds */
		    /* SDsetdatastrs(sds_id, NULL, value, NULL, NULL); */

		    j = naxis - (int)atoi(substr(keyword,6,1));
		    if (j == naxis) 
			InfoMsg("Maybe Wrong FITS keyword CTYPE");
		    /* get the identifier for the desired dimension  */
		    dim_id = SDgetdimid(sds_id,j);
		    
		    strcpy(value,ltrim(value));
		    strcpy(value, trim(value));
	
		    if ((strcmp(substr(value,1,1),"'")==0) && \
			(strcmp(substr(value,strlen(value),1),"'")==0))
		      strcpy(value,substr(value,2,strlen(value)-2));
  
		    strcpy(unitVal[j], value);
		    unitFlag[j]=1;

		    /* set a dimension string attribute  */
		    /* SDsetdimstrs(dim_id,NULL,value,NULL); */
		    
		    /* write keyword's comment  */
		    writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);

		    break;
		}		

		/* fits keyword CLABLn  */
		if (strcmp("CLABL", substr(keyword,1,5))==0){		    
		    strcpy(value, substr(card,11,tmpLen));
		    j = naxis - (int)atoi(substr(keyword,6,1));
		    if (j == naxis) 
			InfoMsg("Maybe Wrong FITS keyword CLABELn");
		    /* get the identifier for the desired dimension  */
		    dim_id = SDgetdimid(sds_id,j);
		    strcpy(value,ltrim(value));
		    strcpy(value, trim(value));
	
		    if ((strcmp(substr(value,1,1),"'")==0) && \
			(strcmp(substr(value,strlen(value),1),"'")==0)) 
		      strcpy(value,substr(value,2,strlen(value)-2));
		    
		    strcpy(labelVal[j], value);
		    labelFlag[j]=1;

		    /* set a dimension string attribute  */
		    /* SDsetdimstrs(dim_id,NULL,value,NULL); */
		    
		    /* write keyword's comment  */
		    writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);

		    break;
		}		

		/* fits keyword CLABLn  */
		if (strcmp("CFMT", substr(keyword,1,4))==0){		    
		    strcpy(value, substr(card,11,tmpLen));
		    j = naxis - (int)atoi(substr(keyword,6,1));
		    if (j == naxis) 
			InfoMsg("Maybe Wrong FITS keyword CFMTn");
		    /* get the identifier for the desired dimension  */
		    dim_id = SDgetdimid(sds_id,j);
		    strcpy(value,ltrim(value));
		    strcpy(value, trim(value));
	
		    if ((strcmp(substr(value,1,1),"'")==0) && \
			(strcmp(substr(value,strlen(value),1),"'")==0)) 
		      strcpy(value,substr(value,2,strlen(value)-2));
		    
		    strcpy(formatVal[j], value);
		    formatFlag[j]=1;

		    /* set a dimension string attribute  */
		    /* SDsetdimstrs(dim_id,NULL,value,NULL); */
		    
		    /* write keyword's comment  */
		    writeSdsAttrComm(dim_id, keyword, DFNT_UINT8, card);

		    break;
		}		
		/* if fits has new keyword DATALABL , DATAFMT or DATACOOR */
		if ((!strcmp(keyword,"DATALABL"))||(!strcmp(keyword,"DATAFMT"))\
		    ||(!strcmp(keyword,"DATACOOR"))) {
		  strcpy(value, substr(card,11,tmpLen));
		  strcpy(value,ltrim(value));
		  strcpy(value, trim(value));
		  
		  if ((strcmp(substr(value,1,1),"'")==0) && \
		      (strcmp(substr(value,strlen(value),1),"'")==0))
		    strcpy(value,substr(value,2,strlen(value)-2));

		  if (!strcmp(keyword,"DATALABL")) {
		    sdslabelFlag = 1;
		    strcpy(sdslabelVal, value);
		  }
		  else 
		    if (!strcmp(keyword,"DATAFMT")) {
		      sdsformatFlag = 1;
		      strcpy(sdsformatVal, value);
		    }
		    else {
		      sdscoordsysFlag = 1;
		      strcpy(sdscoordsysVal, value);
		    }
		
		  /* write keyword's comment  */
		  writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);

		  break;
		}

		/* some other fits keyword  */
		if (!(strlen((ltrim(keyword))))) { /* keyword is '      '  */		
		    ++commentCount;
		    /* get next keyword  */
		    strcpy( commentVal, substr(card, 9,70));
		    
		    /* get vivid next record */
		    if (numberKeys >= (i+1))
			strcpy(nextKey, getKey(iounit, i+1));
		    j=0;  /* control 12 rows blank commentVal   */

		    while ((strncmp("       ",nextKey,7) == 0)&&  \
			   (numberKeys >= (i+1))){
			i++;
			status = 0;
			FCGREC(iounit,i,card,&status);
			CHECKFITSERROR("FCGREC", status, errtxt);

			strcat(commentVal,"\n");

			strcat(commentVal, substr(card,9,70));
			if ((numberKeys >= (i+1)) && ( j <= 10)) {
			    j++;
			    strcpy(nextKey, getKey(iounit, i+1));
			}
			else
			    break;

		    }
		    /* get comment attribute name such as blank1 ... */
		    strcpy(commentAttr, "blank");
		    strcat(commentAttr, num2str(commentCount));
		    writeSdsAttr(sds_id, commentAttr,DFNT_UINT8, commentVal);
		    break;
		} /* (!(strlen((ltrim(keyword))))) { */


		/* NAXIS ...... */
		if (strncmp("NAXIS",substr(keyword,1,5),5)) { /* not include NAXIS  */
		    strcpy(value, substr(card,11,tmpLen ));
		    numberType = getKeyType(value);
		    writeSdsAttr(sds_id, keyword, numberType, value);

		    /* write keyword's comment  */
		    writeSdsAttrComm(sds_id, keyword, DFNT_UINT8, card);
		    break;
		}


	    }  /* switch(keyword)  */
	} /* if (numberType > 0)  */
    }  /*  for (i=1; i<=numberKeys; i++) */


    /* Calculate dimension scale size and allocate buffer space */
  
    /* process of the dimension scale */
    for (i=0; i<naxis; i++) {  /* rank = naxis */

      /* get the identifier for the desired dimension  z,y,x....*/
      dim_id = SDgetdimid(sds_id,i);
      
      if (crvalFlag[i] && cdeltFlag[i] && crpixFlag[i]) { /* make a scale */
	for (j=0; j<naxes[i]; j++) 
	  dimScale[j] = (((float64)j-(float64)crpixVal[i])*(float64)cdeltVal[i] + (float64)crvalVal[i]);

	if ((crotaFlag[i])&& (crotaVal[i] != 0)) {
	    printf("Rotation processing \n ");
	}

	/* stores the values of the specified dimension  */
	status = SDsetdimscale(dim_id,naxes[i] ,DFNT_FLOAT64,dimScale);
	if (status != SUCCEED)
	  InfoMsg("Can't write dimension scale values!");

      }  /* if (crvalFlag[i] && cdeltFlag[i] &  */
      else {
	j = naxis - i;
	if (crvalFlag[i]) {/* CRVALn */
	strcpy(keyword,"CRVAL");
	strcat(keyword,num2str(j));
	SDsetattr(dim_id, keyword, DFNT_FLOAT32, 1, &crvalVal[i]);
	}

	if (crpixFlag[i]) {/* CRPIXn */
	strcpy(keyword,"CRPIX");
	strcat(keyword,num2str(j));
	SDsetattr(dim_id, keyword, DFNT_FLOAT32, 1, &crpixVal[i]);
	}

	if (cdeltFlag[i]) {/* CDELTn */
	strcpy(keyword,"CDELT");
	strcat(keyword,num2str(j));
	SDsetattr(dim_id, keyword, DFNT_FLOAT32, 1, &cdeltVal[i]);
	}

      } /* else { */
    } /* for (i=0, i<naxis, i++) {  */

    /* process of the sds DATA string  information */
    if (sdscoordsysFlag)     
      /* set sds DATA string  information */
      writeDataStrs(sds_id,sdslabelFlag,sdslabelVal,sdsunitFlag,sdsunitVal,sdsformatFlag,sdsformatVal,sdscoordsysVal);
    else
      writeDataStrs(sds_id,sdslabelFlag,sdslabelVal,sdsunitFlag,sdsunitVal,sdsformatFlag,sdsformatVal,NULL);
    

    /* process of the dimension information */
    for (i=0; i<naxis; i++) {  /* rank = naxis */
      /* get the identifier for the desired dimension  z,y,x....*/
      dim_id = SDgetdimid(sds_id,i);
      writeDimStrs(dim_id,labelFlag[i],labelVal[i],unitFlag[i],unitVal[i],formatFlag[i], formatVal[i]);
    }    

    /* fits keyword DATAMAX & DATAMIN  */
    if ((datamaxFlag) || (dataminFlag)) {
	/* DATAMAX and DATAMIN are exist  */
	if ((datamaxFlag) && (dataminFlag)) {
	    /* dataMax & dataMin are of the same number type as the data  */
	    writeSDrange(sds_id,bitpix,  &dataMax, &dataMin ); 
	}
	else {
	    if (datamaxFlag)
		InfoMsg("DATAMAX is exist but DATAMIN is not.");
	    else
		InfoMsg("DATAMIN is exist but DATAMAX is not.");
	}
    }


    /* fits keyword BSCALE & BZERO  */
    sdsBscalerrVal = (float64)bscalerrVal;
    sdsBzeroerrVal = (float64)bzeroerrVal;

    if ((bscaleFlag) || (bzeroFlag)) {
	/* BSCALE and BZERO are exist  */
	if ((bscaleFlag) && (bzeroFlag)) {
	    /* bscaleVal & bzeroVal are of the same number type as the data  */
	    sdsBscaleVal = (float64)bscaleVal;
	    sdsBzeroVal  = (float64)(-bzeroVal/bscaleVal);
	}
	else {
	  if (bscaleFlag) {
	      sdsBscaleVal = (float64)bscaleVal;
	      sdsBzeroVal = 0;
	  }
	  else {
	      sdsBscaleVal = 1;
	      sdsBzeroVal  = (float64)(-bzeroVal);
	  }
	}
	SDsetcal(sds_id,sdsBscaleVal,sdsBscalerrVal,sdsBzeroVal,sdsBzeroerrVal,uncalDataType);  
    } /* if ((bscaleFlag) || (bzeroFlag)) {  */

    /* fits NAXIS */
    if (1) {
	char numStr[2];
	char naxisStr[5];

	for (i=0; i<= naxis; i++) {
	  if (i==0)
	    strcpy(keyword,"NAXIS");
	  else {
	    strcpy(numStr, num2str(i));
	    strcpy(naxisStr,"NAXIS");
	    strcpy (keyword, strcat(naxisStr,numStr));
	  }
	    
	  status = 0;
	  FCGCRD(iounit,keyword,card,&status);
	  /* FCGKEY(iounit, keyword, value, comment, &status); */
	  CHECKFITSERROR("FCGCRD",status,errtxt);
	  strcpy(comment, substr(card,34,60));
	  strcpy(naxisComment, strcat(keyword, "_COMMENT"));
	  writeSdsAttr(sds_id, naxisComment, DFNT_UINT8, comment);
	}
    }

    if (naxis) {
    /* Calculate image size and allocate buffer space */
    nelmt = 1;
    for (i=0; i<naxis; i++){
	nelmt *= naxes[i];
	start[i] = 0;
    }
    
    datasize = (bitpix < 0 ? -bitpix : bitpix)/8 * nelmt;
    databuf.cp = (char *)newSpace(datasize);

    /* FITS file images are displayed using a cartesian coorrdinate system.
       Because the first pixel is in the lower left conner we need fix it.
       That means need reverse the lines order of fits image  */
    /* read the fits image data and convert it to hdf form  */

    group=0;

    switch (bitpix){
    case 8:		/* byte/char */
	/* Does not work yet */
	numberType = DFNT_UINT8;
	if ((bitpix == 8) && ((bscaleFlag)||(bzeroFlag))) {
 	    /* reset the bscale */
	    status = 0;
	    FCPSCL(iounit,1.0, 0, &status);
	    CHECKFITSERROR("FCPSCAL",status,errtxt);
	    FCGPVB(iounit,group, 1, nelmt, 0, (uchar8 *)databuf.cp, &anyflg, &status);
	    /* reset HDF range because it describes the stored data value */
	    if ((datamaxFlag) && (dataminFlag)) {
		dataMax = (dataMin = 1.0);
		findMaxMin8(databuf.cp ,nelmt,&dataMax,&dataMin);
		writeSDrange(sds_id,bitpix,&dataMax,&dataMin); 
	    }
	}
	else	
	  FCGPVB(iounit,group, 1, nelmt, 0, (uchar8 *)databuf.cp, &anyflg, &status); 
	CHECKFITSERROR("FCGPVB",status,errtxt);
	/*
	fdat2cdat_uint8(bitpix,naxis,naxes,databuf.cp);*/

	break; 
    case 16:            /* short */
        numberType = DFNT_INT16;

	/* when BSCALE and BZERO is exist we need to reset calibration factor
	 if we read fits data in this case we can keep the precision */

	if ((bitpix == 16) && ((bscaleFlag)||(bzeroFlag))) {
 	    /* reset the bscale */
	    status = 0;
	    FCPSCL(iounit,1.0, 0, &status);
	    CHECKFITSERROR("FCPSCAL",status,errtxt);
	    /* 
	    int16 *ptr16;
	    ptr16 = databuf.sp;
	    databuf_def = (float32 *)newSpace(nelmt*4);

	    FCGPVE(iounit,group, 1, nelmt, 0, databuf_def, &anyflg, &status);
	    bscaleFactor = getBscaleFactor(bitpix, databuf_def, nelmt);
	    if (bscaleFactor != 1)
		SDsetattr(sds_id, "BSCALEFACTOR", DFNT_INT32, 1, &bscaleFactor);
	    for (i=0; i<nelmt; i++) { 
		*ptr16++ = (int16)((*databuf_def++) * bscaleFactor);	
	    }
	    */ 
	    FCGPVI(iounit,group, 1, nelmt, 0, databuf.sp, &anyflg, &status);
	    /* reset the HDF range because it describes the stored data value */
	    if ((datamaxFlag) && (dataminFlag)) {
		dataMax = (dataMin = 1.0);
		findMaxMin16(databuf.sp ,nelmt,&dataMax,&dataMin);
		writeSDrange(sds_id,bitpix,&dataMax,&dataMin); 
	    }
	}
	else	
	    FCGPVI(iounit,group, 1, nelmt, 0, databuf.sp, &anyflg, &status);

	/* convert fits data to sds data form 
	fdat2cdat_int16(bitpix,naxis,naxes,databuf.sp);*/

        break;
    case 32:            /* integer */
        numberType = DFNT_INT32;

	if ((bitpix == 32) && ((bscaleFlag)||(bzeroFlag))) {
	  /* reset the bscale */
	    status = 0;
	    FCPSCL(iounit, 1.0, 0 ,&status);
	    CHECKFITSERROR("FCPSCAL",status,errtxt);
	    /* 
	    int32 *ptr32;
	    ptr32 = databuf.ip;
	    databuf_def = (float32 *)newSpace(nelmt*4);
	    FCGPVE(iounit,group, 1, nelmt, 0, databuf_def, &anyflg, &status);
	    bscaleFactor = getBscaleFactor(bitpix, databuf_def, nelmt);
	    if (bscaleFactor != 1) 
		SDsetattr(sds_id, "BSCALEFACTOR", DFNT_INT32, 1, &bscaleFactor);
	    for (i=0; i<nelmt; i++) 
		*ptr32++  = (int32)((*databuf_def++) * bscaleFactor);
		*/

	    FCGPVJ(iounit,group, 1, nelmt, 0, databuf.ip, &anyflg, &status);
	    /* rewrite max & min  */
	    if ((datamaxFlag) && (dataminFlag)) {
		dataMax = (dataMin = 1.0);
		findMaxMin32(databuf.ip ,nelmt,&dataMax,&dataMin);
		writeSDrange(sds_id,bitpix,&dataMax,&dataMin);  

	    }


	}
	else	
	    FCGPVJ(iounit,group, 1, nelmt, 0, databuf.ip, &anyflg, &status);

	/* convert fits data to sds data form 
	fdat2cdat_int32(bitpix,naxis,naxes,databuf.ip); */


        break;
    case -32:           /* float */
        numberType = DFNT_FLOAT32;
        FCGPVE(iounit,group, 1, nelmt, 0, databuf.fp, &anyflg, &status);
	
        /* convert fits data to sds data form 
	fdat2cdat_float32(bitpix,naxis,naxes,databuf.fp);  */

        break;

    case -64:		/* double */
	numberType = DFNT_FLOAT64;
	FCGPVD(iounit,group, 1, nelmt, 0, databuf.dp, &anyflg, &status);

	/* convert fits data to sds data form 
	fdat2cdat_float64(bitpix,naxis,naxes,databuf.dp);  */

	break;
    default:
	InfoMsg("Illegal bitpix value");
    }

    CHECKFITSERROR("FCGPV[BIJED]",status, errtxt);
    
    /* write data to sds */
    status = SDwritedata(sds_id, start, NULL, naxes, (void*)databuf.cp); 
    

    if (status != SUCCEED) {
	InfoMsg("Can't write SDS data");
	return(-1);
    }

    } /* if (naxis) */

    status = SDendaccess(sds_id);
    if (status != SUCCEED)
        InfoMsg("Can't close  SDS data file");
}

