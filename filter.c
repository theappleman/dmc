/* dmc :: Copyleft -- nibble (at) develsec (dot) org */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	char b[1024], argv2[1024][1024], *ptr;
	int edit = argc, print = 0, i, j;

	for (i = 0; i < argc; i++)
		if (!strcmp (argv[i], "-h")) {
			printf ("Usage: %s [-h] [headers | :] [-e] [new headers] < mail\n", argv[0]);
			return 1;
		} else if (!strcmp (argv[i], "-e"))
			edit = i;
	for (i = 0; i < argc; i++) {
		strncpy (argv2[i], argv[i], 1023);
		argv2[i][1023] = '\0';
	}
	memset (b, '\0', 1024);
	/* Headers */
	while (fgets (b, 1023, stdin) && b[0] != '\n')
		if ((b[0] == ' ' || b[0] == '\t')) {
			if (print) fputs (b, stdout);
		} else for (i = 1; i < edit && argv[i]; i++)
			if (!strncmp (b, argv[i], strlen(argv[i])) || argv[i][0] == ':') {
				/* Edit/Remove Headers */
				print = 1;
				for (j = edit + 1; j < argc && argv[j]; j++)
					if ((ptr = strchr (argv[j], ':')) &&
						!strncmp (b, argv[j], ptr - argv[j] + 1)) {
						if (ptr[1] != '\0' && argv2[j][0])
							puts (argv[j]);
						argv2[j][0] = '\0';
						print = 0;
						break;
					}
				if (print) fputs (b, stdout);
				break;
			} else print = 0;
	/* New Headers */
	for (i = edit + 1; i < argc; i++)
		if (argv2[i][0]) puts (argv2[i]);
	if (edit < argc) puts ("");
	/* Body */
	while ((argc < 2 || edit < argc) && fgets (b, 1023, stdin))
		fputs (b, stdout);
	return 0;
}
