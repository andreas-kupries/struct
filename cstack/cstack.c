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
cstack_bottom (CSTACK s)
{
    ASSERT_BOUNDS(0,s->top);
    return s->cell [0];
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
cstack_get (CSTACK s, long int at, long int n)
{
    /* (at == 0)     ==> top    */
    /* (at == top-1) ==> bottom */

    ASSERT_BOUNDS (at,              s->top);
    ASSERT_BOUNDS (s->top-at-1+n-1, s->top);

    return cslice_create (n, s->cell + (s->top - at - 1));
}

CSLICE
cstack_getr (CSTACK s, long int at, long int n)
{
    /* Reversed indexing */
    /* (at == top-1) ==> top    */
    /* (at == 0)     ==> bottom */

    ASSERT_BOUNDS (at,     s->top);
    ASSERT_BOUNDS (at+n-1, s->top);

    return cslice_create (n, s->cell + at);
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
cstack_push_slice (CSTACK s, CSLICE sl)
{
    void** cell;
    long int n, i, need;

    cslice_get (sl, &n, &cell);
    need = s->top + n;

    if (need >= s->max) {
	long int expand = s->max ? (2 * s->max) : CSTACK_INITIAL_SIZE;
	long int new = MAX (need, expand);
	void**   cell = (void**) ckrealloc ((char*) s->cell, new * sizeof(void*));
	ASSERT (cell,"Memory allocation failure for cstack");
	s->max  = new;
	s->cell = cell;
    }

    ASSERT_BOUNDS(s->top,s->max);
    for (i=0;i<n;i++) {
	s->cell [s->top] = cell [i];
	s->top ++;
    }
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
cstack_clear (CSTACK s)
{
    if (s->freeCell) {
	while (s->top) {
	    s->top --;
	    ASSERT_BOUNDS(s->top,s->max);
	    s->freeCell ( s->cell [s->top] );
	}
    } else {
	s->top = 0;
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
cstack_drop_all (CSTACK s)
{
    s->top = 0;
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
cstack_move (CSTACK dst, CSTACK src, long int n)
{
    ASSERT (dst->freeCell == src->freeCell, "Ownership mismatch");
    ASSERT ((0 < n) && (n <= cstack_size (src)), "Bad move count")

    /*
     * Note: The destination takes ownership of the moved cell, thus there is
     * no need to run free on them.
     */

    while (n > 0) {
	src->top --;
	n --;
	ASSERT_BOUNDS(src->top,src->max);
	cstack_push (dst, src->cell [src->top] );
    }
}

void
cstack_move_all (CSTACK dst, CSTACK src)
{
    cstack_move (dst, src, cstack_size (src));
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
