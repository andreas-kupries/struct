#ifndef JSW_ATREE_H
#define JSW_ATREE_H

/*
  Andersson tree library

    > Created (Julienne Walker): September 10, 2005

  This code is in the public domain. Anyone may
  use it or change it in any way that they see
  fit. The author assumes no responsibility for 
  damages incurred through use of the original
  code or any variations thereof.

  It is requested, but not required, that due
  credit is given to the original author and
  anyone who has modified the code through
  a header comment, such as this one.
*/
#ifdef __cplusplus
#include <cstddef>

using std::size_t;

extern "C" {
#else
#include <stddef.h>
#endif

/* Opaque types */
typedef struct jsw_tree jsw_tree_t;
typedef struct jsw_trav jsw_trav_t;

/* User-defined item handling */
typedef int   (*cmp_f) ( const void *p1, const void *p2 );
typedef void *(*dup_f) ( void *p );
typedef void  (*rel_f) ( void *p );

/* Andersson tree functions */
jsw_tree_t  *jsw_new    ( cmp_f cmp, dup_f dup, rel_f rel );
void         jsw_delete ( jsw_tree_t *tree );
void        *jsw_find   ( jsw_tree_t *tree, void *data );
int          jsw_insert ( jsw_tree_t *tree, void *data );
int          jsw_erase  ( jsw_tree_t *tree, void *data );
size_t       jsw_size   ( jsw_tree_t *tree );

/* Traversal functions */
jsw_trav_t  *jsw_tnew    ( void );
void         jsw_tdelete ( jsw_trav_t *trav );
void        *jsw_tfirst  ( jsw_trav_t *trav, jsw_tree_t *tree );
void        *jsw_tlast   ( jsw_trav_t *trav, jsw_tree_t *tree );
void        *jsw_tnext   ( jsw_trav_t *trav );
void        *jsw_tprev   ( jsw_trav_t *trav );

#ifdef __cplusplus
}
#endif

#endif
