
#include "stock.h" 

int ssearch_ ( key, f_keysize, index, f_nkeys, f_unique, compare, private, ns, ne, n_key, n_index )
char *key ;
int *f_keysize ;
char *index ;
int *f_nkeys ;
int *f_unique ;
int (*compare)() ;
void *private ;
int *ns ;
int *ne ;
int n_key ;
int n_index ;
{
	int keysize ; 
	int nkeys ; 
	int unique ; 
	int retcode;

	keysize = *f_keysize ; 
	nkeys = *f_nkeys ; 
	unique = *f_unique ; 

	retcode = ssearch(key, keysize, index, nkeys, unique, compare, private, ns, ne );
	(*ns)++ ; 
	(*ne)++ ; 

	return retcode;
 }


/* $Id: ssearch_.c,v 1.1.1.1 1997/04/12 04:19:02 danq Exp $ */
