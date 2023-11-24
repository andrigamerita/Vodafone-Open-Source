ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_TARGET=usb_modeswitch
JMK_O_OBJS_$(JMK_TARGET)+=usb_modeswitch.o
JMK_LDFLAGS_$(JMK_TARGET)+=-lusb-1.0 -lpthread
JMK_INTERNAL_HEADERS+=usb_modeswitch.h
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

$(call JMKE_INCLUDE_RULES)
