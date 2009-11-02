/* dmc :: Copyleft -- nibble (at) develsec (dot) org */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv)
{
	char b[1024], *ptr;
	int edit = argc, print, i;

	for (i = 0; i < argc; i++)
		if (!strcmp (argv[i], "-h")) {
			printf ("usage: [fields | :] %s [-he] [new fields]\n", argv[0]);
			return 1;
		} else if (!strcmp (argv[i], "-e"))
			edit = i + 1;

	memset (b, '\0', 1024);
	/* Headers */
	while (fgets (b, 1023, stdin) && b[0] != '\n')
		for (i = 0, print = 1; i < edit && argv[i]; i++)
			if (strstr (b, argv[i])) {
				for (i = edit; i < argc && argv[i]; i++)
					if ((ptr = strchr (argv[i], ':')) &&
						!strncmp (b, argv[i], ptr - argv[i])) {
						if (ptr[1] != '\0')
							puts (argv[i]);
						argv[i][0] = '\0';
						print = 0;
					}
				if (print) fputs (b, stdout);
			}
	/* New Headers */
	for (i = edit; i < argc; i++)
		if ((ptr = strchr (argv[i], ':')) && ptr[1] != '\0') puts (argv[i]);
	if (edit < argc)
		puts ("");
	/* Body */
	while ((argc < 2 || edit < argc) && fgets (b, 1023, stdin))
		fputs (b, stdout);
	return 0;
}
