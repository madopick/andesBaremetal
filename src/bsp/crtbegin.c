#include "config.h"

#ifdef CFG_GCOV

#pragma GCC optimize "no-test-coverage"
#pragma GCC optimize "no-profile-arcs"

/*  Declare a pointer to void function type.  */
typedef void (*func_ptr) (void);

func_ptr __CTOR_LIST__[1] __attribute__((__unused__)) __attribute__ ((section (".ctors")))
     = { (func_ptr) (-1) };

/* .init section start.
   This must appear at the start of the .init section.  */

asm ("\n\
	.section .init\n\
	.global _crt_init\n\
	.type	_crt_init, @function\n\
	.align 2\n\
_crt_init:\n\
");

#endif	// CFG_GCOV
