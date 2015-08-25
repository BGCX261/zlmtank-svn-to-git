#ifndef _CMDLINE_H
#define _CMDLINE_H

#include "utype.h"
#define VMSG(ms) ms" Version %d.%02d  Copyright (c) 1994-1997 LADsoft",(VERSION)/100,(VERSION)%100

/* Directory buffer size */
#define DIR_BUFFER 256

/* Directory errors */
#define DIR_OK 0
#define DIR_BADFORMAT 2
#define DIR_INVALID 3
#define DRIVE_INVALID 4

/* Mode values for ARGLIST */
#define ARG_CONCATSTRING 1
#define ARG_NOCONCATSTRING 2
#define ARG_BOOL 3
#define ARG_SWITCH 4
#define ARG_SWITCHSTRING 5

/* Valid arg separators */
#define ARG_SEPSWITCH '/'
#define ARG_SEPFALSE '-'
#define ARG_SEPTRUE '+'

/* Return values for dispatch routine */
#define ARG_NEXTCHAR 1
#define ARG_NEXTARG 2
#define ARG_NEXTNOCAT 3
#define ARG_NOMATCH 4
#define ARG_NOARG 5

typedef void (*FILEFUNC)(FILE *input, FILE *output, char *fname, char *dir, int drive);

typedef struct {
  char id;
  uchar mode;
  void (*routine)(char, char *);
} ARGLIST;

#ifdef __cplusplus
extern "C" {
#endif
#include "cmdline.p"
#ifdef __cplusplus
};
#endif

#endif /* _CMDLINE_H */