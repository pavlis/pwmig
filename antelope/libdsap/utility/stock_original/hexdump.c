/*  Copyright (c) 1997 Boulder Real Time Technologies, Inc.           */
/*                                                                    */
/*  This software module is wholly owned by Boulder Real Time         */
/*  Technologies, Inc. Any use of this software module without        */
/*  express written permission from Boulder Real Time Technologies,   */
/*  Inc. is prohibited.                                               */


#include <stdio.h>
#include <ctype.h>

#include "stock.h"

#define ESCAPE '\\'

void
hexdump ( file, memory, l )
FILE *file ;
void *memory ;
int l ;
{
    int i ;
    unsigned char *a ;
    char ascii[17], *ap ;
 
    a = (unsigned char *) memory ;
 
    ap = ascii ;
    for ( i=0 ; i<l ; i++) {
        if ( i % 16 == 0 ) fprintf ( file, "\t%08x ", i ) ;
        if ( i % 2 == 0 ) fprintf ( file, " " ) ;
        fprintf ( file, "%02x", *a ) ;
        *ap++ = isprint (*a) ? *a : '_' ;
        if ( i % 16 == 15 ) {
            *ap = 0 ;
            fprintf ( file, " %s\n", ascii );
            ap = ascii ;
        }
        a++ ;
    }
    i = i % 16 ;
    if ( i> 0 ) {
        for( ; i< 16 ; i++ ) {
            if ( i % 2 == 0 ) fprintf ( file, " " ) ;
            fprintf ( file, "  " ) ;
        }
        *ap = 0 ;
        fprintf ( file, " %s\n", ascii );
    }
}
 

void 
asciidump ( file, memory, l ) 
FILE *file ;
char *memory ; 
int l ; 
{
    int i, c, linecount ; 

    linecount = 0 ; 
    for ( i=0 ; i<l ; i++ ) {
	switch ( c = memory[i])  {
	    case '\n' :
		fputc ( c, file ) ; 
		linecount = 0  ;
		break ;

	    case '\t' :
		fputc ( c, file ) ; 
		linecount ++ ;
		break ;

	    default :
		if ( isprint(c) ) {
		    if ( c == ESCAPE ) {
			fputc( ESCAPE, file); 
			fputc( ESCAPE, file); 
			linecount += 2 ;
		    } else {
			fputc ( c, file ) ; 
			linecount ++ ;
		    }
		} else {
		    fprintf ( file, "%c0x%02x", ESCAPE, c & 0xff ) ;
		    linecount += 5 ;
		}
	}
	if ( linecount >= 80 ) {
	    fputc ( '\n', file ) ; 
	    linecount = 0 ; 
	}
    }

}

int
hex2int ( char c ) 
{
    int val = 0 ;
    switch (c) {
	case '0' :
	    val = 0 ;
	    break; 

	case '1' :
	    val = 1 ;
	    break; 

	case '2' :
	    val = 2 ;
	    break; 

	case '3' :
	    val = 3 ;
	    break; 

	case '4' :
	    val = 4 ;
	    break; 


	case '5' :
	    val = 5 ;
	    break; 

	case '6' :
	    val = 6 ;
	    break; 

	case '7' :
	    val = 7 ;
	    break; 

	case '8' :
	    val = 8 ;
	    break; 

	case '9' :
	    val = 9 ;
	    break; 

	case 'a' :
	case 'A' :
	    val = 10 ;
	    break; 

	case 'b' :
	case 'B' :
	    val = 11 ;
	    break; 

	case 'c' :
	case 'C' :
	    val = 12 ;
	    break; 

	case 'd' :
	case 'D' :
	    val = 13 ;
	    break; 

	case 'e' :
	case 'E' :
	    val = 14 ;
	    break; 

	case 'f' :
	case 'F' :
	    val = 15 ;
	    break; 
    }
    return val ; 
}

void 
read_asciidump ( file, memory, l ) 
FILE *file ;
char *memory ; 
int l ; 
{
    int i, c, linecount ; 
    char temp[4] ;

    linecount = 0 ; 
    for ( i=0 ; i<l ; ) {
	switch ( c = fgetc(file))  {
	    case ESCAPE :
		c = fgetc(file) ;
		if ( c == ESCAPE ) {
		    memory[i++] = ESCAPE ; 
		    linecount++ ; 
		} else {
		    /* string should be \0xXX, but don't check 
		     * for 0x
		     */
		    fread ( temp, 3, 1, file ) ; 
		    c = hex2int(temp[1]) * 16 + hex2int(temp[2]) ;
		    memory[i++] = c ; 
		    linecount += 5 ; 
		}
		break; 

	    case '\n':
		memory[i++] = c ; 
		linecount = 0 ;
		break; 

	    default:
		memory[i++] = c ; 
		linecount ++ ;
		break; 
	}

	if ( linecount >= 80 ) {
	    /* should be linefeed, but don't check */
	    c = fgetc(file) ;
	    linecount = 0 ; 
	}
    }
}

void
read_hexdump ( file, memory, l )  
FILE *file ;
char *memory ; 
int l ; 
{
    int i, c, mode ; 
    char aline[STRSZ], *address, *value ;

    mode = 8 ;
    for ( i=0 ; i<l ; ) {
	switch (mode) {
	    case 8 : /* ready to read a new line */
		if (fgets ( aline, STRSZ, file ) == 0) 
		    return ;

		address = strtok ( aline, " \t" ) ;
		mode = 0 ; 
		break; 

	    case 0: /* ready to scan in value #i */
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    case 5:
	    case 6:
	    case 7:
		value = strtok ( 0, " \t" ) ; 
		if ( value == 0 ) 
		    return ; 
		else
		    mode ++ ;
		c = hex2int(value[0]) * 16 + hex2int(value[1]) ;
		memory[i++] = c ; 
		if ( strlen(value) == 4 ) {
		    c = hex2int(value[2]) * 16 + hex2int(value[3]) ;
		    memory[i++] = c ; 
		}
		break ;
	}

    }
}

int
printable ( char *s, int n )
{
    int i ;
    for ( i = 0 ; i<n ; i++ ) {
        if ( ! isprint(s[i]) ) {
            if ( s[i] == 0 )
                return 1 ;
            else
                return 0 ;
        }
    }
    return 2 ;
}
 
void
Xchardump ( FILE *file, char *label, char *desc, char *bytes, int nbytes )
{
    char *copy ;
    switch ( printable ( bytes, nbytes ) ) {
        case 1:
        case 2:
	    fputs ( label, file ) ;
	    fputs ( " = '", file ) ;
	    fputs ( bytes, file ) ;
	    fputs ( "' ", file ) ; 
	    fputs ( desc, file ) ; 
	    fputs ( "\n", file ) ;  
            break;
 
        case 0:
        default:
            fprintf ( file, "%s (%d bytes) # %s\n", label, nbytes, desc );
            hexdump ( file, bytes, nbytes ) ;
            break;
    }
}

/* $Id: hexdump.c,v 1.4 1997/10/06 04:56:21 danq Exp $ */
