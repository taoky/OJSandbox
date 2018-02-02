#!/bin/bash

MODE0=0755
MODE1=0666
MODE2=0444
OJSUSER=ojs

OUT() {
	echo "$@"
}
ERR() {
	>&2 echo "$@"
}

cleanup() {
	for i in /tmp/ojs-*; do
		rm -rf "$i"
	done
	return 0
}

if [ $UID -ne 0 ]; then
	ERR "This program requires root"
	exit -1
fi

if [ "$1" = "cleanup" ]; then
	cleanup
	exit $?
fi

# id -u $OJSUSER &>/dev/null
# if [ $? -eq 1 ]; then
#   useradd -s /usr/sbin/nologin -r -M -d /dev/null $OJSUSER
# fi

tmpTemplate="/tmp/ojs-XXXXXX"
tmpDir=$(mktemp -d "$tmpTemplate")
if [ $? -ne 0 ]; then
	ERR "Create temporary directory failed"
	exit -1
fi
chmod $MODE0 "$tmpDir"
cd "$tmpDir"

echo -n "grant { };" > /etc/java.policy
chmod $MODE2 /etc/java.policy
OUT "The tmp dir: ${tmpDir}"


