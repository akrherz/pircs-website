/* program:  fits2sds.c
 * function: Convert fits file to sds file
 *
 */

#include <stdio.h> 
#include <string.h>


/* Package include files */
#include "fits2sds.h"
#include "extern.h"
#include "cfitsio.h"

/* HDF include files */
#include "hdf.h"
#include "herr.h"

int  fits2sds(input, output)
char *input, *output;
{
    /* FITS related variables */
    int fits_status, fits_iounit;
    int rwstat;
    int numKeys, numSpace,hdutype;

    /* HDF related variables */
    int sds_status;
    int sd_id;
    int sds_id;
    int dim_id;

    char errtxt[FITS_CLEN_ERRMSG];

    char fitsFileName[100], sdsFileName[100];
    char sdsName[20];

    int  imageNumber=0;
    
    /* get fits file name & sds file name from command  line */
    strcpy(fitsFileName,input);
    strcpy(sdsFileName,output);

    sds_status = SUCCEED;
    fits_status= 0;

    /* open FITS file  */
    rwstat = FITSREADONLY;
    fits_iounit = openFits(fitsFileName, rwstat);
    if (fits_iounit == -1)
	return (-1);
 
    /* open SDS file */
    sd_id = SDstart(sdsFileName, DFACC_CREATE);
    if (sd_id == FAIL) { 
	InfoMsg("Can't create sds file");
	return(-1);
    }
    
    /* Move to the Primary Head Data Unit(PHDU) of FITS file  */
    fits_status= 0;
    FCMAHD(fits_iounit,1,&hdutype,&fits_status);
    CHECKFITSERROR("FITS2SDS",fits_status,errtxt);
    if (hdutype != 0) { /* primary array or Image  */
	InfoMsg("Wrong FITS file structure");
	return(-1);
    }
    
    /* Primary array (Image) convert to SDS  */
    strcpy(sdsName, "FITS_PHDU");
    phdu2sds(fits_iounit, sd_id, sdsName);
    
    /* Moving to next extension in the FITS file */
    fits_status= 0;
    FCMRHD(fits_iounit,1,&hdutype,&fits_status);
     
    while (fits_status == 0) {
	switch (hdutype) {
	case 0: /* Image */
	    /* Primary array (Image) convert to SDS  */
	    imageNumber++;
	    strcpy(sdsName,"FITS_IMAGE");
	    strcat(sdsName,num2str(imageNumber));
	    phdu2sds(fits_iounit, sd_id, sdsName);
	    break;

	case 1: /* ASCII table */
/*	    ahdu2sds(fits_iounit, sd_id); */
	    break;
	case 2: /* Binary Table */
/*	    bhdu2sds(fits_iounit, sd_id);   */
	    break;
	default:
	    printf("There is a unknown HDU Type[%d] in %s \n", hdutype,fitsFileName);
	}
	fits_status= 0;
	FCMRHD(fits_iounit,1,&hdutype,&fits_status);
    }

    /* closing sds file */
    SDend(sd_id);

    /* close the fits file & free the unit number */
    FCCLOS(fits_iounit, &fits_status);
    FCFIOU(fits_iounit, &fits_status);
  return(0);

}





