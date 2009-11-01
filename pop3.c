/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include "sock.c"

static char *cmd = NULL;
static char word[1024];

static char *getword () {
	fscanf (stdin, "%255s", word);
	if (feof (stdin))
		*word = '\0';
	return word;
}

static int waitreply() {
	char result[256];
	char *ch, *str = word;
	int lock = 1;
	int reply = -1;

	ftruncate (2, 0);
	while (lock || sock_ready()) {
		lock = 0;
		if (sock_read (word, 512) <1)
			break;
		if (reply==-1) {
			ch = strchr (str, '\r');
			if (!ch) ch = strchr (str, '\n');
			if (ch) {
				if (!memcmp (word, "+OK", 3))
					reply = 1;
				else
				if (!memcmp (word, "-ERR", 4))
					reply = 0;
				*ch = 0;
				snprintf (result, 1023, "### %s %d \"%s\"\n", cmd, reply, str);
				str = ch+((ch[1]=='\n')?2:1);
			}
		}
		// TODO: \r \n issues
		ch = strstr (str, "\r\n.");
		if (ch)
			*ch = '\0';
		write (2, str, strlen (str));
	}
	write (1, result, strlen (result));
	/* stderr lseek works on pipes :D */
	lseek (2, 0, 0);
	return reply;
}

static int doword (char *word) {
	int ret = 1;
	free (cmd);
	cmd = strdup (word);

	if (*word == '\0') {
		/* Do nothing */
	} else
	if (!strcmp (word, "exit")) {
		sock_printf ("QUIT\n");
		waitreply ();
		ret = 0;
	} else
	if (!strcmp (word, "help") || !strcmp (word, "?")) {
		fprintf(stderr, "Use: ls cat head rm login exit\n");
	} else
	if (!strcmp(word, "ls")) {
		sock_printf("LIST\n");
		waitreply();
	} else
	if (!strcmp(word, "cat")) {
		sock_printf ("RETR %d\n", atoi(getword()));
		waitreply ();
	} else
	if (!strcmp(word, "head")) {
		sock_printf ("TOP %d 50\n", atoi(getword()));
		waitreply ();
	} else
	if (!strcmp(word, "rm")) {
		sock_printf ("DELE %d\n", atoi(getword()));
		waitreply ();
	} else
	if (!strcmp(word, "login")) {
		sock_printf ("USER %s\n", getword());
		waitreply (); // TODO: if user fail, do not send pass
		sock_printf ("PASS %s\n", getword());
		waitreply ();
	} else sock_printf ("NOOP\n");
	return ret;
}

int main(int argc, char **argv) {
	int ssl = 0, ret = 1;
	if (argc>2) {
		if (argc>3)
			ssl = (*argv[3]=='1');
		if (sock_connect (argv[1], atoi (argv[2]), ssl) >= 0) {
			ret = 0;
			atexit (sock_close);
			waitreply ();
			while (doword (getword()));
		} else fprintf (stderr, "Cannot connect to %s %d\n", argv[1], atoi(argv[2]));
	} else fprintf (stderr, "Usage: dmc-pop3 host port [ssl] 2> body > fifo < input\n");
	return 0;
}
