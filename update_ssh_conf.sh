#!/usr/bin/env bash
#
#  Copyright (c) 2012 Andrew Tunnell-Jones
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.
#

set -e
DNSSD="$(dirname $0)/dnssd"
VALID_ARGS=true
WATCH=false

# check args
if [ $# -ne 1 ] && [ $# -ne 2 ]; then VALID_ARGS=false; fi
if [ ! -f "${1}" ] || [ ! -w "${1}" ]; then
    if [ $VALID_ARGS == true ]; then
	echo "${1} does not exist or is not writable" 1>&2
    fi
    VALID_ARGS=false
fi
if [ $# -eq 2 ]; then
    if [ "-w" == "${2}" ]; then
	WATCH=true
    else
	VALID_ARGS=false
    fi
fi
if [ $VALID_ARGS == false ]; then
    echo "Usage: ${0} ssh_config_file [-w]" 1>&2
    exit 1
fi

# watch if requested
if [ $WATCH == true ]; then
    echo "Watching for changes"
    "${DNSSD}" watch _ssh._tcp "'${0}' '${1}'"
    exit $?
fi
# generate config
TARGET="${1}"
TMP="${TARGET}.tmp"
echo "Updating ${TARGET}"
BANNER="### DO NOT EDIT BELOW - SCRIPT GENERATED CONTENT ###"
CUT=$(grep -m 1 -n "${BANNER}" "${TARGET}" | cut -d ":" -f 1)
if [ "${CUT}" == "" ]; then
    cp "${TARGET}" "${TMP}"
    echo "${BANNER}" >> "${TMP}"
else
    head "-n${CUT}" "${TARGET}" >> "${TMP}"
fi
(
    echo "# Generated $(DATE) by"
    echo "# $(PWD)/$(basename ${0})"
    echo " "
) >> "${TMP}"
${DNSSD} list _ssh._tcp 2 | while read -r INSTANCE; do
    DOMAIN=$(echo "${INSTANCE}"|awk '{print $1}')
    NAME=$(echo "${INSTANCE}"|sed "s/${DOMAIN}\ //g")
    HOSTPORT=$("${DNSSD}" lookup "${NAME}" _ssh._tcp "${DOMAIN}" 2)
    if [ "$HOSTPORT" == "" ]; then
	echo "Failed to lookup ${NAME} in ${DOMAIN}" 1>&2
	continue
    fi
    HOST=$(echo "${HOSTPORT}"|awk '{print $1}')
    PORT=$(echo "${HOSTPORT}"|awk '{print $2}')
    ALIAS=$(echo "${HOST}"| sed -e 's/\.$//g' \
	-e 's/\.[0-9][0-9]*\.members\.btmm\.icloud\.com$/.icloud/g')
    (
	echo "Host ${ALIAS}"
	echo "    HostName ${HOST}"
	echo "    Port ${PORT}"
	echo " "
    )  >> "${TMP}"
done
mv "${TMP}" "${TARGET}"