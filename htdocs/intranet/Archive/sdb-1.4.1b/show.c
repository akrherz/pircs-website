/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

/* Modified from vshow.c by Eric Tsui, 12/25/1994. */

#ifdef RCSID
static char RcsId[] = "@(#)show.c,v 1.14 1996/04/19 12:25:17 xlu Exp";
#endif

#include "sdb.h"
#include "vg.h"
#include "show.h"
#include "sdsdump.h"

#define BUFFER 1000000

/*--------------------------------------------------------------------
 NAME
      getVdataInfo -  retrieve the whole information of the vdata
 DESCRIPTION
      Get the whole information of the vdata.
      Currently the unit and display format of each field can not
      be supported. 
	      
 RETURNS
      SUCCEED/FAIL
---------------------------------------------------------------------*/

int32 getVdataInfo(int32 vd, int32 *nvdata, int32 *nfields,int32 *interlace, 
		   char vdtype[VSFIELDMAX][FIELDNAMELENMAX],  
		   char vdform[VSFIELDMAX][FIELDNAMELENMAX],  
		   char vdunit[VSFIELDMAX][FIELDNAMELENMAX],  
		   char vdname[FIELDNAMELENMAX])
{
  int32 vsize;  /* size of the vdata */
  char  fields[VSFIELDMAX*FIELDNAMELENMAX];
  int   i;
  int32 dataType, vdorder;

  /* get the list of the field name */
  if ((VSinquire(vd, nvdata, interlace, fields, &vsize, vdname)) == FAIL)
    return FAIL;
    
  /* Retrieve the total number of fields in the specified Vdata */
  *nfields = VFnfields(vd);
  if (*nfields == FAIL)    
    return FAIL;
 
  for (i=0; i<*nfields; i++) {
    
    /* Retrieves the fieldname for a specified field in a given Vdata. */
    strcpy(vdtype[i], (char *)VFfieldname(vd, i));

    /* currently HDF have  not unit for vdata, just assign " " to that */
    strcpy(vdunit[i], " ");

    /* Retrieves the number type for a specified field in a given Vdata */
    dataType = VFfieldtype(vd, i);

    /* Retrieves the order of a specified field */
    vdorder    = VFfieldorder(vd,i);

    switch(dataType) {
    case DFNT_CHAR8:
    case DFNT_UINT8: /* char */
      sprintf(vdform[i], "%d", vdorder);
      strcat(vdform[i], "A");
      break;
	
    case DFNT_INT8: /* bit */

      sprintf(vdform[i], "%d", vdorder*8);
      strcat(vdform[i], "X");
      break;

    case DFNT_INT16:
    case DFNT_UINT16: /* short */
      sprintf(vdform[i], "%d", vdorder);
      strcat(vdform[i], "I");
      break;

    case DFNT_INT32:
    case DFNT_UINT32: /* integer */
      sprintf(vdform[i], "%d", vdorder);
      strcat(vdform[i], "J");
      break;


    case DFNT_FLOAT32: /* float */
      sprintf(vdform[i], "%d", vdorder);
      strcat(vdform[i], "E");
      break;

    case DFNT_FLOAT64: /* double */
      sprintf(vdform[i], "%d", vdorder);
      strcat(vdform[i], "D");
      break;

    default: 
	strcpy(vdform[i],"1A");
	InfoMsg("Complex Field Data Number Type"); 
    }

  } /* for (i=....) */

  
  return SUCCEED;
}


/*--------------------------------------------------------------------
 NAME
      dumpvd -  retrieve the vdata to be as the HTML table
 DESCRIPTION
      Processing the vdata to be as a subsetting or subsampling. 
      For subsetting, the subsets of the field in the whole vdata will be
          provided, and the range of the vdata record also will be given.
          The subset of the fields should be the index that is the number 
	  of the field order in the vdata.
	  e.g.
	      subIndex = 5,2; 
	      Suppose the whole fields in the vdata is(5 fields):
                    field_name_1(1), field_name_2(2), field_name_3(3), 
		    field_name_4(4), field_name_5(5);

	  ===> the table should look like:
              
	      --------------------------------------
	          field_name_5 | field_name_2  
              --------------------------------------
	      
 RETURNS
      SUCCEED/FAIL
---------------------------------------------------------------------*/

int32
dumpvd(int32 vd, int data_only, FILE * fp, char separater[2],
       int flds_indices[VSFIELDMAX], int dumpallfields, lvar_st *l_vars)
{
    char        vdname[VSNAMELENMAX];
    char        fields[VSFIELDMAX*FIELDNAMELENMAX];
    int32       interlace;

    int i, j, kk;
    
    int length, tmpLen;
    /* variables that get the table header keywords from the vdata */
    int32 rowlen, nrows, tfields, tbcol[VSFIELDMAX], tblen[VSFIELDMAX], \
        torder[VSFIELDMAX],tdataType[VSFIELDMAX];

    int datacode, datatype, repeat, width, tnull;

    static char  ttype[VSFIELDMAX][FIELDNAMELENMAX],  
      tform[VSFIELDMAX][FIELDNAMELENMAX], 
      tdisp[VSFIELDMAX][FIELDNAMELENMAX],  
      tunit[VSFIELDMAX][FIELDNAMELENMAX], extname[VSFIELDMAX];

    int tbrowlen;

    /* variables that get elements from vdata */
    int colnum, frow, felem, nelmt, ncol;
    int order;
    
    char *tmpStr, *ptr;
    char tmpStr1[5000],tmpStr2[FIELDNAMELENMAX];

    int objectNumber=1;
    int32 status;
  
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
    
    startRecord = l_vars->vh_start_rec;
    if (startRecord < 0)
      startRecord =  1;
    endRecord   = l_vars->vh_end_rec;
            
    /* assign subIndex to respond the real field  */ 
    subTfieldNumber = l_vars->nfields;
    for (i=0; i<subTfieldNumber; i++) 
      subIndex[i] = *(l_vars->field_indices)++;

    /*  ===============     TEST FOR SUBSETTING  =============== */

    /* get the vdata information */
    if ((getVdataInfo(vd,&nrows,&tfields,&interlace,ttype,tform,tunit,extname))==FAIL) {
      gateway_err(l_vars->hfp,"dumpvd: failed to get info on Vdata",0,l_vars);
      return FAIL;
    }
    DBUG_PRINT(3,(LOGF,"Enter Dumpvd"));


    fprintf(fp,"<TABLE BORDER>\n");

    /* tittle of the table  */
    if (strlen(extname)) {
      fprintf(fp, "<TR>\n");
      fprintf(fp, "<TH COLSPAN = %d> %s </TH>\n", subTfieldNumber+1 ,extname); 
      fprintf(fp, "</TR>\n");
    }

    
    /* get length & order for each field */
    for (i=0; i<tfields; i++) {
 
      /* Retrieves the number type for a specified field in a given Vdata */
      numberType   = VFfieldtype (vd,i); 
      tdataType[i] = numberType;

      width    = VFfieldesize(vd,i);
      if (width == FAIL)
	 return FAIL;
          
      /* set the default width for each field in the table so the vdata can be
	 filled in appriciately */
      switch(numberType) {
   
      case DFNT_UINT16:    /* short */
      case DFNT_INT16:     /* short */	
	width = 8;
        break;
	
      case DFNT_INT32:     /* int */
      case DFNT_UINT32:
	width = 12;
	break;

      case DFNT_FLOAT32:
	width = 15;
	break;
      case DFNT_FLOAT64:
	width = 15;
        break;
	
      }
      
      /* Retrieves the order of a specified field */
      order    = VFfieldorder(vd,i);
      if (order == FAIL)
	 return FAIL;

      torder[i] = order;
      tbcol[i]  = width*order + order;
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
      
    ptr = tmpStr = (char *)HDmalloc(tbrowlen + tfields); 
    if (tmpStr == NULL)
      return FAIL;

    
    fprintf(fp,"<TR>\n"); 

    /* create the first column with record number */
    fprintf(fp,"<TH> Rec. </TH>\n");


    length = 0;
    tmpStr[0] = '\0';


    /* create table title for each name of the field */  
    for (i=0; i<subTfieldNumber; i++) {
      /* get the real field to display for subsetting */
      j = subIndex[i] - 1;
      fprintf(fp, "<TH> %s </TH>\n", ttype[j]);

    }

    fprintf(fp,"</TR>\n");

    /* for unit 
    fprintf(fp,"<TR>\n");
    */
    tmpStr = ptr;
    tmpStr[0] = '\0';

    /* create the first column for the record number 
    fprintf(fp,"<TH>%s</TH>\n",(char *)space(recNumLen)); */

    /* create table title for each name of the field(unit) */ 
    /* currently HDF have not units associated with vdata */
    /*
    for (i=0; i<tfields; i++) {      
 
     fprintf(fp, "<TH> %s </TH>\n", tunit[i]);
    }
    
    fprintf(fp,"</TR>\n"); */
  
    /* data processing */   
    /* record number of the vdata */
    nelmt  = nrows; /* number of elements (number of records) */

    /* ======= subsetting ====== */
    nelmt = endRecord - startRecord + 1;

    /* adjust tdisp */
    for (i=0; i<tfields; i++) {
    
      numberType = tdataType[i];
  
      switch(numberType) {
   
      case DFNT_UINT8:
	strcpy(tdisp[i],"%u");
        break;

      case DFNT_UINT16:    /* short */
      case DFNT_INT16:    /* short */	
	strcpy(tdisp[i],"%d");
        break;
	
      case DFNT_INT32:     /* int */
      case DFNT_UINT32:
	strcpy(tdisp[i],"%d");
	break;
	
      case DFNT_FLOAT32:
	/* default format to display the vdata for the floating point */
	strcpy(tdisp[i],"%14.7e");
	break;
      case DFNT_FLOAT64:
	/* default format to display the vdata for the double floating point */
	strcpy(tdisp[i],"%14.7e");
        break;
      case DFNT_CHAR8:
      case DFNT_UCHAR8:
	strcpy(tdisp[i],"%s");
	break;
      default:
	strcpy(tdisp[i],"%10d");
      }
    }
    
    /* for each record */
    /* for (kk=1; kk<=nrows; kk++) { */

    /* ======= subsetting ====== */   
    for (kk=startRecord; kk <= endRecord; kk++) {

      fprintf(fp,"<TR>\n");

      tmpStr[0] = '\0';

      /* create the first column */
      sprintf(tmpStr, "%5d", kk);
      fprintf(fp,"<TD ALIGN=center> %s</TD>\n",tmpStr);

      /* read the vdata and move to string   */
      /* ======= subsetting ====== */
      /* for (i=1; i<=tfields; i++) { */
      for (i=1; i<=subTfieldNumber; i++) { 


	tmpStr[0] = '\0';
   
	/* Calculate vdata size and allocate buffer space */
	/* colnum in table */
	colnum = subIndex[i-1];

	frow   = kk;
	
	felem  = 1;

	/* number of order for the current field */
	ncol   = torder[colnum-1];

	/* assume the repeat number is less than 6  */
	if ((ncol > 3) && (tdataType[colnum-1]!=DFNT_CHAR8) && \
	    (tdataType[colnum-1]!=DFNT_UCHAR8))
	  nelmt = 3;
	else
	  nelmt = ncol;
      
	if (nelmt) { /* non dummy */
	  numberType = tdataType[colnum-1];
	  
	  switch(numberType) {
	  case DFNT_INT16:
	  case DFNT_UINT16:
	    
	    datasize = sizeof(int16)*nelmt;
	    break;
	  case DFNT_UINT32:
	  case DFNT_INT32:
	    datasize = sizeof(int32)*nelmt;
	    break;

	  case DFNT_UINT8:
	    datasize = nelmt;
	    break;
 
	  case DFNT_FLOAT32:
	    datasize = sizeof(float32)*nelmt;
	    break;
	  case DFNT_FLOAT64:
	    datasize = sizeof(float64)*nelmt;
	    break;
	  case DFNT_CHAR8:
	  case DFNT_UCHAR8:
	    datasize = sizeof(char)*nelmt;
	    break;
	  }

	  databuf.cp = (char *)HDmalloc(datasize);

    	  /* set read pointer */
	  VSseek(vd, kk-1);
	  
	  /*  deterine the fields that will be read  */
	  status = VSsetfields(vd, ttype[colnum-1]);
	  
	  if (status == FAIL)
	    /* set fields fail */
	    continue;
	    
	  /* read the vdata  */
	  status = VSread(vd, databuf.cp, 1, FULL_INTERLACE);

	  if (status == FAIL)
	    /* fail to read  */
	    continue;
	  
	  switch(numberType) {
	  case DFNT_CHAR8:          /* byte/char */
	  case DFNT_UCHAR8:
	    for (j=0; j<datasize; j++)
	      tmpStr1[j] = databuf.cp[j];
	    tmpStr1[j] = '\0';
	    break;

	  case DFNT_UINT8:
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
					       (uint8)*databuf.cp));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	    
	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1], 
					       (uint8)databuf.cp[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	    break;
	    
	  case DFNT_INT16:             /* short */	 
	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1],
					      (short)*databuf.sp));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1], \
						 (short)databuf.sp[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	    break;
	 	    
	  case DFNT_UINT16:             /* short */	 
	    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], \
			   (unsigned short)*databuf.sp));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1], \
			      (unsigned short)databuf.sp[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	    break;
   
	  case DFNT_INT32:            /* integer */
   
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
					       (int)*databuf.ip));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1], \
						 (int)databuf.ip[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	    break;

	  case DFNT_UINT32:            /* unsigned integer */
   
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
					       (uint32)*databuf.ip));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1], \
						 (uint32)databuf.ip[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	    break;
	    
	  case DFNT_FLOAT32:           /* float */

	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
					       (float)*databuf.fp));

	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");
	  
	      strcpy(tmpStr2,(char *)dispFmtDat(tdisp[colnum-1], 
						(float)databuf.fp[j-1]));

	      strcat(tmpStr1, tmpStr2);
	    }
	    break;

	  case DFNT_FLOAT64:                /* double */
	    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], *databuf.dp));
	    for (j=2; j<=nelmt; j++) {
	      strcat(tmpStr1, ";");

	      strcpy(tmpStr2, (char *)dispFmtDat(tdisp[colnum-1],
						 databuf.dp[j-1]));
	      strcat(tmpStr1, tmpStr2);
	    }

	    break;
	  default:

	    strcpy(tmpStr1,"*");
	  }
	  
	  /* data format processing */

	  /* length of each field */
	  length = tblen[colnum-1];

	  strcat(tmpStr, (char *)substr(tmpStr1,1,length)); 
	  
	  if ((torder[colnum-1] <= 2)||(numberType == DFNT_CHAR8) \
	      ||(numberType == DFNT_UCHAR8))
	    fprintf(fp, "<TD ALIGN = right> %s </TD>\n", tmpStr);   
	  else {
	    sprintf(tmpStr2, "#row%dcol%d",kk,i);
       
	    fprintf(fp, "<TD ALIGN = left> <A HREF=\"%s\" >%s ...</A> </TD>\n"
		    ,tmpStr2,tmpStr);
	  }
	}
	else  /* dummy */
	  fprintf(fp, "<TD ALIGN = right> %s </TD>\n", "*");   

      } /* for (i=1 ...) */

      fprintf(fp,"</TR>\n");
      
    } /* for (kk=1, ..)  */

    fprintf(fp, "</TABLE>\n");
    
      
    /* if the entries of the column contain more than 3 items 
       make a new table to see details ( <A NAME="#row1col3" > ... </A>*/
 
    /* for each record */
    for (kk=startRecord; kk<=endRecord; kk++) {
   
        tmpStr[0] = '\0';
        /* read the vdata and move to string   */
        for (i=1; i<=subTfieldNumber; i++) {

            tmpStr[0] = '\0';
 
            /* Calculate vdata size and allocate buffer space */
            colnum = subIndex[i-1];  /* colnum in table */

            frow   = kk;
            felem  = 1;
            ncol   = torder[colnum-1];  /* number of elements */

            if (ncol > 200) /* limited entries for each column */
                nelmt  = 200;
            else 
                nelmt  = ncol;

            /* order is greater thsn 3 use hyper link to see more datas */
            if ((ncol > 2)&&(tdataType[colnum-1]!=DFNT_CHAR8) \
		&&(tdataType[colnum-1]!=DFNT_UCHAR8)) {

                fprintf(fp,"<HR>\n");
                /* create the HTML */
                sprintf(tmpStr2, "row%dcol%d",kk,i);  
                fprintf(fp, "<A NAME=\"%s\"><H3> Data  within the Vdata(Row: %d; Col: %d)</H3> </A>\n", tmpStr2, kk, i+1);
	
                fprintf(fp, "<TABLE BORDER>\n");
      
		numberType = tdataType[colnum-1];
	  
		switch(numberType) {

		case DFNT_INT16:
		case DFNT_UINT16:	    
		  datasize = sizeof(int16)*nelmt;
		  break;
		case DFNT_UINT32:
		case DFNT_INT32:
		  datasize = sizeof(int32)*nelmt;
		  break;
		  
		case DFNT_UINT8:
		  datasize = nelmt;
		  break;
		  
		case DFNT_FLOAT32:
		  datasize = sizeof(float32)*nelmt;
		  break;
		case DFNT_FLOAT64:
		  datasize = sizeof(float64)*nelmt;
		  break;
		case DFNT_CHAR8:
		case DFNT_UCHAR8:
		  datasize = sizeof(char)*nelmt;
		  break;
		  
		}
		/* reallocate the space for reading data */
		databuf.cp = (char *)HDmalloc(datasize);

		/* set read pointer */
		VSseek(vd, kk-1);
	  
		/*  deterine the fields that will be read  */
		status = VSsetfields(vd, ttype[colnum-1]);
		
		if (status == FAIL)
		  /* set fields fail */
		  continue;
	    
		/* read the vdata  */
		status = VSread(vd, databuf.cp, 1, FULL_INTERLACE);

		if (status == FAIL)
		  /* fail to read  */
		  continue;
	  
		switch(numberType) {
	
		case DFNT_UINT8:

		  objectNumber = 1;
		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						     (uint8)*databuf.cp));
		  
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		  		   
		  for (j=2; j<=nelmt; j++) {
		  
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
                
		    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], 
						      (uint8)databuf.cp[j-1]));

		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		  }
		  fprintf(fp,"</TR>\n");
		  break;
			
		case DFNT_UINT16:            /* usigned short */
   
		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
						     (uint16)*databuf.sp));
		  objectNumber = 1;
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		
		  for (j=2; j<=nelmt; j++) {
		    
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
               
		    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						     (uint16)databuf.sp[j-1]));
		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		    
		  }
		  fprintf(fp,"</TR>\n");
		  
		  break;

		case DFNT_INT16:            /* short */
   
		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
						     (short)*databuf.sp));
		  objectNumber = 1;
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		
		  for (j=2; j<=nelmt; j++) {
		    
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
               
		    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						     (short)databuf.sp[j-1]));
		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		    
		  }
		  fprintf(fp,"</TR>\n");
		  break;
		  
		case DFNT_UINT32:            /* unsigned integer */
   
		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
						     (uint32) *databuf.ip));
		  objectNumber = 1;
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		
		  for (j=2; j<=nelmt; j++) {
		    
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
               
		    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						    (uint32)databuf.ip[j-1]));
		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		    
		  }
		  fprintf(fp,"</TR>\n");
		  
		  break;		   
	    
		case DFNT_INT32:            /* integer */
   
		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
						     (int)*databuf.ip));
		  objectNumber = 1;
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		
		  for (j=2; j<=nelmt; j++) {
		    
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
               
		    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], \
						       (int)databuf.ip[j-1]));
		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		    
		  }
		  fprintf(fp,"</TR>\n");
		  
		  break;
	    
		case DFNT_FLOAT32:           /* float */

		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
						     (float)*databuf.fp));
		  objectNumber = 1;
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		
		  for (j=2; j<=nelmt; j++) {
		    
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
            
		    strcpy(tmpStr1,(char *)dispFmtDat(tdisp[colnum-1], 
						(float)databuf.fp[j-1]));
		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		    		    
		  }
		  fprintf(fp,"</TR>\n");
		  
		  break;

		case DFNT_FLOAT64:                /* double */

		  strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1], *databuf.dp));
					    
		  objectNumber = 1;
		  fprintf(fp,"<TR>\n");
		  fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		
		  for (j=2; j<=nelmt; j++) {
		    
		    ++objectNumber;
		    if (objectNumber > 10) {
		      objectNumber = 1;
		      fprintf(fp,"</TR>\n");
		      fprintf(fp,"<TR>\n");
		    }
          
		    strcpy(tmpStr1, (char *)dispFmtDat(tdisp[colnum-1],
						       databuf.dp[j-1]));
		    fprintf(fp,"<TD ALIGN=center>%s</TD>\n",tmpStr1);
		    
		  }
		  fprintf(fp,"</TR>\n");
		  break;
		default:

		  break;
		}

		fprintf(fp, "</TABLE><P>\n");
		
	    }   /* if (col > 3) */

	} /* for (i=1 ...) */
    
    } /* for (kk=1, ..)  */
    
    /* free memory */
    if (databuf.cp)
      HDfree(databuf.cp);

    HDfree(tmpStr);

    return SUCCEED;
}



















