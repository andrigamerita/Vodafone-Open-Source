JMK_ROOT=../../..
include $(JMK_ROOT)/envir.mak

JMK_TARGET=mkntfs ntfsfix ntfslabel
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_O_OBJS_mkntfs=attrdef.o boot.o sd.o mkntfs.o utils.o
JMK_O_OBJS_ntfsfix=ntfsfix.o utils.o
JMK_O_OBJS_ntfslabel=ntfslabel.o utils.o

JMK_CFLAGS+=-DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 \
  -I.. -I../include/ntfs-3g -I../include/fuse-lite

JMK_LDFLAGS+=-lntfs-3g  $(OPENRG_GPL_LDLIBS)

include $(JMKE_MK)
