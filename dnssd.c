/*
  Copyright (c) 2012 Andrew Tunnell-Jones

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "dnssd.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define STDERR(...) { fprintf(stderr, __VA_ARGS__); fflush(NULL); }
#define STDOUT(...) { fprintf(stdout, __VA_ARGS__); fflush(NULL); }
#define DEBUG(...) {						\
    if(DEBUG) {						\
      STDERR("%s:%i - ", __FILE__, __LINE__);			\
      STDERR(__VA_ARGS__);					\
      STDERR("\n");						\
    }								\
  }
#define EXIT_USAGE { DEBUG("EXIT_USAGE"); return usage(argv[0]); }

#define OP_NONE 0
#define OP_LIST 1
#define OP_LOOKUP 2
#define OP_WATCH 3

#define RC_NOERROR 0
#define RC_BADINPUT 1
#define RC_OTHERERR 2

#define DEFAULT_TIMEOUT 500

bool DEBUG = false;
bool RUN = TRUE;
bool MORE_COMING = false;
int RETURN_CODE = RC_NOERROR;
int OP = OP_NONE;

struct timeval START_TV;

int main(int argc, char **argv) {
  int argi = 1;
  if (argc < 3) EXIT_USAGE;
  if (0 == strcmp("-D", argv[argi])) {
    DEBUG = true;
    DEBUG("DEBUG = true");
    argi++;
  }
  gettimeofday(&START_TV, NULL);
  if (0 == strcmp("list", argv[argi])) {
    OP = OP_LIST;
    DEBUG("OP = OP_LIST");
    return main_list(argi+1, argc, argv);
  } else if (0 == strcmp("lookup", argv[argi])) {
    OP = OP_LOOKUP;
    DEBUG("OP = OP_LOOKUP");
    return main_lookup(argi+1, argc, argv);
  } else if (0 == strcmp("watch", argv[argi])) {
    OP = OP_WATCH;
    DEBUG("OP = OP_WATCH");
    return main_watch(argi+1, argc, argv);
  }
  EXIT_USAGE;
}

int usage(char* self) {
  STDERR("usage: %s [-D] list type\n"					\
	 "       %s [-D] list type domain\n"				\
	 "       %s [-D] list type timeout\n"				\
	 "       %s [-D] list type domain timeout\n"			\
	 "       %s [-D] lookup name type domain\n"			\
	 "       %s [-D] lookup name type domain timeout\n"		\
	 "       %s [-D] watch type exec\n"				\
	 "       %s [-D] watch type domain exec\n"			\
	 "\n"								\
	 " * timeout is specified in seconds\n"			\
	 " * results may expire by the time your script consumes them\n" \
	 " * returns %i on (detectably) bad input and %i for other errors\n",
	 self, self, self, self, self, self, self, self, RC_BADINPUT,
	 RC_OTHERERR);
  return 1;
}

int main_list(int argi, int argc, char **argv) {
  DNSServiceRef sd_ref;
  DNSServiceErrorType err;
  char *ambiguous;
  char *type;
  char *domain = "";
  long timeout = DEFAULT_TIMEOUT;
  switch(argc - argi)
    {
    case 1:
      type = argv[argi];
      break;
    case 2:
      type = argv[argi];
      ambiguous = argv[argi + 1];
      if (isdigit(ambiguous[0])) {
	timeout = atoi(ambiguous) * 1000;
      } else {
	domain = ambiguous;
      }
      break;
    case 3:
      type = argv[argi];
      domain = argv[argi + 1];
      timeout = atoi(argv[argi+2]) * 1000;
    }
  if (timeout <= 0) EXIT_USAGE;
  DEBUG("type: %s domain: %s timeout: %li", type, domain, timeout);
  err = DNSServiceBrowse(&sd_ref, 0, kDNSServiceInterfaceIndexAny, type, domain,
			 (DNSServiceBrowseReply) reply_list, NULL);
  if (err) {
    DEBUG("DNSServiceBrowse returned error: %i", err);
    return err == kDNSServiceErr_BadParam ? RC_BADINPUT : RC_OTHERERR;
  }
  return main_select(sd_ref, timeout);
}

static void DNSSD_API reply_list(DNSServiceRef sd_ref,
				 DNSServiceFlags flags,
				 uint32_t ifIndex,
				 DNSServiceErrorType err,
				 const char * name,
				 const char * type,
				 const char * domain,
				 void * context) {
  if (!err) {
    MORE_COMING = flags & kDNSServiceFlagsMoreComing;
    DEBUG("MORE_COMING = %i", MORE_COMING);
    STDOUT("%s %s\n", domain, name);
  } else {
    DEBUG("reply_list invoked with error %i", err);
    RETURN_CODE = RC_OTHERERR;
    RUN = false;
  }
}

int main_lookup(int argi, int argc, char **argv) {
  DNSServiceRef sd_ref;
  DNSServiceErrorType err;
  char *name;
  char *type;
  char *domain;
  int timeout = DEFAULT_TIMEOUT;
  switch(argc - argi)
    {
    case 4:
      if (!isdigit(argv[argi+3][0])) EXIT_USAGE;
      timeout = atoi(argv[argi+3]) * 1000;
      if (timeout == 0) {
	timeout = 10;
      } else if (timeout <= 0) {
	EXIT_USAGE;
      }
    case 3:
      name = argv[argi];
      type = argv[argi+1];
      domain = argv[argi+2];
      break;
    default:
      EXIT_USAGE;
    }
  DEBUG("name: %s type: %s domain: %s timeout: %i",
	name, type, domain, timeout);
  err = DNSServiceResolve(&sd_ref, 0, kDNSServiceInterfaceIndexAny, name, type,
			  domain, (DNSServiceResolveReply) reply_lookup, NULL);
  if (err) {
    DEBUG("DNSServiceResolve returned error: %i", err);
    return 2;
  }
  return main_select(sd_ref, timeout);
}

static void DNSSD_API reply_lookup(DNSServiceRef sd_ref,
				   DNSServiceFlags flags,
				   uint32_t ifIndex,
				   DNSServiceErrorType err,
				   const char * fullname,
				   const char * hosttarget,
				   uint16_t port,
				   uint16_t txtLen,
				   const unsigned char * txtRecord,
				   void * context) {
  RUN = false;
  int itemIndex = 0;
  const void * txtBytes;
  char key[256];
  uint8_t valueLen;
  char value[256];
  if (!err) {
    STDOUT("%s %i", hosttarget, ntohs(port));
    while (kDNSServiceErr_NoError == \
	   TXTRecordGetItemAtIndex(txtLen, txtRecord, itemIndex, 256,
				   key, &valueLen, &txtBytes)) {
      DEBUG("txtLen %i itemIndex %i valueLen %i", txtLen, itemIndex, valueLen);
      itemIndex++;
      if (0 != strncmp("", key, 1)) {
	if (valueLen == 0) {
	  STDOUT(" %s", key);
	} else {
	  STDOUT(" %s=", key);
	}
      } else {
	STDOUT(" ");
      }
      if (valueLen <= 0) continue;
      memset(value, '\0', 256);
      memcpy(value, txtBytes, valueLen);
      STDOUT("%s", value);
    }
    STDOUT("\n");
  } else {
    DEBUG("reply_lookup invoked with error %i", err);
    RETURN_CODE = RC_OTHERERR;
  }
}

int main_watch(int argi, int argc, char **argv) {
  DNSServiceRef sd_ref;
  DNSServiceErrorType err;
  char *type;
  char *domain = "";
  char *command;
  switch (argc - argi)
    {
    case 2:
      type = argv[argi];
      command = argv[argi+1];
      break;
    case 3:
      type = argv[argi];
      domain = argv[argi+1];
      command = argv[argi+2];
      break;
    default:
      EXIT_USAGE;
    }
  DEBUG("type: %s domain: %s command: %s", type, domain, command);
  err = DNSServiceBrowse(&sd_ref, 0, kDNSServiceInterfaceIndexAny, type, domain,
			 (DNSServiceBrowseReply) reply_watch, (void*) command);
  if (err) {
    DEBUG("DNSServiceBrowse returned error: %i", err);
    return err == kDNSServiceErr_BadParam ? RC_BADINPUT : RC_OTHERERR;
  }
  return main_select(sd_ref, -1);
}

static void DNSSD_API reply_watch(DNSServiceRef sd_ref,
				  DNSServiceFlags flags,
				  uint32_t ifIndex,
				  DNSServiceErrorType err,
				  const char * name,
				  const char * type,
				  const char * domain,
				  void * context) {
  if (!err) {
    MORE_COMING = flags & kDNSServiceFlagsMoreComing;
    DEBUG("MORE_COMING = %i", MORE_COMING);
    if (!MORE_COMING) {
      const char *command = (char*) context;
      DEBUG("command = '%s'", command);
      RUN = 0 == system(command);
    }
  } else {
    DEBUG("reply_list invoked with error %i", err);
    RETURN_CODE = RC_OTHERERR;
    RUN = false;
  }
}

int main_select(DNSServiceRef sd_ref, long timeout) {
  DNSServiceErrorType err;
  struct timeval tv;
  fd_set fdset;
  int fd;
  int res;
  long timeout_left;
  while (RUN) {
    timeout_left = timeleft(timeout);
    DEBUG("timeout %li timeout_left %li", timeout, timeout_left);
    if (MORE_COMING && timeout_left <= 0) timeout_left = 10;
    if (timeout_left < 0) {
      RUN = false;
      continue;
    }
    fd = DNSServiceRefSockFD(sd_ref);
    if (fd == -1) {
      DEBUG("DNSServiceRefSockFD return -1");
      return RC_OTHERERR;
    }
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    tv.tv_sec = timeout_left / 1000;
    tv.tv_usec = (timeout_left - (tv.tv_sec * 1000)) * 1000;
    res = select(fd + 1, &fdset, (fd_set*)NULL, (fd_set*) NULL, &tv);
    if (res > 0) {
      err = DNSServiceProcessResult(sd_ref);
      if (err) {
	DEBUG("DNSServiceProcessResult returned error %i", err);
	RETURN_CODE = RC_OTHERERR;
	RUN = false;
      }
    } else if (res != 0) {
      DEBUG("select() returned %d errno %d %s\n", res, errno, strerror(errno));
      if (errno != EINTR) {
	RETURN_CODE = RC_OTHERERR;
	RUN = false;
      }
    }
  }
  return RETURN_CODE;
}

long timeleft(long timeout) {
  long elapsed;
  struct timeval now;
  if (timeout == -1) return 60000;
  gettimeofday(&now, NULL);
  elapsed = (now.tv_sec-START_TV.tv_sec) * 1000;
  elapsed += (now.tv_usec-START_TV.tv_usec) / 1000;
  return timeout -  elapsed;
}
