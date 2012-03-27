#dnssd shell tool

**License: MIT**

### Overview

#### Building

`make all` on Mac OS X. For other platforms the `Makefile` will need to be
adjusted to find `dns_sd.h` and to link against `dns_sd` (patches welcome).

#### Usage

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

##### Operations

###### list

Browses for services until timeout has elapsed. For each discovered service
instance `domain servicename` will be printed to standard out. If a service
instance is removed no action is taken so scripts must be prepared to deal with
a service instance failing to resolve. Services may also be listed more than
once (patch welcome).

###### lookup

Lookup a specific service instance (details should come from `list` operation)
and print `host port txtKey=txtValue ...` to standard out if the service can be
resolved.

###### watch

Executes `exec` whenever a service instance is added or removed. If `exec`
returns anything other than 0 `dnssd` will exit.

##### Arguments

* `type` should be a valid service type, eg: `_ssh._tcp`
* `domain` should be a valid discovery domain or `""` for all domains
* `timeout` should be a positive integer

### Example use

A shell script called `update_ssh_conf.sh` which is intended to update
`~/.ssh/config` with active SSH instances is bundled. The script demonstrates
all 3 operations and is invoked as `./update_ssh_conf path-to-ssh-conf` for a
once-off update or as `./update_ssh_conf path-to-ssh-conf -w` to continuously
watch for service changes. A launchd agent (`com.dnssd.update_ssh_conf.plist`)
is also included which once updated with the correct paths, placed in
`~/Library/LaunchAgents/` and loaded, will run `update_ssh_conf.sh` in watch
mode (and from then, whenever you're logged in).