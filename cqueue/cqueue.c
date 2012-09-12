#include "cqueueInt.h"

/*
 * = = == === ===== ======== ============= =====================
 * == Declaraction of helper internal function.
 * == Management of the hold field.
 */

static CSTACK NewStack  (CQUEUE q);
static void   HoldStack (CQUEUE q, CSTACK* s);

/*
 * = = == === ===== ======== ============= =====================
 * == Lifecycle management.
 */

CQUEUE
cqueue_create (CQUEUE_CELL_FREE freeCell, void* clientdata)
{
    CQUEUE q = ALLOC (CQUEUE_);

    q->freeCell   = freeCell;
    q->clientData = clientdata;
    q->head       = 0;
    q->middle     = 0;
    q->tail       = 0;
    q->at         = 0;
    q-> hold      = 0;

    return q;
}

void
cqueue_destroy (CQUEUE q)
{
    if (q->head)   cstack_destroy (q->head);
    if (q->middle) cstack_destroy (q->middle);
    if (q->tail)   cstack_destroy (q->tail);
    if (q->hold)   cstack_destroy (q->hold);

    ckfree ((char*) q);
}

/*
 * = = == === ===== ======== ============= =====================
 * Accessors.
 */

long int
cqueue_size (CQUEUE q)
{
    return
	(q->head   ? cstack_size (q->head)   : 0) +
	(q->middle ? cstack_size (q->middle) : 0) +
	(q->tail   ? cstack_size (q->tail)   : 0) -
	q->at;
}

void*
cqueue_first (CQUEUE q)
{
    if (q->head) {
	ASSERT (cstack_size (q->head) > 0, "non-null head is empty");
	return cstack_top (q->head);
    }
    if (q->middle && (q->at < cstack_size (q->middle))) {
	return cstack_atr (q->middle, q->at);
    }
    if (q->tail) {
	ASSERT (cstack_size (q->tail) > 0, "non-null tail is empty");
	return cstack_bottom (q->tail);
    }

    ASSERT (0, "Not enough elements in the cqueue");
    return 0;
}

void*
cqueue_last (CQUEUE q)
{
    if (q->tail) {
	ASSERT (cstack_size (q->tail) > 0, "non-null tail is empty");
	return cstack_top (q->tail);
    }
    if (q->middle && (q->at < cstack_size (q->middle))) {
	return cstack_top (q->middle);
    }
    if (q->head) {
	ASSERT (cstack_size (q->head) > 0, "non-null head is empty");
	return cstack_bottom (q->head);
    }

    ASSERT (0, "Not enough elements in the cqueue");
    return 0;
}

CSLICE
cqueue_head (CQUEUE q, long int take)
{
    CSLICE result = 0;
    ASSERT (take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take > 0, "Bad get count");

    /* The result is incrementally built by taking the necessary slices of the
     * various stacks until the request is fulfilled.
     */

    if (q->head) {
	long int pull, size = cstack_size (q->head);

	ASSERT (size > 0, "non-null head is empty");
	pull = MIN (take, size);

	take -= pull;
	result = cslice_reverse (cstack_get (q->head, 0, pull));

	if (!take) return result;
    }

    if (q->middle && (q->at < cstack_size (q->middle))) {
	CSLICE tmp;
	long int size = cstack_size (q->middle) - q->at;
	long int pull = MIN (take, size);

	take -= pull;
	tmp = cstack_getr (q->head, q->at, pull);
	result = (!result)
	    ? tmp
	    : cslice_concat (result, tmp);

	if (!take) return result;
    }

    if (q->tail) {
	long int pull, size = cstack_size (q->tail);
	CSLICE tmp;

	ASSERT (size > 0, "non-null tail is empty");
	pull = MIN (take, size);

	take -= pull;
	tmp = cstack_getr (q->tail, 0, pull);
        result = (!result)
	    ? tmp
	    : cslice_concat (result, tmp);
    }

    ASSERT (take == 0, "Bad retrieval");
    return result;
}

CSLICE
cqueue_tail (CQUEUE q, long int take)
{
    CSLICE result = 0;
    ASSERT (take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take > 0, "Bad get count");

    /* The result is incrementally built by taking the necessary slices of the
     * various stacks until the request is fulfilled.
     */

    if (q->tail) {
	long int pull, size = cstack_size (q->tail);
	CSLICE tmp;

	ASSERT (size > 0, "non-null tail is empty");
	pull = MIN (take, size);

	take -= pull;
	result = cstack_get (q->tail, pull-1, pull);

	if (!take) return result;
    }

    if (q->middle && (q->at < cstack_size (q->middle))) {
	CSLICE tmp;
	long int size = cstack_size (q->middle) - q->at;
	long int pull = MIN (take, size);

	take -= pull;
	tmp = cstack_get (q->head, pull-1, pull);
	result = (!result)
	    ? tmp
	    : cslice_concat (tmp, result);

	if (!take) return result;
    }

    if (q->head) {
	long int pull, size = cstack_size (q->head);
	CSLICE tmp;

	ASSERT (size > 0, "non-null head is empty");
	pull = MIN (take, size);

	take -= pull;
	tmp = cslice_reverse (cstack_getr (q->head, 0, pull));
        result = (!result)
	    ? tmp
	    : cslice_concat (tmp, result);
    }

    ASSERT (take == 0, "Bad retrieval");
    return result;
}

CSLICE
cqueue_get (CQUEUE q, long int at, long int take)
{
    /* A variant of cqueue_head, coded to skip over 'at-1' elements from the
     * beginning before starting to accumulate the slice.
     */

    CSLICE result = 0;
    ASSERT (at+take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take > 0, "Bad get count");

    /* The result is incrementally built by taking the necessary slices of the
     * various stacks until the request is fulfilled.
     */

    if (q->head) {
	long int sz, pull, size = cstack_size (q->head);

	ASSERT (size > 0, "non-null head is empty");

	sz = size - at;
	if (sz > 0) {
	    pull = MIN (take, sz);

	    take -= pull;
	    result = cslice_reverse (cstack_get (q->head, at, pull));

	    if (!take) return result;
	}
	at -= size;
    }


    if (q->middle && (q->at < cstack_size (q->middle))) {
	CSLICE tmp;
	long int pull, size = cstack_size (q->middle) - q->at;
	long int sz = size - at;

	if (sz > 0) {
	    pull = MIN (take, sz);

	    take -= pull;
	    tmp = cstack_getr (q->head, q->at, pull);
	    result = (!result)
		? tmp
		: cslice_concat (result, tmp);

	    if (!take) return result;
	}
	at -= size;
    }

    if (q->tail) {
	long int sz, pull, size = cstack_size (q->tail);
	CSLICE tmp;

	ASSERT (size > 0, "non-null tail is empty");
	sz = size - at;
	if (sz > 0) {
	    pull = MIN (take, size);

	    take -= pull;
	    tmp = cstack_getr (q->tail, 0, pull);
	    result = (!result)
		? tmp
		: cslice_concat (result, tmp);
	}
    }

    ASSERT (take == 0, "Bad retrieval");
    return result;
}

/*
 * = = == === ===== ======== ============= =====================
 */

void
cqueue_append (CQUEUE q, void* item)
{
    if (!q->tail) {
	q->tail = NewStack (q);
    }
    cstack_push (q->tail, item);
}

void
cqueue_prepend (CQUEUE q, void* item)
{
    if (!q->head) {
	q->head = NewStack (q);
    }
    cstack_push (q->head, item);
}

void
cqueue_remove_head (CQUEUE q, long int take)
{
    ASSERT (take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad removal count");

    if (take == 0) return;

    if (q->head) {
	long int size = cstack_size (q->head);
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    HoldStack (q, &q->head);
	    take -= drop;
	    if (!take) return;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    cstack_pop (q->head, drop);
	    return;
	}
    }

 again:
    if (q->middle && (q->at < cstack_size (q->middle))) {
	long int size = cstack_size (q->middle) - q->at;
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    HoldStack (q, &q->middle);
	    q->at = 0;

	    /* Shift expected tail into middle. The middle is already cleared,
	     * and held for future use.
	     */
	    ASSERT (q->tail,"Tail expected, missing");
	    q->middle = q->tail;
	    q->tail   = 0; /* This ensures that again is run only once! */
	    take     -= drop; /* Before looping via again */
	    if (!take) return;
	    goto again;
	}

	/* drop == take < size ---> take-drop == 0, stop. */
	q->at += drop;
	return;
    }

    /* We are not accessing tail, because this has been done already,
     * implicitly, due to the shift tail -> middle above, see 'again'.
     */

    ASSERT (take == 0, "Bad removal");
}

void
cqueue_remove_tail (CQUEUE q, long int take)
{
    ASSERT (take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad removal count");

    if (take == 0) return;

    if (q->tail) {
	long int size = cstack_size (q->tail);
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    HoldStack (q, &q->tail);
	    take -= drop;
	    if (!take) return;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    cstack_pop (q->tail, drop);
	    return;
	}
    }

 again:
    if (q->middle && (q->at < cstack_size (q->middle))) {
	long int size = cstack_size (q->middle) - q->at;
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    HoldStack (q, &q->middle);
	    q->at = 0;
	    take -= drop;
	    if (!take) return;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    cstack_pop (q->middle, drop);
	    return;
	}
    }

    /* Handling head is a bit more complex now, because it is oriented in the
     * reverse direction, compared to the others. We now move it into the
     * middle. We can assume that the middle is gone. Simply creating it and
     * moving things over does the necessary reversal, and then we drop
     * elements as needed, reusing the previous section.
     */

    if (q->head) {
	q->middle = NewStack (q);
	cstack_move (q->middle, q->head);
	HoldStack (q, &q->head);
	goto again;
    }

    ASSERT (take == 0, "Bad removal");
}

void
cqueue_drop_head (CQUEUE q, long int take)
{
    CSTACK tmp;

    ASSERT (take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad drop count");
    if (take == 0) return;

    if (q->head) {
	long int size = cstack_size (q->head);
	long int drop = MIN (take, size);

	cstack_drop (q->head, drop);
	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    /* Because everything was dropped already from head the destroy
	     * will not release anything (== no remove).
	     */
	    HoldStack (q, &q->head);
	    take -= drop;
	    if (!take) return;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    return;
	}
    }

    if (q->middle) {
	long int size = cstack_size (q->head) - q->at;
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */

	    /* The first part of middle has to be removed, with release, as it
	     * has been returned in the past. And everything else must be
	     * dropped. Then we can clean out.
	     */
	    cstack_drop (q->middle, drop);
	    HoldStack (q, &q->middle);
	    q->at = 0;
	    take -= drop;
	    if (!take) return;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */

	    /* First part must be removed. Second part dropped, and the
	     * remainder kept?
	     * ping/pong ?
	     */

	    XXX

	    q->at = += drop; // roughly
	    return
	}
    }

    if (q->tail) {
	long int size = cstack_size (q->tail);
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size < take --> take-drop > 0, continue */
	    cstack_drop (q->middle, drop);
	    HoldStack (q, &q->tail);
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    ///cstack_drop (q->tail, drop);
	    /// drop removes from the bottom of the stack */
	    /// we can ping/pong this - ping/revers, drop, pong/revers
	    /* And shift remaining tail into the middle */
	    q->middle = q->tail;
	    q->tail = 0;
	    return;
	}

    }





    /* A small optimization before going to the next step:
     *

     * (at == size middle) ==> Everything in middle was returned already and
     * is waiting on destruction. Do that now, and shift the tail in.
     */

    while (q->middle && (q->at == cstack_size (q->middle))) {
	HoldStack (q, &q->middle);
	q->middle = q->tail;
	q->tail   = 0;
	q->at     = 0;
    }

    /* Now we can handle the middle. The topmost elements at+n...top-1
     * (indexed from bottom) must be kept for future use. We save them to a
     * temp stack. And in the originating stack we can drop them.
     * However if at+n > top-1




    /*
     * The top at+n...top-1 elements must be kept for future use.
     * We put them into a new stack to replace 'middle'.
     * In the originating stack we can then drop them.
     */

    {
	void** cell;
	long int i, sn, k = cstack_size (q->middle) - q->at - n - XXX;
	CSLICE s          = cstack_getr (q->middle, q->at+n, k, cslice_normal);

	tmp = NewStack (q);
	cslice_get (s, &cell, &sn);
	for (i=0; i < sn; i++) {
	    cstack_push (q, cell [i]);
	}
	cslice_destroy (s);
	cstack_drop (q->middle, k);
    }

    /*
     * The middle at...at+n-1 elements must be dropped per the request, not
     * removed.
     */

    cstack_drop (q->middle, n);

    /* n+k dropped
     * == n+(size-at-n)
     * == size-at ... at..top-1
     */

    /* The bottom 0...at-1 elements were returned in the past.
     * These must be properly removed.
     */

    cstack_clear (q->middle);
    HoldStack (q, &q->middle);

    /* Now we can put the saved elements back into place
     */

    q->middle = tmp;

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
	cqueue_append (dst, cqueue_first (src));
	cqueue_drop (src, 1);
    }
}

/*
 * = = == === ===== ======== ============= =====================
 * Client data management.
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
 * == Helpers. Implementation.
 */

static CSTACK
NewStack (CQUEUE q) {
    CSTACK result;
    if (q->hold) {
	/* Reuse the help stack */
	result = q->hold;
	q->hold = 0;
    } else {
	/* Nothing to reuse, actually create */
	result = cstack_create (q->freeCell, 0);
    }
    return result;
}

static void
HoldStack (CQUEUE q, CSTACK* s) {
    CSTACK result;
    if (q->hold) {
	/* The hold slot is full, actually destroy */
	cstack_destroy (*s);
    } else {
	/* Hold the stack, only clear it. */
	q->hold = *s;
	cstack_clear (*s);
    }
    *s = 0;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
#error X
