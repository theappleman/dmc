/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *fd;
static char word[1024];

static void mbox_ls () {
  // TODO
  // printf("%d %d\n", i, size);
}

static void mbox_cat (int idx, int body) {
  // TODO
}

static void mbox_rm (int idx) {
  // TODO
}

static char *getword () {
  fscanf (stdin, "%255s", word);
  if (feof (stdin))
    *word = '\0';
  return word;
}

static int doword (char *word) {
  int ret = 1;
  if (*word == '\0') {
    /* Do nothing */
  } else
  if (!strcmp (word, "ls")) {
    mbox_ls ();
  } else
  if (!strcmp (word, "cat")) {
    mbox_cat (atoi (getword ()), 1);
  } else
  if (!strcmp (word, "rm")) {
    mbox_rm (atoi (getword ()));
  } else
  if (!strcmp (word, "head")) {
    mbox_cat (atoi (getword ()), 0);
  } else
  if (!strcmp (word, "login")) {
    getword (); // ignore login
    getword (); // ignore password
  } else
  if (!strcmp (word, "exit"))
    ret = 0;
  return ret;
}

int main (int argc, char **argv) {
  int ret = 1;
  if (argc>1) {
    fd = fopen (argv[1], "r");
    if (fd != NULL) {
      ret = 0;
      while (doword (getword ()));
      fclose (fd);
    } else fprintf (stderr, "Cannot open %s\n", argv[1]);
  } else fprintf (stderr, "Usage: dmc-mbox [mbox-file] 2> body > fifo < input\n");
  return ret;
}
