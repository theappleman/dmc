/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "sock.c"

static char *cmd = NULL;
static char word[4096];
static int ctr = 1;
static char *dir;

/* TODO: make getword() ready() and cleanup() shared between smtp,pop,imap? */

/* XXX full of bugs and ugly code */
static char *getword() {
	char *p = NULL;
	char *str = word;
	*word = 0;
reread:
	fscanf (stdin, "%255s", str);
	if (feof (stdin))
		*str = '\0';
	else {
		if (str == word) {
			if (*word=='"') {
				strcpy (word, word+1);
				p = strchr (word, '"');
				if (!p) {
					str = word + strlen (word);
					*str = ' ';
					str++;
					*str = 0;
					goto reread;
				} else *p = 0;
			}
		} else {
			p = strchr (str, '"');
			if (!p) {
				*str = ' ';
				str++;
				*str = 0;
				goto reread;
			} else *p = 0;
		}
	}
	return word;
}

static int waitreply() {
	char *ptr;
	int lock = 1;
	int line = 0;
	int reply = -1;
	char result[256];

	ftruncate (2, 0);
	lseek (2, 0, SEEK_SET);
	word[0] = result[0] = '\0';
	while(lock || sock_ready ()) {
		lock = 0;
		if (sock_read (word, 4095) <1)
			break;
		if (line++ == 0) {
			ptr = strchr (word, ' ');
			if (ptr) {
				if (!memcmp (ptr+1, "OK", 2))
					reply = 1;
				else
				if (!memcmp (ptr+1, "FETCH", 5))
					reply = 1;
				else
				if (!memcmp (ptr+1, "NO", 2))
					reply = 0;
				else // TODO: Do 'BAD' should be -1 ?
				if (!memcmp (ptr+1, "BAD", 3))
					reply = 0;
				else
					reply = -1;
			} else reply = -1;

			ptr = strchr (word, '\r');
			if (!ptr)
				ptr = strchr (word, '\n');
			if (ptr) {
				*ptr = '\0';
				snprintf (result, 254, "### %s %d \"%s\"\n", cmd, reply, word);
				*ptr = '\n';
				if (reply != -1)
					strcpy (word, ptr+1);
			}
		}
		write (2, word, strlen (word));
	}

	write (1, result, strlen(result));
	return reply;
}

#if 0
LIST - list all folders
LSUB - list all subscribed folders
SUBSCRIBE - subcribe a folder
CHECK - ???
CLOSE - commit the delete stuff (maybe must be done after rm)
EXPUNGE - permanent remove of deltec
RECENT - show the number of recent messages
#endif
static int doword(char *word) {
	int ret = 1;
	free (cmd);
	cmd = strdup (word);
	if (*word == '\0') {
		/* Do nothing */
	} else
	if (!strcmp (word, "exit")) {
		sock_printf ("%d LOGOUT\n", ctr++);
		waitreply ();
		ret = 0;
	} else
	if (!strcmp (word, "help") || !strcmp (word, "?")) {
		fprintf (stderr, "Use: login exit find cd pwd ls cat head rm rmdir mkdir mvdir\n");
	} else
	if (!strcmp (word, "pwd")) {
		fprintf (stderr, "%s\n", dir);
	} else
	if (!strcmp (word, "cd")) {
		free(dir);
		dir = strdup (getword ());
		if (!strcmp (dir, "\"\""))
			*dir = 0;
		sock_printf ("%d SELECT \"%s\"\n", ctr++, dir);
		waitreply ();
	} else
	if (!strcmp (word, "find")) {
		sock_printf ("%d SEARCH TEXT \"%s\"\n", ctr++, getword ());
		waitreply ();
	} else
	if (!strcmp (word, "ls")) {
		sock_printf ("%d LIST \"%s\" *\n", ctr++, dir);
		waitreply ();
	} else
	if (!strcmp (word, "cat")) {
		sock_printf ("%d FETCH %d body[]\n",
			ctr++, atoi (getword ()));
		waitreply ();
	} else
	if (!strcmp (word, "head")) {
		sock_printf ("%d FETCH %d body[header]\n",
			ctr++, atoi (getword ()));
		waitreply ();
	} else
	if (!strcmp (word, "mvdir")) {
		sock_printf ("%d RENAME %s %s\n",
			ctr++, getword (), getword ());
	} else
	if (!strcmp (word, "mkdir")) {
		sock_printf ("%d CREATE \"%s\"\n", ctr++, getword ());
	} else
	if (!strcmp (word, "rm")) {
		sock_printf ("%d DELE %d\n", ctr++, atoi (getword ()));
		waitreply ();
	} else
	if (!strcmp (word, "rmdir")) {
		printf("%d DELETE \"%s\"\n",
			ctr++, getword ());
		waitreply ();
	} else
	if (!strcmp (word, "login")) {
		char *user = strdup (getword ());
		char *pass = strdup (getword ());
		sock_printf ("%d LOGIN \"%s\" \"%s\"\n",
			ctr++, user, pass);
		free (user);
		free (pass);
		waitreply ();
	} else {
		sock_printf ("%d NOOP\n", ctr++);
		waitreply ();
	}
	return ret;
}

int main (int argc, char **argv) {
	int ssl = 0, ret = 0;
	if (argc>2) {
		if (argc>3)
			ssl = (*argv[3]=='1');
		if (sock_connect (argv[1], atoi (argv[2]), ssl) >= 0) {
			ret = 0;
			atexit (sock_close);
			waitreply ();
			dir = strdup ("");
			while (doword (getword ()));
		} else printf ("Cannot connect to %s %d\n", argv[1], atoi (argv[2]));
	} else printf ("Usage: dmc-imap4 host port 2> body > fifo < input\n");
	return ret;
}
