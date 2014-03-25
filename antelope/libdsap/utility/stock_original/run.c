#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

#include "stock.h"

int 
runcmd ( char *argv[], char **result )  
{
    int		    tochild[2], fromchild[2] ;
    char	    *out ;
    int		    pid, statusp, n ;
    int		    nread, size ;

    if ( pipe (tochild) || pipe (fromchild)) {
	register_error ( 1, "Can't open pipes\n" ) ; 
	return -1 ;
    }
    pid = fork() ; 
    if ( pid == -1 ) {
	register_error ( 1, "Can't fork" ) ; 
	return -1 ;
    } else if ( pid == 0 ) {
	/* child */
	close(tochild[1]) ; 
	dup2(tochild[0], 0);
	close(fromchild[0]) ; 

	fclose(stdout) ;
	dup2(fromchild[1], 1);
	dup2(fromchild[1], 2) ;
	close(fromchild[1] ) ; 

	if ( execvp (argv[0], argv) ) 
	    die ( 1, "Couldn't exec %s\n", argv[0] ) ; 
    }
    /* parent */
    close(tochild[0]) ; 
    close(fromchild[1]) ; 
    size = 1024 ;
    allot ( char *, out, size ) ; 
    nread = 0 ;
    while ( n = read (fromchild[0], out+nread, size-nread) ) {
	nread += n ; 
	if ( n > size-nread ) {
	    size *= 2 ;
	    reallot ( char *, out, size ) ;
	}
    }
    waitpid ( pid, &statusp, 0 ) ; 
    close(tochild[1]) ; 
    close(fromchild[0]) ; 
    *result = out ;
    return WEXITSTATUS(statusp) ;
}
