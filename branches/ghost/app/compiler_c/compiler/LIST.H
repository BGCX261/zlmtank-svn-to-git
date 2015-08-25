#ifndef _LIST_H
#define _LIST_H

/*
 * Linked list type.  Used for filenames, module number list, segment lists
 */
typedef struct _list_ {
    struct _list_ *link;		/* Link to next element */
    void *data;							/* Generic data */
} LIST;

#include "list.p"

#endif /* _LIST_H */