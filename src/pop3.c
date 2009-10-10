/* dmc :: Copyleft -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>


static char word[1024];
static char *fifo;
static int ff;

static char *getword() {
	fscanf(stdin, "%255s", word);
	if (feof(stdin))
		*word = '\0';
	return word;
}

static int ready() {
        struct pollfd fds[1];
        fds[0].fd = ff;
        fds[0].events = POLLIN|POLLPRI;
        fds[0].revents = POLLNVAL|POLLHUP|POLLERR;
        return poll((struct pollfd *)&fds, 1, 10);
}

static int waitreply(int lock) {
	int ret, reply = -1;
	fflush(stdout);
	while(lock || ready()) {
		lock = 0;
		ret = read(ff, word, 512);
		if (ret<1)
			break;
		if (reply==-1) {
			if (!memcmp(word, "+OK", 3))
				reply = 1;
			else
			if (!memcmp(word, "-ERR", 4))
				reply = 0;
		}
		fprintf(stderr, "\n\n-->(%d)(%s)<--\n\n", ret, word);
	}
	fprintf(stderr, "REPLY BOOL IS %d\n", reply);
	return reply;
}

static int doword(char *word) {
	int ret = 1;
	if (*word == '\0' || !strcmp(word, "exit")) {
		printf("QUIT\n");
		waitreply(1);
		ret = 0;
	} else
	if (!strcmp(word, "help") || !strcmp(word, "?")) {
		fprintf(stderr, "Use: ls cat head rm login exit\n");
	} else
	if (!strcmp(word, "ls")) {
		printf("LIST\n");
		waitreply(1);
	} else
	if (!strcmp(word, "cat")) {
		printf("RETR %d\n", atoi(getword()));
		waitreply(1);
	} else
	if (!strcmp(word, "head")) {
		printf("TOP %d\n", atoi(getword()));
		waitreply(1);
	} else
	if (!strcmp(word, "rm")) {
		printf("DELE %d\n", atoi(getword()));
		waitreply(1);
	} else
	if (!strcmp(word, "login")) {
		printf("USER %s\n", getword());
		waitreply(1);
		printf("PASS %s\n", getword());
		waitreply(1);
	} else printf("NOOP\n");
	return ret;
}

static void parseoutput(void) {
	while(!feof(stdin)) {
		sleep(1);
		printf("TODO\n");
	}
}

static void cleanup(int foo) {
	close(ff);
	unlink(fifo);
	exit(0);
}

int main(int argc, char **argv) {
	if (argc>1) {
		signal(SIGINT, cleanup);
		fifo = argv[1];
		mkfifo(fifo, 0600);
		ff = open(fifo, O_RDONLY); //O_NONBLOCK|O_RDWR);
		waitreply(1);
		while(doword(getword()));
		cleanup(0);
	} else parseoutput();
	return 0;
}
