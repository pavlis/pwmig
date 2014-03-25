#include <stdlib.h>
#include <stdio.h>
#include "dsap_regex.h"
#include "stock.h"

int patsub ( old, pattern_buffer, sub, new ) 
char *old ; 
struct re_pattern_buffer *pattern_buffer ; 
char *sub ;
char *new ; 
{
    int n ; 
    char *op = old ; 
    char c ; 
    int cnt ; 
    int result ;
    int nregs = NSUBEXP ; 
    struct re_registers regs ; 

    result = re_search ( pattern_buffer, 
	    old, 
	    strlen(old), 
	    0, 
	    strlen(old),
	    &regs ) ; 

    if ( result < 0 ) 
	{
	strcpy ( new, old ) ; 
	return 0 ; 
	}

    *new = 0 ; 
    cnt = pattern_buffer->buffer->startp[0] - old ; 
    while ( cnt-- > 0 ) 
	*new++ = *op++ ; 

    while ( c = *sub++ )
	{
	if ( c == '$' ) 
	    {
	    c = *sub++ ; 
	    n = c - '0' ; 
	    if ( n < nregs && n >= 0 && pattern_buffer->buffer->startp[n] != 0) 
		{
		op = pattern_buffer->buffer->startp[n] ; 
		cnt = pattern_buffer->buffer->endp[n] - pattern_buffer->buffer->startp[n] ; 
		while ( cnt-- > 0 ) 
		    *new++ = *op++ ; 
		}
	    }
	else if ( c == '\\' ) 
	    *new++ = *sub++ ;
	else
	    *new++ = c ;
	}

    op = pattern_buffer->buffer->endp[0] ;

    while ( c = *op++ ) 
	*new++ = c ; 

    *new = 0 ; 
    return 1 ; 
}

/* $Id: patsub.c,v 1.2 1997/07/21 00:41:29 danq Exp $ */
