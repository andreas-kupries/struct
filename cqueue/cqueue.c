#include "cqueueInt.h"

/*
 * = = == === ===== ======== ============= =====================
 */

static CSTACK
NewStack (CQUEUE q) {
    CSTACK result;
    if (q->hold) {
	result = q->hold;
	q->hold = 0;
    } else {
	result = cstack_create (q->freeCell, 0);
    }
    return result;
}

static void
HoldStack (CQUEUE q, CSTACK* s) {
    CSTACK result;
    if (q->hold) {
	cstack_destroy (*s);
    } else {
	q->hold = *s;
	cstack_clear (*s);
    }
    *s = 0;
}

/*
 * = = == === ===== ======== ============= =====================
 */

CQUEUE
cqueue_create (CQUEUE_CELL_FREE freeCell, void* clientdata)
{
    CQUEUE q = ALLOC (CQUEUE_);

    q->unget      = 0;
    q->result     = 0;
    q->append     = 0;
    q->at         = 0;
    q->freeCell   = freeCell;
    q->clientData = clientdata;
    q-> hold      = 0;

    return q;
}

void
cqueue_destroy (CQUEUE q)
{
    if (q->unget)  cstack_destroy (q->unget);
    if (q->result) cstack_destroy (q->result);
    if (q->append) cstack_destroy (q->append);
    if (q->hold)   cstack_destroy (q->hold);

    ckfree ((char*) q);
}

/*
 * = = == === ===== ======== ============= =====================
 */

long int
cqueue_size (CQUEUE q)
{
    return
	(q->unget  ? cstack_size (q->unget)  : 0) +
	(q->result ? cstack_size (q->result) : 0) +
	(q->append ? cstack_size (q->append) : 0) -
	q->at;
}

void*
cqueue_head (CQUEUE q)
{
    if (q->unget) {
	ASSERT (cstack_size (q->unget) > 0, "non-null unget is empty");
	return cstack_top (q->unget);
    }
    if (q->result && (q->at < cstack_size (q->result))) {
	return cstack_atr (q->result, q->at);
    }
    if (q->append) {
	ASSERT (cstack_size (q->append) > 0, "non-null append is empty");
	return cstack_atr (q->append, 0); /* bottom */
    }

    ASSERT (0, "Not enough elements in the cqueue");
    return 0;
}

void*
cqueue_tail (CQUEUE q)
{
    if (q->append) {
	ASSERT (cstack_size (q->append) > 0, "non-null append is empty");
	return cstack_top (q->append);
    }
    if (q->result && (q->at < cstack_size (q->result))) {
	return cstack_top (q->result);
    }
    if (q->unget) {
	ASSERT (cstack_size (q->unget) > 0, "non-null unget is empty");
	return cstack_atr (q->unget, 0); /* bottom */
    }

    ASSERT (0, "Not enough elements in the cqueue");
    return 0;
}

CSLICE
cqueue_get (CQUEUE q, long int n, CSLICE_DIRECTION dir)
{
    CSLICE result = 0;
    ASSERT (n <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (n >= 0, "Bad get count");

    /* The result slice is incrementally built by taking the necessary slices
     * of the various stacks until the request is fulfilled.
     */

    /* XXX n == 0 -- empty slice, or fail ? */

    if (q->unget) {
	long int k    = cstack_size (q->unget);
	long int take = MIN (n, k);

	n -= take;
	result = cstack_get (q->unget, 0, take, !dir);
    }

    if (n && q->result && (q->at < cstack_size (q->result))) {
	CSLICE tmp;
	long int k    = cstack_size (q->result) - q->at;
	long int take = MIN (n, k);

	n -= take;
	tmp = cstack_getr (q->unget, q->at, take, dir);
	result = (!result)
	    ? tmp
	    : cslice_concat (result, tmp);
    }

    if (n && q->append) {
	CSLICE tmp;
	long int k    = cstack_size (q->append);
	long int take = MIN (n, k);

	n -= take;
	tmp = cstack_getr (q->append, 0, take, dir);
        result = (!result)
	    ? tmp
	    : cslice_concat (result, tmp);
    }

    ASSERT (n == 0, "Bad retrieval");
    return result;
}

/*
 * = = == === ===== ======== ============= =====================
 */

void
cqueue_put (CQUEUE q, void* item)
{
    if (!q->append) {
	q->append = NewStack (q);
    }
    cstack_push (q->append, item);
}

void
cqueue_unget (CQUEUE q, void* item)
{
    if (!q->unget) {
	q->unget = NewStack (q);
    }
    cstack_push (q->unget, item);
}

void
cqueue_remove (CQUEUE q, long int n)
{
    ASSERT (n <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (n >= 0, "Bad removal count");
    if (n == 0) return;

    if (q->unget) {
	long int k    = cstack_size (q->unget);
	long int take = MIN (n, k);

	if (take == k) {
	    HoldStack (q, &q->unget);
	} else {
	    cstack_pop (q->unget, take);
	}

	n -= take;
	if (!n) return
    }

 again:
    if (n && q->result && (q->at < cstack_size (q->result))) {
	long int k    = cstack_size (q->result) - q->at;
	long int take = MIN (n, k);

	n -= take; /* Before looping via again */
	if (take == k) {
	    HoldStack (q, &q->result);
	    q->at = 0;
	    if (q->append) {
		/* Shift append into result. The result stack was already
		 * cleared, and saved for future use by append, unget.
		 */
		q->result = q->append;
		q->append = 0; /* This ensures that again is run only once! */
		goto again;
	    }
	} else {
	    q->at += take;
	}
    }

    /* We are not accessing append, because this has been done already,
     * implicitly, due to the shift append -> result, see use of 'again'
     * above.
     */

    ASSERT (n == 0, "Bad removal");
}

void
cqueue_trim (CQUEUE q, long int n)
{
    ASSERT (n >= 0, "Bad removal count");
    long int remove = cqueue_size (q) - n;

    if (remove > 0) {
	cqueue_remove (q, remove);
    }
}

void
cqueue_drop (CQUEUE q, long int n)
{
    CSTACK tmp;

    ASSERT (n >= 0, "Bad removal count");
    if (n == 0) return;

    ASSERT (n <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (n >= 0, "Bad drop count");
    if (n == 0) return;

    if (q->unget) {
	long int k = cstack_size (q->unget);
	long int take = MIN (n, k);

	cstack_drop (q->unget, take);
	if (take == k) {
	    /* Because everything was dropped already the destroy will not
	     * free anything
	     */
	    cstack_destroy (q->unget);
	    q->unget = 0;
	}

	n -= take;
	if (!n) return;
    }

    // at == size ==> all processed in result, simply destroy.
    // These must be properly removed, and nothing to drop.
    // Shift append over.

    while (q->result && (q->at == cstack_size (q->result))) {
	HoldStack (q, &q->result);
	q->result = q->append;
	q->append = 0;
	q->at     = 0;
    }

    /*
     * The top at+n...top-1 elements must be kept for future use.
     * We put them into a new stack to replace 'result'.
     * In the originating stack we can then drop them.
     */

    {
	void** cell;
	long int i, sn, k = cstack_size (result) - q->at - n XXX;
	CSLICE s          = cstack_getr (q->result, q->at+n, k, cslice_normal);

	tmp = NewStack (q);
	cslice_get (s, &cell, &sn);
	for (i=0; i < sn; i++) {
	    cstack_push (q, cell [i]);
	}
	cslice_destroy (s);
	cstack_drop (q->result, k);
    }

    /*
     * The middle at...at+n-1 elements must be dropped per the request, not
     * removed.
     */

    cstack_drop (q->result, n);

    /* n+k dropped
     * == n+(size-at-n)
     * == size-at ... at..top-1
     */

    /* The bottom 0...at-1 elements were returned in the past.
     * These must be properly removed.
     */

    cstack_clear (q->result);
    HoldStack (q, &q->result);

    /* Now we can put the saved elements back into place
     */

    q->result = tmp;

}

void
cqueue_move (CQUEUE dst, CQUEUE src)
{
    ASSERT (dst->freeCell == src->freeCell, "Ownership mismatch");

    /*
     * Note: The destination takes ownership of the moved cell, thus there is
     * no need to run free on them.
     *
     * The loop below is simple, but slow, especially the repeated drops.
     * XXX This is better inlined, transfering larger slices.
     */

    while (cqueue_size(src)) {
	cqueue_put (dst, cqueue_head (src));
	cqueue_drop (src, 1);
    }
}

/*
 * = = == === ===== ======== ============= =====================
 */

void
cqueue_clientdata_set (CQUEUE q, void* clientdata)
{
    q->clientData = clientdata;
}

void*
cqueue_clientdata_get (CQUEUE q)
{
    return q->clientData;
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
