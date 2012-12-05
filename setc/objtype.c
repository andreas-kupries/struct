/* struct::set/C Tcl_ObjType for CSET values
 */

#include <string.h>
#include "objtype.h"
#include <c_slice/c_sliceDecls.h>

/* .................................................. */

static void cset_objtype_free (Tcl_Obj* obj);
static void cset_objtype_dup  (Tcl_Obj* obj, Tcl_Obj* dup);
static void cset_objtype_2str (Tcl_Obj* obj);
static int  cset_objtype_from (Tcl_Interp* ip, Tcl_Obj* obj);

static
Tcl_ObjType cset_objtype = {
    "struct/c::set",
    cset_objtype_free,
    cset_objtype_dup,
    cset_objtype_2str,
    cset_objtype_from
};

/* .................................................. */

int
setc_get (Tcl_Interp* interp, Tcl_Obj* o, CSET* s)
{
    if (o->typePtr != &cset_objtype) {
	int res = cset_objtype_from (interp, o);
	if (res != TCL_OK) {
	    return res;
	}
    }

    *s = (CSET) o->internalRep.otherValuePtr;
    return TCL_OK;
}

Tcl_Obj*
setc_new (CSET s)
{
    Tcl_Obj* o = Tcl_NewObj();
    Tcl_InvalidateStringRep(o);

    o->internalRep.otherValuePtr = s;
    o->typePtr                   = &cset_objtype;
    return o;
}

/* .................................................. */

static void
cset_objtype_free (Tcl_Obj* o)
{
    cset_destroy ((CSET) o->internalRep.otherValuePtr);
    o->internalRep.otherValuePtr = NULL;
}

static void
cset_objtype_dup (Tcl_Obj* obj, Tcl_Obj* dup)
{
    CSET s = cset_dup ((CSET) obj->internalRep.otherValuePtr);

    dup->internalRep.otherValuePtr = s;
    dup->typePtr	           = &cset_objtype;
}

static void
cset_objtype_2str (Tcl_Obj* obj)
{
    /* iterate over keys and generate a list-like string rep */

#   define LOCAL_SIZE 20
    int localFlags[LOCAL_SIZE], *flag;
    int localLen  [LOCAL_SIZE], *len;
    char *elem, *dst;
    int i, length;

    /* Get the keys */

    CSLICE    s = cset_contents ((CSET) obj->internalRep.otherValuePtr);
    long int  oc;
    Tcl_Obj** ov;

    cslice_get (s, &oc, (void***) &ov);

    /*
     * Convert each key of the set to string form and then convert it to
     * proper list element form, adding it to the result buffer.
     */

    /*
     * Pass 1: estimate the space needed, and gather flags.
     */

    if (oc <= LOCAL_SIZE) {
	flag = localFlags;
	len  = localLen;
    } else {
	flag = (int *) ckalloc((unsigned) oc * sizeof(int));
	len  = (int *) ckalloc((unsigned) oc * sizeof(int));
    }
    obj->length = 1;

    for(i = 0; i < oc; i++) {
	elem = Tcl_GetString (ov [i]);
	len [i] = strlen (elem);

	obj->length += Tcl_ScanCountedElement(elem, len[i], &flag[i]) + 1;
    }

    /*
     * Pass 2: copy into string rep buffer.
     */

    obj->bytes = ckalloc((unsigned) obj->length);
    dst = obj->bytes;

    for(i = 0; i < oc; i++) {
	elem = Tcl_GetString (ov [i]);
	dst += Tcl_ConvertCountedElement(elem, len[i], dst, flag[i]);
	*dst = ' ';
	dst++;
    }

    /*
     * Endgame: Cleanup, and finalization of generated string, and its length.
     */

    cslice_destroy (s);

    if (flag != localFlags) {
	ckfree((char *) flag);
	ckfree((char *) len);
    }
    if (dst == obj->bytes) {
	*dst = 0;
    } else {
	dst--;
	*dst = 0;
    }

    obj->length = dst - obj->bytes;
}

static int
cset_objtype_from (Tcl_Interp* ip, Tcl_Obj* obj)
{
    /* We go through an intermediate list rep.
     */

    int          lc, i, new;
    Tcl_Obj**    lv;
    Tcl_ObjType* oldTypePtr;
    CSET         s;

    if (Tcl_ListObjGetElements (ip, obj, &lc, &lv) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * Remember the list objtype after the conversion to list, or we will try
     * to free a list intrep using the free-proc of whatever type the word had
     * before. For example 'parsedvarname'. That would be bad. Segfault like
     * bad.
     */

    oldTypePtr = obj->typePtr;

    /* Now, if the value was pure we forcibly generate the string-rep, to
     * capture the existing semantics of the value. Because we now enter the
     * realm of unordered, and the actual value may not be. If so, then not
     * having the string-rep will later cause the generation of an arbitrarily
     * ordered string-rep when the value is shimmered to some other type. This
     * is most visible for lists, which are ordered. A shimmer from list to
     * set and back to list may reorder the elements if we do not capture
     * their order in the string-rep.
     *
     * See test case -15.0 in sets.testsuite demonstrating this.
     * Disable the Tcl_GetString below and see the test fail.
     */

    Tcl_GetString (obj);

    /* Gen CSET from list */

    s = cset_create (setc_dup, setc_free, setc_compare, 0);

    for (i=0; i < lc; i++) {
	cset_vadd (s, lv[i]);
    }

    /*
     * Free the old internalRep before setting the new one. We do this as
     * late as possible to allow the conversion code, in particular
     * Tcl_ListObjGetElements, to use that old internalRep.
     */

    if ((oldTypePtr != NULL) && (oldTypePtr->freeIntRepProc != NULL)) {
	oldTypePtr->freeIntRepProc(obj);
    }

    obj->internalRep.otherValuePtr = s;
    obj->typePtr                   = &cset_objtype;
    return TCL_OK;
}

/* .................................................. */

void*
setc_dup (void const* e)
{
    Tcl_Obj* o = (Tcl_Obj*) e;
    Tcl_IncrRefCount (o);
    return (void*) e;
}

void
setc_free (void* e)
{
    Tcl_Obj* o = e;
    Tcl_DecrRefCount (o);
}

int
setc_compare (void const* a, void const* b)
{
    Tcl_Obj *objPtr1 = (Tcl_Obj*) a;
    Tcl_Obj *objPtr2 = (Tcl_Obj*) b;
    register const char *p1, *p2;
    register int l1, l2;

    /*
     * If the object pointers are the same then they match.
     */

    if (objPtr1 == objPtr2) {
	return 0;
    }

    /*
     * Don't use Tcl_GetStringFromObj as it would prevent l1 and l2 being
     * in a register.
     */

    p1 = Tcl_GetString (objPtr1);
    l1 = objPtr1->length;
    p2 = Tcl_GetString (objPtr2);
    l2 = objPtr2->length;

    /*
     * Only compare if the string representations are of the same length.
     */

    if (l1 < l2) {
	return -1;
    } else if (l1 > l2) {
	return 1;
    } else {
	for (;l1; p1++, p2++, l1--) {
	    if (*p1 < *p2) {
		return -1;
	    } else if (*p1 > *p2) {
		return 1;
	    }
	}

	return 0;
    }
}

/* .................................................. */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
