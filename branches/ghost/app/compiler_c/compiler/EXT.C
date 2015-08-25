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
#include <stdio.h>
#include <string.h>
#include "cmdline.h"
/*
 * If no extension, add the one specified
 */
void AddExt(char *buffer, char *ext)
{
  char *pos = strrchr(buffer,'.');
  if (!pos || (*(pos-1) == '.'))
    strcat(buffer, ext);
}
/*
 * Strip extension, if it has one
 */
void StripExt(char *buffer)
{
  char *pos = strrchr(buffer,'.');
  if (pos && (*(pos-1) != '.'))
    *pos = 0;
}
/*
 * Return path of EXE file
 */
void EXEPath(char *buffer, char*filename)
{
  char *temp;
  strcpy(buffer,filename);
  if ((temp = strrchr(buffer,'\\')) != 0)
    *(temp+1) = 0;
  else
    buffer[0] = 0;
}