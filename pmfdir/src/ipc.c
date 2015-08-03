/*
 *  ipc.c -- inter-process communication
 *  Part of the contents of this file was originally copied from TinyTalk.
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: May 25, 1992
 *
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include "safe_malloc.h"
#include "pmf.h"
#include "config.h"
#include "globals.h"

#ifndef FD_ZERO                                       /* For BSD 4.2 systems */
#define DESCR_MASK int
#define FD_ZERO(p) (*p = 0)
#define FD_SET(n,p) (*p |= (1<<(n)))
#define FD_ISSET(n,p) (*p & (1<<(n)))
#else /* FD_ZERO */                                   /* For BSD 4.3 systems */
#define DESCR_MASK fd_set
#endif /* FD_ZERO */

extern char *get_now_date_string();

extern int errno;

static struct in_addr host_address;
static struct sockaddr_in socket_address;
static int mud_socket;

/* This is a character that we read "accidentaly" when checking the socket.
 * The value 0 means empty, else it will be handled by the next call to
 * getmud that returns some more characters.
 */
static int a_character_that_we_read = 0;

/*---------------------------------------------------------------------------*/

/* Get the address (in the internal format) to the mud host. */
static int get_host_address(name, addr)
char *name;
struct in_addr *addr;
{

    if (*name == '\0') {
        fatal("This cannot happen: No host address specified.");
    }
    else if (isdigit(name[0])) {
	/* Numerical address, e g "130.236.254.13" */
        union {
	    /* Some people have expressed disapproval of the the inetaddr routine. */
            long signed_thingy;
            unsigned long unsigned_thingy;
        } thingy;

        addr->s_addr = (unsigned long)inet_addr(name);
        thingy.unsigned_thingy = addr->s_addr;
        if (thingy.signed_thingy == -1)
	    fatal("Couldn't find host '%s'.", name);
    }
    else {
	/* A name address, t ex "nanny.lysator.liu.se" */
        struct hostent *the_host_entry_p;

        if ((the_host_entry_p = gethostbyname(name)) == NULL)
	    fatal("Couldn't find host '%s'.", name);
        bcopy(the_host_entry_p->h_addr, (char *)addr, sizeof(struct in_addr));
    }
    return 0;
} /* get_host_address */

/* Create a socket and connect it to mud. */
int connect_to_mud(host_string, port_string)
char *host_string, *port_string;
{

    if (get_host_address(host_string, &host_address) != 0)
        fatal("connect_to_mud: This shouldn't happen.\n");
    socket_address.sin_addr.s_addr = host_address.s_addr;
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(atoi(port_string));
    if ((mud_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        fatal("Couldn't open socket.");
    if (connect(mud_socket, (struct sockaddr *)&socket_address,
		sizeof(struct sockaddr_in)) < 0) {
        fatal("Couldn't connect to socket.");
    }
#ifdef FNDELAY
    fcntl(mud_socket, F_SETFL, FNDELAY);
#endif

    set_variable("host", host_string);
    set_variable("port", port_string);
    connected = 1;
    return 0;
} /* connect_to_mud */

int disconnect()
{
    connected = 0;
    return close(mud_socket);
}

/* Is there anyhting to read from the socket? */
static int output_from_mud_is_available()
{
    long nr_to_read;
    int ioctl_result, read_result;
    char dummy_buf[4];

    ioctl_result = ioctl(mud_socket, FIONREAD, &nr_to_read);
    if (ioctl_result)
	fatal("First ioctl FIONREAD for MUD socket returned %d, nr_to_read = %ld.",
	      ioctl_result, nr_to_read);
    else if (nr_to_read)
	return (int)nr_to_read;

#ifdef FNDELAY
    else {
	/*  No text from mud waiting to be read. Check that the socket
	 *  is still open.
	 *  Pell says: Om errno {r EWOULDBLOCK s} {r allt ok. Annars eof. Eller?
	 */
	read_result = read (mud_socket, dummy_buf, 1);
	if (read_result == -1 && errno == EWOULDBLOCK)
	    return NULL;	/* Normal! */
	else if (read_result == 0)
	    say_goodbye_and_exit(0);
	else {
	    /*  Sigh! Some text arrived between the ioctl call and the read!
	     *  Save this accidentaly read character, and see how many more
	     *  characters the idiots sent us.
	     */
	    a_character_that_we_read = dummy_buf[0];
	    ioctl_result = ioctl(mud_socket, FIONREAD, &nr_to_read);
	    if (ioctl_result)
		fatal("Second ioctl FIONREAD for MUD socket returned %d, nr_to_read = %ld.",
		      ioctl_result, nr_to_read);
	    else
		return (int)nr_to_read;
	}
    }
#else
    else
	return 0;
#endif
} /* output_from_mud_is_available */

/* Has the player typed anything on the terminal? */
int input_from_player_is_available()
{
    long nr_to_read;
    int result;

    if (stdin->_cnt > 0)
	return 1;
    else if (result = ioctl (fileno(stdin), FIONREAD, &nr_to_read))	/* =, not == */
	fatal("ioctl FIONREAD for stdin returns %d, nr_to_read=%ld.", result, nr_to_read);
    else
	return (int)nr_to_read;
} /* input_from_player_is_available */

/*---------------------------------------------------------------------------*/

/* Send a command to the Mud process. Add a newline. */
send_to_mud(str)
  char *str;
{
    int nr_to_send, nr_actually_sent;
    char *buffer;
    
    nr_to_send = strlen(str);
    /* All this work just to add a newline: */
    buffer = safe_malloc(nr_to_send + 3);
    strcpy(buffer, str);
    buffer[nr_to_send] = '\r';
    ++nr_to_send;
    buffer[nr_to_send] = '\n';
    ++nr_to_send;
    buffer[nr_to_send] = '\0';

    IPC_DEBUG(("send_to_mud: sending (%d chars): %s", nr_to_send, buffer));

    nr_actually_sent = write(mud_socket, buffer, nr_to_send);
    if (log_file != NULL) {
	if (debug_log)
	    fprintf(log_file, "Sent (%d chars) at %8.8s:\n%s",
		    nr_actually_sent, get_now_date_string() + 11, buffer);
	else
	    fprintf(log_file, "%s", buffer);
	fflush(log_file);
    }
    if (nr_actually_sent != nr_to_send)
	fatal("Wrote %d instead of %d characters to MUD socket in sendmud.",
	      nr_actually_sent, nr_to_send);

    /* When typing the password, echo is turned off. Turn it back on! */
    if (echo_is_off) {	/* A bit kludgy. */
	ldisplay("\n");
	echo_on(); /* This is done using the telnet protocol too! */
    }
    safe_free(buffer);
} /* send_to_mud */

/* Find the length of the text up to next null byte or including next newline. */
static int get_line_length(cp)
  register char *cp;
{
    register int len;

    len = 0;
    while (*cp && *cp != '\n') {
	++cp;
	++len;
    }
    if (*cp == '\n')
	++len;
    return len;
} /* get_line_length */

/* Get ONE line from mud. */
char *get_line_from_mud()
{
    static char *rec_buffer = NULL, *current_bufp, *after_buffer;
    char *the_line;
    int buffer_size, nr_to_read, nr_actually_read, line_length;

    /* Just to be sure */
    if (rec_buffer && *current_bufp == '\0') {
	message("Problem in get_line_from_mud: Null byte in buffer changed to '?'.");
	*current_bufp = '?';
    }

    buffer_size = 0;
    if (!rec_buffer) {
	/* The buffer is empty. Maybe we can get more from the socket? */
	if (!connected)
	    return NULL;	/* In this case, obviously we cannot! */
	nr_to_read = output_from_mud_is_available();
	/* Note: Now, a_character_that_we_read might be set! */
	if (nr_to_read) {
	    /* Wow! There IS something to read from the socket! */
	    buffer_size = nr_to_read + 1;	/* The null byte too. */
	    rec_buffer = safe_malloc(buffer_size);
	    nr_actually_read = read(mud_socket, rec_buffer, nr_to_read);
	    IPC_DEBUG(("read(mud_socket, rec_buffer, nr_to_read) returned %d", nr_actually_read));
	    rec_buffer[nr_actually_read] = '\0';
	    if (log_file != NULL) {
		if (debug_log)
		    fprintf(log_file, "Received (%d chars) at %8.8s:\n%s",
			    nr_actually_read, get_now_date_string() + 11, rec_buffer);
		else
		    fprintf(log_file, "%s", rec_buffer);
		fflush(log_file);
	    }
	    if (nr_actually_read != nr_to_read)
		fatal("Read %d instead of %d characters from MUD socket in get_line_from_mud.",
		      nr_actually_read, nr_to_read);
	}
	if (a_character_that_we_read) {
	    /* One more character to take care of. Put it first in the buffer. */
	    if (!buffer_size)
		buffer_size = 1;
	    ++buffer_size;
	    if (rec_buffer)
		rec_buffer = safe_realloc(rec_buffer, buffer_size);
	    else
		rec_buffer = safe_malloc(buffer_size);
	    copy_bytes_down(rec_buffer + 1, rec_buffer, buffer_size - 1);
	    rec_buffer[0] = a_character_that_we_read;
	    a_character_that_we_read = 0;
	}
	current_bufp = rec_buffer;
	after_buffer = rec_buffer + buffer_size - 1;
    }
    if (!rec_buffer)
	return NULL;

    /* If this text was just received: remove telnet control sequences! */
    if (current_bufp == rec_buffer) {
	char *cp;

/* -- for efficiency, but not needed --
	while (*(unsigned char *)current_bufp == 255) {
	    IPC_DEBUG(("Got three telnet control characters (1) in get_line_from_mud"));
	    telnet_protocol((unsigned char)current_bufp[0],
			    (unsigned char)current_bufp[1],
			    (unsigned char)current_bufp[2]);
	    current_bufp += 3;
	}
*/

	cp = current_bufp;
	while (cp < after_buffer) {

	    if (*(unsigned char *)cp == 255) {
		IPC_DEBUG(("Got three telnet control characters (2) in get_line_from_mud"));
		telnet_protocol((unsigned char)cp[0], (unsigned char)cp[1], (unsigned char)cp[2]);
		after_buffer -= 3;
		if (after_buffer + 3 > cp)
		    /* Yes, NannyMud once sent me \377 as the last byte... */
		    copy_bytes_up(cp, cp+3, (after_buffer - cp) + 1);
			/* +1: The null-byte too! */
		else {
		    *cp = '\0';
		    break;	/* End loop */
		}
	    }
	    else
		++cp;
	}

	if (current_bufp == after_buffer) {
	    safe_free(rec_buffer);
	    rec_buffer = NULL;
	    return NULL;
	}

    }

    /*  Now, we have some text to take care of. It was either waiting for us
     *  in the buffer, or the buffer was empty and we found some text to read
     *  from the mud game.
     */
    line_length = get_line_length(current_bufp);
    the_line = safe_malloc(line_length + 1);
    strncpy(the_line, current_bufp, line_length);
    the_line[line_length] = '\0';	/* After the newline (if there is one) */
    current_bufp += line_length;
    if (current_bufp >= after_buffer) {
	IPC_DEBUG(("get_line_from_mud: freeing the buffer"));
	safe_free(rec_buffer);
	rec_buffer = NULL;
    }

    /* If the line ends with CR + LF and not only LF, remove that CR! */
    if (the_line[line_length - 2] == '\015' && the_line[line_length - 1] == '\012') {
	the_line[line_length - 2] = '\012';
	the_line[line_length - 1] = '\0';
    }

    return the_line;
} /* get_line_from_mud */


/*  This function takes the three bytes from a telnet command,
 *  and implements a very limited telnet protocol.
 */
telnet_protocol(one, two, three)
unsigned int one, two, three;
{
    unsigned char reply[3];
    int nr_actually_sent;

    IPC_DEBUG(("telnet_protocol(%d, %d, %d)", one, two, three));
    ASSERT(one == 255);

    reply[0] = 255;	/* The code for IAC, i. e. "interpret as command:" */
    reply[2] = three;	/* Usually we answer with the same option. */

    /* 251 is the code for WILL, i. e. "I will use option" */
    /* 252 is the code for WONT, i. e. "I won't use option" */
    /* 253 is the code for DO, i. e. "please, you use option" */
    /* 1 is the option code for ECHO */

    if (two == 251 && three == 1) {
	/* Got "I will use option echo" -- reply "I won't use option echo" */
	reply[1] = 252;	/* The code for WONT, i. e. "I won't use option" */
	echo_off();
    }
    else if (two == 252 && three == 1) {
	/* Got "I will not use option echo" -- reply "I will use option echo" */
	reply[1] = 251;	/* The code for WILL, i. e. "I will use option" */
	echo_on();
    }
    else {
	IPC_DEBUG(("telnet_protocol, not responding to that command"));
	return;
    }

    IPC_DEBUG(("telnet_protocol responding: %d, %d, %d", reply[0], reply[1], reply[2]));
    nr_actually_sent = write(mud_socket, reply, 3);
    if (log_file != NULL) {
	fprintf(log_file, "Sent (%d chars):\n%s", nr_actually_sent, reply);
	fflush(log_file);
    }
    if (nr_actually_sent != 3)
	fatal("Wrote %d instead of %d characters to MUD socket in telnet_protocol.",
	      nr_actually_sent, 3);
} /* telnet_protocol */
