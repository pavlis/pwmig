#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
int isnetorder( void );
int rev2 (unsigned char *from, unsigned char *to, int n);
int rev4 (unsigned char *from, unsigned char *to, int n);
int rev8 (unsigned char *from, unsigned char *to, int n);

double htond (val)

double        val;

{
	if (!isnetorder()) rev8 ((unsigned char *)&val, (unsigned char *)&val, 1);
	return (val);
}

double ntohd (val)

double        val;

{
	if (!isnetorder()) rev8 ((unsigned char *)&val, (unsigned char *)&val, 1);
	return (val);
}

void htondp (valp_from, valp_to)

double *     valp_from;
double *                valp_to;

{
	if (!isnetorder()) rev8 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) *valp_to = *valp_from;
}

void ntohdp (valp_from, valp_to)

double *     valp_from;
double *                valp_to;

{
	if (!isnetorder()) rev8 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) *valp_to = *valp_from;
}

float htonf (val)

float	     val;

{
	if (!isnetorder()) rev4 ((unsigned char *)&val, (unsigned char *)&val, 1);
	return (val);
}

float ntohf (val)

float        val;

{
	if (!isnetorder()) rev4 ((unsigned char *)&val, (unsigned char *)&val, 1);
	return (val);
}

void htonfp (valp_from, valp_to)

float *     valp_from;
float *                valp_to;

{
	if (!isnetorder()) rev4 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) *valp_to = *valp_from;
}

void ntohfp (valp_from, valp_to)

float *     valp_from;
float *                valp_to;

{
	if (!isnetorder()) rev4 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) *valp_to = *valp_from;
}

void hton4p (void *valp_from, void *valp_to)

{
	if (!isnetorder()) rev4 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) memcpy (valp_to, valp_from, 4);
}

void ntoh4p (void *valp_from, void *valp_to)

{
	if (!isnetorder()) rev4 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) memcpy (valp_to, valp_from, 4);
}

void hton8p (void *valp_from, void *valp_to)

{
	if (!isnetorder()) rev8 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) memcpy (valp_to, valp_from, 8);
}

void ntoh8p (void *valp_from, void *valp_to)

{
	if (!isnetorder()) rev8 ((unsigned char *)valp_from, (unsigned char *)valp_to, 1);
	else if (valp_from != valp_to) memcpy (valp_to, valp_from, 8);
}

short int revshort ( short int val )

{
	short int rval;

	rev2 ((unsigned char *)&val, (unsigned char *)&rval, 1);
	return ( rval );
}

int revint ( int val )

{
	int rval;

	rev4 ((unsigned char *)&val, (unsigned char *)&rval, 1);
	return ( rval );
}

 
int
rev2 (from, to, n)
 
unsigned char *from, *to;
int n;
 
{
        unsigned char word[2];
        int i;
 
        for (i=0; i<n; i++) {
                word[1] = *(from++);
                word[0] = *(from++);
                *(to++) = word[0];
                *(to++) = word[1];
        }
}
 
 
int
rev4 (from, to, n)
 
unsigned char *from, *to;
int n;
 
{
        unsigned char word[4];
        int i;
 
        for (i=0; i<n; i++) {
                word[3] = *(from++);
                word[2] = *(from++);
                word[1] = *(from++);
                word[0] = *(from++);
                *(to++) = word[0];
                *(to++) = word[1];
                *(to++) = word[2];
                *(to++) = word[3];
        }
}
 
int
rev8 (from, to, n)
 
unsigned char *from, *to;
int n;
 
{
        unsigned char word[8];
        int i;
 
        for (i=0; i<n; i++) {
                word[7] = *(from++);
                word[6] = *(from++);
                word[5] = *(from++);
                word[4] = *(from++);
                word[3] = *(from++);
                word[2] = *(from++);
                word[1] = *(from++);
                word[0] = *(from++);
                *(to++) = word[0];
                *(to++) = word[1];
                *(to++) = word[2];
                *(to++) = word[3];
                *(to++) = word[4];
                *(to++) = word[5];
                *(to++) = word[6];
                *(to++) = word[7];
        }
}

int
isnetorder ()

{
	static int netorder = -1;
	int i, j;

	if (netorder >= 0) return (netorder);
	i = 1234567890;
	j = htonl(i);
	if (j == i) netorder = 1; else netorder = 0;
	return (netorder);
}
 

/* $Id: htond.c,v 1.1.1.1 1997/04/12 04:19:00 danq Exp $ */
