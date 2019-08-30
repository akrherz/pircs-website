/* fitsbrowser.h

 */

#ifndef FITSBROWSER_H

#define FITSBROWSER_H

#define SUCCEED  0
#define FAIL    -1


typedef struct {
    char name[128];
    char val[128];
} entry;

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
} hserv_st;

#endif


