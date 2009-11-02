/* dmc :: Copyleft -- nibble (at) develsec (dot) org */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

void b64_encode(unsigned char in[3], unsigned char out[4], int len) {
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

int b64_decode(unsigned char in[4], unsigned char out[3]) {
	unsigned char len = 3, i, v[4];
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

void mime_pack(char **files, int nfiles) {
	FILE *fd = NULL;
	char b[1024], cmd[1024], *ptr = NULL;
	unsigned char bd[1024];
	int header = 1, len, in, out, i;

	memset (b, '\0', 1024);
	while(fgets (b, 1023, stdin)) {
		if (header && b[0] == '\n') {
			printf ("Content-Type: multipart/mixed; boundary=\"dmc-multipart\"\n\n"
				"--dmc-multipart\n"
				"Content-Type: text/plain\n");
			header = 0;
		}
		fputs (b, stdout);
	}
	for(i = 0; i < nfiles; i++) {
		snprintf (cmd, 1023, "file -i \"%s\"", files[i]);
		if (!(fd=popen (cmd, "r")))
			continue;
		fgets (b, 1023, fd);
		pclose (fd);
		if (!(ptr = strchr(b, ' ')))
			continue;
		if (!(fd=fopen(files[i], "r")))
			continue;
		puts ("--dmc-multipart");
		printf ("Content-Type: %s\n", ptr+1);
		printf ("Content-Disposition: attachment; filename=\"%s\"\n", files[i]);
		if (strstr (ptr, "text")) {
			printf("Content-Transfer-Encoding: quoted-printable\n\n");
			while (fgets(b, 1023, fd))
				printf("%s", b);
		} else {
			puts ("Content-Transfer-Encoding: base64\n");
			while ((len=fread(b, 1, 57, fd))) {
				memset(bd,'\0',1024);
				for(in=out=0;in<len;in+=3,out+=4)
					b64_encode((unsigned char*)b+in,bd+out,len-in>3?3:len-in);
				puts ((char *)bd);
			}
		}
		fclose (fd);
	}
	puts ("--dmc-multipart--");
}

void mime_unpack (int xtr) {
	FILE *fd = NULL;
	char b[1024], boundary[1024], encoding[1024], filename[1024], *ptr = NULL;
	unsigned char bd[1024];
	int entity = 0, dump = 0, len, in, out, i;

	boundary[0] = encoding[0] = filename[0] = '\0';
	memset (b, '\0', 1024);
	while (fgets(b, 1023, stdin)) {
		if (!memcmp(b, "--", 2)) {
			if (boundary[0] && strstr(b, boundary) &&
					!memcmp(b+strlen(b)-3, "--", 2)) {
				entity = 0;
				dump = 0;
			} else {
				strncpy(boundary, b+2, 1023);
				if ((len = strlen(boundary)) > 0)
					boundary[len-1] = '\0';
				if (fgets(b, 1023, stdin) && strstr(b, "Content-Type:")) {
					entity = 1;
					dump = 0;
				}
			}
		}
		if (entity) {
			if ((ptr = strstr(b, "Content-Transfer-Encoding:"))) {
				strncpy(encoding, ptr+26, 1023);
			} else if ((ptr = strstr(b, "filename="))) {
				strncpy(filename, ptr+10, 1023);
				if ((len=strlen(filename)) > 1)
					filename[len-2] = '\0';
			} else if (b[0] == '\n') {
				if (xtr) {
					if (!dump && filename[0] && (fd = fopen(filename, "w"))) {
						printf("%s\n", filename);
						dump = 1;
						continue;
					} else if (dump && strstr(encoding, "base64"))
						dump = 0;
				} else {
					if (!dump && filename[0])
						puts (filename);
				}
			} 
		} else boundary[0] = '\0';
		if (dump) {
			if (strstr(encoding, "base64")) {
				memset(bd,'\0',1024);
				if ((len = strlen(b)) > 0)
					b[len-1] = '\0';
				for(in=out=0;in<len-1;in+=4)
					out+=b64_decode((unsigned char*)b+in,bd+out);
				for(i=0;i<out;i++)
					fputc(bd[i], fd);
			} else fputs(b, fd); 
		} else if (fd) {
			fclose(fd);
			fd = NULL;
			encoding[0] = filename[0] = '\0';
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 2 || !strcmp(argv[1], "-h"))
		printf ("Usage: %s [-hlu | attachment1 attachment2...] < mail\n", argv[0]);
	else if (!strcmp(argv[1], "-l"))
		mime_unpack (0);
	else if (!strcmp(argv[1], "-u"))
		mime_unpack (1);
	else mime_pack(argv+1, argc-1);
	return 0;
}
