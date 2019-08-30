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

#ifndef SDB_H
#define SDB_H

/* for HDF3.3 include files, HDF4.0 shouldn't require this 
* add your favourite HDF supported platfrom */
#if defined(__sgi)
#define IRIS4
#endif
#if defined(_IBMR2)
#define IBM6000
#endif
#if defined(sun)
#define SUN
#endif
#if defined(cray)
#define UNICOS
#endif
#if defined(__alpha)
#define DEC_ALPHA
#endif
#if defined(__hpux)
#define HP9000
#endif
/* VMS defines itself */

/* --------------- includes  ----------------------------- */
#if defined(hpux) || defined(__hpux)
#include <sys/syscall.h>
#define getrusage(a, b)  syscall(SYS_GETRUSAGE, a, b)
#endif /* hpux */

/* required headers */
#include <stdio.h>
#ifdef _POSIX_SOURCE
#include <sys/stat.h>
#endif /* _POSIX_SOURCE */

/* To use utility macros...*/

#include "myutil.h" 
#include "fbm.h"          /* For gif handling code */
#include "mfhdf.h"
#include "hdf.h"
#include "herr.h"         /* To get some decent error reporting...*/
#include "vg.h"
#include "glist.h"

/* If we have DTM support */
#ifdef HAVE_DTM
#include "netdata.h"
#endif

/* If we have CCI support */
#ifdef HAVE_CCI
#include "cci.h"
#endif

/*---------------------- Local defines -----------------------
 * The following info from should be sent from the browser 
 * But until forms are used they are put here
 */

/* make all in-lined images smaller than hdfImageSize X hdfImageSize */
/* #define hdfImageSize Rdata.hdf_max_image_dimension */
#define hdfImageSize 200

/* limits on datasets and attribute counts to send into brief mode */
/* #define MAX_DATASET_DISPLAY   Rdata.hdf_max_displayed_datasets */
#define MAX_DATASET_DISPLAY   10

/* #define MAX_ATTRIBUTE_DISPLAY Rdata.hdf_max_displayed_attributes */
#define MAX_ATTRIBUTE_DISPLAY 10


/* Send reply buffer size */
#define SND_BUF_SIZE 524288

/* hmm...eric what limits are these? */
#define NUM_VGS       20
#define DOC_WIDTH   132  /* Width of the html document for display a tree */
#define MAX_ENTRIES 10000
#define strequal(s1,s2)    (!strcmp((s1),(s2)))


/* for FITS */
#define FITSfree(p)   free((void *)p)
#define FITSmalloc(p) malloc((size_t)p)
#define FITSFILE  0
#define FITSREF   1
#define SDBREF    1
#define FITSPLANE 2
#define SDBPLANE  2
#define FITSVIEW  3
#define SDBVIEW   3
#define SDSVIEW   31    /* for hdf sds */
#define FITSTAB   4
#define FITSBINTAB 5
#define LISTHEAD  10
#define HEAD      11
#define HISTORY   12


/*-------------------- Local structures -----------------------------*/

/* Stole this structure framework for handling Images from libhtml/HTML.h */
typedef struct image_rec {
    int ismap;                 /* Is this an ISMAP */
    int width;                 
    int height;
    int num_colors;            /* 256 we handle only raster8 */
    int *reds;                 /* R */
    int *greens;               /* G */
    int *blues;                /* B */
    unsigned char *image_data; /* actual data */
    unsigned char *color_map;  /* palette */
} ImageInfo, *ImageInfoPtr;

/* from util.c - used in main() */
typedef struct {
    char *name;
    char *val;
} entry;

/* Enumerated types for MIME */
typedef enum {
  TEXT_HTML,       /* plain html text */
  IMAGE_GIF        /* a gif image */
} mime_content_type;

/* This structure is used to get all relavant environment variables
   passed to this CGI program by 'httpd' */
typedef struct hserv_t {
  char *request_method;      /* GET / PUT */
  char *path_info;
  char *path_translated;
  char *script_name;         /* CGI script name */
  char *query_string;        /* used by GET */
  char *remote_host;
  char *remote_addr;
  char *auth_type;
  char *remote_user;
  char *content_type;
  char *content_length;
  char *http_accept;
  char *http_user_agent;
} hserv_st, *hserv_stp;


/* Create a local struct to hold local variables that can be passed around */
typedef struct lstruct_t {
    FILE *hfp;      /* File pointer to temproray HTML description */
    FILE *gfp;      /* File pointer to temproray gif image */
    int    brief;    /* brief flag */
    char * my_url;   /* Pointer to URL ? */

    /* Environment */
    hserv_st *h_env;  /* Structure holding server environment variables */

    /* Path related */
    char *hdf_path;   /* hdf path w/ filename to show in URL? */
    char *hdf_path_t; /* tranlsated relative path to file passed to script by httpd*/
    char *f_name;     /* name of HDF/netCDF file being browsed */
    char *f_path_r;   /* relative path to HDF/netCDF file being browsed */
    char *hdf_file;   /* full HDF file name w/path if specified */
    char *hdf_file_r; /* name of HDF/netCDF w/ relative path */

    /* Object stuff */
    char *hdf_ref;    /* current 'hdf_ref' i.e.tag/ref in HDF/netCDF file */
    uint16 obj_tag;   /* object tag from 'hdf_ref' */
    uint16 obj_ref;   /* object ref from 'hdf_ref' */
    int    sub_s;     /* subsetting/sampling flag from 'hdf_ref' */

#if 0
    /* The ungainly GIF code isn't set up to handle multiple contexts 
    Ugg...*/
    unsigned char *mypixels;
    int myrowlen;
#endif

    /* These are used when dumping contents to HTML files */
    int do_dump;            /* Dumping flag, 1=TRUE, 0=FALSE */
    int display_mode;       /* Values 1,2 or 3 */
    int display_type;       /* Values 1,2 or 3 */
    char *hdf_file_name;    /* HDF filename w/path */
    char *cgi_path;         /* path to use for cgi script in URL */
    char *hdf_path_r;       /* Relative path to HDF file to use in URL */
    char *html_dir;         /* Directory to dump html file into */

    /* for CCI */
    int  do_cci;            /* CCI flag */
    int  cci_port;          /* CCI port to connect to host */
    char *cci_host;         /* Host to connect to */
#ifdef HAVE_CCI
    MCCIPort   h_cci_port;  /* CCI prot structure */
#endif

    /* SDS - subsetting */
    int32 *sd_dim_start;    /* array for starting dimensions */
    int32 *sd_dim_stride;   /* array for strides */
    int32 *sd_dim_end;      /* array for ending dimensions */
    int32 ndims;            /* numer of dimesions read in */

    /* Vdata field name selecting and subsetting  */
    int *field_indices;     /* array of selected Vdata field names(indices) */
    int nfields;            /* nubmer of valid fields */
    int vh_start_rec;       /* starting record for subsetting */
    int vh_end_rec;         /* ending record for subsetting */

    /* DTM */
    char *dtm_outport;     /* DTM port to send data to on client */

    /* FITs */
    int  do_fits;          /* fits flag */
    int  unit;             /* handle to fits file */
    int  hdu;
    int  plane;            /* plane number to be viewed */

    /* FITS PHDU or IMAGE extension variables regarding the subsetting  
       the following variables are  the common variable shared by fits
       and sds(convert to image */
    int sdb_ref;            /* the reference number  */
    int sdb_imgFlag;        /*  do   image subsetting   */   
    int sdb_plane;          /* nubmer of planes    */
    int sdb_start_rec;      /* starting record for subsetting */
    int sdb_end_rec;        /* ending record for subsetting       */

    /* FITS Ascii table or Binary table field name selecting and subsetting */
    int ft_tabFlag;        /*  do fits table subsetting (Ascii or Bin tab)  */
    int ft_ref;            /* reference number */
    int *ft_fld_indices;   /*  array of selected fits field names(indices)  */
    int ft_nfields;        /* nubmer of selected fields of the fits table   */
    int ft_start_rec;      /* starting record for subsetting(fits table     */
    int ft_end_rec;        /* ending record for subsetting(fits table)      */

    /* netCDF */
    int  do_netcdf;        /* netcdf flag */

    /* SDS lists */
    Generic_list  *sds_list;    /* List of both dimension and regualr variables */
    Generic_list  *gattr_list;  /* global attribute List */
    Generic_list  *dimvar_list; /* dimension variable List */
    Generic_list  *regvar_list; /* regular variable List */

    /* palette list */
    Generic_list  *pal_list;  /* Palette List */

    /* raster list */
    Generic_list  *rig_list;  /* Raster8 List */

    /* annotation lists */
    Generic_list  *an_list;   /* all annotations List */
    Generic_list  *flan_list; /* file label List */
    Generic_list  *fdan_list; /* file desc List */
    Generic_list  *olan_list; /* object label List */
    Generic_list  *odan_list; /* object desc List */

    /* Vgroup list */
    Generic_list  *vg_list;   /* all NON-SDS Vgroups */
    Generic_list  *tvg_list;  /* Top level Vgroup list */
    Generic_list  *ovg_list;  /* Not top level Vgroup */

    /* Vdata list */
    Generic_list  *vd_list;  /* Vdata List */

} lvar_st, *lvar_stp;

/*-----------------------------------------------------------------------*/

/* Used for "getopt()" -- from "unistd.h" */
IMPORT char *optarg;         /* For getopt() */
IMPORT int optind, optopt;   /* ..ditto here ..*/

#ifdef DMAIN_C
/* Program usage */
char *hdf_usage[] = {
  "Usage:hdf [-d] [-i] [-h] [-c path] [-r path] [-D directory] ",
  "          [-H host] [-p] [-f hdf filename] ",
  "\t -d      : dump html descriptions of objects in file",
  "\t -i      : enable CCI",
  "\t -h      : print this help usage",
  "\t -c path : CGI path(i.e. path to this program (used w/ '-d')",
  "\t -r path : relative path to files to use (used with '-d')",
  "\t -D directory  : Directory to dump HTML descriptions of file",
  "\t -H host : CCI host to connect to",
  "\t -p port : CCI port to connect to",
  "\t -f hdf filename : specify hdf filename to dump (used w/ '-d')",
  ""};
#endif
#endif /* SDB_H */
