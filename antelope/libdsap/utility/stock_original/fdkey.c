/*  Copyright (c) 1997 Boulder Real Time Technologies, Inc.           */
/*                                                                    */
/*  This software module is wholly owned by Boulder Real Time         */
/*  Technologies, Inc. Any use of this software module without        */
/*  express written permission from Boulder Real Time Technologies,   */
/*  Inc. is prohibited.                                               */

#include <stropts.h>
#include <poll.h>

#ifndef POLLRDNORM
#define POLLRDNORM 0
#endif
 
#ifndef POLLRDBAND
#define POLLRDBAND 0  
#endif
 
int
fdkey (fd)
int fd ;
{
    struct      pollfd fds2poll[1] ;
    unsigned long nfds = 1 ;
    int timeout = 0, retcode ;
 
    fds2poll[0].fd = fd ;
    fds2poll[0].events = POLLIN | POLLRDNORM | POLLPRI | POLLRDBAND | POLLERR ;
 
    switch ( poll(fds2poll, nfds, timeout) ) {
        case 1:
            if ( fds2poll[0].revents & POLLIN
             || fds2poll[0].revents & POLLRDNORM
             || fds2poll[0].revents & POLLRDBAND
             || fds2poll[0].revents & POLLPRI ) {
                retcode = 1 ;
             } else
                retcode = -1 ;
            break ;
 
        case 0:
            retcode = 0 ;
            break ;
 
        default:
            retcode = -1 ;
            break ;
    }
 
    return retcode;
}
 
int
fdwait (int fd, int msec) 
{
    struct      pollfd fds2poll[1] ;
    unsigned long nfds = 1 ;
    int retcode ;
 
    fds2poll[0].fd = fd ;
    fds2poll[0].events = POLLIN | POLLRDNORM | POLLPRI | POLLRDBAND | POLLERR ;
 
    switch ( poll(fds2poll, nfds, msec) ) {
        case 1:
            if ( fds2poll[0].revents & POLLIN
             || fds2poll[0].revents & POLLRDNORM
             || fds2poll[0].revents & POLLRDBAND
             || fds2poll[0].revents & POLLPRI ) {
                retcode = 1 ;
             } else
                retcode = -1 ;
            break ;
 
        case 0:
            retcode = 0 ;
            break ;
 
        default:
            retcode = -1 ;
            break ;
    }
 
    return retcode;
}
 
/* $Id: fdkey.c,v 1.3 1997/09/22 16:31:04 danq Exp $ */
