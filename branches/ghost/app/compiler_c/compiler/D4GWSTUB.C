/*
   Compile with WATCOM C :
	wcl /y /ms /d0 /ox d4gwstub.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <errno.h>
#include <string.h>

// #define QUIET

/* Add environment strings to be searched here */
char *paths_to_check[] = {
        "DOS4GPATH",
        "PATH"
};

char *dos4g_path(void) {
    static char fullpath[80];
    int i;

    for( i = 0; i < sizeof( paths_to_check )/sizeof( paths_to_check[0] ); i++) {
        _searchenv( "dos4g.exe", paths_to_check[i], fullpath );
        if( fullpath[0] ) 
             return( &fullpath );
    }
    for( i = 0; i < sizeof( paths_to_check )/sizeof( paths_to_check[0] ); i++) {
        _searchenv( "dos4gw.exe", paths_to_check[i], fullpath );
        if( fullpath[0] ) 
             return( &fullpath );
    }
    return( "dos4gw.exe" );
}


void main( int argc, char *argv[] ) {
    char        *av[4];
    auto char   cmdline[128];

    av[0] = dos4g_path();               /* Locate the DOS/4G loader */
    av[1] = argv[0];                    /* name of executable to run */
    av[2] = getcmd( cmdline );          /* command line */
    av[3] = NULL;                       /* end of list */
#ifdef QUIET
    putenv( "DOS4G=QUIET" );    /* disables DOS/4G Copyright banner */
#endif
    execvp( av[0], av );
    puts( "Stub exec failed:" );
    puts( av[0] );
    puts( strerror( errno ) );
    exit( 1 );                  /* indicate error */
}
