/*
 *  cryptsay.c -- handling encrypted conversation
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include "safe_malloc.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

extern void do_remote_command();

/*---------------------------------------------------------------------------*/

char *cryptsay_handle_line(mudline)
char *mudline;
{
    static char *kx_buffer = NULL;
    register char *cp;
    register int offset;

    ASSERT(cryptkey && cryptkey[0]);

    USER_DEBUG(("cryptsay_handle_line: \"%s\"", mudline));
    USER_DEBUG(("Key is: \"%s\"", cryptkey));

    if (!cryptkey)
	return mudline;
    else {
	cp = mudline;
	offset = 0;
	while (*cp) {
	    if (*cp == 'P' && strncmp(cp, "PMF_CRYPTSAY: ", 14) == 0) {
		message("An encrypted line was received and de-crypted by PMF:");
		cp += 14; /* Skip the "PMF_CRYPTSAY: " */
		if (kx_buffer)
		    safe_free(kx_buffer);
		kx_buffer = safe_malloc(strlen(mudline) - 14 + 1);
		strncpy(kx_buffer, mudline, offset);
		pmf_decrypt(kx_buffer + offset, cp);

		USER_DEBUG(("cryptsay_handle_line returns decrypted: \"%s\"", kx_buffer));
		return kx_buffer;
	    }
	    ++cp;
	    ++offset;
	}
    }

    USER_DEBUG(("cryptsay_handle_line returns same: \"%s\"", mudline));
    return mudline;
} /* cryptsay_handle_line */

cmd_cryptsay(str)
char *str;
{
    register char *kx_cp, *ky_cp, *ky_buffer;

    if (cryptkey == NULL)
	error("You tried to \"cryptsay\" with no \"cryptkey\" defined.");

    /* strlen("say PMF_CRYPTSAY: ") == 18 */
    ky_buffer = safe_malloc(strlen(str) + 18 + 1);
    strcpy(ky_buffer, "say PMF_CRYPTSAY: ");
    ky_cp = ky_buffer + 18;
    kx_cp = str;
    pmf_encrypt(ky_cp, kx_cp);
    queue_mudline(ky_buffer, "");
    safe_free(ky_buffer);
} /* cmd_cryptsay */

cmd_crypttell(player, str)
char *player, *str;
{
    register char *kx_cp, *ky_cp, *ky_buffer;

    if (cryptkey == NULL)
	error("You tried to \"cryptrell\" with no \"cryptkey\" defined.");

    /* strlen("tell " + " PMF_CRYPTSAY: ") == 20 */
    ky_buffer = safe_malloc(strlen(player) + strlen(str) + 20 + 1);
    ky_cp = safe_malloc(strlen(str) + 1);
    pmf_encrypt(ky_cp, str);
    sprintf(ky_buffer, "tell %s PMF_CRYPTSAY: %s", player, ky_cp);
    queue_mudline(ky_buffer, "");
    safe_free(ky_buffer);
} /* cmd_crypttell */

pmf_encrypt(ky_cp, kx_cp)
register char *ky_cp, *kx_cp;
{
    register char *key_cp;
    char *ky_start;

    ky_start = ky_cp;
    USER_DEBUG(("Going to encrypt: \"%s\"", kx_cp));
    USER_DEBUG(("Key is: \"%s\"", cryptkey));

    key_cp = cryptkey;
    while (*kx_cp){ 
	if (isprint(*kx_cp))
	    *ky_cp = ((*kx_cp - 040) + (*key_cp - 040)) % (0176 - 040 + 1) + 040;
	else
	    *ky_cp = *kx_cp;
	++kx_cp;
	++ky_cp;
	++key_cp;
	if (!*key_cp)
	    key_cp = cryptkey;
    }
    *ky_cp = '\0';

    USER_DEBUG(("Encrypted result: \"%s\"", ky_start));
} /* pmf_encrypt */


pmf_decrypt(kx_cp, ky_cp)
register char *kx_cp, *ky_cp;
{
    register char *key_cp;
    char *kx_start;

    kx_start = kx_cp;
    USER_DEBUG(("Going to decrypt: \"%s\"", ky_cp));
    USER_DEBUG(("Key is: \"%s\"", cryptkey));

    key_cp = cryptkey;
    while (*ky_cp){ 
	if (isprint(*ky_cp))
	    *kx_cp = ((*ky_cp - 040) - (*key_cp - 040) + (0176 - 040 + 1)) % (0176 - 040 + 1) + 040;
	else
	    *kx_cp = *ky_cp;
	++ky_cp;
	++kx_cp;
	++key_cp;
	if (!*key_cp)
	    key_cp = cryptkey;
    }
    *kx_cp = '\0';

    USER_DEBUG(("Decrypted result: \"%s\"", kx_start));
} /* pmf_decrypt */
