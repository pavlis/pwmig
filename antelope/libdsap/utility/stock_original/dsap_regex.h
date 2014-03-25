#ifndef __re_compile__
#define __re_compile__
/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#define NSUBEXP  10
typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
	char *regmust;		/* Internal use only. */
	int regmlen;		/* Internal use only. */
	char program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

#define	MAGIC	0234

typedef struct re_pattern_buffer
{
  regexp *buffer;
  unsigned long allocated;
  unsigned long used;	
  int regs_allocated ;
  int syntax;
  char *fastmap;
  char *translate;
  size_t re_nsub;
} re_pattern_buffer ;

struct re_registers { 
    char *null ; 
} ;
#define RE_SYNTAX_EGREP 0
#define REGS_UNALLOCATED 0
#define REGS_FIXED 0
extern int re_syntax_options ;

#if __STDC__
#define PL_(x) x
#else
#define PL_(x) ( )
#endif /* __STDC__ */
 
extern char * re_compile_pattern PL_(( char *exp, int length, re_pattern_buffer *prog ));
extern int re_search PL_(( re_pattern_buffer *prog, char *string, int size, int startpos, int range, void *regs ));
extern int re_match ( re_pattern_buffer *buffer, char *string, int size, int startpos, void *regs );
extern void re_free PL_(( re_pattern_buffer *prog ));
extern int patsub ( char *old, struct re_pattern_buffer *pattern_buffer, char *sub, char *new );
 
#undef PL_

#endif

/* $Id: dsap_regex.h,v 1.3 1998/05/28 19:52:01 danq Exp $ */
