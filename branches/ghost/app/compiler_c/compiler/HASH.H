#ifndef _HASH_H
#define _HASH_H

/* Local hash table functions.  Librarian uses a different set of functions */
/* Size is a prime number */
#define HASHSIZE 1021

/* Rotations in C */
#define ROTR(x,bits) (((x << (16 - bits)) | (x >> bits)) & 0xffff)
#define ROTL(x,bits) (((x << bits) | (x >> (16 - bits))) & 0xffff)

/* Hash table record definition, all entries in a hash table must be
 * structures with the first two elements as given because hash table
 * entries are sometimes handled generically */
typedef struct _hashrec_ {
   struct _hashrec_ *link;	/* Link to next element in list */
   char *key;	/* Full key */
} HASHREC;

#include "hash.p"

#endif _HASH_H