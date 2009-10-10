/* dmc :: Copyleft -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static char word[1024];
static char *fifo;
static int ff;

static char *getword() {
	fscanf(stdin, "%255s", word);
	if (feof(stdin))
		*word = '\0';
	return word;
}

static int waitreply() {
	int ret = -1;
	fflush(stdout);
	fgets(word, sizeof(word), stdin);
	if (!memcmp(word, "+OK", 3))
		ret = 1;
	else
	if (!memcmp(word, "-ERR", 4))
		ret = 0;
	return ret; // 1 if true, 0 if false
}

static int doword(char *word) {
	int ret = 1;
	if (*word == '\0' || !strcmp(word, "exit")) {
		printf("QUIT\n");
		waitreply();
		ret = 0;
	} else
	if (!strcmp(word, "help") || !strcmp(word, "?")) {
		fprintf(stderr, "Use: ls cat head rm login exit\n");
	} else
	if (!strcmp(word, "ls")) {
		printf("LIST\n");
		waitreply();
	} else
	if (!strcmp(word, "cat")) {
		printf("RETR %d\n", atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "head")) {
		printf("TOP %d\n", atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "rm")) {
		printf("DELE %d\n", atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "login")) {
		printf("USER %s\n", getword());
		printf("PASS %s\n", getword());
		waitreply();
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
		ff = open(fifo, O_NONBLOCK|O_RDWR);
		while(doword(getword()));
		cleanup(0);
	} else parseoutput();
	return 0;
}
