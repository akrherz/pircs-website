/* fitsutil.h
 * External routine and variable definitions
 *
 */
 
#ifndef FITSUTIL_H
#define FITSUTIL_H

/* fitsutils.c  */

extern void CHECKFITSERROR();
extern int  openFits();
extern void CHECKFITSERROR();
extern int  fitsnimages();
extern int  fitsntables();
extern int  fitsnbintables();
extern int  fitsInfo();
extern int  fitsImage();

/* fitsutil.c - table */
extern int  fitsTableInfo();
extern int  getTabDatType();
extern int  fitsTable();

/* fitsutil.c - binary Table */
extern int   fitsBinTabInfo();
extern int   getBinTabDatType();
extern int   fitsBinTabWidth();
extern void  fitsBinTabDispFmt();
extern int   fitsBinTable();

/* fitsutil.c - read fits */
extern int   readFits();

/* fitsutil.c - listhead */
extern int   fitsnobject();
extern char  *getDataType();
extern int   getDatRange();
extern char  *readFitsKey();
extern int   readCoordinate();
extern int   getFitsDesc();

#endif /* FISUTIL_H */





