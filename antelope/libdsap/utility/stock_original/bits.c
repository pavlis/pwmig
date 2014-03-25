
#include "stock.h"

#define ELEMENT(I)	((I)/BITS_PER_INT)
#define BIT(I)		(BITS_PER_INT - ((I) % BITS_PER_INT) - 1)
#define BITMASK(I)	(1<<BIT(I))

int 
bitset (bvec, index)
     Bitvector *bvec;
     int index;
{
    int             element,
                    bit;
    int             retcode,
                    value;

    element = ELEMENT (index);
    bit = BITMASK (index);
    value = (int) gettbl (bvec, element);
    retcode = ((bit & value) != 0);
    value |= bit;
    settbl (bvec, element, (char *) value);
    return retcode;
}

int 
bitclr (bvec, index)
     Bitvector *bvec;
     int index;
{
    int             element,
                    bit;
    int             retcode,
                    value;

    element = ELEMENT (index);
    bit = BITMASK (index);
    value = (int) gettbl (bvec, element);
    retcode = ((bit & value) != 0);
    value &= ~bit;
    settbl (bvec, element, (char *) value);
    return retcode;
}

int 
bittst (bvec, index)
     Bitvector *bvec;
     int index;
{
    int             element,
                    bit;
    int             retcode,
                    value;

    element = ELEMENT (index);
    bit = BITMASK (index);
    value = (int) gettbl (bvec, element);
    retcode = ((bit & value) != 0);
    return retcode;
}

int
bitmax (b)
     Bitvector *b; 
{
    return maxtbl(b)* BITS_PER_INT ; 
}

Bitvector      *
bitand (b1, b2)
     Bitvector *b1;
     Bitvector *b2;
{
    Bitvector      *result;
    int             n1,
                    n2,
                    n,
                    i,
                    r1,
                    r2,
                    r;

    result = bitnew ();

    n1 = maxtbl (b1);
    n2 = maxtbl (b2);
    n = MIN (n1, n2);

    for (i = 0; i < n; i++) {
	r1 = (int) gettbl (b1, i);
	r2 = (int) gettbl (b2, i);
	r = r1 & r2;
	settbl (result, i, (char *) r);
    }

    return result;
}

Bitvector      *
bitor (b1, b2)
     Bitvector *b1;
     Bitvector *b2;
{
    Bitvector      *result;
    int             n1,
                    n2,
                    n,
                    i,
                    r1,
                    r2,
                    r;
    result = bitnew ();

    n1 = maxtbl (b1);
    n2 = maxtbl (b2);
    n = MAX (n1, n2);

    for (i = 0; i < n; i++) {
	r1 = (int) gettbl (b1, i);
	r2 = (int) gettbl (b2, i);
	r = r1 | r2;
	settbl (result, i, (char *) r);
    }

    return result;
}

Bitvector      *
bitnot (b)
     Bitvector *b;
{
    Bitvector      *result;
    int             n,
                    i,
                    r;
    result = bitnew ();

    n = maxtbl (b);

    for (i = 0; i < n; i++) {
	r = (int) gettbl (b, i);
	r = ~r;
	settbl (result, i, (char *) r);
    }

    return result;
}

Bitvector      *
bitxor (b1, b2)
     Bitvector *b1;
     Bitvector *b2;
{
    Bitvector      *result;
    int             n1,
                    n2,
                    n,
                    i,
                    r1,
                    r2,
                    r;
    result = bitnew ();

    n1 = maxtbl (b1);
    n2 = maxtbl (b2);
    n = MAX (n1, n2);

    for (i = 0; i < n; i++) {
	r1 = (int) gettbl (b1, i);
	r2 = (int) gettbl (b2, i);
	r = r1 ^ r2;
	settbl (result, i, (char *) r);
    }

    return result;
}

/* $Id: bits.c,v 1.1.1.1 1997/04/12 04:18:59 danq Exp $ */
