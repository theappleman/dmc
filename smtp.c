/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/nameser.h>
#include <resolv.h>
#define BIND_4_COMPAT

static int resmx(const char *domain) {
	char host[NS_MAXDNAME+1];
	char last[NS_MAXDNAME+1];
	typedef union {
	HEADER head;
		char buf[PACKETSZ];
	} pkt_t;
	unsigned char buf[PACKETSZ];
	unsigned char *rrptr;
	pkt_t *pkt = (pkt_t *)buf;
	int querylen, len, n, exprc;
	int rrtype, antrrtype;
	int rrpayloadsz;

	querylen = res_querydomain(domain, "", C_IN,T_MX,
		(void*)&buf, PACKETSZ);

	if (ntohs(pkt->head.rcode) == NOERROR) {
		n = ntohs(pkt->head.ancount);
		if (n==0) {
			fprintf(stderr, "No MX found\n");
			return 1;
		}

		/* expand DNS query */
		len = dn_expand( buf,
			buf+querylen, buf+sizeof(HEADER),
			host, sizeof(host));

		if (len<0) {
			fprintf(stderr, "No MX found\n");
			return 1;
		}

		rrptr = buf+sizeof(HEADER)+4+len;

		while(rrptr < buf+querylen) {
			/* expand NAME resolved */
			exprc = dn_expand(buf,buf+querylen,rrptr,host,sizeof(host));
			if (exprc<0) {
				fprintf(stderr, "No MX found\n");
				return 1;
			}
			rrptr += exprc;
			rrtype = (rrptr[0]<<8|rrptr[1]);
			rrpayloadsz = (rrptr[8]<<8|rrptr[9]);
			rrptr += 10; 
			switch(rrtype) {	
			/* TODO support for IPv6: case T_AAAA: */
			case T_A:
				if (strcmp(host, last)) {
					printf("%s\n", host);
					if (--n==0) querylen=0;
				}
				break;
			}
			antrrtype = rrtype;
			rrptr += rrpayloadsz;
		 }
	} else {
		printf("%s\n", domain);
		//fprintf(stderr, "No MX found\n");
		return 1;
	}
	return 0;
}

int main(int argc, char **argv) {
	if (argc>1) {
		char *ch = strchr(argv[1], '@');
		if (ch) return resmx(ch+1);
		else {
			/* do the daemon stuff here */
			fprintf(stderr, "TODO: SMTP protocol not yet implemented\n");
		}
	} else printf("Usage: dmc-smtp [user@domain]     # Get MX for domain\n"
		"Usage: dmc-smtp fifo | nc host 25 > fifo\n");
	return 0;
}
