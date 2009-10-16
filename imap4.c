/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static char word[1024];
static int ctr = 1;

static char *getword() {
	fscanf(stdin, "%127s", word);
	if (feof(stdin))
		*word = '\0';
	return word;
}

static int waitreply() {
	char *str;
	int ret = -1;
	fgets(word, sizeof(word), stdin);
	if (atoi(word) != ctr-1)
		fprintf(stderr, "Invalid sequence number received\n");
	if ( (str = strchr(word, ' ')) ) {
		if (!memcmp(str+1, "OK", 3))
			ret = 1;
		else if (!memcmp(str+1, "NO", 4))
			ret = 0;
	}
	return ret; // 1 if true, 0 if false
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
	if (*word == '\0' || !strcmp(word, "exit")) {
		printf("%d LOGOUT\n", ctr++);
		waitreply();
		ret = 0;
	} else
	if (!strcmp(word, "help") || !strcmp(word, "?")) {
		fprintf(stderr, "Use: ls lsdir cat head rm rmdir login exit mvdir\n");
	} else
	if (!strcmp(word, "ls")) {
// TODO:
		printf("LIST\n");
		waitreply();
	} else
	if (!strcmp(word, "cd")) {
		printf("%d SELECT %s\n", ctr++, getword());
		waitreply();
	} else
	if (!strcmp(word, "search")) {
		printf("%d SEARCH TEXT \"%s\"\n", ctr++, getword());
		waitreply();
	} else
	if (!strcmp(word, "lsdir")) {
		printf("%d LIST \"\" *\n", ctr++);
		waitreply();
	} else
	if (!strcmp(word, "cat")) {
		printf("%d FETCH %d ALL\n",
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
	if (!strcmp(word, "rm")) {
// TODO:
		printf("DELE %d\n", atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "rmdir")) {
		printf("%d DELETE %d\n",
			ctr++, atoi(getword()));
		waitreply();
	} else
	if (!strcmp(word, "login")) {
		printf("%d LOGIN %s %s\n",
			ctr++, getword(), getword());
		waitreply();
	} else {
		printf("%d NOOP\n", ctr++);
		waitreply();
	}
	return ret;
}

int main() {
	while(doword(getword()));
	return 0;
}
