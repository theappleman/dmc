include config.mk

ifeq ($(HAVE_SSL),1)
SSL_LIBS=`pkg-config libssl --libs`
CFLAGS+=`pkg-config libssl --cflags`
endif

CFLAGS+=-DHAVE_SSL=${HAVE_SSL}
CFLAGS+=-DVERSION=\"${VERSION}\"

BINS=dmc-mbox dmc-smtp dmc-pop3 dmc-imap4 dmc-pack dmc-mbox

all: config.mk ${BINS}

config.mk: 
	@echo creating $@ from config.def.mk
	cp config.def.mk config.mk
	${MAKE} clean

dmc-smtp: smtp.o
	${CC} ${LDFLAGS} smtp.o -o dmc-smtp -lresolv

dmc-mbox: mbox.o
	${CC} ${LDFLAGS} mbox.o -o dmc-mbox

dmc-pack: pack.o
	${CC} ${LDFLAGS} pack.o -o dmc-pack

dmc-pop3: pop3.o
	${CC} ${LDFLAGS} ${SSL_LIBS} pop3.o -o dmc-pop3

dmc-imap4: imap4.o
	${CC} ${LDFLAGS} ${SSL_LIBS} imap4.o -o dmc-imap4

install:
	chmod +x dmc dmc-tag dmc-mdir
	cp -f dmc.1 ${PREFIX}/share/man/man1
	cp -f dmc ${PREFIX}/bin
	cp -f dmc-tag ${PREFIX}/bin
	cp -f dmc-smtp ${PREFIX}/bin
	cp -f dmc-pop3 ${PREFIX}/bin
	cp -f dmc-imap4 ${PREFIX}/bin
	cp -f dmc-pack ${PREFIX}/bin
	cp -f dmc-mbox ${PREFIX}/bin
	cp -f dmc-mdir ${PREFIX}/bin

uninstall:
	rm -f ${PREFIX}/bin/dmc
	rm -f ${PREFIX}/bin/dmc-*
	rm -f ${PREFIX}/share/man/man1/dmc.1

clean:
	rm -f ${BINS} *.o

dist:
	mkdir -p dmc-${VERSION}
	cd dmc-${VERSION} && hg clone .. .
	rm -rf dmc-${VERSION}/.hg
	tar czvf dmc-${VERSION}.tar.gz dmc-${VERSION}
	rm -rf dmc-${VERSION}

loc:
	sloccount .
