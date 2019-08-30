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
/* sdb_util.c,v 1.22 1996/05/16 14:06:19 xlu Exp */
#ifdef RCSID
static char RcsId[] = "@(#)sdb_util.c,v 1.22 1996/05/16 14:06:19 xlu Exp";
#endif

/* sdb_util.c,v 1.22 1996/05/16 14:06:19 xlu Exp */

/**************************************************************************
UTILITY MODULES
   getNewScript      - Fits
   print_strings     - print array of strings
   gateway_err       - report gateway error to client(using stdout)
   write_html_header - Write MIME complient html header depending
                       upon mime type being handled(text,gif,)
   write_html_trailer - Write trailer stuff at end of document
   send_reply        - send html description or GIF image back to client
   pull_filename - Get hdf file name from query string(lvars.f_name)
   pull_ref       - Get hdf tag/ref from query string(lvars.hdf_ref)
   get_type          - returns string descrtption of HDF number type
   get_atype         - returns string descrtption of HDF annotation type
   buffer_to_string  - convert buffer to string depending on number type
   string_length     -
****************************************************************************/

/* Include our stuff */
#include "sdb.h"
#include "myutil.h"
#include "sdb_util.h"


/* Print an informational message */
static char   retStr[5000];


/*------------------------------------------------------------------------ 
 NAME
     InfoMsg - Print an informational message 
 DESCRIPTION
     Print an informational message   
 RETURNS
     NONE
-------------------------------------------------------------------------*/  
void InfoMsg(str)
char *str;
{
    printf("%s\n", str);
}
/*------------------------------------------------------------------------ 
 NAME
     trim -  get rid of the right space of the string
 DESCRIPTION
      get rid of the right space of the string
 RETURN
     the trimed string
-------------------------------------------------------------------------*/  
char *trim(str)
char *str;
{

    int  i=strlen(str);
    if (i) {
        while ((str[i-1]== ' ') && (i>0))
            --i;

        retStr[i]='\0';
        --i;
        while (i>=0) {

            retStr[i] = str[i];
            --i;
        }
    }
    else 
        retStr[0]='\0';

    return(retStr);
}

/*------------------------------------------------------------------------ 
 NAME
     mod -  get mod number
 DESCRIPTION
      get mod number
 RETURN
     the mod number
-------------------------------------------------------------------------*/  
int mod(a,b)
int a,b;
{

    int tmp;

    tmp = (int) (a/b);

    if ((tmp * b) == a)
        return 0;
    else
        return ((tmp * b)-a);
}

/*------------------------------------------------------------------------ 
 NAME
     substr -  get the substring of the given string
 DESCRIPTION
     get the substring of the given string from the provided position.
 RETURN
     the substring
-------------------------------------------------------------------------*/  
char *substr( str,start, stop)
char *str;
int  start, stop;
{
    int x ;
    int i = 0;
    strcpy(retStr,"");
    if (strlen(str) < start)
      { retStr[0] = '\0';
      return(retStr);
      }
    {
        for(x=start-1;(stop>i)  ;x++,i++){
            if (strlen(str) < start+i)
                break;
            retStr[i] = str[x];
        }
    }
    retStr[i] = '\0';
    return(retStr);
}

/*------------------------------------------------------------------------ 
 NAME
     dispFmtDat -   return the string based on the  variable format
 DESCRIPTION
     return string based on the  variable format 
     called  by:
        strcpy(str, (char *)dispFmtDat(format, arg1, arg2, ...)  
 Reference:
     Man page of the vsprintf.
 RETURN
     the formated string
-------------------------------------------------------------------------*/   
char *
dispFmtDat (va_alist) va_dcl
{
    va_list args;
    char *fmt;

    va_start(args);

    /* str = va_arg(args, char *); */
    fmt = va_arg(args, char *);

    /* print out remainder of message */
    (void) vsprintf(retStr,fmt, args);

    va_end(args);
    return (retStr);
}

/*----------------------------------------------------------------------------
 NAME
      
 DESCRIPTION
      
. RETURNS
      
----------------------------------------------------------------------------*/
char *
file_href(lvar_st *l_vars)
{
    static char href_str[1024];

    HDmemset(href_str,'\0',1024);
    sprintf(href_str,"%s%s?%s",
            l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name);

    return href_str;
}

/*----------------------------------------------------------------------------
 NAME
      
 DESCRIPTION
      
. RETURNS
      
----------------------------------------------------------------------------*/
char *
obj_href(uint16 tag, 
         uint16 ref, 
         int    sampling,
         lvar_st *l_vars)
{
    static char href_str[1024];

    HDmemset(href_str,'\0',1024);
    if (l_vars->dtm_outport != NULL)
      {
    sprintf(href_str,"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d,dtmport=%s",
            l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
            tag, ref, sampling,l_vars->dtm_outport);
      }
    else
      {
    sprintf(href_str,"%s%s?%s!hdfref;tag=%d,ref=%d,s=%d",
            l_vars->h_env->script_name,l_vars->h_env->path_info,l_vars->f_name,
            tag, ref, sampling);
      }

    return href_str;
}

/*----------------------------------------------------------------------------
 NAME
      getNewScript - FITS to HDF cgi script name
 DESCRIPTION
      get new script name from fileName.
.
 RETURNS
      Returns new script name if successful and FAIL otherwise.
----------------------------------------------------------------------------*/
char *
getNewScript(char *oldScript, 
             char *fileName)
{
    int   i;
    char  *tmpStr;
    char *ret_value = NULL;

    if ((tmpStr = (char *)malloc(50)) == NULL)
      {
        ret_value = NULL;
        goto done;
      }
    
    for (i=strlen(oldScript); oldScript[i] != '/'; i--)
        ;
    oldScript[i+1] ='\0';
    
    strcpy(tmpStr, oldScript);
    strcat(tmpStr, fileName);

    ret_value = tmpStr;

done:
    return ret_value;
} /* getNewscript() */

/*-------------------------------------------------------------------- 
 NAME
     print_strings  - prints an array of strings
 DESCRIPTION
     Given a array of strings, prints them out in order.
 RETURNS
     Nothing
--------------------------------------------------------------------*/ 
void 
print_strings(FILE *output, 
              char **str)
{
    int i;
    for(i=0; strlen(str[i]); i++)
      {
          fprintf(output,"%s\n", str[i]);
      }
} /* print_strings() */

/*-------------------------------------------------------------------- 
 NAME
     gateway_err - print error and exit
 DESCRIPTION
     Report that the gateway encountered an error. Handles different
     cases(CCI, Dumping, CGI).
 RETURNS
     Doesn't return but calls exit()
--------------------------------------------------------------------*/ 
void 
gateway_err(FILE *fout,  /* output stream */
            char *msg,   /* error message to output */
            int fatal,   /* 0= not fatal, 1= fatal error message */
            lvar_st *l_vars)
{
    if (fatal)
      {

          /* If were are dummping just print the error */
          if (l_vars->do_dump)
              printf("%s", msg);
          else if(l_vars->do_cci)
            { /* else we are a CCI app */
                printf("This CCI app encountered an error:\n");
                printf("%s", msg);
            }
          else
            { /* else we are CGI program */
                printf("Content-type: text/html%c%c",10,10);
                printf("<title>Mapping HDF Gateway Error</title>");
                printf("<h1>Mapping HDF Gateway Error</h1>");
                printf("This HDF Gateway encountered an error:<p>");
                printf("%s", msg);
            }
	  exit(-1);   
      }
    else
        fprintf(fout,"<p> Error:<pre> %s <pre> \n",msg);

} /* gateway_err */

/*-------------------------------------------------------------------- 
 NAME
     write_html_header
 DESCRIPTION
     Write MIME complient header depending upon mime type
     i.e. is text/html or image/gif
 RETURNS
     Return SUCCEED if successful and FAIL otherwise.
--------------------------------------------------------------------*/ 
int
write_html_header(FILE *fhandle,             /* output stream */
                  mime_content_type mc_type, /* MIME type */
                  lvar_st *l_vars)
{
    char newScript[50];
    char tmp[50];
    int ret_value = SUCCEED;
    ENTER(2,"write_html_header");

    DBUG_PRINT(3,(LOGF,"l_vars->do_fits = %d \n",l_vars->do_fits));    
    
    /* Switch on MIME type */
    switch(mc_type)
      {
      case TEXT_HTML :
          /* Prepare response as MIME type text/HTML */

	  DBUG_PRINT(3,(LOGF,"MIME_TYPE: TEXT_HTML\n"));  
 
          if (!l_vars->do_dump && !l_vars->do_cci)
            { /* Don't need this for dumping or CCI */
                fprintf(fhandle, "Content-type: text/html%c%c",10,10);

                fprintf(fhandle, "<TITLE>Scientific Data Browser</TITLE>\n");
                fprintf(fhandle, "<H1>Scientific Data Browser</H1>\n");
                fprintf(fhandle, "<hr>\n");

                if(!strcmp(l_vars->h_env->request_method,"GET"))
                  {
                      if (l_vars->do_fits)
                        {
                            strcpy(newScript, (l_vars->h_env)->script_name);

                            fprintf(fhandle,"To see the FITS header information from  <I>%s</I>,  click <A HREF=\"%s%s?%s!listhead\"> <b>here</b></A><p>\n",             \
                                    l_vars->f_name,                     \
                                    newScript,l_vars->h_env->path_info, \
                                    l_vars->f_name);
#ifdef HAVE_FITS2HDF
                            strcpy(tmp, (l_vars->h_env)->script_name);
                            strcpy(newScript, (char *)getNewScript(tmp, "fits2hdf"));

                            fprintf(fhandle,"This FITS file can be converted to HDF. Click <A HREF=\"%s%s?%s\"> <b>here</b> </A> to see the HDF version of this file <p>\n", 
                                    newScript,l_vars->h_env->path_info,l_vars->f_name);
#endif
              
                        }
                      else
                        {
                            fprintf(fhandle,"This info came from <A HREF=\"%s/%s\"> %s </A><p>",
                                    l_vars->h_env->path_info,l_vars->f_name,l_vars->f_name);
                        }
                  
                  }
                else if(!strcmp(l_vars->h_env->request_method,"POST"))
                  {
                      if(l_vars->do_fits)
                        {
                            if(l_vars->hdf_path_r != NULL) 
                              {

                                  strcpy(newScript, (l_vars->h_env)->script_name);
                                  fprintf(fhandle,"To see the FITS header information from  <I>%s</I>, click <A HREF=\"%s%s?%s!listhead\"> <b>here</b></A><p>\n",         \
                                          l_vars->f_name,                     \
                                          newScript,l_vars->hdf_path_r,      \
                                          l_vars->f_name);
#ifdef HAVE_FITS2HDF
                                  strcpy(tmp, (l_vars->h_env)->script_name);
                                  strcpy(newScript, (char *)getNewScript(tmp, "fits2hdf"));
                                  fprintf(fhandle,"This FITS file can be converted to HDF. Click <A HREF=\"%s%s?%s\"> <b>here</b> </A> to see the HDF version of this file <p>\n", 
                                          newScript,l_vars->hdf_path_r,l_vars->f_name);
#endif
                              }
                            else 
                              {
                                  strcpy(newScript, (l_vars->h_env)->script_name);
                                  fprintf(fhandle,"To see the FITS header from  <I>%s</I>, click <A HREF=\"%s%s?%s!listhead\"> <b>here</b></A><p>\n",         \
                                          l_vars->f_name,                     \
                                          newScript,l_vars->h_env->path_info,      \
                                          l_vars->f_name);
#ifdef HAVE_FITS2HDF
                                  strcpy(tmp, (l_vars->h_env)->script_name);
                                  strcpy(newScript, (char *)getNewScript(tmp, "fits2hdf"));
                                  fprintf(fhandle,"This FITS file can be converted to HDF. Click <A HREF=\"%s%s?%s\"> <b>here</b> </A> to see the HDF version of this file <p>\n", 
                                          newScript,l_vars->f_path_r,l_vars->f_name);
#endif


                              }
                        }
                      else
                        {
                            fprintf(fhandle,"This info came from <A HREF=\"%s/%s\"> %s </A><p>",
                                    l_vars->h_env->path_info,l_vars->f_name,l_vars->f_name);
#if 0
                            if(l_vars->hdf_path_r != NULL)
                                fprintf(fhandle,"This info came from <A HREF=\"%s/%s\"> %s </A><p>",
                                        l_vars->hdf_path_r,l_vars->f_name,l_vars->f_name);
                            else
                                fprintf(fhandle,"This info came from <A HREF=\"%s/%s\"> %s </A><p>",
                                        l_vars->f_path_r,l_vars->f_name,l_vars->f_name);
#endif
                        }
                  }
            }
          /* fprintf(fhandle, "<hr>\n"); */
          break;
      case IMAGE_GIF :
          /* Prepare response as MIME type image/gif */
 
          printf("Content-type: image/gif%c%c",10,10);
	  DBUG_PRINT(3,(LOGF,"MIME_TYPE: IMAGE/GIF\n"));  
          break;
      default:
          break;
      }
     DBUG_PRINT(3,(LOGF,"RET_VALUE = %d\n", ret_value));  
     EXIT(2,"write_html_header");
   
    return ret_value;
} /* write_html_header */

/*-------------------------------------------------------------------- 
 NAME
     write_html_trailer
 DESCRIPTION
     Write trailer stuff at end of document

 RETURNS
     Return SUCCEED if successful and FAIL otherwise.
--------------------------------------------------------------------*/ 
int
write_html_trailer(FILE *fhandle,             /* Output stream */
                   mime_content_type mc_type, /* MIME type */
                   lvar_st *l_vars)
{
    int ret_value = SUCCEED;

    /* Switch on MIME type */
    switch(mc_type)
      {
      case TEXT_HTML :
          /* Prepare response */
          fprintf(fhandle, "<hr>\n");
          if (l_vars->do_fits)
            {
                fprintf(fhandle, "<ADDRESS>\n");
                fprintf(fhandle, "<A HREF=\"http://hdf.ncsa.uiuc.edu/fits/\"> HDF-FITS Conversion Page </a>\n");
                fprintf(fhandle, "</ADDRESS>\n");
            }

          break;
      case IMAGE_GIF :
          /* dont' do anything for gif */

          break;
      default:
          break;
      }
    return ret_value;
} /* write_html_trailer */

/*-------------------------------------------------------------------- 
 NAME
     recieve_request - handles each request     
 DESCRIPTION
     Does nothing for now

 RETURNS
     
--------------------------------------------------------------------*/ 
int
recieve_request(FILE *shandle, 
                char *request, 
                lvar_st *l_vars)
{
    int i;

    return SUCCEED;
} /* recieve_request */

/*----------------------------------------------------------------------------
 NAME
      send_reply
 DESCRIPTION
      Send the HTML description or GIF image to the client 
 RETURNS
      Returns SUCCEED if successful and FAIL otherwise.
----------------------------------------------------------------------------*/
int
send_reply(FILE *shandle,             /* Stream handle to send data output */
           char *fname,               /* file name to read data from */
           mime_content_type mc_type, /* MIME type for reply */
           lvar_st *l_vars)
{
    int i;
    int len;        /* length of data */
    int  rlen;
    int  data_left;
    int  buf_size = SND_BUF_SIZE;  /* size of sending buffer */
    int  num_reps;
    char *data = NULL;     /* data to send to client */
    FILE *fptr = NULL;     /* pointer to html/gif file */
    char uri[1024]; /* Univeral Resource Identifier */
    int  ret_value = SUCCEED;

    ENTER(2,"send_reply");
    DBUG_PRINT(1,(LOGF,"  fname=%s\n",fname));

    /* Check arguements */
    if (shandle == NULL || fname == NULL ) 
      {
          gateway_err(shandle,"send_reply: filename is NULL",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

#if 0
#ifdef HAVE_CCI
    if(l_vars->do_cci)
      {
          sprintf(uri,"file://localhost%s",fname);
          DBUG_PRINT(1,(LOGF,"  uri=%s\n",uri));
          if (MCCIGet(l_vars->h_cci_port,uri,MCCI_DEFAULT,MCCI_ABSOLUTE,NULL) != MCCI_OK)
              gateway_err(shandle,"send_reply: sending URL to Mosaic",1,l_vars);

          ret_value = SUCCEED;
          goto done;
      }
#endif /* !HAVE_CCI */
#endif

    /* Open file for reading */
    fptr = fopen(fname, "r");
    if (fptr != NULL) 
      {
#ifdef _POSIX_SOURCE
          /* Lets get file information using "stat()" 
           * can't use fstat() since we need filedesc instead */
          {
              struct stat stat_buf; /* buffer for file status information */
              if (stat(fname, &stat_buf) == 0)
                { /* Get file size for now */
                    len = stat_buf.st_size;
                }
          }
#else  /* !_POSIX_SOURCE */
          /* Find the length of the file the really cheesy way! */
          fseek(fptr, 0L, 0);
          fseek(fptr, 0L, 2);
          len = ftell(fptr);
          fseek(fptr, 0L, 0);
#endif /* !_POSIX_SOURCE */

          if(l_vars->do_cci)
            {
#ifdef HAVE_CCI
                /* Allocate space for file data */
                if ((data = HDgetspace((len + 1) * sizeof(unsigned char))) == NULL)
                  {
                      gateway_err(shandle,"send_reply: space could not be allocated",0,l_vars);
                      ret_value =FAIL;
                      goto done;
                  }

                /* read the data and null terminate*/
                len = fread(data, sizeof(char), len, fptr);
                data[len] = '\0';

                /* Now we want to send the Form to Mosaic for display */
                MCCIDisplay(l_vars->h_cci_port,
                            "No_URL", /* URL that should be displayed in Mosaic*/
                            "text/html", /* the data type being sent for display*/
                            data,             /* the data to be displayed*/
                            len,     /* length of data to display */
                            MCCI_OUTPUT_CURRENT); /* diplay this data in 
                                                     current Mosaic window */
#endif
            }
          else /* not CCI */
            {
                num_reps = len / buf_size;  /* number of loop iterations */
                data_left = len % buf_size; /* odd size left to read */

                if ((data = HDgetspace(buf_size * sizeof(unsigned char))) == NULL)
                  {
                      gateway_err(shandle,"send_reply: space could not be allocated",0,l_vars);
                      ret_value = FAIL;
                      goto done;
                  }

                for ( i = 0; i < num_reps; i++)
                  { /* read data from file and send back to client */
                      rlen = fread(data, sizeof(char), buf_size, fptr);
                      fwrite(data, sizeof(char), rlen, shandle); 
                  }

                if (data_left != 0)
                  {
                      rlen = fread(data, sizeof(char), data_left, fptr);
                      fwrite(data, sizeof(char), rlen, shandle); 
                  }
            }

          /* close and remove temproary file */
          fclose(fptr);	 
          remove(fname);

          /* Switch on MIME type */
          switch(mc_type)
            {
            case TEXT_HTML :
                DBUG_PRINT(1,(LOGF,"  case TEXT_HTML\n"));
                DBUG_PRINT(4,(LOGF,"HTML description = %s \n", (char *)data));

#ifdef OLD_WAY
                /* Send HTML description back to client */

                fprintf(shandle, "%s",data);
#endif
                break;
            case IMAGE_GIF :
                DBUG_PRINT(1,(LOGF,"  case IMAGE_GIF\n"));

                /* Send gif image back to client */
#ifdef OLD_WAY
                fwrite(data, sizeof(char), len, shandle); 
#endif
                break;
            default:
                DBUG_PRINT(1,(LOGF,"  case default\n"));

                break;
            } /* end switch mime */
      } /* ftpr != NULL */
    else
      {
          gateway_err(shandle,"send_reply: file could not be opened",0,l_vars);
          ret_value = FAIL;
          goto done;
      }

done:
    if (ret_value == FAIL)
      {
      }

    /* free buffer space */
    if (data != NULL)
        HDfree(data);

    EXIT(2,"send_reply");
    return ret_value;
} /* send_reply */


/*----------------------------------------------------------------------------
 NAME
      pull_filename
 DESCRIPTION
      Get hdf file name from query string and place in variable 'l_vars->f_name'.
 RETURNS
      Returns SUCCEED if successful and FAIL otherwise.
----------------------------------------------------------------------------*/
int
pull_filename(char *target,    /* string to extract variables from */
              lvar_st *l_vars)
{
    char *bptr;    /* pointer to last backslash i.e. before file name */
    int ret_value = SUCCEED;

    ENTER(2,"pull_filename");
    DBUG_PRINT(1,(LOGF," target=%s \n", target));

    if (target)
      {
          /* get base of target */
          if ((bptr = (char *)base_name(target,'/')) == NULL)
            {
              ret_value = FAIL; 
              goto done;
            }
          DBUG_PRINT(1,(LOGF," basename=%s \n", bptr));

          /* get file name minus extra stuff tagged on at end */
          if ((l_vars->f_name = (char *)path_name(bptr,'!')) == NULL)
            {
              ret_value = FAIL;   
              goto done;
            }

          /* check for no path */
          if (!strcmp(l_vars->f_name,"."))
              FREE_CLEAR_SET(l_vars->f_name, (char *)mk_compound_str(1, bptr));
          DBUG_PRINT(1,(LOGF," f_name=%s \n", l_vars->f_name));

          /* get relative path from target */
          if ((l_vars->f_path_r = (char *)path_name(target,'/')) == NULL)
            {
              ret_value = FAIL;   
              goto done;
            }
          DBUG_PRINT(1,(LOGF," f_path_r=%s \n", l_vars->f_path_r));
      }
    else
        return FAIL;

  done:
    EXIT(2,"pull_filename");
    return ret_value;
} /* pull_filename */

/*----------------------------------------------------------------------------
 NAME
      pull_ref
 DESCRIPTION
      Pull HDF tag/ref from target and place in global variable "l_vars->hdf_ref".
 RETURNS
      Returns SUCCEED if successful and FAIL otherwise.
----------------------------------------------------------------------------*/
int 
pull_ref (char *target,    /* string to extract variables from */
          lvar_st *l_vars)
{
    char *sptr;    /* pointer to seperator */
    char *hptr;    /* pointer to hdf ref */
    int  tlen;     /* target string length */
    int  ret_value = SUCCEED;

    int  ref;

    ENTER(2,"pull_ref");
    DBUG_PRINT(1,(LOGF," target=%s \n", target));

    if (target)
      {
          /* Calculate offset pointers of hdfref in target */
          if ((sptr = strrchr(target,'!')) == NULL)
            {
              ret_value = FAIL;
              goto done;
            }

          /* If the first seven characters don't match "hdfref;", then we know
             it's not a target. */
          if (!strncmp (sptr+1, "hdfref;", 7))
            {/* offset pointers */
                hptr = sptr + 8;
                tlen = (target+strlen(target)) - hptr;   /* hdfref length */

                /* copy hdfref to global buffer */
                if (tlen != 0 && hptr[0] != '\0')
                  {
                      if ((l_vars->hdf_ref = (char *)mk_compound_str(1, hptr)) == NULL)
                          return FAIL;
                      l_vars->hdf_ref[tlen] = '\0';
                  }
                else
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                DBUG_PRINT(1,(LOGF,"  hdfref =%s \n", l_vars->hdf_ref));

            }
          else if (!(strncmp (sptr+1, "sdbref;", 7))) 
            {
                /* offset pointers */
                hptr = sptr + 8;
                tlen = (target+strlen(target)) - hptr;   /* sdbref length */

                /* copy sdbref to global buffer */
                if (tlen != 0 && hptr[0] != '\0')  
                  {
                      if ((l_vars->hdf_ref = (char *)mk_compound_str(1, hptr)) == NULL)
                        {
                          ret_value = FAIL;
                          goto done;
                        }
                      l_vars->hdf_ref[tlen] = '\0';
                  }
                else  
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                DBUG_PRINT(1,(LOGF,"  sdbref =%s \n", l_vars->hdf_ref));

                ret_value = SDBREF;
            }
          else if (!(strncmp (sptr+1, "sdbplane;", 9))) 
            {
                /* offset pointers */
                hptr = sptr + 10;
                tlen = (target+strlen(target)) - hptr;   /* sdbplane length */

                /* copy sdbplane to global buffer */
                if (tlen != 0 && hptr[0] != '\0')  
                  {
                      if ((l_vars->hdf_ref=(char *)mk_compound_str(1,hptr))==NULL)
                        {
                          ret_value = FAIL;
                          goto done;
                        }
                      l_vars->hdf_ref[tlen] = '\0';
		  
                      if (sscanf(l_vars->hdf_ref, "plane=%d", &l_vars->plane)!=1)
                        {
			  if (sscanf(l_vars->hdf_ref, "ref=%d,plane=%d", 
				     &ref, &l_vars->plane)!=2) {

                          ret_value = FAIL;
                          goto done;
			  }
                        }
                  }
                else  
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                DBUG_PRINT(1,(LOGF,"  plane =%d \n", l_vars->plane));
                
                ret_value = SDBPLANE;
            }
          else if (!(strncmp (sptr+1, "sdbview;", 8))) 
            {
                /* offset pointers */
                hptr = sptr + 9;
                tlen = (target+strlen(target)) - hptr;  /* sdbview length */
		  
                /* copy sdbview to global buffer */
                if (tlen != 0 && hptr[0] != '\0')  
                  {
                      if ((l_vars->hdf_ref=(char *)mk_compound_str(1,hptr))==NULL)
                        {
                          ret_value = FAIL;
                          goto done;
                        }
                      l_vars->hdf_ref[tlen] = '\0';
                  }
                else  
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                DBUG_PRINT(1,(LOGF,"  plane =%d \n", l_vars->plane));
                ret_value = SDBVIEW;
            }
          else if (!(strncmp (sptr+1, "fitstab;", 8))) 
            {
                /* offset pointers */
                hptr = sptr + 9;
                tlen = (target+strlen(target)) - hptr;  /* fitstab length */
		  
                /* copy fitstab to global buffer */
                if (tlen != 0 && hptr[0] != '\0')  
                  {
                      if ((l_vars->hdf_ref=(char *)mk_compound_str(1,hptr))==NULL)
                        {
                          ret_value = FAIL;
                          goto done;
                        }
                      l_vars->hdf_ref[tlen] = '\0';
                  }
                else  
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                ret_value = FITSTAB;
            }
          else if (!(strncmp (sptr+1, "fitsbintab;", 11))) 
            {
                /* offset pointers */
                hptr = sptr + 12;
                tlen = (target+strlen(target)) - hptr; /* fitstab length */
		  
                /* copy fitstab to global buffer */
                if (tlen != 0 && hptr[0] != '\0')  
                  {
                      if ((l_vars->hdf_ref=(char *)mk_compound_str(1,hptr))==NULL)
                        {
                          ret_value = FAIL;
                          goto done;
                        }
                      l_vars->hdf_ref[tlen] = '\0';
                  }
                else  
                  {
                      ret_value = FAIL;
                      goto done;
                  }
                ret_value = FITSBINTAB;
            }
          else if (!(strncmp (sptr+1, "listhead", 8))) 
            {
                ret_value = LISTHEAD;
            }
          else   if (!(strncmp (sptr+1, "head", 4))) 
            {
                ret_value = HEAD;
            }
          else if (!(strncmp (sptr+1, "history", 7))) 
            {
                ret_value= HISTORY;
            }
          else
            { 
                /* It's not an hdfref; we don't know what the hell it is. */
                ret_value = FAIL;
            }
      }
    else
        ret_value = FAIL;

done:
    EXIT(2,"pull_ref");
    return ret_value;
} /* pull_ref */

/*----------------------------------------------------------------------------
 NAME
      pull_dtmport
 DESCRIPTION
      
 RETURNS
      Returns SUCCEED if successful and FAIL otherwise.
----------------------------------------------------------------------------*/
int
pull_dtmport(char *target,    /* string to extract variables from */
             lvar_st *l_vars)
{
    char dtmport[40] = {""};
    int  s = -1;
    int  tag = -1;
    int  ref = -1;
    char *cptr = NULL;
    int num = 0;
    int ret_value = SUCCEED;

    ENTER(2,"pull_dtmport");
    DBUG_PRINT(1,(LOGF," target=%s \n", target));

    if (target)
      {
          if ((cptr = strchr(target,'!')) != NULL)
            {
          DBUG_PRINT(1,(LOGF," cptr=%s \n", cptr));
          /* get base of string */
          if ((num = sscanf(cptr,"!hdfref;tag=%d,ref=%d,s=%d,dtmport=%s",
                     &tag,&ref,&s,dtmport)) != 4)
            {
              ret_value = FAIL; 
              goto done;
            }
          DBUG_PRINT(1,(LOGF," dtmport=%s \n", dtmport));
          if ((l_vars->dtm_outport = (char *)HDmalloc(sizeof(char)*(HDstrlen(dtmport)+1))) == NULL)
            {
              ret_value = FAIL; 
              goto done;
            }
          HDstrcpy(l_vars->dtm_outport,dtmport);
          DBUG_PRINT(1,(LOGF," l_vars->dtm_outport=%s \n", 
                        l_vars->dtm_outport));
            }
      }
    else
        return FAIL;

  done:
    DBUG_PRINT(1,(LOGF," num=%d \n", num));
    EXIT(2,"pull_dtmport");
    return ret_value;
} /* pull_dtmport */

/*-------------------------------------------------------------------- 
 NAME
     get_type
 DESCRIPTION
     Used to get string descritpion of HDF number type.
 RETURNS
     Return a string description of the given number type
     if successful else return default string.
--------------------------------------------------------------------*/
char *
get_type(int32 nt /* Number type */ )
{   /* Switch on number type "nt" */
    switch(nt) {
    case DFNT_CHAR   : return("8-bit characters"); break;
    case DFNT_INT8   : return("signed 8-bit integers"); break;
    case DFNT_UINT8  : return("unsigned 8-bit integers"); break;
    case DFNT_INT16  : return("signed 16-bit integers"); break;
    case DFNT_UINT16 : return("unsigned 8-bit integers"); break;
    case DFNT_INT32  : return("signed 32-bit integers"); break;
    case DFNT_UINT32 : return("unsigned 32-bit integers"); break;
    case DFNT_FLOAT32  : return("32-bit floating point numbers"); break;
    case DFNT_FLOAT64  : return("64-bit floating point numbers"); break;
    case DFNT_NCHAR   : return("native 8-bit characters"); break;
    case DFNT_NINT8   : return("native signed 8-bit integers"); break;
    case DFNT_NUINT8  : return("native unsigned 8-bit integers"); break;
    case DFNT_NINT16  : return("native signed 16-bit integers"); break;
    case DFNT_NUINT16 : return("native unsigned 8-bit integers"); break;
    case DFNT_NINT32  : return("native signed 32-bit integers"); break;
    case DFNT_NUINT32 : return("native unsigned 32-bit integers"); break;
    case DFNT_NFLOAT32  : return("native 32-bit floating point numbers"); break;
    case DFNT_NFLOAT64  : return("native 64-bit floating point numbers"); break;
    case DFNT_LCHAR   : return("little-endian 8-bit characters"); break;
    case DFNT_LINT8   : return("little-endian signed 8-bit integers"); break;
    case DFNT_LUINT8  : return("little-endian unsigned 8-bit integers"); break;
    case DFNT_LINT16  : return("little-endian signed 16-bit integers"); break;
    case DFNT_LUINT16 : return("little-endian unsigned 8-bit integers"); break;
    case DFNT_LINT32  : return("little-endian signed 32-bit integers"); break;
    case DFNT_LUINT32 : return("little-endian unsigned 32-bit integers"); break;
    case DFNT_LFLOAT32  : return("little-endian 32-bit floating point numbers"); break;
    case DFNT_LFLOAT64  : return("little-endian 64-bit floating point numbers"); break;
    default : return("unknown number type");
    } /* switch */
} /* get_type */

/*-------------------------------------------------------------------- 
 NAME
     get_atype
 DESCRIPTION
     Used to get string descritpion of annotation type.
 RETURNS
     Return a string description of the given annotation type
     if successful else return default string.
--------------------------------------------------------------------*/
char *
get_atype(ann_type atype /* annotation type */)
{   /* Switch on annotation type "atype" */
    char *ann_desc;

    switch(atype) 
      {
      case AN_FILE_LABEL: ann_desc = "File Label";       break;
      case AN_FILE_DESC:  ann_desc = "File Description"; break;
      case AN_DATA_LABEL: ann_desc = "Data Label";       break;
      case AN_DATA_DESC:  ann_desc = "Data Description"; break;
      default: ann_desc = "unknown annotation type";
      } /* switch */
    return ann_desc;
} /* get_atype */

/*-------------------------------------------------------
 NAME
      buffer_to_string
 DESCRIPTION
       Converts the incoming buffer data in "tbuff" to its 
       equivalent ascii form in the form of a large string
       using its number type.
 RETURNS
       Return a string dump of buffer "tbuff" if succesful
       and NULL otherwise.
 WARNINGS
       FREES the incoming buffer !!!!!!!!!!!!!!!!!! 
--------------------------------------------------------*/
char *
buffer_to_string(char * tbuff,  /* Buffer to be converted */
                 int32 nt,      /* Number type of data in buffer */
                 int32 count    /* Size of buffer */)
{
    intn i;        /* Loop variable */
    char * buffer = NULL; /* Conversion buffer */
    char *ret_value = NULL;
    
    ENTER(2,"buffer_to_string");
    DBUG_PRINT(2,(LOGF, "count=%d, nt=%s \n", count,get_type(nt)));
    /* If it is a character buffer, NULL terminate it 
     * I believe size of "tbuff" is 1 greater than data size */
    if(nt == DFNT_CHAR) 
      {
          DBUG_PRINT(2,(LOGF, "Number type is CHAR \n"));
          tbuff[count] = '\0';
          ret_value = tbuff;
          goto done;
      }

    /* Hmm. we are allocating space for conversion buffer but 
     * where does the hard coded 80 fit in? */
    if ((buffer = (char *) HDgetspace(80 * count)) == NULL)
      {
        ret_value = NULL;
        goto done;
      }

    buffer[(80*count)-1] = '\0';  /* Null terminate buffer */

    /* Each element will comma seperated in the conversion buffer */
    switch(nt) 
      {
      case DFNT_INT8   : 
          sprintf(buffer, "%d", ((int8 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %d", buffer, ((int8 *)tbuff)[i]);
          break;
      case DFNT_UINT8  : 
          sprintf(buffer, "%u", ((uint8 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %u", buffer, ((uint8 *)tbuff)[i]);
          break;
      case DFNT_INT16   : 
          sprintf(buffer, "%d", ((int16 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %d", buffer, ((int16 *)tbuff)[i]);
          break;
      case DFNT_UINT16  : 
          sprintf(buffer, "%u", ((uint16 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %u", buffer, ((uint16 *)tbuff)[i]);
          break;
      case DFNT_INT32   : 
          sprintf(buffer, "%d", ((int32 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %d", buffer, ((int32 *)tbuff)[i]);
          break;
      case DFNT_UINT32  : 
          sprintf(buffer, "%u", ((uint32 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %u", buffer, ((uint32 *)tbuff)[i]);
          break;
      case DFNT_FLOAT32 : 
          sprintf(buffer, "%f", ((float32 *)tbuff)[0]);
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %f", buffer, ((float32 *)tbuff)[i]);
          break;
      case DFNT_FLOAT64 : 
          sprintf(buffer, "%f", ((float64 *)tbuff)[0]);        
          for(i = 1; i < count; i++)
              sprintf(buffer, "%s, %f", buffer, ((float64 *)tbuff)[i]);
          break;
      }

    DBUG_PRINT(4,(LOGF," coverted buffer =%s \n", buffer)); 

    /* Clean up, free buffer passed in */
    if(tbuff != NULL)
        HDfreespace((void *)tbuff);

    ret_value = buffer;

done:

    EXIT(2,"buffer_to_string");
    return ret_value;
} /* buffer_to_string */


/*-------------------------------------------------------------------- 
 NAME
     string_length
 DESCRIPTION
     

 RETURNS
     
--------------------------------------------------------------------*/ 
int32 
string_length(char *name)
{
    char t_name[120], *ptr1, *ptr2;

    strcpy(t_name, name);
    ptr1 = (char *) strrchr(t_name, '"');
    ptr2 = (char *) strrchr(t_name, '<');
    ptr1 = ptr1 + 2; 
    *ptr2 = '\0';
    return(strlen(ptr1));
} /* string_length() */

