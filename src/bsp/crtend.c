#include "config.h"

#ifdef CFG_GCOV

#pragma GCC optimize "no-test-coverage"
#pragma GCC optimize "no-profile-arcs"

asm (".text");

/*  Declare a pointer to void function type.  */
typedef void (*func_ptr) (void);

/* Put a word containing zero at the end of each of our two lists of function
   addresses.  Note that the words defined here go into the .ctors and .dtors
   sections of the crtend.o file, and since that file is always linked in
   last, these words naturally end up at the very ends of the two lists
   contained in these two sections.  */

func_ptr __CTOR_END__[1] __attribute__((__unused__)) __attribute__ ((section (".ctors")))
     = { (func_ptr) 0 };

/* Run all global constructors for the program.
   Note that they are run in reverse order.  */

void __do_global_ctors (void)
{
	func_ptr *p;
	for (p = __CTOR_END__ - 1; *p != (func_ptr) -1; p--)
		(*p) ();
}

/* .init section end.
   This must live at the end of the .init section.  */

asm ("\n\
	.section .init\n\
	j	__do_global_ctors\n\
");

asm (".text");

#endif	// CFG_GCOV
