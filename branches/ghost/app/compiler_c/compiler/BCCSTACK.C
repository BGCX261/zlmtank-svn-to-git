/*
 * this is the only way to tell the linker we need a large stack size
 * with the 16-bit borland package.  Note by the way that this is one
 * of those constructs my compiler doesn't know how to handle...
 */
extern int _stklen= 20000;