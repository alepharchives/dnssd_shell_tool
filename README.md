#dnssd shell tool

**License: MIT**

Usage:

```
$ ./dnssd
usage: ./dnssd [-D] list type
       ./dnssd [-D] list type domain
       ./dnssd [-D] list type timeout
       ./dnssd [-D] list type domain timeout
       ./dnssd [-D] lookup name type domain
       ./dnssd [-D] lookup name type domain timeout
       ./dnssd [-D] watch type exec
       ./dnssd [-D] watch type domain exec

 * timeout is specified in seconds
 * results may expire by the time your script consumes them
 * returns 1 on (detectably) bad input and 2 for other errors
 ```

The included `update_ssh_conf.sh` (for updating `~/.ssh/config` with DNSSD
advertised SSH instances) might be illustrative.