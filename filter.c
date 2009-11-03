/* dmc :: Copyleft -- nibble (at) develsec (dot) org */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	char b[1024], *ptr;
	int edit = argc, print = 0, i;

	for (i = 0; i < argc; i++)
		if (!strcmp (argv[i], "-h")) {
			fprintf (stderr, "Usage: %s [-h] [headers | :] [-e] [new headers]\n", argv[0]);
			return 1;
		} else if (!strcmp (argv[i], "-e"))
			edit = i;
	memset (b, '\0', 1024);
	/* Headers */
	while (fgets (b, 1023, stdin) && b[0] != '\n')
		if (print && (b[0] == ' ' || b[0] == '\t'))
			fputs (b, stdout);
		else for (i = 1; i < edit && argv[i]; i++)
			if (strstr (b, argv[i])) {
				/* Edit/Remove Headers */
				print = 1;
				for (i = edit + 1; i < argc && argv[i]; i++)
					if ((ptr = strchr (argv[i], ':')) &&
							!strncmp (b, argv[i], ptr - argv[i])) {
						if (ptr[1] != '\0')
							puts (argv[i]);
						argv[i][0] = '\0';
						print = 0;
						break;
					}
				if (print) fputs (b, stdout);
				break;
			} else if (b[0] != ' ' && b[0] != '\t')
				print = 0;
	/* New Headers */
	for (i = edit + 1; i < argc; i++)
		if (argv[i][0]) puts (argv[i]);
	if (edit < argc)
		puts ("");
	/* Body */
	while ((argc < 2 || edit < argc) && fgets (b, 1023, stdin))
		fputs (b, stdout);
	return 0;
}
