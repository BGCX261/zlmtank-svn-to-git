#ifndef _TYPE_H
#define _TYPE_H

//types
#ifndef NULL
#define NULL 0
#endif

#define DWORD unsigned long
#define WORD   unsigned short
#define BYTE     unsigned char
#define CHAR    char
#define BOOL    bool
#define FLOAT  float

#define U32 unsigned long
#define U16 unsigned short
#define U8   unsigned char
#define S32 long
#define S16 short
#define S8   char




#ifdef WIN32
#define ASSERT(X) assert(X)
#else
#define ASSERT(X)
#endif

class FPOINT
{
public:
    FLOAT x;
	FLOAT y;
	FPOINT(){x=0;y=0;};
	void Set(FLOAT a,FLOAT b){x=a;y=b;};
	void SetZero(){x=0;y=0;};
};


#endif
