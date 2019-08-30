# ****************************************************************************
# * NCSA HDF                                                                 *
# * Software Development Group                                               *
# * National Center for Supercomputing Applications                          *
# * University of Illinois at Urbana-Champaign                               *
# * 605 E. Springfield, Champaign IL 61820                                   *
# *                                                                          *
# * For conditions of distribution and use, see the accompanying             *
# * COPYING file.                                                            *
# *                                                                          *
# ***************************************************************************
# Makefile,v 1.29 1996/05/20 19:00:05 georgev Exp
#
# Makefile to build HDF SDB program
#
# You should only have to change/modify the 
# following variables:
#
# CC                        -> cc, gcc
# SYSCONFIGFLAGS            -> (see below) 
# CFLAGS                    -> -DDEBUG=3
# hdfdir,cdfdir,jpegdir,zlibdir -> location of libdf.a, libmfhdf.a, libjeg.a,libz.a
# fitsiodir                 -> location of FITS4.08 version
# HDF_LIBS                  -> (see below)
# HDF_FLAGS                 -> -DHAVE_HDF 
# HDF_INC                   -> (see below)
# AUX_FLAGS                 -> -DHAVE_PROTOTYPES 
# AUX_INC                   ->
# AUX_LIBS                  -> Additional system libraries(-lm ..etc)
#
# ----------------------------------- Customize -----------------------------
# PLEASE NOTE that this version is not fully ANSI
# and has been mostly compilied with GCC.
#
# Set your favourite Compilier here
#
# For gcc
#CC= gcc

# For native ANSI compilers
CC= cc

#--------------------------------------------------
# Some system/compilier flags 
#

# for GCC
#SYSCONFIGFLAGS = 

# For HP native CC compilier we include ANSI options 
#SYSCONFIGFLAGS =-Aa -D_HPUX_SOURCE 

# For Solaris 2.5
#SYSCONFIGFLAGS = -Xa 

# For Sunos w/ FITS
# SYSCONFIGFLAGS = -DSUN

# For IRIX 5.x
SYSCONFIGFLAGS = -ansi -64

# For Dec Alpha OSF3.0
#SYSCONFIGFLAGS = -Olimit 2048 -std1 

#--------------------------------------------------
# 
#For HPUX native Optimization
#CFLAGS= +O3 $(SYSCONFIGFLAGS)

# use this if you're using 'cc' on a DEC Alpha (OSF/1) or MIPS (Ultrix) system:
#CFLAGS = -std1 -Olimit 750

#For GCC Optimization
#CFLAGS= -O2 $(SYSCONFIGFLAGS)

#For debugging
#CFLAGS= -g $(SYSCONFIGFLAGS) -pedantic -Wall -DDEBUG=3 -DRCSID
#CFLAGS= -g $(SYSCONFIGFLAGS) -DDEBUG=3 
CFLAGS= -g $(SYSCONFIGFLAGS)

#--------------------------------------------------
# Indicate where HDF4.0 and FITS libraries are located.
#
# To compile the FITS only version you stil need the Header
# files to the HDF library but not the libraries
#
# (i.e. libdf.a, libmfhdf.a ,libjpeg.a,libz.a)
#
#hdfdir = /hdf2/install/hp
hdfdir = /usr/people/fils

#cdfdir = /hdf2/install/hp
cdfdir = /usr/people/fils

#jpegdir = /hdf2/install/hp
jpegdir = /usr/people/fils

#zlibdir = /hdf2/install/hp
zlibdir = /usr/people/fils

# FITS
fitsiodir = /hdf2/fitsio/fortran/hp
FITSINC   = $(fitsiodir)/include
FITSLIB   = $(fitsiodir)/lib

# Not supported yet
# DTM and libnet(X-Mosaic)
#dtmlib = /hdf4/users/georgev/web/xmosaic3/libdtm/libdtm.a
#dtminc = /hdf4/users/georgev/web/xmosaic3/libdtm

# libnet code came from X-Mosaic distribution (not supported)
#libnetdir = ./libnet

#--------------------------------------------------
# Set Libraries to use
#
# for HDF4.0 only
HDF_LIBS =  $(cdfdir)/lib/libmfhdf.a $(hdfdir)/lib/libdf.a \
$(jpegdir)/lib/libjpeg.a $(zlibdir)/lib/libz.a 

#
# HDF4.0 w/ FITS(fortran)
#HDF_LIBS =  $(cdfdir)/lib/libmfhdf.a $(hdfdir)/lib/libdf.a \
# $(jpegdir)/lib/libjpeg.a $(zlibdir)/lib/libz.a $(FITSLIB)/libfitsio.a 

# FITS only
#HDF_LIBS =  $(FITSLIB)/libfitsio.a 

#--------------------------------------------------
# set additional HDF Flags(not supported)
#
# For HDF need -DHAVE_HDF
# For FITS support need -DHAVE_FITS
# For FITS to HDF conversion need -DHAVE_FITS2HDF

# For HDF only
HDF_FLAGS = -DHAVE_HDF

#for FITS only
#HDF_FLAGS = -DHAVE_FITS

# for HDF w/ FITS
#HDF_FLAGS = -DHAVE_HDF -DHAVE_FITS -DHAVE_FITS2HDF

#----------------------------------------------------
# set Includes to use
#
# HDF only
# Include directories(i.e where are the HDF/netCDF headers files located?)
HDF_INC = -I$(hdfdir)/include -I$(cdfdir)/include -I$(jpegdir)/include \
 -I$(zlibdir)/include

# HDF with FITS
#HDF_INC = -I$(hdfdir)/include -I$(cdfdir)/include -I$(jpegdir)/include \
# -I$(zlibdir)/include -I$(FITSINC)

# FITS only still requires the HDF headers
#HDF_INC = -I$(hdfdir)/include -I$(cdfdir)/include -I$(jpegdir)/include \
#-I$(zlibdir)/include -I$(FITSINC)

#--------------------------------------------------
# Set -DHAVE_CCI flag if you have the CCI 1.X library and want it enabled
# NOTE: (not supported)
#
# For DTM need -DHAVE_DTM
#
# General
AUX_FLAGS= $(HDF_FLAGS) -DHAVE_PROTOTYPES 
#
# For HDF w/ DTM
#AUX_FLAGS= $(HDF_FLAGS) -DHAVE_PROTOTYPES -DHAVE_DTM

#------------------------------------------------
# Includes(CCI & DTM not supported ) 
#
#AUX_INC= $(HDF_INC) -I/hdf4/users/georgev/cci-1.0
#AUX_INC= $(HDF_INC) -I/hdf4/users/georgev/libcci
#
# w/ DTM support(DTM directory and libnet directory(form X-Mosaic)
#AUX_INC= $(HDF_INC) -I$(dtminc) -I$(libnetdir)
#

AUX_INC= $(HDF_INC) 

########################################################################
# For FITS support
#
# F77LFLAGS is a platform-specific environment variable which specifies
# the Fortran 77 libraries  that need to be linked with the program.
# More details about CFITSIO can be found at:
#  http://heasarc.gsfc.nasa.gov/docs/software/fitsio/cfitsio.html

# FITS - for Solaris and Sun the location of the Fortan libraries is needed
# when linking the objects.

# FITS - For Solaris
#F77HOME=/export/home/local/SUNWspro/SC3.0/lib
#F77LFLAG=-L$(F77HOME) -lF77 -lM77 -lsunmath -lm -lc 

# FITS -for SunOS
#F77HOME=/export/local/lang-sun4/SC1.0
#F77LFLAG=-L$(F77HOME) -lF77  -lm -lc 

#F77LFLAGS=

#-----------------------------------------------
# Additional Libraries(CCI,DTM and others)
# 
# For FITS support place additional Fortran libraies that might
# needed here
#
# CCI libs(not supported)
#AUX_LIBS= /hdf4/users/georgev/cci-1.0/libcci.a
#AUX_LIBS= /hdf4/users/georgev/libcci/libcci.a

#For HPUX using native compilier need this
#AUX_LIBS= $(F77LFLAG) -lm

#for FITS on HPUX need
#AUX_LIBS= $(F77LFLAG) -lm -lcl -lisamstub

#for FITS on DEC/OSF need
#AUX_LIBS= $(F77LFLAG) -lUfor -lfor -lFutil -lm -lots -lc

#for FITS on SGI irix5.x need
# AUX_LIBS= $(F77LFLAG) -lftn -lm -lc

#Needed by Solaris 2.5 
#AUX_LIBS= $(F77LFLAG) -lm -lnsl

#Needed by Sun
#AUX_LIBS= $(F77LFLAG) -lm 

#for DTM (X-Mosaic) not supported
#AUX_LIBS=  $(F77LFLAG) $(libnetdir)/libnet.a $(dtmlib)

#for FITS & DTM (X-Mosaic) not supported
#AUX_LIBS=  $(F77LFLAG) $(libnetdir)/libnet.a $(dtmlib)  -lm -lcl -lisamstub

# remove utility
RM= /bin/rm -f

# -------------------------------------------------------------------------
# -------------- You shouldn't have to edit anything else -----------------
# -------------------------------------------------------------------------
HDRS = myutil.h util.h show.h sdsdump.h sdb_ann.h sdb_attr.h sdb_cci.h sdb_dtm.h \
       sdb_pal.h sdb_ri.h sdb_sds.h sdb_util.h sdb_vd.h sdb_vg.h sdb_img.h sdb.h

CSRCS = myutil.c util.c show.c sdsdump.c sdb_ann.c sdb_attr.c sdb_cci.c sdb_dtm.c \
       sdb_pal.c sdb_ri.c sdb_sds.c sdb_util.c sdb_vd.c sdb_vg.c  \
       flgifc.c flgife.c sdb.c glist.c sdb_img.c fitsutils.c

COBJS = myutil.o util.o show.o sdsdump.o sdb_ann.o sdb_attr.o sdb_cci.o sdb_dtm.o \
       sdb_pal.o sdb_ri.o sdb_sds.o sdb_util.o sdb_vd.o sdb_vg.o  \
       flgifc.o flgife.o sdb.o glist.o sdb_img.o fitsutils.o

FCOBJS = myutil.o util.o \
       sdb_util.o  \
       flgifc.o flgife.o  glist.o sdb_img.o fitsutils.o sdb.o

.c.o: $(HDRS)
	$(CC) -c $(CFLAGS) $(AUX_FLAGS) $(AUX_INC) $<

all: sdb

debug: psdb

#
# These targets are for making 'sdb' with only HDF support
# i.e. not FITS support
#
sun:
	make all CC=gcc CFLAGS="-ansi -O2 -Dsun"\
        hdfdir="/hdf2/install/sun" \
        cdfdir="/hdf2/install/sun" \
        jpegdir="/hdf2/install/sun" \
	zlibdir="/hdf2/install/sun" \
	fitsiodir="/hdf2/fitsio/fortran/sun" \
    F77LFLAG="-L/export/local/lang-sun4/SC1.0 " \
	AUX_LIBS=" -lF77 -lm -lc"

sun-debug:
	make all CC=gcc CFLAGS="-ansi -g -DDEBUG=3 -Dsun"\
        hdfdir="/hdf2/install/sun" \
        cdfdir="/hdf2/install/sun" \
        jpegdir="/hdf2/install/sun" \
	zlibdir="/hdf2/install/sun" \
	fitsiodir="/hdf2/fitsio/fortran/sun" \
    F77LFLAG="-L/export/local/lang-sun4/SC1.0 " \
	AUX_LIBS=" -lF77 -lm -lc"
	
solaris:
	make all CC=cc CFLAGS="-Xa "\
        hdfdir="/hdf2/install/solaris" \
        cdfdir="/hdf2/install/solaris" \
        jpegdir="/hdf2/install/solaris" \
	zlibdir="/hdf2/install/solaris" \
	fitsiodir="/hdf2/fitsio/fortran/solaris" \
	F77LFLAG="-L/export/home/local/SUNWspro/SC3.0/lib" \
	AUX_LIBS=" -lF77 -lM77 -lsunmath -lm -lnsl -lc"

solaris-debug:
	make all CC=cc CFLAGS="-Xa -g -DDEBUG=3"\
        hdfdir="/hdf2/install/solaris" \
        cdfdir="/hdf2/install/solaris" \
        jpegdir="/hdf2/install/solaris" \
	zlibdir="/hdf2/install/solaris" \
	fitsiodir="/hdf2/fitsio/fortran/solaris" \
	F77LFLAG="-L/export/home/local/SUNWspro/SC3.0/lib" \
	AUX_LIBS=" -lF77 -lM77 -lsunmath -lm -lnsl -lc"

solaris-gcc:
	make all CC=gcc CFLAGS="-ansi"\
        hdfdir="/hdf2/install/solaris" \
        cdfdir="/hdf2/install/solaris" \
        jpegdir="/hdf2/install/solaris" \
	zlibdir="/hdf2/install/solaris" \
	fitsiodir="/hdf2/fitsio/fortran/solaris" \
	F77LFLAG="-L/export/home/local/SUNWspro/SC3.0/lib" \
	AUX_LIBS=" -lF77 -lM77 -lsunmath -lm -lnsl -lc"

solaris-gcc-debug:
	make all CC=gcc CFLAGS="-ansi -g -DDEBUG=3"\
        hdfdir="/hdf2/install/solaris" \
        cdfdir="/hdf2/install/solaris" \
        jpegdir="/hdf2/install/solaris" \
	zlibdir="/hdf2/install/solaris" \
	fitsiodir="/hdf2/fitsio/fortran/solaris" \
	F77LFLAG="-L/export/home/local/SUNWspro/SC3.0/lib" \
	AUX_LIBS=" -lF77 -lM77 -lsunmath -lm -lnsl -lc"

hp: 
	make all CC=cc CFLAGS="-Aa -D_HPUX_SOURCE +O3"\
        hdfdir="/hdf2/install/hp" \
        cdfdir="/hdf2/install/hp" \
        jpegdir="/hdf2/install/hp" \
	zlibdir="/hdf2/install/hp" \
	fitsiodir="/hdf2/fitsio/fortran/hp" \
	F77LFLAG="" \
	AUX_LIBS="-lm -lcl -lisamstub"

hp-debug: 
	make all CC=cc CFLAGS="-Aa -D_HPUX_SOURCE -g"\
        hdfdir="/hdf2/install/hp" \
        cdfdir="/hdf2/install/hp" \
        jpegdir="/hdf2/install/hp" \
	zlibdir="/hdf2/install/hp" \
	fitsiodir="/hdf2/fitsio/fortran/hp" \
	F77LFLAG="" \
	AUX_LIBS="-lm -lcl -lisamstub"

hp-gcc: 
	make all CC=gcc CFLAGS="-ansi -O2"\
        hdfdir="/hdf2/install/hp" \
        cdfdir="/hdf2/install/hp" \
        jpegdir="/hdf2/install/hp" \
	zlibdir="/hdf2/install/hp" \
	fitsiodir="/hdf2/fitsio/fortran/hp" \
	F77LFLAG="" \
	AUX_LIBS="-lm -lcl -lisamstub"

hp-gcc-debug: 
	make all CC=gcc CFLAGS="-ansi -g -DDEBUG=3"\
        hdfdir="/hdf2/install/hp" \
        cdfdir="/hdf2/install/hp" \
        jpegdir="/hdf2/install/hp" \
	zlibdir="/hdf2/install/hp" \
	fitsiodir="/hdf2/fitsio/fortran/hp" \
	F77LFLAG="" \
	AUX_LIBS="-lm -lcl -lisamstub"

sgi:
	make all CC=cc CFLAGS="-ansi"\
        hdfdir="/hdf2/install/sgi" \
        cdfdir="/hdf2/install/sgi" \
        jpegdir="/hdf2/install/sgi" \
	zlibdir="/hdf2/install/sgi"  \
	fitsiodir="/hdf2/fitsio/fortran/sgi"  \
	F77LFLAG="" \
	AUX_LIBS="-lftn -lm -lc"

sgi-debug:
	make sdb CC=cc CFLAGS="-ansi -g -DDEBUG=3"\
        hdfdir="/hdf2/install/sgi" \
        cdfdir="/hdf2/install/sgi" \
        jpegdir="/hdf2/install/sgi" \
	zlibdir="/hdf2/install/sgi"  \
	fitsiodir="/hdf2/fitsio/fortran/sgi" \
	F77LFLAG="" \
	AUX_LIBS="-lftn -lm -lc"

decaxp:
	make all CC=cc CFLAGS="-O2 -Olimit 2048 -std1"\
        hdfdir="/hdf2/install/osf" \
        cdfdir="/hdf2/install/osf" \
        jpegdir="/hdf2/install/osf" \
	zlibdir="/hdf2/install/osf" \
	fitsiodir="/hdf2/fitsio/fortran/osf"  \
	F77LFLAG="" \
	AUX_LIBS="-lUfor -lfor -lFutil -lm -lots -lc"

decaxp-debug:
	make all CC=cc CFLAGS="-Olimit 2048 -std1 -g -DDEBUG=3"\
        hdfdir="/hdf2/install/osf" \
        cdfdir="/hdf2/install/osf" \
        jpegdir="/hdf2/install/osf" \
	zlibdir="/hdf2/install/osf"  \
	fitsiodir="/hdf2/fitsio/fortran/osf" \
	F77LFLAG="" \
	AUX_LIBS="-lUfor -lfor -lFutil -lm -lots -lc"

# HDF/netCDF/FITS version
sdb: $(HDRS) $(COBJS)
	$(CC) $(CFLAGS) $(AUX_FLAGS) $(F77LFLAG) $(COBJS) \
	$(HDF_LIBS) $(DTM_LIBS) $(AUX_LIBS)  -o sdb

psdb: $(HDRS) $(COBJS)
	purify $(CC) $(CFLAGS) $(AUX_FLAGS) $(F77LFLAG) $(COBJS) \
	$(HDF_LIBS) $(DTM_LIBS) $(AUX_LIBS) -o psdb

#FITS only version
fdb: $(HDRS) $(FCOBJS)
	$(CC) $(CFLAGS) $(AUX_FLAGS) $(F77LFLAG) $(FCOBJS) \
	$(HDF_LIBS) $(DTM_LIBS) $(AUX_LIBS)  -o fdb

sdsdump.o: sdsdump.c sdsdump.h
	$(CC) -c $(CFLAGS) $(AUX_FLAGS) $(AUX_INC) sdsdump.c

show.o: show.c show.h
	$(CC) -c $(CFLAGS) $(AUX_FLAGS) $(AUX_INC) show.c

sdb.o: sdb.h
sdb_ann.o: sdb.h sdb_util.h sdb_ann.h
sdb_attr.o: sdb.h sdb_util.h sdb_attr.h
sdb_img.o: sdb.h sdb_util.h sdb_img.h
sdb_cci.o: sdb.h sdb_util.h sdb_cci.h
sdb_dtm.o: sdb.h sdb_util.h sdb_dtm.h
sdb_pal.o: sdb.h sdb_util.h sdb_pal.h
sdb_ri.o: sdb.h sdb_util.h sdb_ri.h
sdb_sds.o: sdb.h sdb_util.h sdb_sds.h
sdb_util.o: sdb.h sdb_util.h
sdb_vd.o: sdb.h sdb_util.h sdb_vd.h
sdb_vg.o: sdb.h sdb_util.h sdb_vg.h

fitsutils.o: fitsutils.h

glist.o: glist.h

clean:
	rm -f *.o hdf sdb fdb
