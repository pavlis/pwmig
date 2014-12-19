#ifndef __elog__
#define __elog__

extern char *Program_Name ;

#define ELOG_NSEVERITY   5
#define ELOG_LOG 	0
#define ELOG_NOTIFY 	1
#define ELOG_COMPLAIN 	2
#define ELOG_DIE	3
#define ELOG_FAULT	4

enum Elog_code { 
	ELOG_TBL,
	ELOG_ARGV,
	ELOG_ARGC,
	ELOG_COUNT,
	ELOG_VERSION,
	ELOG_MAXMSG,
	ELOG_TAG,
	ELOG_DELIVER,
	ELOG_DEBUG,
	ELOG_SIGNALS,
	ELOG_RTMSEND 
    } ;

typedef struct Elog_msg { 
    char *tag ; 	/* tag -- program name, pid, time, ... */
    char *msg ; 	/* msg from program */
    char *sys ;		/* system error message */
    int severity ;	/* severity level */
    int n ; 		/* # of repetitions */
} Elog_msg ;

typedef struct Signal_info {
    int sig ; 
    char *name, *description ; 
} Signal_info ;

#define register_error       	elog_log
/*#define notify   	     	elog_notify*/
#define complain 		elog_complain
#define die      elog_die
/* #define fault    elog_fault*/
#define clear_register elog_clear_register

#ifdef __STDC__
#define PL_(x) x
#else
#define PL_(x) ( )
#endif /* __STDC__ */
 
extern int elog_init PL_(( int argc, char **argv ));

/*PRINTFLIKE2*/
extern void elog_log PL_(( int flag, char *format, ... ));

/*PRINTFLIKE2*/
extern void elog_notify PL_(( int flag, char *format, ... ));

/*PRINTFLIKE2*/
extern void elog_complain PL_(( int flag, char *format, ... ));

/*PRINTFLIKE2*/
extern void elog_die PL_(( int flag, char *format, ... ));

/*PRINTFLIKE2*/
extern void elog_fault PL_(( int flag, char *format, ... ));

extern void elog_msg_print PL_(( FILE *file, Elog_msg *elog_msg ));
extern void elog_print PL_(( FILE *file, int first ));
extern void elog_free_msg PL_(( void *vmsg ));
extern void elog_clear PL_(( void ));
extern void elog_clear_register PL_(( int flag ));
extern void elog_flush PL_(( int deliver, int n )) ; 

extern int elog_query PL_(( enum Elog_code code, int severity, void **value ));
extern int elog_set PL_(( enum Elog_code code, int severity, void *value ));

extern Signal_info *signal_bysig PL_(( int sig ));
extern Signal_info *signal_byname PL_(( char *name ));

extern int elog_mark PL_(( void ));

/*
The following can be removed at some point; they're here only to keep programs
linked with the old interface and libstock.so running until they're recompiled.
*/
extern void register_error PL_(( int flag, char *format, ... ));
extern void complain PL_(( int flag, char *format, ... ));
extern void die PL_(( int flag, char *format, ... ));
extern void clear_register PL_(( int deliver ));
extern void get_register PL_(( char ***msgs, int **msgcount, int *maxmsgs, int *last, int *wrap ));
 
#undef PL_

#endif

/* $Id: elog.h,v 1.4 1997/07/23 03:59:23 danq Exp $ */
