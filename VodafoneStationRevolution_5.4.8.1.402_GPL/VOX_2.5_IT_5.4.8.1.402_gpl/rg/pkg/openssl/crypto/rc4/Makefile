ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_INTERNAL_HEADERS=rc4_local.h

JMK_O_OBJS=rc4_enc.o rc4_skey.o

ifdef CONFIG_RG_PPP_MPPE
  SRC=rc4_enc.c rc4_skey.c rc4_local.h
  JMK_CD_EXPORTED_FILES+=$(SRC)
endif

$(call JMKE_INCLUDE_RULES)
