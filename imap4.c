/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>


static char *cmd = NULL;
static char word[4096];
static int ctr = 1;
static char *fifo;
static int ff;
static char *dir;

/* XXX full of bugs and ugly code */
static char *getword() {
	char *p = NULL;
	char *str = word;
	*word=0;
reread:
	fscanf(stdin, "%255s", str);
	if (feof(stdin))
		*str = '\0';
	else {
		if (str == word) {
			if (*word=='"') {
				strcpy(word, word+1);
				p = strchr(word, '"');
				if (p) {
					*p=0;
				} else {
					str = word+strlen(word);
					*str = ' ';
					str++;
					*str = 0;
					goto reread;
				}
			}
		} else {
			p = strchr(str, '"');
			if (p) {
				*p=0;
			} else {
				*str = ' ';
				*str = 0;
				str++;
				goto reread;
			}
		}
	}
	return word;
}

static int ready() {
        struct pollfd fds[1];
        fds[0].fd = ff;
        fds[0].events = POLLIN|POLLPRI;
        fds[0].revents = POLLNVAL|POLLHUP|POLLERR;
        return poll((struct pollfd *)&fds, 1, 10);
}

static int waitreply() {
	char *ptr, *str = word;
	int lock = 1;
	int line = 0;
	int ret, reply = -1;
	fflush(stdout);
// XXX ugly sleep hack
	while(lock || !ready()) {
		lock = 0;
		ret = read(ff, str, 1024);
		if (ret<1) {
			fprintf(stderr, " BREAK BREAK\n");
			break;
		}
		str[ret] = 0;
		if (line == 0) {
			ptr = strchr(word, ' ');
			if (ptr) {
				if (!memcmp(ptr+1, "OK", 2))
					reply = 1;
				else
				if (!memcmp(ptr+1, "NO", 2))
					reply = 0;
				else // TODO: Make 'BAD' be -1 ?
				if (!memcmp(ptr+1, "BAD", 3))
					reply = 0;
			}
			// XXX: Fix output, just show first line
			fprintf(stderr, "### %s %d \"%s\"\n", cmd, reply, str);
		}
		str = str+strlen(str);
		line++;
	//	fprintf(stderr, "--> %s\n", str);
	}
	fprintf(stderr, "==> (((%s)))\n", word);
	fflush(stderr);
	write(2, "\x00", 1); // end of output
	return reply;
}

#if 0
LIST - list all folders
LSUB - list all subscribed folders
SUBSCRIBE - subcribe a folder
CHECK - ???
CLOSE - commit the delete stuff (maybe must be done after rm)
EXPUNGE - permanent remove of deltec
SEARCH - ...
RECENT - show the number of recent messages
#endif
static int doword(char *word) {
	int ret = 1;
	free (cmd);
	cmd = strdup(word);
	if (*word == '\0') {
		/* Do nothing */
	} else
	if (!strcmp(word, "exit")) {
		printf("%d LOGOUT\n", ctr++);
		waitreply();
		ret = 0;
	} else
	if (!strcmp(word, "help") || !strcmp(word, "?")) {
		fprintf(stderr, "Use: login exit ls cat head rm rmdir mkdir mvdir\n");
	} else
	if (!strcmp(word, "pwd")) {
		fprintf(stderr, "%s\n", dir);
	} else
	if (!strcmp(word, "cd")) {
		free(dir);
		dir = strdup(getword());
		if (!strcmp(dir, "\"\""))
			*dir=0;
		printf("%d SELECT \"%s\"\n", ctr++, dir);
		waitreply();
	} else
	if (!strcmp(word, "search")) {
		printf("%d SEARCH TEXT \"%s\"\n", ctr++, getword());
		waitreply();
	} else
	if (!strcmp(word, "ls")) {
		printf("%d LIST \"%s\" *\n", ctr++, dir);
		waitreply();
	} else
	if (!strcmp(word, "cat")) {
		printf("%d FETCH %d body[]\n",
			ctr++, atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "head")) {
		printf("%d FETCH %d body[header]\n",
			ctr++, atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "mvdir")) {
		printf("%d RENAME %s %s\n",
			ctr++, getword(), getword());
	} else
	if (!strcmp(word, "mkdir")) {
		printf("%d CREATE \"%s\"\n",
			ctr++, getword());
	} else
	if (!strcmp(word, "rm")) {
// TODO:
		printf("%d DELE %d\n", ctr++, atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "rmdir")) {
		printf("%d DELETE \"%s\"\n",
			ctr++, getword());
		waitreply();
	} else
	if (!strcmp(word, "login")) {
		char * user = strdup(getword());
		char * pass = strdup(getword());
		printf("%d LOGIN \"%s\" \"%s\"\n",
			ctr++, user, pass);
		free(user);
		free(pass);
		waitreply();
	} else {
		printf("%d NOOP\n", ctr++);
		waitreply();
	}
	return ret;
}

/* TODO: make it shared btwn pop3 + imap + smtp?  */
static void cleanup(int foo) {
	close(ff);
	unlink(fifo);
	exit(0);
}

int main(int argc, char **argv) {
	int ret = 0;
	if (argc>1) {
		signal(SIGINT, cleanup);
		fifo = argv[1];
		mkfifo(fifo, 0600);
		ff = open(fifo, O_RDONLY);
		if (ff != -1) {
			dir = strdup("");
			waitreply();
			while(doword(getword()));
			cleanup(0);
			ret = 0;
		} else fprintf(stderr, "Cannot open fifo file.\n");
	} else fprintf(stderr, "Usage: dmc-imap4 fifo | nc host 443 > fifo\n");
	return ret;
}
