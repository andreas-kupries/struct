#include "cstackInt.h"

/*
 * = = == === ===== ======== ============= =====================
 */

CSTACK
cstack_create (CSTACK_CELL_FREE freeCell, void* clientdata)
{
    CSTACK s = ALLOC (CSTACK_);
    s->cell = NALLOC (CSTACK_INITIAL_SIZE, void*);
    s->max  = CSTACK_INITIAL_SIZE;
    s->top  = 0;
    s->freeCell   = freeCell;
    s->clientData = clientdata;

    return s;
}

void
cstack_destroy (CSTACK s)
{
    if (s->freeCell && s->top) {
	long int i;
	for (i=0; i < s->top; i++) {
	    ASSERT_BOUNDS(i,s->max);
	    s->freeCell ( s->cell [i] );
	}
    }

    ckfree ((char*) s->cell);
    ckfree ((char*) s);
}

/*
 * = = == === ===== ======== ============= =====================
 */

long int
cstack_size (CSTACK s)
{
    return s->top;
}

void*
cstack_top (CSTACK s)
{
    ASSERT_BOUNDS(s->top-1,s->max);
    return s->cell [s->top - 1];
}

void*
cstack_at (CSTACK s, long int i)
{
    /* (i == 0)     ==> top    */
    /* (i == top-1) ==> bottom */

    ASSERT_BOUNDS(s->top-1,s->max);
    ASSERT_BOUNDS(i,s->top);
    return s->cell [s->top-i-1];
}

void*
cstack_atr (CSTACK s, long int i)
{
    /* Reversed indexing */
    /* (i == top-1) ==> top    */
    /* (i == 0)     ==> bottom */

    ASSERT_BOUNDS(s->top-1,s->max);
    ASSERT_BOUNDS(i,s->top);
    return s->cell [i];
}

CSLICE
cstack_get (CSTACK s, long int at, long int n, CSLICE_DIRECTION dir)
{
    /* (at == 0)     ==> top    */
    /* (at == top-1) ==> bottom */

    ASSERT_BOUNDS (at,     s->top);
    ASSERT_BOUNDS (at+n-1, s->top);

    /*
     * Note the double negation below. To get the normal order of the result,
     * the order has to be reversed. To get the reverted order, nothing is to
     * be done. So we revers on dir == cslice_normal.
     *
     * An optimization: We know that the direction is irrrelevant when
     * returning a single element, therefore we can use the code path not
     * doing allocation.
     */

    if ((dir == cslice_revers) || (n < 2)) {
	return cslice_create (cslice_normal, s->cell + (s->top - at - n), n);
    } else {
	return cslice_create (cslice_revers, s->cell + (s->top - at - n), n);
    }
}

CSLICE
cstack_getr (CSTACK s, long int at, long int n, CSLICE_DIRECTION dir)
{
    /* Reversed indexing */
    /* (at == top-1) ==> top    */
    /* (at == 0)     ==> bottom */

    ASSERT_BOUNDS (at,     s->top);
    ASSERT_BOUNDS (at+n-1, s->top);

    /*
     * Note the double negation below. To get the normal order of the result,
     * the order has to be reversed. To get the reverted order, nothing is to
     * be done. So we revers on dir == cslice_normal.
     *
     * An optimization: We know that the direction is irrrelevant when
     * returning a single element, therefore we can use the code path not
     * doing allocation.
     */

    if ((dir == cslice_revers) || (n < 2)) {
	return cslice_create (cslice_normal, s->cell + at, n);
    } else {
	return cslice_create (cslice_revers, s->cell + at, n);
    }
}

/*
 * = = == === ===== ======== ============= =====================
 */

void
cstack_push (CSTACK s, void* item)
{
    if (s->top >= s->max) {
	long int new  = s->max ? (2 * s->max) : CSTACK_INITIAL_SIZE;
	void**   cell = (void**) ckrealloc ((char*) s->cell, new * sizeof(void*));
	ASSERT (cell,"Memory allocation failure for cstack");
	s->max  = new;
	s->cell = cell;
    }

    ASSERT_BOUNDS(s->top,s->max);
    s->cell [s->top] = item;
    s->top ++;
}

void
cstack_pop (CSTACK s, long int n)
{
    ASSERT (n >= 0, "Bad pop count");
    if (n == 0) return;

    if (s->freeCell) {
	while (n) {
	    s->top --;
	    ASSERT_BOUNDS(s->top,s->max);
	    s->freeCell ( s->cell [s->top] );
	    n --;
	}
    } else {
	s->top -= n;
    }
}

void
cstack_trim (CSTACK s, long int n)
{
    ASSERT (n >= 0, "Bad trimsize");

    if (s->freeCell) {
	while (s->top > n) {
	    s->top --;
	    ASSERT_BOUNDS(s->top,s->max);
	    s->freeCell ( s->cell [s->top] );
	}
    } else {
	s->top = n;
    }
}

void
cstack_drop (CSTACK s, long int n)
{
    ASSERT (n >= 0, "Bad pop count");
    if (n == 0) return;
    s->top -= n;
}

void
cstack_rol (CSTACK s, long int n, long int steps)
{
    long int i, j, start = s->top - n;
    void**   cell = s->cell;
    void**   tmp;

    steps = steps % n;
    while (steps < 0) steps += n;
    steps = n - steps;
    cell += start;

    tmp = NALLOC(n,void*);

    for (i = 0; i < n; i++) {
	j = (i + steps) % n;
	ASSERT_BOUNDS (i,n);
	ASSERT_BOUNDS (j,n);
	tmp[i] = cell [j];
    }
    for (i = 0; i < n; i++) {
	ASSERT_BOUNDS (i,n);
	cell [i] = tmp [i];
    }

    ckfree ((char*) tmp);
}

void
cstack_move (CSTACK dst, CSTACK src)
{
    ASSERT (dst->freeCell == src->freeCell, "Ownership mismatch");

    /*
     * Note: The destination takes ownership of the moved cell, thus there is
     * no need to run free on them.
     */

    while (src->top > 0) {
	src->top --;
	ASSERT_BOUNDS(src->top,src->max);
	cstack_push (dst, src->cell [src->top] );
    }
}

/*
 * = = == === ===== ======== ============= =====================
 */

void
cstack_clientdata_set (CSTACK s, void* clientdata)
{
    s->clientData = clientdata;
}

void*
cstack_clientdata_get (CSTACK s)
{
    return s->clientData;
}

/*
 * = = == === ===== ======== ============= =====================
 */


/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
