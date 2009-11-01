include config.mk

ifeq ($(HAVE_SSL),1)
SSL_LIBS=`pkg-config libssl --libs`
CFLAGS+=`pkg-config libssl --cflags`
endif
CFLAGS+=-DHAVE_SSL=${HAVE_SSL}
CFLAGS+=-DVERSION=\"${VERSION}\"

all: config.mk dmc-smtp dmc-pop3 dmc-imap4 dmc-pack

config.mk: 
	@echo creating $@ from config.def.mk
	cp config.def.mk config.mk
	${MAKE} clean

dmc-smtp: smtp.o
	${CC} ${LDFLAGS} smtp.o -o dmc-smtp -lresolv

# sock.c ?
dmc-pop3: pop3.o
	${CC} ${LDFLAGS} ${SSL_LIBS} pop3.o -o dmc-pop3

dmc-imap4: imap4.o
	${CC} ${LDFLAGS} ${SSL_LIBS} imap4.o -o dmc-imap4

dmc-pack: pack.o
	${CC} ${LDFLAGS} pack.o -o dmc-pack

install:
	chmod +x dmc dmc-tag
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

dist:
	mkdir -p dmc-${VERSION}
	cd dmc-${VERSION} && hg clone .. .
	rm -rf dmc-${VERSION}/.hg
	tar czvf dmc-${VERSION}.tar.gz dmc-${VERSION}
	rm -rf dmc-${VERSION}

loc:
	sloccount .
