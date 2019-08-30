/* extern.h
 * External routine and variable definitions
 *
 */
 
#ifndef extern_h
#define extern_h

 
/* phdu2sds.c */

extern int  phdu2sds();
extern void ahdu2sds();

/* utils.c  */

extern char *NewTempFile();
extern void printData();
extern void InfoMsg();
extern int  FileExists();
extern void FileOverWriteMsg();
extern int  overWrite();
extern char *substr();
extern char *upper();
extern int  at();
extern char *newSpace();
extern char *num2str();
extern void FatalError();
extern void Quit();
extern int32  findNumberType();
extern int32 openFits();
extern void CHECKFITSERROR();
extern void writeSdsAttr();
extern void writeSdsAttrComm();
extern char *getKey();
extern int32 locFitsAttrKey();
extern int32 locateKeywordIndex();
extern void writeSDrange();
extern char *ltrim();
extern char *trim();
extern int32 getKeyType();
extern int32 getBscaleFactor();
extern void findMaxMin8();
extern void findMaxMin16();
extern void findMaxMin32();
extern void fdat2cdat_uint8();
extern void fdat2cdat_int16();
extern void fdat2cdat_int32();
extern void fdat2cdat_float32();

extern void fdat2cdat_float64();

extern void writeSDfillval();
extern void writeDataStrs();
extern void writeDimStrs();

extern  int writeSdsGenericAttr();

#endif





