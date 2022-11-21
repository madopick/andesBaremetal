/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef CFG_MAKEFILE

// Address mode select
//#define	CFG_16MB		// platform is 16MB, if it isn't defined, platform is 4GB

#define	CFG_HWZOL

// Code coverage select
//#define	CFG_GCOV		// do code coverage support

// Simulation environment select
//#define	CFG_SIMU		// do simulation on SID

// Build mode select
// The BUILD_MODE can be specified to BUILD_XIP/BUILD_BURN/BUILD_LOAD only.
//
// NOTE: The BUILD_SIMU is not supported any more. Please uncomment CFG_SIMU
//       to select simulation environment.
#define BUILD_MODE	BUILD_XIP	// NOTE: AE210P support BUILD_XIP only


//----------------------------------------------------------------------------------------------------
// The followings are predefined settings
// Please do not modify them

#define	BUILD_LOAD	1		// The program is loaded by GDB or eBIOS
#define	BUILD_BURN	2		// The program is burned to the flash, but run in RAM
#define	BUILD_XIP	3		// The program is burned to the flash, and run in flash
							// To use this mode, xip linker script files must be used

#if BUILD_MODE == BUILD_LOAD
#define CFG_LOAD
#endif
#if BUILD_MODE == BUILD_BURN
#define CFG_BURN
#endif
#if BUILD_MODE == BUILD_XIP
#define CFG_XIP
#endif

#endif

#endif // __CONFIG_H__
