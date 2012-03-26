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
#include <dns_sd.h>

int main(int argc, char **argv);

int usage(char* self);

int main_list(int argi, int argc, char **argv);
static void DNSSD_API reply_list(DNSServiceRef sd_ref,
				 DNSServiceFlags flags,
				 uint32_t ifIndex,
				 DNSServiceErrorType err,
				 const char * name,
				 const char * type,
				 const char * domain,
				 void * context);

int main_lookup(int argi, int argc, char **argv);
static void DNSSD_API reply_lookup(DNSServiceRef sd_ref,
				   DNSServiceFlags flags,
				   uint32_t ifIndex,
				   DNSServiceErrorType err,
				   const char * fullname,
				   const char * hosttarget,
				   uint16_t port,
				   uint16_t txtLen,
				   const unsigned char * txtRecord,
				   void * context);

int main_watch(int argi, int argc, char **argv);
static void DNSSD_API reply_watch(DNSServiceRef sd_ref,
				  DNSServiceFlags flags,
				  uint32_t ifIndex,
				  DNSServiceErrorType err,
				  const char * name,
				  const char * type,
				  const char * domain,
				  void * context);

int main_select(DNSServiceRef sd_ref, long timeout);

long timeleft(long timeout);
