/*
    Compile with Borland C/C++ v3.1
        bcc -v- -ml cl386.c
    or Compile with Watcom C/C++ v10.6
	wcl -y -ox -ml cl386.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <process.h>
#ifdef __TURBOC__
  #include <dir.h>
//  extern unsigned _stklen = 16*1024;
#else
  #include <direct.h>
#endif


#define VERSION "4.1m"
#define TO_CURR_DIR /* define, if you want put .ASM & .OBJ file to current dir */

static char usage[] = /* Usage */ \
"CL386 Version " VERSION " (c) Kirill Joss. Compile " __DATE__ "\n"
"Usage: CL386.EXE [options] files\n"
"Options:                          @fname   specify response file   \n"
"/l[+|-]  dump listing file        /w-xxx   disable a warning       \n"
"/i[+|-]  dump preprocessed file   /Dxxx    define something        \n"
"/e[-|+]  dump errors to file      /Ennn    max number of errors    \n"
"/k[+|-]  keep generate files      /Ipath   specify include path    \n"
"/K[+|-]  keep response files      /Lpath   specify .LIB & .OBJ dir \n"
"/A[+|-]  disable extensions       /Cxxx    codegen parameters      \n"
"/v[+|-]  debug info               /Oxxx    optimzer parameters     \n"
"/m[+|-]  generate .MAP file       /Tp      use TLINK & PMODE       \n"
"/n[+|-]  no default .LIB & .OBJ   /Tl      use TLINK & PMODE(low)  \n"
"/a       generate .ASM file       /Tw      use WLINK & PMODE/W     \n"
"/c       generate .OBJ file       /Td      use WLINK & DOS/4GW     \n"
"/?,/h,/H this help                /Rname   specify .EXE name\n";

#define FALSE 0
#define TRUE  1

#define _ASM ".ASM"
#define _OBJ ".OBJ"
#define _LIB ".LIB"
#define _LSA ".LSA"
#define _CFG ".CFG"

#define COMPILER     "CC386.EXE"   /* Name of compiler */
#define COMPILER_RSP "CC386.$$$"   /* Response file   for compiler */
#define COMPILER_OPT ""            /* Default options for compiler */

#define ASSEMBLER     "TASM.EXE"   /* Name of assembler */
#define ASSEMBLER_RSP "TASM.$$$"   /* Response file   for assembler */
#define ASSEMBLER_OPT "/t/ml/m3"   /* Default options for assembler */

#define TLINK "TLINK.EXE"
#define WLINK "WLINK.EXE"

#define LINKER_RSP  "LINK.$$$"

#define DELETE_RSP  "DELETE.$$$"

#define MAX_STR 256 /* Max len of string in .CFG file */

/* For error() */
#define ERR_NO        0  /* no error ! */
#define ERR_FATAL   128  /* fatal error */
#define ERR_RUNTIME 129  /* stdlib error - see 'errno' */

// is param ?
#define isparam(s) ( *(s) == '-' || *(s) == '/' || *(s) == '+' )
// check for NOT NULL !!!
#define chkNULL(exp, msg )  if ( !(exp) ) error( ERR_FATAL, msg )
// empty string ?
#define empty_string(s)  ( (s) == NULL || *(s) == '\0' )

typedef struct _Item {
	struct _Item *next;
	int temp;
	char data[1];
} Item;

Item *ItemCreate( char *dat, int tmp );

#define ItemDelete(item) free(item)

char *FileDisk( Item *item );
char *FilePath( Item *item );
char *FileName( Item *item );
char *FileExt ( Item *item );


typedef struct _List {
	Item *head;
	Item *tail;
	unsigned count;
} List;

void ListAdd( List *list, char *data, int temp );
void ListDelete( List *list );

int  ListFind( List *list, char *data );
void ListIter( List *list, void (*iter)( Item *) );

#define FileAdd(list, file, temp) ListAdd((list),(file),(temp))
#define ParamAdd(list, param) ListAdd((list),(param), FALSE);

List f_cpp; // list of .CPP files
List f_asm; // list of .ASM files
List f_obj; // list of .OBJ files
List f_lib; // list of .LIB files

List l_inc;     // list of INCLUDE path
List l_def;     // list of DEFINE var
List l_cc386;   // list of CC386 options

char _libpath[ _MAX_PATH ]; // Path of default LIB & OBJ
char _exename[ _MAX_PATH ]; // Name of .EXE file

int _compile  = TRUE;  // need compile
int _assemble = TRUE;  // need assemble
int _link     = TRUE;  // need link

int _dump_cpp = FALSE; // dump preprocessed file
int _dump_err =  TRUE; // dump error file
int _dump_lst = FALSE; // dump listing file
int _ansi     = FALSE; // ANSI
int _map_file = FALSE; // map file
int _nodef_lib= FALSE; // no default lib
int _debug    = FALSE; // debug info
int _keep_rsp = FALSE; // keep response files
int _keep_gen = FALSE; // keep generate files

int _output = 'p';     // Output format, same /Tx


void help( void );

void get_param( char *exe_name, char *env_var, int argc, char *argv[] );
 int get_param_file( char *file );
void get_param_str( char *str );
void get_one_param( char *param );

void get_option( char *param );

void get_source( char *filename );
void get_one_source( char *filename );
void find_source( char *fpath, char *fname );

void error( int exitcode, char *msg );

void response( void );
void compile( void );
void assemble( void );
void link( void );
void remove_temp( void );

void DeleteList( void );
void DeleteAll( void ); /* for atexit() */


void main( int argc, char *argv[] ) {

	if ( argc == 1 )
		help();

	get_param( argv[0], "CL386", argc - 1, argv + 1 );

	atexit( DeleteAll );

	if ( f_cpp.count == 0 && f_asm.count == 0 &&
	  f_obj.count == 0 && f_lib.count == 0 )
		help();

	if ( f_cpp.count == 0 ) _compile = FALSE;
	if ( f_asm.count == 0 ) _assemble = FALSE;
	if ( f_obj.count == 0 && f_lib.count == 0 ) _link = FALSE;

	response();

	DeleteList();

	compile();
	assemble();
	link();

	remove_temp();
}


void help( void ) {
	printf( usage ); exit( 0 );
}


void get_param( char *exe_name, char *env_var, int argc, char *argv[] ) {
	char file[ _MAX_PATH ], disk[ _MAX_DRIVE ],
		path[ _MAX_DIR ], name[ _MAX_FNAME ], ext [ _MAX_EXT ];
	char *env;
	int i;

	_splitpath( exe_name, disk, path, name, ext );
	_makepath( file, NULL, NULL, name, _CFG );
	if ( ! get_param_file( file ) ) {
		_makepath( file, disk, path, name, _CFG );
		get_param_file( file );
	}

	if ( (env = getenv( env_var ) ) != NULL )
		get_param_str( env );

	for ( i = 0; i < argc; i++ )
		get_one_param( argv[ i ] );
}


int get_param_file( char *file ) {
	static char s[ MAX_STR ];
	FILE *f = fopen( file, "rt" );

	if ( f == NULL )
		return FALSE;

	while( fgets( s, MAX_STR - 1, f) != NULL )
		get_param_str( s );

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


void get_option( char *param ) {
	switch( param[ 1 ] ) {

        case 'i' : // dump preprocessed file
            if ( param[2] == '-' ) {        // -i-
                _dump_cpp = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -i or -i+ :)
                _dump_cpp = TRUE;  return;
            }
            break;

        case 'e' : // dump errors to file
            if ( param[2] == '-' ) {        // -e-
                _dump_err = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -e or -e+ :)
                _dump_err = TRUE; return;
            }
            break;

		 case 'l' : // dump listing file
			if ( param[2] == '-' ) {            // -l-
                _dump_lst = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -l or -l+ :)
                _dump_lst = TRUE; return;
            }
            break;

        case 'A' : // disable extensions
            if ( param[2] == '-' ) { // -A-
                _ansi = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -A or -A+
                _ansi = TRUE; return;
            }
            break;

        case 'm' : // gen map file
			if ( param[2] == '-' ) { // -m-
				_map_file = FALSE; return;
			}
            else if ( param[2] == '+' || param[2] == '\0' ) { // -m or -m+
                _map_file = TRUE; return;
            }
            break;

        case 'n' : // no default lib
            if ( param[2] == '-' ) { // -n-
                _nodef_lib = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -n or -n+
                _nodef_lib = TRUE; return;
            }
            break;

        case 'v' : // debug info
            if ( param[2] == '-' ) { // -v-
                _debug = FALSE; return;
			}
			else if ( param[2] == '+' || param[2] == '\0' ) { // -v or -v+
				_debug = TRUE; return;
            }
            break;

        case 'K' : // keep response files
            if ( param[2] == '-' ) { // -K-
                _keep_rsp = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -K or -K+
                _keep_rsp = TRUE; return;
            }
            break;

        case 'k' : // keep generate files
            if ( param[2] == '-' ) { // -k-
                _keep_gen = FALSE; return;
            }
            else if ( param[2] == '+' || param[2] == '\0' ) { // -k or -k+
				_keep_gen = TRUE; return;
			}
			break;

        case 'a' :
            _compile = TRUE;
            _assemble = _link = FALSE;
            return;

        case 'c' :
            _compile = _assemble = TRUE;
            _link = FALSE;
            return;

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
            strupr( strncpy( _libpath, param + 2, _MAX_PATH ) );
//            ParamAdd( &l_lib, param + 2 );
            return;

        case 'R' : // .EXE name
            strupr( strncpy( _exename, param + 2, _MAX_PATH ) );
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


Item *ItemCreate( char *dat, int tmp ) {
    Item *item;
    int len;

	if ( empty_string( dat ) )
		return NULL;

    len = strlen( dat );

    if ( (item = (Item *)malloc( sizeof( Item ) + len )) != NULL ) {
        item->next = NULL;
        item->temp = tmp;
        strcpy( item->data, dat );
    }

    return item;
}


static char disk[ _MAX_DRIVE ];
static char path[ _MAX_DIR   ];
static char name[ _MAX_FNAME ];
static char ext [ _MAX_EXT   ];


char *FileDisk( Item *item ) {
	_splitpath( item->data, disk, path, name, ext );
    return disk;
}


char *FilePath( Item *item ) {
    _splitpath( item->data, disk, path, name, ext );
    return path;
}


char *FileName( Item *item ) {
    _splitpath( item->data, disk, path, name, ext );
    return name;
}


char *FileExt ( Item *item ) {
	_splitpath( item->data, disk, path, name, ext );
	return ext;
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


void ListAdd( List *list, char *file, int tmp ) {
	Item *item;

	chkNULL( list, "Pointer is NULL !" );

	item = ItemCreate( file, tmp );

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
	_splitpath( filename, disk, path, name, ext );

	if ( strchr( filename, '?' ) || strchr( filename, '*' ) ) {  /* Check for '*' and '?' */
		_makepath( findpath, disk, path, NULL, NULL );
		_makepath( findname, NULL, NULL, name, ext );
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
    int temp = FALSE;
    static char file[ _MAX_PATH ];
    static char disk[ _MAX_DRIVE ];
    static char path[ _MAX_DIR ];
    static char name[ _MAX_FNAME ];
	static char ext [ _MAX_EXT ];

	strncpy( file, srcfile, _MAX_PATH );
    _splitpath( strupr(file) , disk, path, name, ext );
//  _makepath( file, disk, path, name, ext );

    if ( strcmp( ext, _ASM ) != 0 && \
         strcmp( ext, _OBJ ) != 0 && strcmp( ext, _LIB ) != 0 ) {
        if ( ListFind( &f_cpp, file ) )
            fprintf( stderr, "Warning! File already exist : %s\n", file );
        else
            FileAdd( &f_cpp, file, temp++ );
#ifdef TO_CURR_DIR
		_makepath( file, disk, NULL, name, strcpy( ext, _ASM ) );
#else
        _makepath( file, disk, path, name, strcpy( ext, _ASM ) );
#endif
    }

    if ( strcmp( ext, _ASM ) == 0 ) {
		if ( ListFind( &f_asm, file ) )
			fprintf( stderr, "Warning! File already exist : %s\n", file );
		else
            FileAdd( &f_asm, file, temp++ );
        _makepath( file, disk, path, name, strcpy( ext, _OBJ) );
    }

    if ( strcmp( ext, _OBJ ) == 0 ) {
        if ( ListFind( &f_obj, file ) )
            fprintf( stderr, "Warning! File already exist : %s\n", file );
        else
            FileAdd( &f_obj, file, temp++ );
    }

    if ( strcmp( ext, _LIB ) == 0 ) {
        if ( ListFind( &f_lib, file ) )
            fprintf( stderr, "Warning! File already exist : %s\n", file );
        else
            FileAdd( &f_lib, file, 0 );
    }
}


void DeleteList( void ) {  // delete all list from memory
	ListDelete( &f_cpp );
	ListDelete( &f_asm );
	ListDelete( &f_obj );
	ListDelete( &f_lib );
	ListDelete( &l_def );
	ListDelete( &l_inc );
//    ListDelete( &l_lib );
	ListDelete( &l_cc386 );
}

void DeleteAll( void ) {   // for atexit()
	DeleteList();
	if ( !_keep_rsp ) {
		remove( COMPILER_RSP );
		remove( ASSEMBLER_RSP );
		remove( LINKER_RSP );
		remove( DELETE_RSP );
	}
}

void exec( char *prg, char *rsp ) {
	static char *args[3];
	int err;

	args[0] = prg;
	args[1] = rsp;
	args[2] = NULL;

	err = spawnvp( P_WAIT, prg, args );

	if ( err <  0 )
		error( ERR_RUNTIME, prg );
	else if ( err > 0 )
		error( err, prg );
}


FILE *frsp;


static void compile_def  ( Item *p ) { fprintf( frsp, " /D%s", p->data ); }
static void compile_inc  ( Item *p ) { fprintf( frsp, "%s;", p->data ); }
static void compile_opt  ( Item *p ) { fprintf( frsp, " %s", p->data ); }
static void compile_files( Item *p ) { fprintf( frsp, "\n%s", p->data ); }

static void compile_rsp( char *file, char *def_opt ) {
	frsp = fopen( file, "wt" );
	if ( frsp == NULL )
		error( ERR_RUNTIME, file );
	fprintf( frsp, " %s", def_opt );
	ListIter( &l_cc386, compile_opt );
	fprintf( frsp, " %ci", ( _dump_cpp ? '+' : '-' ) );
	fprintf( frsp, " %ce", ( _dump_err ? '+' : '-' ) );
	fprintf( frsp, " %cl", ( _dump_lst ? '+' : '-' ) );
	fprintf( frsp, " %cA", (     _ansi ? '+' : '-' ) );
	ListIter( &l_def, compile_def );
	if ( l_inc.count > 0 ) {
		fprintf( frsp, " /I" );
		ListIter( &l_inc, compile_inc );
	}
	ListIter( &f_cpp, compile_files );
	fprintf( frsp, "\n" );
	fclose( frsp );
}

void compile( void ) {
	if ( _compile /* && f_cpp.count > 0 */ )
	{
//		compile_rsp( COMPILER_RSP, COMPILER_OPT );
		exec( COMPILER, "/f" COMPILER_RSP );
		if ( !_keep_rsp )
			remove( COMPILER_RSP );
//		ListDelete( &f_cpp );
//		ListDelete( &l_cc386 );
	}
}

static void delete_temp( Item *p ) { if ( p->temp ) remove( p->data ); }

static void assemble_def( Item *p ) { fprintf( frsp, " /d%s", p->data ); }
static void assemble_inc( Item *p ) { fprintf( frsp, " /i%s", p->data ); }

char *asm_opt;

static void gen_for_asm( Item *p ) {
	fprintf( frsp, " %s", asm_opt );
	fprintf( frsp, " /z%c", ( _debug ? 'i' : 'n' ) );
	ListIter( &l_inc, assemble_inc );
	ListIter( &l_def, assemble_def );
	if ( _dump_lst )
		fprintf( frsp, " %s,%s%s%s.OBJ,%s%s%s.LSA;\n", p->data,
			FileDisk(p), FilePath(p), FileName(p),
			FileDisk(p), FilePath(p), FileName(p) );
	else
		fprintf( frsp, " %s,%s%s%s.OBJ,NUL;\n", p->data,
			FileDisk(p), FilePath(p), FileName(p) );
}

void assemble_rsp( char *file, char *def_opt ) {
	frsp = fopen( file, "wt" );
	if ( frsp == NULL )
		error( ERR_RUNTIME, file );
	asm_opt = def_opt;
	ListIter( &f_asm, gen_for_asm );
	fclose( frsp );
}

void assemble(void) {
	if ( _assemble /* && f_asm.count > 0 */ )
	{
//		assemble_rsp( ASSEMBLER_RSP, ASSEMBLER_OPT );
		exec( ASSEMBLER, "@" ASSEMBLER_RSP );
		if ( !_keep_rsp )
			remove( ASSEMBLER_RSP );
//		if ( !_keep_gen )
//			ListIter( &f_asm, delete_temp );
//		ListDelete( &f_asm );
//		ListDelete( &l_def );
//		ListDelete( &l_inc );
	}
}


char *first( void ) {
	if ( strlen( _exename ) > 0 )
		return _exename;
	else if ( f_obj.count > 0 )
		return FileName( f_obj.head );
	else if ( f_lib.count > 0 )
		return FileName( f_lib.head );
	else
		return NULL;
}

char *libpath( char *lib ) {
	static char library[ _MAX_PATH ];
	strcpy( library, _libpath );
	if ( library[ strlen( library) - 1 ] != '\\' )
		strcat( library, "\\" );
	return strcat( library, lib );
}

static void out_obj( Item *p ) { fprintf( frsp, " %s", p->data ); }

void tlink_rsp( char *file ) {
	char *name = first();

	frsp = fopen( file, "wt" );
	if ( frsp == NULL )
		error( ERR_RUNTIME, file );
	fprintf( frsp, "/3/c/d" );
	if ( _map_file )
		fprintf( frsp, "/m/l/s" );
	else
		fprintf( frsp, "/x" );
	if ( _debug )
		fprintf( frsp, "/v" );
	if ( !_nodef_lib ) {
		if ( _output == 'p' )
			fprintf( frsp, " %s", \
				libpath( ( _debug ? "C0DOSD.OBJ" : "C0DOS.OBJ" ) )  );
		else
			fprintf( frsp, " %s", \
				libpath( ( _debug ? "C0DOSLD.OBJ" : "C0DOSL.OBJ" ) )  );
	}
	ListIter( &f_obj, out_obj );
	fprintf( frsp, ",%s,%s,", name, name );
	if ( !_nodef_lib )
		fprintf( frsp, " %s", libpath( "CLDOS.LIB" )  );
	ListIter( &f_lib, out_obj );
	fprintf( frsp, "\n" );
	fclose( frsp );
}

void tlink(void) {
//	tlink_rsp( LINKER_RSP );
	exec( TLINK, "@" LINKER_RSP );
}

static void wout_obj( Item *p) { fprintf( frsp, "file %s\n", p->data ); }
static void wout_lib( Item *p) { fprintf( frsp, "library %s\n", p->data ); }

void wlink_rsp( char *file ) {
	char *name = first();

	frsp = fopen( file, "wt" );
	if ( frsp == NULL )
		error( ERR_RUNTIME, file );
	fprintf( frsp, "# Generate from CL386.EXE\n" );
	fprintf( frsp, "format os2 le\n" );
	fprintf( frsp, "option nod\n" );
	if ( _map_file )
		fprintf( frsp, "option map\n" );
	if ( _debug )
		fprintf( frsp, "option symf\n" );
	if ( _output == 'w' )
		fprintf( frsp, "option osname='CC386+PMODE/W'\n"
					   "option stub=%s\n", libpath( "PMODEW.EXE" ) );
	else
		fprintf( frsp, "option osname='CC386+DOS/4GW'\n"
					   "option stub=%s\n", libpath( "D4GWSTUB.EXE" ) );
	fprintf( frsp, "name %s\n", name );

	if ( !_nodef_lib )
		fprintf( frsp, "file %s\n",
			libpath( ( _debug ? "C0DOSWD.OBJ" : "C0DOSW.OBJ" ) )  );

	ListIter( &f_obj, wout_obj );

	if ( !_nodef_lib )
		fprintf( frsp, "library %s", libpath( "CLDOS.LIB" )  );

	ListIter( &f_lib, wout_lib );
	fclose( frsp );
}

void wlink(void) {
//	wlink_rsp( LINKER_RSP );
	exec( WLINK, "@" LINKER_RSP );
}

void link(void) {
	if ( _link /* && ( f_obj.count > 0 || f_lib.count > 0 ) */ )
	{
		if ( _output == 'p' || _output == 'l' )
			tlink();
		else if ( _output == 'w' || _output == 'd' )
			wlink();
		else
			printf( "Unknow link!\n" );
		if ( !_keep_rsp )
			remove( LINKER_RSP );
//		if ( !_keep_gen )
//			ListIter( &f_obj, delete_temp );
//		ListDelete( &f_obj );
//		ListDelete( &f_lib );
	}
}

static void delete_file(Item *p) {
	if (p->temp)
		fprintf( frsp, "%s\n", p->data );
}

void delete_rsp( char *file ) {
	frsp = fopen( file, "wt" );
	if ( frsp == NULL )
		error( ERR_RUNTIME, file );
	ListIter( &f_asm, delete_file );
	ListIter( &f_obj, delete_file );
	fclose( frsp );
}

void response( void ) {
	if ( _compile && f_cpp.count > 0 ) {
		compile_rsp( COMPILER_RSP, COMPILER_OPT );
	}
	if ( _assemble && f_asm.count > 0 )	{
		assemble_rsp( ASSEMBLER_RSP, ASSEMBLER_OPT );
	}
	if ( _link && ( f_obj.count > 0 || f_lib.count > 0 ) ) {
		if ( _output == 'p' || _output == 'l' )
			tlink_rsp( LINKER_RSP );
		else if ( _output == 'w' || _output == 'd' )
			wlink_rsp( LINKER_RSP );
	}
	delete_rsp( DELETE_RSP );
}

void remove_temp( void ) {
	int len;
	char s[ _MAX_PATH ];

	if ( !_keep_gen ) {

		frsp = fopen( DELETE_RSP, "rt" );
		if ( frsp == NULL )
			error( ERR_RUNTIME, DELETE_RSP );

		while( fgets( s, _MAX_PATH, frsp ) != NULL ) {
			len = strlen( s );
			if ( len > 0 && s[ len - 1] == '\n' )
				s[ len - 1 ] = '\0';
			remove( s );
		}
		fclose( frsp );
	}
	if ( !_keep_rsp )
		remove( DELETE_RSP );
}
