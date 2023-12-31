#!/bin/sh

export LC_ALL=POSIX
export LC_CTYPE=POSIX

if [ "$JMKE_RAMDISK_RO_DIR" = "" ]; then
    echo "No installation directory for read only files, aborting."
    exit 1;
fi
if [ "$JMKE_RAMDISK_RW_DIR" = "" ]; then
    echo "No installation directory for read/write files, aborting."
    exit 1;
fi
if [ "$2" = "--hardlinks" ]; then
    linkopts="-f"
else
    linkopts="-fs"
fi
h=`sort busybox.links | uniq`

mkdir -p $JMKE_RAMDISK_RO_DIR/bin || exit 1
mkdir -p $JMKE_RAMDISK_RW_DIR/bin || exit 1

for i in $h ; do
	appdir=`dirname $i`
	mkdir -p $JMKE_RAMDISK_RW_DIR/$appdir || exit 1
	ln $linkopts $JMKE_MAINFS_RUNTIME_MOUNT_POINT/bin/busybox \
	  $JMKE_RAMDISK_RW_DIR/$i || exit 1
done

exit 0
