/* dmc - dynamic mail client
 * See LICENSE file for copyright and license details.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <stdarg.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if HAVE_SSL
#include <openssl/ssl.h>
static int ssl = 0;
static SSL_CTX *ctx;
static SSL *sfd;
#endif
static int fd = -1;

int sock_ssl (int fd) {
#if HAVE_SSL
	// TODO Check certificate
	SSL_library_init ();
	SSL_load_error_strings ();
	OpenSSL_add_all_algorithms ();
	ctx = SSL_CTX_new (SSLv23_method ());
	sfd = SSL_new (ctx);
	SSL_set_fd (sfd, fd);
	if (SSL_connect (sfd) < 1)
		return -1;
	else
#endif
		return fd;
}

int sock_connect(const char *host, int port, int ssl_enable) {
	struct sockaddr_in sa;
	struct hostent *he;
	int s = socket (AF_INET, SOCK_STREAM, 0);
	ssl = ssl_enable, fd = -1;
	if (s != -1) {
		memset (&sa, 0, sizeof (sa));
		sa.sin_family = AF_INET;
		he = (struct hostent *)gethostbyname (host);
		if (he != (struct hostent*)0) {
			sa.sin_addr = *((struct in_addr *)he->h_addr);
			sa.sin_port = htons (port);
			if (!connect (s, (const struct sockaddr*)&sa, sizeof (struct sockaddr)))
				fd = ssl ? sock_ssl (s) : s;
		}
		if (fd == -1)
			close (s);
	}
	return fd;
}

int sock_ready() {
#if HAVE_SSL
	if (ssl && SSL_pending (sfd))
		return 1;
#endif
	struct pollfd fds[1];
	fds[0].fd = fd;
	fds[0].events = POLLIN|POLLPRI;
	fds[0].revents = POLLNVAL|POLLHUP|POLLERR;
	return poll(fds, 1, 1000);
}

void sock_close() {
#if HAVE_SSL
	SSL_CTX_free (ctx);
	SSL_free (sfd);
#endif
	close (fd);
}

int sock_write(const char *str) {
#if HAVE_SSL
	if (ssl)
		return SSL_write (sfd, str, strlen(str));
	else
#endif
	return write (fd, str, strlen(str));
}

int sock_printf(const char *fmt, ...) {
	va_list ap;
	int ret = -1;
	char buf[1024];
	va_start (ap, fmt);
	vsnprintf (buf, 1023, fmt, ap);
	// XXX check len
	ret = sock_write (buf);
	va_end (ap);
	return ret;
}

int sock_read (char *buf, int len) {
	int ret;
	memset (buf, 0, len+1);
#if HAVE_SSL
	if (ssl)
		ret = SSL_read (sfd, buf, len);
	else
#endif
		ret = read (fd, buf, len);
	if (ret>0)
		buf[ret] = '\0';
	return ret;
}
