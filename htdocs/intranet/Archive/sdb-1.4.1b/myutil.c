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

/**********************************************************************
 * myutil.c - useful routines and other stuff...
 *
 * Author GeorgeV.
 * *********************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)myutil.c,v 1.5 1996/04/15 17:57:34 georgev Exp";
#endif

#include <stdio.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "myutil.h"


/*
 * This returns a compound string created
 * from strings given. We assume each string is NULL terminated.
 */
#ifdef __STDC__
char *mk_compound_str(int nstr, ...)
#else
char *mk_compound_str(nstr, va_alist)
     int nstr;    /* number of strings */
     va_dcl
#endif
{
  va_list  pargs;
  register unsigned str_len = 0;
  register unsigned i;
  register char *sptr = NULL;
  register char *cptr = NULL;
  char *compound_string = NULL;

  /* make sure number of strings is greater than 0 */
  if (nstr == 0)
    return(NULL);

  /* Process arguement strings to find total length */
#ifdef __STDC__
  va_start(pargs, nstr) ;
#else
  va_start(pargs) ;
#endif
  for (i = 0; i < nstr; i++)
    {
      if ((sptr = va_arg(pargs, char *)) == NULL)
        continue;
      str_len += strlen(sptr);
    }
  va_end(pargs) ;

  /* Allocate space for compound string */
  if ((compound_string = (char *)malloc((str_len+1)*sizeof(char))) == NULL)
    return(NULL);

  /* Copy each string into new string */
  cptr = compound_string;
#ifdef __STDC__
  va_start(pargs, nstr);
#else
  va_start(pargs);
#endif
  for (i = 0; i < nstr; i++)
    {
      if ((sptr = va_arg(pargs, char *)) == NULL)
        continue;
      while (*cptr++ = *sptr++);  /* copy string */
      cptr--;
    }
  va_end(pargs);
  return(compound_string);
} /* mk_compound_string */

/*
 * This returns a path name created from from strings given
 * using the given seperator. We assume each string is NULL terminated.
 * We assume first argument is seperator.
 */
#ifdef __STDC__
char *mk_path_name(int nstr, char seperator, ...)
#else
char *mk_path_name(nstr, seperator, va_alist)
     int nstr ;    /* number of strings including seperator */
     char seperator;
     va_dcl
#endif
{
  va_list  pargs;
  register unsigned str_len = 0;
  register unsigned i;
  register char *sptr = NULL;
  register char *cptr = NULL;
  char *path_name = NULL;

  if (nstr == 0)
    return(NULL);

  /* Process first argument to get seperator */
#ifdef __STDC__
  va_start(pargs, seperator);
#else
  va_start(pargs);
#endif
#if 0
  if ((seperator = va_arg(pargs, char *)) == NULL)
    return(NULL);
#endif
  /* Process arguement strings to find total length */
  for (i = 0; i < nstr; i++)
    {
      if ((sptr = va_arg(pargs, char *)) == NULL)
        continue;
      str_len += strlen(sptr);
    }
  va_end(pargs) ;

  /* Allocate space for path_name include seperators */
  if ((path_name = (char *)malloc((str_len+nstr)*sizeof(char))) == NULL)
    return(NULL);

  /* Copy each string into new string */
  cptr = path_name;
#ifdef __STDC__
  va_start(pargs, seperator);
#else
  va_start(pargs);
#endif

  for (i = 0; i < nstr; i++)
    {
      if ((sptr = va_arg(pargs, char *)) == NULL)
        continue;
      while (*cptr++ = *sptr++);  /* copy string */
      *(cptr-1) = seperator;	 /* change '\0' to seperator */
    }
  *(cptr-1) = '\0'; /* NULL terminate path name */
  va_end(pargs);

  return(path_name);
} /* mk_path_name */

/*
 * Return the base name in a path given the seperator.
 * Note that we don't handle empty input strings.
 */
#ifdef __STDC__
char *base_name(char *path_name, char seperator)
#else
char *base_name(path_name, seperator)
   char *path_name;
   char seperator;     
#endif
{
  char *sptr = NULL; /* pointer to last seperator */

  if ((sptr = (char *)strrchr(path_name, seperator)) == NULL)
     return(path_name);
  else
     return(sptr + 1);
} /* base_name */

/*
 * Return the path in path name excluding the base
 * If the path only contains a "seperator" we return the seperator.
 * If their is no directory path we return "."
 * to distinguish between a failure(i.e. NULL return)
 * NOTE that this routine always returns malloced memeory.
 *      This not a good routine to use nested 
 *      e.g.
 *         base_name(path_name(target,':'),'/');
 *      since the memory will be leaked. It would work
 *      better if there was a nice garbage collector:-)...
 */
#ifdef __STDC__
char *path_name(char *path_name, char seperator)
#else
char *path_name(path_name, seperator)
     char *path_name;
     char seperator;
#endif
{
  int  path_len ;    /* path name length */
  char *sptr = NULL; /* pointer to last seperator */
  char *dptr = NULL; /* pointer to path */ 

  /* If no path return "." to distinguish between failure */
  if ((sptr = (char *)strrchr(path_name, (int)seperator)) == NULL)
    return(mk_compound_str(1,"."));

  /* If only seperator in path, return that */
  if ((path_len = sptr - path_name) == 0)
    return(mk_compound_str(1,seperator));

  /* Allocate space for directory path and copy path over */
  if ((dptr = (char *)malloc((path_len+1)*sizeof(char))) == NULL)
    return(NULL);
  else
    {
      strncpy(dptr, path_name, path_len);
      dptr[path_len] = '\0';
      return(dptr) ;
    }
} /* path_name */

/*
 * Return the path in path name excluding the base
 * If the path only contains a "seperator" we return the seperator.
 * If their is no directory path we return "."
 * to distinguish between a failure(i.e. NULL return)
 * NOTE that this routine destroys the original pathname.
 *      It insertsa '\0' character where the last seperator was.
 *      reminds me of "strtok()".....
 */
#ifdef __STDC__
char *path_name_destr(char *path_name, char seperator)
#else
char *path_name_destr(path_name, seperator)
     char *path_name;
     char seperator;
#endif
{
  int  path_len ;    /* path name length */
  char *sptr = NULL; /* pointer to last seperator */

  /* If no path return NULL, not good since can't distinguish failure
   * oh well.....*/
  if ((sptr = (char *)strrchr(path_name, (int)seperator)) == NULL)
    return(NULL);

  /* If only seperator in path, return that */
  if ((path_len = sptr - path_name) == 0)
    return(path_name);

  /* Replace last seperator with NULL */
  sptr = '\0';
  return(sptr) ;

} /* path_name_destr */



