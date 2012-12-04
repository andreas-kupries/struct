/* struct::set/C Tcl_ObjType for CSET values
 */
#ifndef _STRUCT_SET_OBJTYPE_H
#define _STRUCT_SET_OBJTYPE_H 1

#include "tcl.h"

int      setc_get (Tcl_Interp* interp, Tcl_Obj* o, CSET* s);
Tcl_Obj* setc_new (CSET s);

#endif /* _STRUCT_SET_OBJTYPE_H */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
