//
//  Compile with Borland C/C++ v3.1 : bcc -v- -ml cl386.c
//  Compile with Watcom C/C++ v10.6 : wcl -y -ox -ml cl386.c
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <process.h>
#ifdef __TURBOC__
  #include <dir.h>
#else
  #include <direct.h>
#endif

#define VERSION "1.0"

static char usage[] = /* Usage */ \
"MK386 Version " VERSION " (c) Kirill Joss. Compile " __DATE__ "\n"
"Usage: MK386.EXE [options] files\n"
"Options:       (default parameters if first)\n"
"/l[-|+]  dump listing file        /w-xxx   disable a warning       \n"
"/i[-|+]  dump preprocessed file   /Dxxx    define something        \n"
"/e[+|-]  dump errors to file      /Ennn    max number of errors    \n"
"/k[+|-]  keep generate files      /Ipath   specify include path    \n"
"/K[-|+]  keep response files      /Lpath   specify .LIB & .OBJ dir \n"
"/A[+|-]  disable extensions       /Cxxx    codegen parameters      \n"
"/v[-|+]  debug info               /Oxxx    optimzer parameters     \n"
"/m[-|+]  generate .MAP file       /Tp      use TLINK & PMODE       \n"
"/n[-|+]  no default .LIB & .OBJ   /Tl      use TLINK & PMODE(low)  \n"
"/M[+|-]  run MAKE                 /Tw      use WLINK & PMODE/W     \n"
"/B[-|+]  build all                /Td      use WLINK & DOS/4GW     \n"
"/a       generate .ASM file       /c       generate .OBJ file      \n"
"/ename   set name of .EXE file    /opath   set out path .OBJ files \n"
"/?,/h,/H this help                @fname   specify response file   \n";


#define FALSE 0
#define TRUE  1

#define MAX_STR 256 /* Max len of string in .CFG file */

// is param ?
#define isparam(s) ( *(s) == '-' || *(s) == '/' || *(s) == '+' )
// check for NOT NULL !!!
#define chkNULL(exp, msg )  if ( !(exp) ) error( ERR_FATAL, msg )
// empty string ?
#define empty_string(s)  ( (s) == NULL || *(s) == '\0' )

typedef struct _Item {
    struct _Item *next;
    char data[1];
} Item;

typedef struct _List {
    Item *head;
    Item *tail;
    unsigned count;
} List;

Item *ItemCreate( char *dat );
#define ItemDelete(item) free(item)

void ListAdd( List *list, char *data );
void ListDelete( List *list );
int  ListFind( List *list, char *data );
void ListIter( List *list, void (*iter)( Item *) );

#define FileAdd(list, file) ListAdd((list),(file))
#define ParamAdd(list, param) ListAdd((list),(param));

List files; // list of all files

List l_inc; // list of INCLUDE path
List l_def; // list of DEFINE var
List l_cc386; // list of CC386 options

int _compile  = TRUE;
int _assemble = TRUE;
int _link     = TRUE;

int _dump_cpp = FALSE; // dump preprocessed file
int _dump_err =  TRUE; // dump error file
int _dump_lst = FALSE; // dump listing file
int _ansi     =  TRUE; // ANSI

int _map_file = FALSE; // map file
int _nodef_lib= FALSE; // no default lib
int _debug    = FALSE; // debug info

int _keep_rsp = FALSE; // keep response files
int _keep_gen = TRUE;  // keep generate files

int _make_run = TRUE;
int _build_all = FALSE;

int _output = 'p';     // Output format, same /Tx, can be 'p', 'l', 'w', 'd'

char path_lib[ _MAX_PATH ]; // path for default LIB & OBJ
char path_obj[ _MAX_PATH ]; // path for output .OBJ files
char name_exe[ _MAX_PATH ]; // name of .EXE file

 // For error()
#define ERR_NO        0  /* no error ! */
#define ERR_FATAL   128  /* fatal error */
#define ERR_RUNTIME 129  /* stdlib error - see 'errno' */

 // output errmsg & exit()
void error( int exitcode, char *msg );
 // output help & exit(0)
void help( void );
 // get parameters from <name>.CFG, CL386, cmdline ...
void get_param( char *exe_name, char *env_var, int argc, char *argv[] );
 // get parameters from file
int get_param_file( char *file );
 // get parameters from string
void get_param_str( char *str );
 // check ONE parameter
void get_one_param( char *param );
 // get ONE options
void get_option( char *param );
 // get ONE file ( can '*' & '?' )
void get_source( char *filename );
 // get ONE file without '*' & '?'
void get_one_source( char *filename );
 // find source files
void find_source( char *fpath, char *fname );
 // clear all list
void cleanup( void );
 // generate MAKEFILE.
void gen_makefile( void );
 // run overlay MAKE
void make( char *maker );


void main( int argc, char *argv[] ) {

    if ( argc == 1 ) // no argument
        help();
    get_param( argv[0], "CL386", argc - 1, argv + 1 );
    atexit( cleanup );  // set exit function
    if ( files.count == 0 ) // no files
        help();
    gen_makefile();
    cleanup();
    if ( _make_run )
        make( "MAKE.EXE" );
}


void help( void ) {
    printf( usage ); exit( 0 );
}


static char _file[ _MAX_PATH  ];
static char fdisk[ _MAX_DRIVE ];
static char fpath[ _MAX_DIR   ];
static char fname[ _MAX_FNAME ];
static char fext [ _MAX_EXT   ];


void get_param( char *exe_name, char *env_var, int argc, char *argv[] ) {
    static char _cfg[] = ".CFG";
    int i;
    char *env;

    _splitpath( exe_name, fdisk, fpath, fname, fext );
    _makepath( _file, NULL, NULL, fname, _cfg );
    if ( ! get_param_file( _file ) ) {
        _makepath( _file, fdisk, fpath, fname, _cfg );
        get_param_file( _file );
    }

    if ( (env = getenv( env_var ) ) != NULL )
        get_param_str( env );

    for ( i = 0; i < argc; i++ )
        get_one_param( argv[ i ] );
}


int get_param_file( char *file ) {
    static char str[ MAX_STR ];
    FILE *f = fopen( file, "rt" );

    if ( f == NULL )
        return FALSE;

    while( fgets( str, MAX_STR - 1, f) != NULL )
        get_param_str( str );

    fclose( f );
    return TRUE;
}


void get_param_str( char *s ) {
    char arg[ MAX_STR ];
    int  len;

    for ( len = 0; *s; s++ ) {
        if ( *s == ' ' || *s == '\t' || *s == '\n' ) {
            if ( len > 0 ) {
                arg[ len ] = '\0';
                get_one_param( arg );
            }
            arg[ len = 0 ] = '\0';
        }
        else {
            arg[ len++ ] = *s;
        }
    }

    if ( len > 0 ) {
        arg[ len ] = '\0';
        get_one_param( arg );
    }
}


void get_one_param( char *arg ) {
    if ( isparam( arg ) )                 // -X , +X or /X
        get_option( arg );
    else if ( *arg == '@' ) {               // @filename
        if ( ! get_param_file( arg + 1 )  )
            error( ERR_RUNTIME, arg + 1 );
    }
    else
        get_source( arg );             // file.ext
}


void error( int exitcode, char *msg ) {
    switch ( exitcode ) {
        case ERR_NO: break;

        case ERR_FATAL: fprintf( stderr, "Fatal : %s\n", msg );
            break;

        case ERR_RUNTIME: fprintf( stderr, "%s : %s", msg, strerror( errno ) );
            break;

        default: fprintf( stderr, "Program error : %s !\n", msg );
            break;
    }
    exit( exitcode );
}


static int setop( char *param, int *option ) {
    if ( param[2] == '-' && param[3] == '\0' )
        *option = FALSE;
    else if ( ( param[2] == '+' && param[3] == '\0' ) || param[2] == '\0' )
        *option = TRUE;
    else
        return FALSE;
    return TRUE;
}


void get_option( char *param ) {
    int len;

    switch( param[ 1 ] ) {

        case 'i' : // dump preprocessed file
            if ( setop( param, &_dump_cpp ) )
                return;
            break;

        case 'e' : // dump errors to file
            if ( setop( param, &_dump_err ) )
                return;
            else
                strupr( strncpy( name_exe, param + 2, _MAX_PATH ) );
            break;

         case 'l' : // dump listing file
            if ( setop( param, &_dump_lst ) )
                return;
            break;

        case 'A' : // disable extensions
            if ( setop( param, &_ansi ) )
                return;
            break;

        case 'm' : // gen map file
            if ( setop( param, &_map_file ) )
                return;
            break;

        case 'n' : // no default lib
            if ( setop( param, &_nodef_lib ) )
                return;
            break;

        case 'v' : // debug info
            if ( setop( param, &_debug ) )
                return;
            break;

        case 'K' : // keep response files
            if ( setop( param, &_keep_rsp ) )
                return;
            break;

        case 'k' : // keep generate files
            if ( setop( param, &_keep_gen ) )
                return;
            break;

        case 'B' : // building all
            if ( setop( param, &_build_all ) )
                return;
            break;

        case 'M' : // run make, else ONLY generate MAKEFILE.
            if ( setop( param, &_make_run ) )
                return;
            break;

        case 'a' :
            if ( param[2] == '\0' ) {
                _compile = TRUE;
                _assemble = _link = FALSE;
                return;
            }
            break;

        case 'c' :
            if ( param[2] == '\0' ) {
                _compile = _assemble = TRUE;
                _link = FALSE;
                return;
            }
            break;

        case '?' :
        case 'h' :
        case 'H' :
            help(); /* Not return ! */

        case 'D' : // define
            ParamAdd( &l_def, param + 2 );
            return;

        case 'I' : // include path
            ParamAdd( &l_inc, param + 2 );
            return;

        case 'L' : // lib path
            strupr( strncpy( path_lib, param + 2, _MAX_PATH ) );
            len = strlen( path_lib );
            if ( len > 0 && path_lib[ len - 1 ] == '\\' )
                path_lib[ len - 1 ] = '\0';
            return;

        case 'o' : // output obj path
            strupr( strncpy( path_obj, param + 2, _MAX_PATH ) );
            len = strlen( path_obj );
            if ( len > 0 && path_obj[ len - 1 ] == '\\' )
                path_obj[ len - 1 ] = '\0';
            return;

        case 'w' :
        case 'C' :
        case 'E' :
        case 'O' :
            ParamAdd( &l_cc386, param );
            return;

        case 'T' :
            if ( param[3] == '\0' )
                if ( param[2] == 'p' || param[2] == 'l' || \
                param[2] == 'w' || param[2] == 'd' ) {
                    _output = param[2]; return;
                }
            break;

    }
    fprintf( stderr, "Warning! Parameter error : %s\n", param );
}


Item *ItemCreate( char *dat ) {
    Item *item;
    int len;

    if ( empty_string( dat ) )
        return NULL;

    len = strlen( dat );

    if ( (item = (Item *)malloc( sizeof( Item ) + len )) != NULL ) {
        item->next = NULL;
        strcpy( item->data, dat );
    }

    return item;
}


void ListDelete( List *list ) {
    Item *p, *q;

    chkNULL( list, "Pointer is NULL !" );

    for( p = list->head; p; p = q ) {
        q = p->next;
        free(p);
    }
    list->head = list->tail = NULL;
    list->count = 0;
}


void ListAdd( List *list, char *file ) {
    Item *item;

    chkNULL( list, "Pointer is NULL !" );

    item = ItemCreate( file );

    chkNULL( item, "Out of memory !" );

    if ( list->count == 0 )
        list->head = list->tail = item;
    else {
        (list->tail)->next = item;
        list->tail = item;
    }
    list->count++;
}


void ListIter( List *list, void (*iter)( Item *) ) {
    Item *p;

    chkNULL( list, "Pointer is NULL !" );

    if ( list->count && iter ) {
        for ( p = list->head; p; p = p->next )
            (*iter)(p);
    }
}


static char *need;
static int found = 0;


static void find( Item *item ) {
    if ( strcmp( item->data, need ) == 0 )
        found = 1;
}


int  ListFind( List *list, char *data ) {
    found = 0;
    need = data;
    ListIter( list, find );
    return found;
}


char findpath[ _MAX_PATH ];
char findname[ _MAX_PATH ];


void get_source( char *filename ) {
    _splitpath( filename, fdisk, fpath, fname, fext );

    if ( strchr( filename, '?' ) || strchr( filename, '*' ) ) {  /* Check for '*' and '?' */
        _makepath( findpath, fdisk, fpath, NULL, NULL );
        _makepath( findname, NULL, NULL, fname, fext );
        find_source( findpath, findname );
    }
    else
        get_one_source( filename );
}


#ifdef __TURBOC__


void find_source( char *fpath, char *fname ) {
    int fpath_len = strlen( fpath );
    struct ffblk fb;

    if ( !findfirst( strcat( fpath, fname), &fb, FA_ARCH) )
    do {
        fpath[ fpath_len ] = '\0';
        get_one_source( strcat( fpath, fb.ff_name) );
    } while (! findnext(&fb) );
}


#else


void find_source( char *fpath, char *fname ) {
    int fpath_len = strlen( fpath );
    struct find_t fb;

    if (!_dos_findfirst( strcat( fpath, fname), _A_NORMAL, &fb ) )
    do {
        fpath[ fpath_len ] = '\0';
        get_one_source( strcat( fpath, fb.name) );
    } while( ! _dos_findnext( &fb ) );
}


#endif


void get_one_source( char *srcfile ) {
    char file[ _MAX_PATH ];
    strupr( strncpy( file, srcfile, _MAX_PATH ) );
    FileAdd( &files, file );
}


void cleanup( void ) {   // for atexit()
    ListDelete( &files );
    ListDelete( &l_def );
    ListDelete( &l_inc );
    ListDelete( &l_cc386 );
}


#define Def(v) ( (v) ? 'D' : 'U' )

void make( char *maker ) {
    static char make_opt[ _MAX_PATH ];
//  static char *args[3];
    int err;

    sprintf( make_opt,
" -%cMAP -%cDEBUG -%cNODEFLIB -%cKEEP_SRC -%cKEEP_LST -%cLINK -%cASSEMBLE -%cCOMPILE",
        Def( _map_file), Def( _debug), Def( _nodef_lib),
        Def( _keep_gen ), Def( _dump_lst ),
        Def( _link), Def( _assemble ), Def( _compile ) );
    if ( _keep_rsp )
        strcat( make_opt, " -K" );
    if ( _build_all )
        strcat( make_opt, " -B" );
    printf( "%s%s\n", maker, make_opt );
    err = spawnlp( P_OVERLAY, maker, maker, make_opt, NULL );

    if ( err <  0 )
        error( ERR_RUNTIME, maker );
    else if ( err > 0 )
        error( err, maker );
}

char *name( void ) {
    if ( strlen( name_exe ) > 0 )
        return name_exe;
    _splitpath( files.head->data, fdisk, fpath, fname, fext );
    return fname;
}

char *libpath( char *lib ) {
    if ( strlen( path_lib ) == 0 )
        return lib;
    strcpy( _file, path_lib );
    strcat( _file, "\\" );
    return strcat( _file, lib );
}

#define L_( s ) fprintf( frsp, "\t%s", (s) )
#define L0( s ) fprintf( frsp, "%s", (s) )
#define L1( s ) fprintf( frsp, "%s\n", (s) )

static FILE *frsp;


static void compile_inc  ( Item *p ) { fprintf( frsp, "%s;", p->data );   }
static void compile_def  ( Item *p ) { fprintf( frsp, " /D%s", p->data ); }
static void compile_opt  ( Item *p ) { fprintf( frsp, " %s", p->data );   }

static void assemble_def( Item *p ) { fprintf( frsp, " /d%s", p->data ); }
static void assemble_inc( Item *p ) { fprintf( frsp, " /i%s", p->data ); }


static void get_obj( Item *p ) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".OBJ" ) == 0 )
        fprintf( frsp, "%s \\\n", p->data );
    else if ( strcmp( fext, ".ASM" ) == 0 ) {
        fprintf( frsp, "$(PATH_OBJ)\\%s.OBJ \\\n", fname );
    }
    else if ( strcmp( fext, ".LIB" ) != 0 ) {
        fprintf( frsp, "$(PATH_OBJ)\\%s.OBJ \\\n", fname );
    }
}

static void get_asm( Item *p ) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".OBJ" ) == 0 || strcmp( fext, ".LIB" ) == 0 )
        return;
    else if ( strcmp( fext, ".ASM" ) == 0 )
        fprintf( frsp, "%s \\\n", p->data );
    else
        fprintf( frsp, "%s.ASM \\\n", fname );
}

static void get_lib( Item *p ) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".LIB" ) == 0 ) {
        fprintf( frsp, "%s \\\n", p->data );
    }
}

static void get_dep( Item *p ) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".LIB" ) == 0 )
        return; // .LIB file
    else if ( strcmp( fext, ".OBJ" ) == 0 )
        return; // .OBJ file
    else if ( strcmp( fext, ".ASM" ) == 0 ) {   // .ASM file
        fprintf( frsp, "$(PATH_OBJ)\\%s.OBJ : %s\n", fname, p->data );
        fprintf( frsp, "  $(AS) %s,$(PATH_OBJ)\\%s.OBJ,$(PATH_OBJ)\\%s.LST,NUL\n",
            p->data, fname, fname );
        fprintf( frsp, "  $(DEL_LST) $(PATH_OBJ)\\%s.LST\n", fname );
    }
    else {  // .CPP file
        fprintf( frsp, "$(PATH_OBJ)\\%s.OBJ : %s.ASM\n", fname, fname );
        fprintf( frsp, "  $(AS) %s.ASM,$(PATH_OBJ)\\%s.OBJ,$(PATH_OBJ)\\%s.LST,NUL\n",
            fname, fname, fname );
        fprintf( frsp, "  $(DEL_LST) $(PATH_OBJ)\\%s.LST\n", fname );
        fprintf( frsp, "  $(DEL_SRC) %s.ASM\n", fname );
        fprintf( frsp, "%s.ASM : %s\n", fname, p->data );
        fprintf( frsp, "  $(CC) %s\n",  p->data );
    }
}


static void wout_obj( Item *p) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".LIB" ) == 0 )
        return;
    else if ( strcmp( fext, ".OBJ" ) == 0 )
        fprintf( frsp, "file %s\n", p->data );
    else if ( strcmp( fext, ".ASM" ) == 0 )
        fprintf( frsp, "file $(PATH_OBJ)\\%s.OBJ\n", fname );
    else
        fprintf( frsp, "file $(PATH_OBJ)\\%s.OBJ\n", fname );
}

static void wout_lib( Item *p) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".LIB" ) == 0 )
        fprintf( frsp, "library %s\n", p->data );
}

static void delete_obj( Item *p ) {
    _splitpath( p->data, fdisk, fpath, fname, fext );
    if ( strcmp( fext, ".LIB" ) == 0 || strcmp( fext, ".OBJ" ) == 0 )
        return;
    else
        fprintf( frsp, "  $(DEL_SRC) $(PATH_OBJ)\\%s.OBJ\n", fname );
}

void gen_makefile( void ) {
    int tlink = ( _output == 'p' || _output == 'l' );

    frsp = fopen( "MAKEFILE.", "wt" );
    if ( frsp == NULL )
        error( ERR_RUNTIME, "MAKEFILE." );

L1( "#" );
L1( "#  this MAKEFILE. generate MK386.EXE program"  );
L1( "#" );
L1( ".AUTODEPEND" );
L1( ".SWAP" );
L1( "# LINK     = # define LINK for COMPILE, ASSEMBLE & LINK" );
L1( "# ASSEMBLE = # define ASSEMBLE for COMPILE & ASSEMBLE" );
L1( "# COMPILE  = # define COMPILER for ONLY COMPILE" );
L1( "!ifndef LINK" );
L1( "!ifndef ASSEMBLE" );
L1( "!ifndef COMPILE" );
L1( "LINK       =" );
L1( "!endif" );
L1( "!endif" );
L1( "!endif" );
L1( "# DEBUG    = # define DEBUG for debug info and debug startup module" );
L1( "# MAP      = # define MAP for generate .MAP file" );
L1( "# NODEFLIB = # define NODEFLIB for NO link default .OBJ & .LIB" );
L1( "# KEEP_SRC = # define KEEP_SRC for save generate .ASM & .OBJ files" );
L1( "# KEEP_LST = # define KEEP_LST for save generate .LST files" );
L1( "" );
L1( "                                       ### name of .EXE file" );
L0( "NAME = " ); L1( name() );
L1( "                                       ### define tools" );
L1( "CC  = CC386.EXE" );
L1( "AS  = TASM.EXE" );
L1( "                                       ### define options for CC" );
L0( "CC_INC = " );
    if ( l_inc.count > 0 ) {
        L0( "/I" ); ListIter( &l_inc, compile_inc );
    }
L1( "" );
L0( "CC_DEF = " );
    ListIter( &l_def, compile_def );
L1( "" );
L0( "CC_OPT = " );
    fprintf( frsp, "%ci %ce %cA",
	( _dump_cpp ? '+' : '-' ), ( _dump_err ? '+' : '-' ), ( !_ansi ? '+' : '-' ) );
    ListIter( &l_cc386, compile_opt );
L1( "" );
L1( "!ifdef KEEP_LST" );
L1( "CC_OPT = $(CC_OPT) +l" );
L1( "!endif" );
L1( "" );
L1( "CC = $(CC) $(CC_OPT) $(CC_DEF) $(CC_INC)" );
L1( "                                       ### define options for AS" );
L0( "AS_INC = " );
    ListIter( &l_inc, assemble_inc );
L1( "" );
L0( "AS_DEF = " );
    ListIter( &l_def, assemble_def );
L1( "" );
L1( "!ifdef DEBUG");
L1( "AS_OPT = /t /m3 /ml /zi" );
L1( "!else" );
L1( "AS_OPT = /t /m3 /ml /zn" );
L1( "!endif");
L1( "!ifdef KEEP_LST" );
L1( "AS_OPT = $(AS_OPT) /la" );
L1( "!endif" );
L1( "" );
L1( "AS = $(AS) $(AS_OPT) $(AS_DEF) $(AS_INC)" );
L1( "                                       ### .LIB & output .OBJ path" );
L0( "PATH_LIB = " ); L1( path_lib );
L0( "PATH_OBJ = " );
    if ( strlen( path_obj ) > 0 )
        L1( path_obj );
    else
        L1( "." );
L1( "                                       ### default .OBJ & .LIB" );
L1( "!ifdef NODEFLIB");
L1( "DEFAULT_OBJS =" );
L1( "DEFAULT_LIBS =" );
L1( "WLINK_OBJS   =" );
L1( "WLINK_LIBS   =" );
L1( "!else" );
L1( "DEFAULT_LIBS = $(PATH_LIB)\\CLDOS.LIB" );
L1( "WLINK_LIBS   = library $(DEFAULT_LIBS)" );
L1( "!ifdef DEBUG" );
    if ( _output == 'l' )
        L1( "DEFAULT_OBJS = $(PATH_LIB)\\C0DOSLD.OBJ " );
    else if ( _output == 'p' )
        L1( "DEFAULT_OBJS = $(PATH_LIB)\\C0DOSD.OBJ " );
    else
        L1( "DEFAULT_OBJS = $(PATH_LIB)\\C0DOSWD.OBJ " );
L1( "!else" );
    if ( _output == 'l' )
        L1( "DEFAULT_OBJS = $(PATH_LIB)\\C0DOSL.OBJ " );
    else if ( _output == 'p' )
        L1( "DEFAULT_OBJS = $(PATH_LIB)\\C0DOS.OBJ " );
    else
        L1( "DEFAULT_OBJS = $(PATH_LIB)\\C0DOSW.OBJ " );
L1( "!endif" );
L1( "WLINK_OBJS   = file $(DEFAULT_OBJS)" );
L1( "!endif" );
L1( "                                       ### define macro for KEEP_SRC" );
L1( "!ifdef KEEP_SRC");
L1( "DEL_SRC = @rem" );
L1( "!else" );
L1( "DEL_SRC = -del" );
L1( "!endif");
L1( "                                       ### define macro for KEEP_LST" );
L1( "!ifdef KEEP_LST");
L1( "DEL_LST = @rem" );
L1( "!else" );
L1( "DEL_LST = -del" );
L1( "!endif");
L1( "                                       ### define macro for MAP" );
L1( "!ifdef MAP");
L1( "TLINK_OPT = /3/d/c/m/l/s" );
L1( "WLINK_OPT = option map" );
L1( "!else" );
L1( "TLINK_OPT = /3/d/c/x" );
L1( "WLINK_OPT = " );
L1( "!endif");
L1( "                                       ### define macro for DEBUG" );
L1( "!ifdef DEBUG");
L1( "TLINK_DBG = /v" );
L1( "WLINK_DBG = debug option symf" );
L1( "!else" );
L1( "TLINK_DBG = " );
L1( "WLINK_DBG = option quiet" );
L1( "!endif");
L1( "                                       ### .ASM files" );
    fprintf( frsp, "ASMS = \\\n" );
    ListIter( &files, get_asm );
L1( "                                       ### .OBJ files" );
	fprintf( frsp, "OBJS = $(DEFAULT_OBJS) \\\n" );
	ListIter( &files, get_obj );
L1( "                                       ### .LIB files" );
	fprintf( frsp, "LIBS = $(DEFAULT_LIBS) \\\n" );
	ListIter( &files, get_lib );
L1( "                                       ###  main make depend" );
L1( "!ifdef LINK" );
L1( "$(NAME).EXE : $(OBJS) $(LIBS) makefile." );
	if ( tlink )
	{
		L1( "  TLINK.EXE @&&|" );
		L1( "$(TLINK_OPT) $(TLINK_DBG) $(OBJS), $(NAME), $(NAME), $(LIBS)" );
	}
	else
	{
		L1( "  WLINK.EXE @&&|" );
		L1( "$(WLINK_OPT) $(WLINK_DBG)" );
		L1( "format os2 le" );
		L1( "option nod" );
		if ( _output == 'w' ) {
			L1( "option osname='CC386+PMODE/W'" );
			L1( "option stub=$(PATH_LIB)\\PMODEW.EXE" );
		}
		else {
			L1( "option osname='CC386+DOS/4GW'" );
			L1( "option stub=$(PATH_LIB)\\D4GWSTUB.EXE" );
        }
        L1( "name $(NAME)" );
        L1( "$(WLINK_OBJS)" );
        ListIter( &files, wout_obj );
        L1( "$(WLINK_LIBS)" );
        ListIter( &files, wout_lib );
    }
L1( "|" );
    ListIter( &files, delete_obj );
L1( "!else" );
L1( "!ifdef ASSEMBLE");
L1( "assemble : $(OBJS) " );
L1( "!else" );
L1( "!ifdef COMPILE");
L1( "compile : $(ASMS) " );
L1( "!endif" );
L1( "!endif" );
L1( "!endif" );
L1( "                                       ### files depend" );
    ListIter( &files, get_dep );
    fclose( frsp );
}
