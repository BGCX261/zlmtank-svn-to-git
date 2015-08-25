/*
 * 68K/386 32-bit C compiler.
 *
 * copyright (c) 1997, David Lindauer
 * 
 * This compiler is intended for educational use.  It may not be used
 * for profit without the express written consent of the author.
 *
 * It may be freely redistributed, as long as this notice remains intact
 * and either the original sources or derived sources 
 * are distributed along with any executables derived from the originals.
 *
 * The author is not responsible for any damages that may arise from use
 * of this software, either idirect or consequential.
 *
 * v1.35 March 1997
 * David Lindauer, gclind01@starbase.spd.louisville.edu
 *
 * Credits to Mathew Brandt for original K&R C compiler
 *
 */
/*
 * Search for a file along a path list
 */
#include <stdio.h>
#include <string.h>
#include "cmdline.h"
/*
 * Pull the next path off the path search list
 */
static char *parsepath(char *path, char *buffer)
{
  char *pos = path;

  /* Quit if hit a ';' */
  while (*pos) {
		if (*pos == ';') {
			pos++;
			break;
    }
		*buffer++ = *pos++;
	}
  *buffer = 0;

  /* Return a null pointer if no more data */
  if (*pos)
	  return(pos);

  return(0);
}
/*
 * For each library:
 * Search local directory and all directories in the search path
 *  until it is found or run out of directories
 */
FILE *SearchPath(char *string, char *searchpath, char *mode)
{
		FILE *in;
		char *newpath = searchpath;

		/* Search local path */
    in = fopen((char *) string,mode);
		if (in) {
			return(in);
		}
		else {
			/* If no path specified we search along the search path */
			if (!strchr(string,'\\')) {
			  char buffer[200];
				while (newpath) {
					/* Create a file name along this path */
				  newpath = parsepath(newpath,buffer);
					if (buffer[strlen(buffer)-1] != '\\')
						strcat(buffer,"\\");
				  strcat(buffer, (char *)string);

					/* Check this path */
					in = fopen(buffer, mode);
					if (in) {
						strcpy(string,buffer);
						return(in);
					}
				}
			}
		}
	return(0);
}