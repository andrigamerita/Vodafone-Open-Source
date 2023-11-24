ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_INTERNAL_HEADERS=arch_intrinsics.h f_impl.h

JMK_CFLAGS+=-I$(JMK_ROOT)/pkg/openssl/crypto/ec/curve448/
JMK_LOCAL_CFLAGS+=-I$(JMK_ROOT)/pkg/openssl/crypto/ec/curve448/
JMK_O_OBJS=f_impl.o

$(call JMKE_INCLUDE_RULES)

