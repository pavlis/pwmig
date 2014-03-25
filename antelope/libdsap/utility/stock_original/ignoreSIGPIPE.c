#include <sys/ucontext.h>
#include <signal.h>
#include <stdlib.h>

#include "stock.h"


static void
ignore(int sig, siginfo_t *sip, ucontext_t *uap)
{
    return ;
}

void
ignoreSIGPIPE()
{
    struct sigaction action;

    action.sa_handler = ignore ;
    sigemptyset(&action.sa_mask) ;
    action.sa_flags = SA_RESTART ;

    insist (sigaction(SIGPIPE, &action, (struct sigaction *)0) == 0)  ;
}
