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

#ifndef UTIL_H
#define UTIL_H

/*-----------------procedures defined in 'util.c'--------------------- */ 
IMPORT void getword(char *word, char *line, char stop);
IMPORT char *makeword(char *line, char stop);
IMPORT char *fmakeword(FILE *f, char stop, int *len);
IMPORT void getword(char *word, char *line, char stop);
IMPORT char x2c(char *what);
IMPORT void unescape_url(char *url);
IMPORT void plustospace(char *str);
IMPORT int rind(char *s, char c);
IMPORT int getline(char *s, int n, FILE *f);
IMPORT void send_fd(FILE *f, FILE *fd);
IMPORT int ind(char *s, char c); 
IMPORT void escape_shell_cmd(char *cmd); 
#endif /* UTIL_H */
