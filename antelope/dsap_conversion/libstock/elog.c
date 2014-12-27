/* This is a grossly simplified of elog utilities from antelope.
   The original has fancy things to push errors to a common list
   and accumulate them.  Also made for multithreading.   This is
   much simpler just writing messages to stderr using variable
   arguments to match the original interface used in antelope.
   This is patterned loosely after the old dsap 3.4 version of this code.
   */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "elog.h"
void elog_log (int flag, const char *format, ...)
{
    va_list args ;
    va_start(args, format) ; 
    fprintf(stderr,"Logged Message:  ");
    vfprintf(stderr,format,args);
}
void elog_notify (int flag, const char *format, ...)
{
    va_list args ;
    va_start(args, format) ; 
    fprintf(stderr,"WARNING(possible problem:  ");
    vfprintf(stderr,format,args);
}


void elog_complain (int flag, const char *format, ...)
{
    va_list args ;
    va_start(args, format) ; 
    fprintf(stderr,"WARNING (Serious error):  ");
    vfprintf(stderr,format,args);
    fprintf(stderr,"\nResults may not be reliable\n");
}


void elog_die (int flag, const char *format, ...)
{
    va_list args ;
    va_start(args, format) ; 
    fprintf(stderr,"Fatal Error:  ");
    vfprintf(stderr,format,args);
    exit (1) ;
}

