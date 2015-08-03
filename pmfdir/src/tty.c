/*
 *  tty.c -- handle the terminal, e. g. turn on and off echo
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Oct 29, 1993
 *
 */

#include <strings.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include "config.h"
#include "pmf.h"

/* Added by dodurham@stimpy.ualr.edu for Linux machines */
#ifdef SYSV
#    include <termio.h>
#else
#    include <sgtty.h>
#endif

extern int echo_is_off;

/*---------------------------------------------------------------------------*/

#ifdef GNU_READLINE

extern readline_echoing_p;

echo_off()
{
    echo_is_off = 1;
    readline_echoing_p = 0;
    fflush(stdout);
} /* echo_off */

echo_on()
{
    echo_is_off = 0;
    readline_echoing_p = 1;
} /* echo_on */

/*---------------------------------------------------------------------------*/

#else
    /* Not GNU_READLINE */

#ifdef USE_TIOCGETP

echo_off()
{
    struct sgttyb sgttyb;
    
    echo_is_off = 1;
    fflush(stdout);
    
    if (ioctl(1, TIOCGETP, &sgttyb) == -1)
	fatal("ioctl TIOCGETP failed in echo_off.");
    sgttyb.sg_flags &= ~ECHO;
    if (ioctl(1, TIOCSETP, &sgttyb) == -1)
	fatal("ioctl TIOCSETP failed in echo_off.");
} /* echo_off */

echo_on()
{
    struct sgttyb sgttyb;
    
    echo_is_off = 0;

    if (ioctl(1, TIOCGETP, &sgttyb) == -1)
	fatal("ioctl TIOCGETP failed in echo_on.");
    sgttyb.sg_flags |= ECHO;
    if (ioctl(0, TIOCSETP, &sgttyb) == -1)
	fatal("ioctl TIOCSETP failed in echo_on.");
} /* echo_on */

set_cbreak()
{
    struct sgttyb sgttyb;

    if (gtty(fileno(stdin), &sgttyb))
	fatal("gtty failed in set_cbreak.");
    sgttyb.sg_flags |= CBREAK;
    if(stty(fileno(stdin), &sgttyb))
	fatal("stty failed in set_cbreak.");
} /* set_cbreak */

#endif

/*---------------------------------------------------------------------------*/

#ifdef USE_TCGETA

echo_off()
{
    struct termio termio;
    
    echo_is_off = 1;
    fflush(stdout);
    
    if (ioctl(0, TCGETA, &termio) == -1)
	fatal("ioctl TCGETA failed in echo_off.");
    termio.c_lflag &= ~ECHO;
    if (ioctl(0, TCSETA, &termio) == -1)
	fatal("ioctl TCSETA failed in echo_off.");
} /* echo_off */

echo_on()
{
    struct termio termio;
    
    echo_is_off = 0;

    if (ioctl(0, TCGETA, &termio) == -1)
	fatal("ioctl TCGETA failed in echo_on.");
    termio.c_lflag |= ECHO;
    if (ioctl(0, TCSETA, &termio) == -1)
	fatal("ioctl TCSETA failed in echo_on.");
} /* echo_on */

set_cbreak()
{
    struct termio termio;

    if (ioctl(0, TCGETA, &termio) == -1)
	fatal("ioctl TCGETA failed in set_cbreak.");
    termio.sg_flags |= CBREAK;
    if (ioctl(0, TCSETA, &termio) == -1)
	fatal("ioctl TCSETA failed in set_cbreak.");
} /* set_cbreak */

#endif
#endif

/*---------------------------------------------------------------------------*/

#ifdef USE_TIOCGETP

static struct sgttyb terminal_state;

pmf_save_terminal()
{
    if (gtty(fileno(stdin), &terminal_state))
	fatal("gtty failed in pmf_save_terminal.");
} /* pmf_save_terminal */

pmf_restore_terminal()
{
    if(stty(fileno(stdin), &terminal_state))
	fatal("stty failed in pmf_restore_terminal.");
} /* pmf_restore_terminal */

#endif

#ifdef USE_TCGETA

static struct termio terminal_state;

pmf_save_terminal()
{
    if (ioctl(0, TCGETA, &terminal_state) == -1)
	fatal("ioctl TCGETA failed in pmf_save_terminal.");
} /* pmf_save_terminal */

pmf_restore_terminal()
{
    if (ioctl(0, TCSETA, &terminal_state) == -1)
	fatal("ioctl TCSETA failed in pmf_restore_terminal.");
} /* pmf_restore_terminal */

#endif
