/* dmc :: Copyleft 2009 -- pancake (at) nopcode (dot) org */

#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <stdarg.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "config.h"

#if HAVE_SSL
#include <openssl/ssl.h>
static int ssl = 0;
#endif

static int fd = -1;

// TODO: cleanup all those fd=-1
int sock_connect (const char *host, int port) {
  struct sockaddr_in sa;
  struct hostent *he;
  int s = socket (AF_INET, SOCK_STREAM, 0);
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

int sock_close () {
  return close (fd);
}

int sock_write (const char *str) {
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
  int ret = read (fd, buf, 1024);
  if (ret>0)
    buf[ret] = '\0';
  return ret;
}
