#ifndef _MEM_H_
#define _MEM_H_
/*
 * EMS mem control type
 */
#define MAX_EMS_READWRITE 16384

typedef struct {
	BYTE *address;
	long size;
} EMSMEM;

/*
 * Linked list type.  Used for filenames, module number list, segment lists
 */
typedef struct _list_ {
    struct _list_ *link;		/* Link to next element */
		void *data;							/* Generic data */
} LIST;

/* Hash table record definition, all entries in a hash table must be
 * structures with the first two elements as given because hash table
 * entries are sometimes handled generically */

typedef struct _hashrec_ {
   struct _hashrec_ *link;	/* Link to next element in list */
   char *key;	/* Full key */
} HASHREC;

#include "umem.p"

#endif /* _MEM_H_ */