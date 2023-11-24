ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_WARN2ERR=n

JMK_INTERNAL_HEADERS=statem.h statem_local.h

JMK_O_TARGET=ssl_statem.o
JMK_LOCAL_O_TARGET=local_ssl_statem.o
JMK_O_OBJS=extensions.o extensions_clnt.o statem_lib.o statem_srvr.o \
  extensions_cust.o extensions_srvr.o statem.o statem_clnt.o statem_dtls.o

$(call JMKE_INCLUDE_RULES)
