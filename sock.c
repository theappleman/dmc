/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
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

int sock_ssl (int enable) {
#if HAVE_SSL
  int err;
  if (ssl) {
    // challenge, check cert, etc..
    sfd = SSL_new (ctx);
    SSL_set_fd (sfd, fd);
    err = SSL_connect (sfd);
    /* TODO: check cert */
    SSL_set_accept_state (sfd);
  }
  ssl = enable;
  return 1;
#else
  return 0;
#endif
}

// TODO: cleanup all those fd=-1
int sock_connect (const char *host, int port, int ssl) {
  struct sockaddr_in sa;
  struct hostent *he;
  int s = socket (AF_INET, SOCK_STREAM, 0);
  sock_ssl (ssl);
  fd = -1;
  if (s != -1) {
    fd = s;
    memset (&sa, 0, sizeof (sa));
    sa.sin_family = AF_INET;
    he = (struct hostent *)gethostbyname (host);
    if (he != (struct hostent*)0) {
      sa.sin_addr = *((struct in_addr *)he->h_addr);
      sa.sin_port = htons (port);
      if (connect (fd, (const struct sockaddr*)&sa, sizeof (struct sockaddr)))
        fd = -1;
    } else fd = -1;
    if (fd == -1)
      close (s);
  } else fd = -1;
  return fd;
}

static int sock_ready() {
  struct pollfd fds[1];
  fds[0].fd = fd;
  fds[0].events = POLLIN|POLLPRI;
  fds[0].revents = POLLNVAL|POLLHUP|POLLERR;
  return poll((struct pollfd *)&fds, 1, 10);
}

void sock_close () {
#if HAVE_SSL
  SSL_free (sfd);
#endif
  close (fd);
}

int sock_write (const char *str) {
#if HAVE_SSL
  if (ssl)
    return SSL_write (sfd, str, strlen(str));
  else
#endif
  return write (fd, str, strlen(str));
}

int sock_printf (const char *fmt, ...) {
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
#if HAVE_SSL
  if (ssl)
    ret = SSL_read (sfd, buf, 1024);
  else
#endif
    ret = read (fd, buf, 1024);
  if (ret>0)
    buf[ret] = '\0';
  return ret;
}
