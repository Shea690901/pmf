/*
 *  pmf.h -- some miscellaneous global definitions
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#ifndef NULL
#   define NULL 0
#endif

#ifdef DEBUG
    extern int debug;
#   define USER_DEBUG(args) {	\
	if (debug)		\
		print_debug_message args;	\
	}
#   define INTERNAL_DEBUG(args) {	\
	if (internal_debug)		\
		print_internal_debug_message args;	\
	}
#   define IPC_DEBUG(args) {	\
	if (ipc_debug)		\
		print_ipc_debug_message args;	\
	}
#else
#   define USER_DEBUG(args)
#   define INTERNAL_DEBUG(args)
#   define IPC_DEBUG(args)
#endif

#ifdef DEBUG
#   define ASSERT(p)		\
	{			\
		if (!(p))	\
			fatal("Assert failed in \"%s\" line %d.", __FILE__, __LINE__);	\
	}
#else
#   define ASSERT(p)	(p)
#endif
