#!/bin/bash

MODE0=0755
MODE1=0666
OJSUSER=ojs

OUT() {
	echo "$@"
}
ERR() {
	>&2 echo "$@"
}

cleanup() {
	for i in /tmp/ojs-*; do
		for j in $i/dev/* $i/usr/* $i/etc/*; do
			umount "$j"
		done
		for j in $i/*; do
			umount "$j"
		done
		umount "$i"
		rm -rf "$i"
	done
	return 0
}


id -u $OJSUSER &>/dev/null
if [ $? -eq 1 ]; then
  useradd -s /usr/sbin/nologin -r -M -d /dev/null $OJSUSER
fi

if [ $UID -ne 0 ]; then
	ERR "This program requires root"
	exit -1
fi

if [ "$1" = "cleanup" ]; then
	cleanup
	exit $?
fi

tmpTemplate="/tmp/ojs-XXXXXX"
tmpDir=$(mktemp -d "$tmpTemplate")
if [ $? -ne 0 ]; then
	ERR "Create temporary directory failed"
	exit -1
fi
chmod $MODE0 "$tmpDir"
cd "$tmpDir"

mkdir -p proc dev tmp
chmod $MODE0 proc dev tmp

mount -t proc -o nosuid proc proc
mknod dev/null c 1 3
mknod dev/urandom c 1 9
chmod $MODE1 dev/null dev/urandom
mount -o bind /dev/null dev/null
mount -o bind /dev/urandom dev/urandom
mount -o size=16m,nr_inodes=4k,nosuid -t tmpfs tmp tmp

for i in bin etc/alternatives lib lib64 usr/bin usr/include usr/lib usr/share #usr/lib64 usr/libexec
do
	mkdir -p "$i"
	chmod $MODE0 "$i"
	mount -o ro,nosuid,bind "/$i" "$i"
done

echo -n "grant { };" > etc/java.policy
OUT "The tmp dir: ${tmpDir}"


