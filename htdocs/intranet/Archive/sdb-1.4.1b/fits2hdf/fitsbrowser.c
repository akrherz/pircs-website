
#include <stdio.h>

#ifndef NO_STDLIB_H
#include <stdlib.h>
#else
char *getenv();
#endif

#include <string.h>

#include "fitsbrowser.h"

void getword(char *word, char *line, char stop);
char x2c(char *what);
void unescape_url(char *url);
void plustospace(char *str);

/*--------------------------------------------------------------------
 NAME
     get_env
 DESCRIPTION
     Gets the enviromental variables that might be useful to us

 RETURNS

--------------------------------------------------------------------*/
void

#ifdef __STDC__
get_env(hserv_st *s_env )
#else
get_env(s_env )
hserv_st *s_env;
#endif
{
  /* Get relavant enironment variables passed to us by httpd server */
  s_env->request_method  = (char *)getenv("REQUEST_METHOD");
  s_env->path_info       = (char *)getenv("PATH_INFO");
  s_env->path_translated = (char *)getenv("PATH_TRANSLATED");
  s_env->script_name     = (char *)getenv("SCRIPT_NAME");
  s_env->query_string    = (char *)getenv("QUERY_STRING");
  s_env->remote_host     = (char *)getenv("REMOTE_HOST");
  s_env->remote_addr     = (char *)getenv("REMOTE_ADDR");
  s_env->auth_type       = (char *)getenv("AUTH_TYPE");
  s_env->remote_user     = (char *)getenv("REMOTE_USER");
  s_env->content_type    = (char *)getenv("CONTENT_TYPE");
  s_env->content_length  = (char *)getenv("CONTENT_LENGTH");
  s_env->http_accept     = (char *)getenv("HTTP_ACCEPT");
  s_env->http_user_agent = (char *)getenv("HTTP_USER_AGENT");

} /* get_env() */

/*----------------------------------------------------------------------------
 NAME
      getNewScript(oldScript, fileName)
 DESCRIPTION
      get new script name from fileName.
.
 RETURNS
      Returns new script name if successful and FAIL otherwise.
----------------------------------------------------------------------------*/
char *
#ifdef __STDC__
getNewScript(char *oldScript, char *fileName)
#else
pull_fitsref (oldScript, fileName)
char *oldScript;
char *fileName;
#endif
{
    int   i;
    char  *tmpStr;

    tmpStr = (char *)malloc(50);

    for (i=strlen(oldScript); oldScript[i] != '/'; i--)
        ;
    oldScript[i+1] ='\0';

    strcpy(tmpStr, oldScript);
    strcat(tmpStr, fileName);

    return(tmpStr);
}

int  MAIN_;

void 
#ifdef __STDC__
main(int argc, char *argv[])
#else
main(argc, argv)
int argc;
char *argv[];
#endif

{
    entry entries[100];
    register int x,m=0;
    char *cl, *path_trans;
    static char inputfile[100];
    static char outputfile[100];
    char tmpStr[100];
    int  status;
    int i;
   
    hserv_st serv_env;      /* Server environemnt variables */
   
    /* get Server Environment variables */
    get_env(&serv_env);  

    printf("Content-type: text/html%c%c",10,10);
  
    if(strcmp(getenv("REQUEST_METHOD"),"GET")) {     
        printf("This script should be referenced with a METHOD of GET.\n");
        printf("If you don't understand this, see this ");
        printf("<A HREF=\"http://www.ncsa.uiuc.edu/SDG/Software/Mosaic/Docs/fill-out-forms/overview.html\">forms overview</A>.%c",10);
        exit(1);
    }

    cl = getenv("QUERY_STRING");
 
    if(cl == NULL) {
        printf("No query information to decode.\n");
        exit(1);
    }


    for(x=0;cl[0] != '\0';x++) {
        m=x;
        getword(entries[x].val,cl,'&');
        plustospace(entries[x].val);
        unescape_url(entries[x].val);
        getword(entries[x].name,entries[x].val,'=');
    }
   
    printf("<html>\n");
    printf("<head>\n");

    printf("<title>FITS Data Browser</title>\n");
    printf("</head>\n");

    printf("<body>\n");

    printf("<H2>FITS Data Browser</H2>\n");
    /*
    printf("<HR>\n");
    printf("<pre> The following message is from FITS-HDF Converter. Please check \n");
    printf("your FITS file if the error message is comming. <p>\n");
    */
 
    path_trans = (char *)serv_env.path_translated; 

    for (i=0; i<strlen(path_trans); i++) {
	inputfile[i]  = path_trans[i];
	outputfile[i] = path_trans[i];
    }
    inputfile[i]  = '\0';
    outputfile[i] = '\0';

    strcat(inputfile,"/");
    strcat(inputfile,entries[0].name);
   
    strcpy(outputfile, (char *)getNewScript(outputfile,"tmp/tmp.hdf"));

    status = fits2sds(inputfile, outputfile); 
     
    if (!status) {
	        strcpy(tmpStr, serv_env.path_info);
        	strcpy(tmpStr, (char *)getNewScript(tmpStr, "tmp?tmp.hdf"));
					    
	printf("<HR>\n");
	printf(" <h2>FITS file has been converted to <A HREF=\"/cgi-bin/hdf%s\"> HDF</A> file </h2>\n", tmpStr);
	/* printf("Currently only Primary array and Image extension in the FITS file can be converted.\n"); */

    }
    else {
	printf("\nFITS file can not be converted, please check your fits file <p> \n");
 	printf("and the output file path(%s) \n", outputfile);
    }
    printf("<HR>\n");
    printf("<ADDRESS>\n");
    printf("<A HREF=\"http://hdf.ncsa.uiuc.edu/fits/\"> HDF-FITS Conversion Page </a>\n");
    printf("</ADDRESS>\n");
    
    printf("</Body>\n");
    printf("</HTML>\n");

}
