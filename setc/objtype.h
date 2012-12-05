/* struct::set/C Tcl_ObjType for CSET values
 */
#ifndef _STRUCT_SET_OBJTYPE_H
#define _STRUCT_SET_OBJTYPE_H 1

#include "tcl.h"
#include <c_set/c_setDecls.h>

/* Unboxing and boxing of CSET values in Tcl_Obj*
 */

int      setc_get (Tcl_Interp* interp, Tcl_Obj* o, CSET* s);
Tcl_Obj* setc_new (CSET s);

/* Helper functions for CSET's containing Tcl_Obj* keys.
 */

void* setc_dup     (void const* a);
void  setc_free    (void* a);
int   setc_compare (void const* a, void const* b);

#endif /* _STRUCT_SET_OBJTYPE_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
