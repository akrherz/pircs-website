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
#ifdef RCSID
static char RcsId[] = "@(#)util.c,v 1.6 1996/04/15 17:58:30 georgev Exp";
#endif


#include <stdio.h>
#include "sdb.h"
#include "util.h"

#define LF 10
#define CR 13

#ifdef __STDC__
void getword(char *word, char *line, char stop) 
#else
void getword(word, line, stop) 
char *word;
char *line;
char stop;
#endif
{
    int x = 0,y;

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while(line[y++] = line[x++]);
}

#ifdef __STDC__
char *makeword(char *line, char stop) 
#else
char *makeword(line, stop) 
char *line;
char stop;
#endif
{
    int x = 0,y;
    char *word = (char *) malloc(sizeof(char) * (strlen(line) + 1));

    for(x=0;((line[x]) && (line[x] != stop));x++)
        word[x] = line[x];

    word[x] = '\0';
    if(line[x]) ++x;
    y=0;

    while(line[y++] = line[x++]);
    return word;
}

#ifdef __STDC__
char *fmakeword(FILE *f, char stop, int *cl) 
#else
char *fmakeword(f, stop, cl) 
FILE *f;
char stop;
int *cl;
#endif
{
    int wsize;
    char *word;
    int ll;

    wsize = 102400;
    ll=0;
    word = (char *) malloc(sizeof(char) * (wsize + 1));

    while(1) {
        word[ll] = (char)fgetc(f);
        if(ll==wsize) {
            word[ll+1] = '\0';
            wsize+=102400;
            word = (char *)realloc(word,sizeof(char)*(wsize+1));
        }
        --(*cl);
        if((word[ll] == stop) || (feof(f)) || (!(*cl))) {
            if(word[ll] != stop) ll++;
            word[ll] = '\0';
            return word;
        }
        ++ll;
    }
}

#ifdef __STDC__
char x2c(char *what) 
#else
char x2c(what) 
char *what;
#endif
{
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

#ifdef __STDC__
void unescape_url(char *url) 
#else
void unescape_url(url) 
char *url;
#endif
{
    register int x,y;

    for(x=0,y=0;url[y];++x,++y) {
        if((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

#ifdef __STDC__
void plustospace(char *str) 
#else
void plustospace(str) 
char *str;
#endif
{
    register int x;

    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

#ifdef __STDC__
int rind(char *s, char c) 
#else
int rind(s, c) 
char *s;
char c;
#endif
{
    register int x;
    for(x=strlen(s) - 1;x != -1; x--)
        if(s[x] == c) return x;
    return -1;
}

#ifdef __STDC__
int getline(char *s, int n, FILE *f) 
#else
int getline(s, n, f) 
char *s;
int n;
FILE *f;
#endif
{
    register int i=0;

    while(1) {
        s[i] = (char)fgetc(f);

        if(s[i] == CR)
            s[i] = fgetc(f);

        if((s[i] == 0x4) || (s[i] == LF) || (i == (n-1))) {
            s[i] = '\0';
            return (feof(f) ? 1 : 0);
        }
        ++i;
    }
}

#ifdef __STDC__
void send_fd(FILE *f, FILE *fd)
#else
void send_fd(f, fd)
FILE *f;
FILE *fd;
#endif
{
    int num_chars=0;
    char c;

    while (1) {
        c = fgetc(f);
        if(feof(f))
            return;
        fputc(c,fd);
    }
}

#ifdef __STDC__
int ind(char *s, char c) 
#else
int ind(s, c) 
char *s;
char c;
#endif
{
    register int x;

    for(x=0;s[x];x++)
        if(s[x] == c) return x;

    return -1;
}

#ifdef __STDC__
void escape_shell_cmd(char *cmd) 
#else
void escape_shell_cmd(cmd) 
char *cmd;
#endif
{
    register int x,y,l;

    l=strlen(cmd);
    for(x=0;cmd[x];x++) {
        if(ind("&;`'\"|*?~<>^()[]{}$\\",cmd[x]) != -1){
            for(y=l+1;y>x;y--)
                cmd[y] = cmd[y-1];
            l++; /* length has been increased */
            cmd[x] = '\\';
            x++; /* skip the character */
        }
    }
}

