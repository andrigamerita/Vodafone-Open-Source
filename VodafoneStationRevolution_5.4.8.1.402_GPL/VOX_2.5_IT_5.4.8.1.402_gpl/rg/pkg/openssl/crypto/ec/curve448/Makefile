ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS=arch_32

JMK_INTERNAL_HEADERS=ed448.h point_448.h curve448_local.h word.h field.h \
  curve448utils.h

JMK_CFLAGS+=-I$(JMK_ROOT)/pkg/openssl/crypto/ec/curve448/arch_32/
JMK_LOCAL_CFLAGS+=-I$(JMK_ROOT)/pkg/openssl/crypto/ec/curve448/arch_32/
JMK_O_OBJS=curve448.o curve448_tables.o eddsa.o f_generic.o scalar.o

$(call JMKE_INCLUDE_RULES)

