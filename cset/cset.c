#include "csetInt.h"
#include <c_slice/c_sliceDecls.h>

/*
 * TODO: Add assertions in each operator that the sets match structurally
 * (identical dup, rel, cmp functions).
 *
 * = = == === ===== ======== ============= =====================
 * Client data management.
 */

void
cset_clientdata_set (CSET s, void const* clientdata)
{
    s->clientData = clientdata;
}

void*
cset_clientdata_get (const CSET s)
{
    return (void*) s->clientData;
}

/*
 * = = == === ===== ======== ============= =====================
 * == Lifecycle management.
 */

CSET
cset_create (CSET_CELL_DUP dup,
	     CSET_CELL_REL release,
	     CSET_CELL_CMP compare,
	     void const*   clientdata)
{
    CSET s = ALLOC (CSET_);

    s->dup	  = dup;  
    s->release    = release;
    s->compare    = compare;
    s->clientData = clientdata;
    s->set        = jsw_new (compare, dup, release);
    return s;
}

void
cset_destroy (CSET s)
{
    jsw_delete (s->set);
    ckfree ((char*) s);
}

/*
 * = = == === ===== ======== ============= =====================
 * Accessors.
 */

CSLICE
cset_contents (const CSET s)
{
    CSLICE slice = cslice_make (jsw_size (s->set));
    long int c, i;
    void**   v;
    void*    item;
    jsw_trav_t* t = jsw_tnew ();

    cslice_get (slice, &c, &v);
    for (i=0, item = jsw_tfirst (t, s->set);
	 item;
	 i++, item = jsw_tnext (t)) {
	v[i] = item;
    }
    jsw_tdelete (t);

    return slice;
}

int
cset_contains (const CSET s, void const* item)
{
    return !jsw_find (s->set, (void*) item);
}

int
cset_empty (const CSET s)
{
    return jsw_size (s->set) == 0;
}

long int
cset_size (const CSET s)
{
    return jsw_size (s->set);
}

int
cset_equal (const CSET a, const CSET b)
{
    /* A == B <=> A subset B && |A| == |B| */

    return
	(jsw_size (a->set) == jsw_size (b->set)) &&
	cset_subset (a, b);
}

int
cset_subset (const CSET a, const CSET b)
{
    /* A subset of B <=> empty (A - B)
     */

    CSET ab = cset_difference (a, b);
    int empty = cset_empty (ab);
    cset_destroy (ab);
    return empty;
}

int
cset_superset (const CSET a, const CSET b)
{
    /* A superset of B <=> empty (B - A)
     */

    CSET ba = cset_difference (b, a);
    int empty = cset_empty (ba);
    cset_destroy (ba);
    return empty;
}

/*
 * = = == === ===== ======== ============= =====================
 * Set operations, returning new values, input untouched.
 */

CSET
cset_dup (const CSET s)
{
    /* Z := A (copy constructor) */

    CSET r = cset_create (s->dup, s->release, s->compare, 0);

    if (jsw_size (s->set)) {
	jsw_trav_t* t = jsw_tnew ();
	void* item;
	for (item = jsw_tfirst (t, s->set);
	     item;
	     item = jsw_tnext (t)) {
	    jsw_insert (r->set, item);
	}
	jsw_tdelete (t);
    }

    return r;
}

CSET
cset_difference (const CSET a, const CSET b)
{
    /* Z = A - B
     *
     * Shortcuts: Nothing - B = Nothing.
     *            A - Nothing = A.
     */

    if (jsw_size (a->set) == 0) return cset_dup (a);
    if (jsw_size (b->set) == 0) return cset_dup (a);

    /* Full work required. Traverse A and add its elements to the result if
     * they are not found in B. Essentially a filtered duplicate.
     */

    {
	CSET r = cset_create (a->dup, a->release, a->compare, 0);
	jsw_trav_t* t = jsw_tnew ();
	void* item;

	for (item = jsw_tfirst (t, a->set);
	     item;
	     item = jsw_tnext (t)) {
	    if (jsw_find (b->set, item)) continue;
	    jsw_insert (r->set, item);
	}
	jsw_tdelete (t);

	return r;
    }
}

CSET
cset_intersect (const CSET a, const CSET b)
{
    /* Z = A * B
     *
     * Shortcuts: Nothing * B = Nothing.
     *            A * Nothing = Nothing.
     */

    if (jsw_size (a->set) == 0) return cset_dup (a);
    if (jsw_size (b->set) == 0) return cset_dup (b);

    /* Full work required. Traverse A and add its elements to the result if
     * they are found in B. Essentially a filtered duplicate. As a small
     * optimization iterate over the smaller set.
     */

    {
	CSET r = cset_create (a->dup, a->release, a->compare, 0);
	jsw_trav_t* t = jsw_tnew ();
	void* item;
	jsw_tree_t* tr;
	jsw_tree_t* fi;

	if (jsw_size (a->set) < jsw_size (b->set)) {
	    tr = a->set;
	    fi = b->set;
	} else {
	    tr = b->set;
	    fi = a->set;
	}

	for (item = jsw_tfirst (t, tr);
	     item;
	     item = jsw_tnext (t)) {
	    if (!jsw_find (fi, item)) continue;
	    jsw_insert (r->set, item);
	}
	jsw_tdelete (t);

	return r;
    }
}

CSET
cset_symdiff (const CSET a, const CSET b)
{
    /* Z = (A - B) + (B - A)
     * 
     * For simplicity we call on the existing operations instead of trying to
     * program it directly.
     */

    CSET ab = cset_difference (a, b);
    CSET ba = cset_difference (b, a);
    CSET r  = cset_union (ab, ba);

    cset_destroy (ab);
    cset_destroy (ba);

    return r;
}

CSET
cset_union (const CSET a, const CSET b)
{
    /* Z = A + B
     *
     * Shortcuts: Nothing + B = B.
     *            A + Nothing = A.
     */

    if (jsw_size (a->set) == 0) return cset_dup (b);
    if (jsw_size (b->set) == 0) return cset_dup (a);

    /* Full work required. Traverse A and B add their elements to the result.
     */

    {
	CSET r = cset_create (a->dup, a->release, a->compare, 0);
	jsw_trav_t* t = jsw_tnew ();
	void* item;

	for (item = jsw_tfirst (t, a->set);
	     item;
	     item = jsw_tnext (t)) {
	    jsw_insert (r->set, item);
	}
	for (item = jsw_tfirst (t, b->set);
	     item;
	     item = jsw_tnext (t)) {
	    jsw_insert (r->set, item);
	}
	jsw_tdelete (t);

	return r;
    }
}

/*
 * = = == === ===== ======== ============= =====================
 * Modifiers for the first argument, i.e. editing in place.
 * 2nd argument is set.
 */

void
cset_vdifference (CSET a, const CSET b)
{
    /* A = A - B
     *
     * Shortcuts: Nothing - B = Nothing.
     *            A - Nothing = A.
     */

    if (jsw_size (a->set) == 0) return;
    if (jsw_size (b->set) == 0) return;

    /* Full work required. Traverse B and remove its elements from A.
     */

    {
	jsw_trav_t* t = jsw_tnew ();
	void* item;

	for (item = jsw_tfirst (t, b->set);
	     item;
	     item = jsw_tnext (t)) {

	    jsw_erase (a->set, item);
	}
	jsw_tdelete (t);
    }
}

void
cset_vintersect (CSET a, const CSET b)
{
    /* A = A * B
     *
     * Shortcuts: Nothing * B = Nothing.
     *            A * Nothing = Nothing
     */

    if (jsw_size (a->set) == 0) return;

    if (jsw_size (b->set) == 0) {
	jsw_tree_t* r = jsw_new (a->compare, a->dup, a->release);
	jsw_delete (a->set);
	a->set = r;
	return;
    }

    /* Full work required. Traverse A and add its elements to the result if
     * they are found in B. Essentially a filtered duplicate. We avoid
     * creating a full CSET and simply switch A to use the new tree.
     */

    {
	jsw_tree_t* r = jsw_new (a->compare, a->dup, a->release);
	jsw_trav_t* t = jsw_tnew ();
	void* item;

	for (item = jsw_tfirst (t, a->set);
	     item;
	     item = jsw_tnext (t)) {
	    if (!jsw_find (b->set, item)) continue;
	    jsw_insert (r, item);
	}
	jsw_tdelete (t);
	jsw_delete (a->set);
	a->set = r;
    }

}

void
cset_vsymdiff (CSET a, const CSET b)
{
    /* A = (A - B) + (B - A)
     *
     * For simplicity we call on the existing operations instead of trying to
     * program it directly. At the end we switch trees to modify A
     */

    CSET ab = cset_difference (a, b);
    CSET ba = cset_difference (b, a);
    CSET r  = cset_union (ab, ba);
    jsw_tree_t* tmp;

    cset_destroy (ab);
    cset_destroy (ba);

    tmp    = a->set;
    a->set = r->set;
    r->set = tmp;

    cset_destroy (r); /* This now destroys the old A */
}

void
cset_vunion (CSET a, const CSET b)
{
    /* A = A + B
     *
     * Shortcuts: A + Nothing = A.
     */

    if (jsw_size (b->set) == 0);

    /* Full work required. Traverse B and add its elements to A.
     */

    {
	jsw_trav_t* t = jsw_tnew ();
	void* item;

	for (item = jsw_tfirst (t, b->set);
	     item;
	     item = jsw_tnext (t)) {
	    jsw_insert (a->set, item);
	}

	jsw_tdelete (t);
    }
}

/*
 * = = == === ===== ======== ============= =====================
 * Modifiers for the first argument, i.e. editing in place.
 * 2nd argument is item.
 */

void
cset_vadd (CSET s, void const* item)
{
    /* A = A + item
     */

    jsw_insert (s->set, (void*) item);
}

void
cset_vsubtract (CSET s, void const* item)
{
    /* A = A - item
     */

    jsw_erase (s->set, (void*) item);
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
