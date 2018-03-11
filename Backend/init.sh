#!/bin/bash

MODE0=0755
MODE1=0666
OJSUSER=ojs
RAMDISKSIZE=24

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
    rm -f "./config.sh"
	return 0
}


if ! id -u $OJSUSER &>/dev/null; then
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

# Load config before starting the main procedure

if [ -r "./config.sh" ]; then
	. ./config.sh
fi

tmpTemplate="/tmp/ojs-XXXXXX"
tmpDir=$(mktemp -d "$tmpTemplate")
if [ $? -ne 0 ]; then
	ERR "Create temporary directory failed"
	exit -1
fi
chmod $MODE0 "$tmpDir"
mount -t tmpfs -o size=8m,nr_inodes=4096 tmpfs "$tmpDir"
cd "$tmpDir"

mkdir -p proc dev tmp
chmod $MODE0 proc dev tmp
chmod 00777 tmp

mount -t proc -o nosuid proc proc
mknod dev/null c 1 3
mknod dev/urandom c 1 9
chmod $MODE1 dev/null dev/urandom
mount -o bind /dev/null dev/null
mount -o bind /dev/urandom dev/urandom
mount -o "size=${RAMDISKSIZE}m,nr_inodes=4k,nosuid" -t tmpfs tmpfs tmp

for i in bin etc/alternatives lib lib64 usr/bin usr/include usr/lib usr/share etc/java-9-openjdk #usr/lib64 usr/libexec
do
	mkdir -p "$i"
	chmod $MODE0 "$i"
	mount -o ro,nosuid,bind "/$i" "$i"
done

setcap cap_kill,cap_setuid,cap_setgid,cap_sys_resource,cap_sys_chroot+ep "${0%/*}/safeJudger"

echo -n "grant { };" > etc/java.policy
OUT "The tmp dir: ${tmpDir}"

