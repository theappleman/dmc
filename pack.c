/* dmc :: Copyleft -- nibble (at) develsec (dot) org */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* XXX: here? */
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

void b64_encode(unsigned char in[3], unsigned char out[4], int len)
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

int b64_decode(unsigned char in[4], unsigned char out[3])
{
	unsigned char v[4];
	int len = 3, i;

	for(i=0;i<4;i++) {
		if (in[i]<43||in[i]>122)
			return -1;
		v[i] = cd64[in[i]-43];
		if (v[i]!='$') v[i]-=62;
		else { len = i-1; break; }
	}
	out[0] = v[0] << 2 | v[1] >> 4;
	out[1] = v[1] << 4 | v[2] >> 2;
	out[2] = ((v[2] << 6) & 0xc0) | v[3];

	return len;
}

int mime_pack(char **files, int nfiles)
{
	/*TODO*/
	int i;

	for(i = 0; i < nfiles; i++)
		printf("%s\n", files[i]);
	return 0;
}

int mime_unpack()
{
	FILE *fd = NULL;
	char b[1024], bd[1024], boundary[1024], encoding[1024], filename[1024];
	char *ptr;
	int entity = 0, dump = 0, i, o;

	boundary[0] = encoding[0] = filename[0] = '\0';
	while(fgets(b, 1023, stdin)) {
		if (!memcmp(b, "--", 2)) {
			if (boundary[0] && strstr(b, boundary) &&
					!memcmp(b+strlen(b)-3, "--", 2)) {
				if (dump)
					fclose(fd);
				boundary[0] = encoding[0] = filename[0] = '\0';
				entity = dump = 0;
			} else {
				strncpy(boundary, b+2, 1023);
				boundary[strlen(boundary)-1] = '\0';
				if (fgets(b, 1023, stdin) && strstr(b, "Content-Type:")) {
					dump = 0;
					entity = 1;
				} else boundary[0] = '\0';
			}
		} else if (entity) {
			if ((ptr = strstr(b, "Content-Transfer-Encoding:")))
				strncpy(encoding, ptr+26, 1023);
			else if ((ptr = strstr(b, "filename="))) {
				strncpy(filename, ptr+10, 1023);
				filename[strlen(filename)-2] = '\0';
				printf("%s\n", filename);
			} else if (b[0] == '\n') {
				if (!dump && filename && (fd = fopen(filename, "w")))
					dump = 1;
				else dump = 0;
			} else if (dump) {
				if (strstr(encoding, "base64")) {
					memset(bd,'\0',1024);
					for(i=o=0;i<strlen(b)-1;i+=4,o+=3)
						b64_decode(b+i,bd+o);
					for(i=0;i<o;i++)
						fputc(bd[i], fd);
				} else
					fputs(b, fd); 
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	int i;

	if(argc < 2 || !strcmp(argv[1], "-h"))
		printf("usage: %s [-uh | attachment1 attachment2...]\n", argv[0]);
	else if(!strcmp(argv[1], "-u"))
		mime_unpack();
	else mime_pack(argv+1, argc-1);

	return 0;
}
