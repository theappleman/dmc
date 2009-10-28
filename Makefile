CC?=gcc
PREFIX?=/usr
CFLAGS?=-Wall

all: dmc-smtp dmc-pop3 dmc-imap4 dmc-pack

dmc-smtp: smtp.o
	${CC} ${LDFLAGS} smtp.o -o dmc-smtp -lresolv

dmc-pop3: pop3.o
	${CC} ${LDFLAGS} pop3.o -o dmc-pop3

dmc-imap4: imap4.o
	${CC} ${LDFLAGS} imap4.o -o dmc-imap4

dmc-pack: pack.o
	${CC} ${LDFLAGS} pack.o -o dmc-pack

install:
	cp -f dmc.1 ${PREFIX}/share/man/man1
	cp -f dmc ${PREFIX}/bin
	cp -f dmc-tag ${PREFIX}/bin
	cp -f dmc-smtp ${PREFIX}/bin
	cp -f dmc-pop3 ${PREFIX}/bin
	cp -f dmc-imap4 ${PREFIX}/bin
	cp -f dmc-pack ${PREFIX}/bin

uninstall:
	rm -f ${PREFIX}/bin/dmc
	rm -f ${PREFIX}/bin/dmc-smtp
	rm -f ${PREFIX}/bin/dmc-pop3
	rm -f ${PREFIX}/bin/dmc-imap4
	rm -f ${PREFIX}/bin/dmc-pack

clean:
	rm -f dmc-pop3 dmc-imap4 dmc-smtp dmc-pack *.o

loc:
	sloccount .
