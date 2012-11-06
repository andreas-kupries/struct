#include "cqueueInt.h"

/*
 * = = == === ===== ======== ============= =====================
 * Declarations of helper functions
 */

static void
Rebalance (CSTACK* empty, CSTACK* notempty);

/*
 * = = == === ===== ======== ============= =====================
 * Internal debug helper. Activate manually. Assumes! that the queue contains
 * Tcl_Obj* (i.e. like struct::queue/C).
 */

#if 0
#define DUMP(s,q) ___dump (s,q)
#define DUMPS(t,s) ___dumps (t,s)

static void
___dumps (const char* t, CSTACK s)
{
    long int i, n;

    n = cstack_size (s);
    fprintf (stdout, "S %s %p = %d: {", t, s, n);

    if (n) {
	CSLICE tmp = cstack_get (s, n-1, n);
	for (i=0;i<n;i++) {
	    fprintf (stdout, " (%s)", Tcl_GetString (cslice_at (tmp, i)));
	    fflush (stdout);
	}
	fflush (stdout);
	cslice_destroy (tmp);
    }
    fprintf (stdout, " }\n", n);
}

static void
___dump (const char* s, CQUEUE q)
{
    /* Assume elements are Tcl_Obj* */
    fprintf (stdout, "%s %p = {\n", s, q);
    {
	long int i, n = cstack_size (q->head);
	if (n) {
	    CSLICE tmp = cstack_get (q->head, n-1, n);
	    fprintf (stdout, "    H %d <(", n);
	    fflush (stdout);
	    for (i=0;i<n;i++) {
		fprintf (stdout, " (%s)", Tcl_GetString (cslice_at (tmp, i)));
		fflush (stdout);
	    }
	    fprintf (stdout, " )\n", n);
	    fflush (stdout);
	    cslice_destroy (tmp);
	} else {
	    fprintf (stdout, "    H %d <()\n", n);
	}
    }
    {
	long int i, n = cstack_size (q->tail);
	if (n) {
	    CSLICE tmp = cstack_getr (q->tail, 0, n);
	    fprintf (stdout, "    T %d (", n);
	    fflush (stdout);
	    for (i=0;i<n;i++) {
		fprintf (stdout, " (%s)", Tcl_GetString (cslice_at (tmp, i)));
		fflush (stdout);
	    }
	    fprintf (stdout, " )>\n", n);
	    fflush (stdout);
	    cslice_destroy (tmp);
	} else {
	    fprintf (stdout, "    T %d ()>\n", n);
	}
    }
    fprintf (stdout, "}\n");
    fflush (stdout);
}
#else
#define DUMP(s,q)
#define DUMPS(s,q)
#endif

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
 * == Lifecycle management.
 */

CQUEUE
cqueue_create (CQUEUE_CELL_FREE freeCell, void* clientdata)
{
    CQUEUE q = ALLOC (CQUEUE_);

    q->freeCell   = freeCell;
    q->clientData = clientdata;
    q->head       = cstack_create (q->freeCell, 0);
    q->tail       = cstack_create (q->freeCell, 0);

    return q;
}

void
cqueue_destroy (CQUEUE q)
{
    cstack_destroy (q->head);
    cstack_destroy (q->tail);

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
	cstack_size (q->head) +
        cstack_size (q->tail) ;
}

void*
cqueue_first (CQUEUE q)
{
    if (cstack_size (q->head)) {
	return cstack_top (q->head);
    }
    if (cstack_size (q->tail)) {
	return cstack_bottom (q->tail);
    }

    ASSERT (0, "Not enough elements in the cqueue");
    return 0;
}

void*
cqueue_last (CQUEUE q)
{
    if (cstack_size (q->tail)) {
	return cstack_top (q->tail);
    }
    if (cstack_size (q->head)) {
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

    {
	long int pull, size = cstack_size (q->head);

	pull = MIN (take, size);
	if (pull) {
	    take -= pull;
	    result = cslice_reverse (cstack_get (q->head, pull-1, pull));

	    if (!take) return result;
	}
    }

    {
	long int pull, size = cstack_size (q->tail);
	CSLICE tmp;

	pull = MIN (take, size);
	if (pull) {
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

CSLICE
cqueue_tail (CQUEUE q, long int take)
{
    CSLICE result = 0;
    ASSERT (take <= cqueue_size (q), "Not enough elements in the cqueue");
    ASSERT (take > 0, "Bad get count");

    /* The result is incrementally built by taking the necessary slices of the
     * various stacks until the request is fulfilled.
     */

    {
	long int pull, size = cstack_size (q->tail);
	CSLICE tmp;

	pull = MIN (take, size);
	if (pull) {
	    take -= pull;
	    result = cstack_get (q->tail, pull-1, pull);

	    if (!take) return result;
	}
    }

    {
	long int pull, size = cstack_size (q->head);
	CSLICE tmp;

	pull = MIN (take, size);

	if (pull) {
	    take -= pull;
	    tmp = cslice_reverse (cstack_getr (q->head, 0, pull));
	    result = (!result)
		? tmp
		: cslice_concat (tmp, result);
	}
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

    {
	long int have = cstack_size (q->head);

	if (at >= have) {
	    /* The slice starts behind this buffer.
	     * Skip.
	     */
	    at -= have;
	} else if ((at+take) <= have) {
	    /* The slice is fully contained in this buffer.
	     * Take it and stop.
	     */
	    return cslice_reverse (cstack_get (q->head, at+take-1, take));

	} else /* (at+take > have) */ {
	    /* The slice starts in this buffer and extends into the next.
	     * Take prefix and continue.
	     *
	     * Math:
	     *   at+take-have => excess (size of the part in the next buffer(s).
	     *   take-excess  => here   (size of the part in this buffer)
	     *   = take-(at+take-have)
	     *   = take-at-take+have
	     *   = have-at
	     */
	    result = cslice_reverse (cstack_getr (q->head, 0, have-at));
	    take -= have-at;
	    at = 0;
	}
    }

    {
	long int have = cstack_size (q->tail);

	if (at >= have) {
	    /* The slice starts behind this buffer.
	     * That should not be possible.
	     */
	    ASSERT (0, "cqueue_get retrieval inconsistency");
	} else if ((at+take) <= have) {
	    /* The slice is fully contained in this buffer.
	     * Take it and stop.
	     * Integrate a prefix saved before, if there is any.
	     */
	    CSLICE tmp = cstack_getr (q->tail, at, take);
	    return (!result)
		? tmp
		: cslice_concat (result, tmp);

	} else /* (at+take > have) */ {
	    /* The slice starts in this buffer and extends into the next.
	     * That should not be possible.
	     */
	    ASSERT (0, "cqueue_get retrieval inconsistency");
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
    cstack_push (q->tail, item);
}

void
cqueue_prepend (CQUEUE q, void* item)
{
    cstack_push (q->head, item);
}

void
cqueue_append_slice (CQUEUE q, CSLICE s)
{
    cstack_push_slice (q->tail, s);
}

void
cqueue_prepend_slice (CQUEUE q, CSLICE s)
{
    cstack_push_slice (q->head, s);
}

void
cqueue_remove_head (CQUEUE q, long int take)
{
    long int total = cqueue_size (q);

    ASSERT (take <= total, "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad removal count");

    if (take == 0) return;
    if (take == total) {
	/* (**) Drop everything. */
	cqueue_clear (q);
	return;
    }

 again: {
	long int size = cstack_size (q->head);
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    cstack_clear (q->head);

	    /* Head is empty, tail is not, shift data over, early */
	    if (cstack_size (q->tail)) Rebalance (&q->head, &q->tail);

	    take -= drop;
	    if (!take) return;

	    goto again;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    cstack_pop (q->head, drop);
	    return;
	}
    }

    ASSERT (0,"");
    {
	/* Head is empty here. Move the data to keep into it. Then clear the
	 * remainder in the tail. At last move half of the data back to
	 * balance the sides. Special case: Drop everything. Cannot happen.
	 * Already caught at (**) above.
	 */

	long int size = cstack_size (q->tail);
	long int drop = MIN (take, size);

	ASSERT (drop < size, "Remove everything should have been caught at (**).");

	/* drop == take < size ---> take-drop == 0, stop. */

	/* Save the data to keep in the head, which we know to be empty */
	cstack_move (q->head, q->tail, size-drop);
	take -= drop;

	/* Kill the requested data */
	cstack_clear (q->tail);

	/* Make the saved data the tail again, and head empty. Oh, and revert
	 * the order in the tail, because the move did a reversal too.
	 */
	SWAP (q->head,q->tail);
	cstack_reverse_all (q->tail);
	DUMP ("rh/3",q);

	Rebalance (&q->head, &q->tail);
	DUMP ("rh/4",q);
    }

    ASSERT (take == 0, "Bad removal");
}

void
cqueue_remove_tail (CQUEUE q, long int take)
{
    long int total = cqueue_size (q);

    ASSERT (take <= total, "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad removal count");

    if (take == 0) return;
    if (take == total) {
	/* (**) Drop everything. */
	cqueue_clear (q);
	return;
    }

 again: {
	long int size = cstack_size (q->tail);
	long int drop = MIN (take, size);

	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    cstack_clear (q->tail);

	    /* Tail is empty, head is not, shift data over, early */
	    if (cstack_size (q->head)) Rebalance (&q->tail, &q->head);

	    take -= drop;
	    if (!take) return;

	    goto again;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    cstack_pop (q->tail, drop);
	    return;
	}
    }

    ASSERT (0,"");
    {
	/* Tail is empty here. Move the data to keep into it. Then clear the
	 * remainder in the head. At last move half of the data back to
	 * balance the sides. Special case: Drop everything. Cannot happen.
	 * Already caught at (**) above.
	 */

	long int size = cstack_size (q->head);
	long int drop = MIN (take, size);

	ASSERT (drop < size, "Remove everything should have been caught at (**).");

	/* drop == take < size ---> take-drop == 0, stop. */

	/* Save the data to keep in the head, which we know to be empty */
	cstack_move (q->tail, q->head, size-drop);
	take -= drop;

	/* Kill the requested data */
	cstack_clear (q->head);

	/* Make the saved data the head again, and tail empty. Oh, and revert
	 * the order in the head, because the move did a reversal too.
	 */
	SWAP (q->head,q->tail);
	cstack_reverse_all (q->head);

	Rebalance (&q->tail, &q->head);
    }

    ASSERT (take == 0, "Bad removal");
}

void
cqueue_clear (CQUEUE q)
{
    cstack_clear (q->head);
    cstack_clear (q->tail);
}

void
cqueue_drop_head (CQUEUE q, long int take)
{
    long int total = cqueue_size (q);
    CSTACK tmp;

    ASSERT (take <= total, "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad drop count");

    if (take == 0) return;
    if (take == total) {
	cqueue_drop_all (q);
	return;
    }

 again: {
	long int size = cstack_size (q->head);
	long int drop = MIN (take, size);

	cstack_drop (q->head, drop);
	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    /* Because everything was dropped already from head we do not
	     * release anything (== no remove).
	     */

	    /* Head is empty, tail is not, shift data over, early */
	    if (cstack_size (q->tail)) Rebalance (&q->head, &q->tail);

	    take -= drop;
	    if (!take) return;

	    goto again;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    return;
	}
    }

    ASSERT (0,"");
    {
	long int size = cstack_size (q->tail);
	long int drop = MIN (take, size);

	ASSERT (drop < size, "Drop everything should have been caught at (**).");

	/* drop == take < size ---> take-drop == 0, stop. */

	/* Save the data to keep in the head, which we know to be empty */
	cstack_move (q->head, q->tail, size-drop);
	take -= drop;

	/* Kill the requested data */
	cstack_drop_all (q->tail);

	/* Make the saved data the tail again, and head empty. */
	SWAP (q->head,q->tail);

	Rebalance (&q->head, &q->tail);
    }

    ASSERT (take == 0, "Bad drop");
}

void
cqueue_drop_tail (CQUEUE q, long int take)
{
    long int total = cqueue_size (q);
    CSTACK tmp;

    ASSERT (take <= total, "Not enough elements in the cqueue");
    ASSERT (take >= 0, "Bad drop count");

    if (take == 0) return;
    if (take == total) {
	cqueue_drop_all (q);
	return;
    }

 again: {
	long int size = cstack_size (q->tail);
	long int drop = MIN (take, size);

	cstack_drop (q->tail, drop);
	if (drop == size) {
	    /* drop == size <= take --> take-drop >= 0, continue */
	    /* Because everything was dropped already from tail we do not
	     * release anything (== no remove).
	     */
	    take -= drop;
	    if (!take) return;

	    goto again;
	} else {
	    /* drop == take < size ---> take-drop == 0, stop. */
	    return;
	}
    }

    ASSERT (0,"");
    {
	/* Tail is empty here. Move the data to keep into it. Then clear the
	 * remainder in the head. At last move half of the data back to
	 * balance the sides. Special case: Drop everything. Cannot happen.
	 * Already caught at (**) above.
	 */

	long int size = cstack_size (q->head);
	long int drop = MIN (take, size);

	ASSERT (drop < size, "Remove everything should have been caught at (**).");

	/* drop == take < size ---> take-drop == 0, stop. */

	/* Save the data to keep in the head, which we know to be empty */
	cstack_move (q->tail, q->head, size-drop);
	take -= drop;

	/* Kill the requested data */
	cstack_drop_all (q->head);

	/* Make the saved data the head again, and tail empty. */
	SWAP (q->head,q->tail);

	Rebalance (&q->tail, &q->head);
    }

    ASSERT (take == 0, "Bad drop");
}

void
cqueue_drop_all (CQUEUE q)
{
    cstack_drop_all (q->head);
    cstack_drop_all (q->tail);
}

void
cqueue_move (CQUEUE dst, CQUEUE src, long int n)
{
    ASSERT (dst->freeCell == src->freeCell, "Ownership mismatch");

    /*
     * Note: The destination takes ownership of the moved cell, thus there is
     * no need to release them => drop instead of remove.
     */

    CSLICE s = cqueue_head (src, n);
    cqueue_append_slice (dst, s);
    cslice_destroy (s);
    cqueue_drop_head (src, n);

    /* XX AK Possibly optimizable without using
     * a slice. We can use the buffers directly.
     * I.e. interleave the head/append/drop
     * operations.
     */
}

void
cqueue_move_all (CQUEUE dst, CQUEUE src)
{
    cqueue_move (dst, src, cqueue_size (src));
    /* XX AK Possibly optimizable without using
     * a slice. See 'move' for the idea.
     */
}


/*
 * = = == === ===== ======== ============= =====================
 * Implement the helper.
 */

static void
Rebalance (CSTACK* empty, CSTACK* notempty)
{
    CSTACK e = *empty;
    CSTACK n = *notempty;
    long int se = cstack_size (e);
    long int sn = cstack_size (n);
    long int half;

    ASSERT (!se && sn, "Bad call to rebalance");

    DUMPS("re1/e",e);
    DUMPS("re1/n",n);

    /* We rebalance by splitting the data in notempty in halves and putting
     * one into the empty stack. For an odd number of elements the smaller
     * half ends up in empty. Except for a single element, which we can
     * optimize by simply swapping the stacks.
     */

    if (sn == 1) {
	*empty    = n;
	*notempty = e;
	return;
    }

    half = sn / 2;
    cstack_move (e, n, half);


    DUMPS("re2/e",e);
    DUMPS("re2/n",n);

    /* To keep the semantics of the queue we have to reverse the stack
     * contents and swap them
     */

    cstack_reverse_all (e);
    cstack_reverse_all (n);

    DUMPS("re3/e",e);
    DUMPS("re3/n",n);

    *empty    = n;
    *notempty = e;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
