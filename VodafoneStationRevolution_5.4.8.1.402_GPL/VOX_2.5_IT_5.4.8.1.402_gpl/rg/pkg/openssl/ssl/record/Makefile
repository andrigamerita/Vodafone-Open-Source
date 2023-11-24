ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_INTERNAL_HEADERS=record.h record_local.h

JMK_O_TARGET=ssl_record.o
JMK_LOCAL_O_TARGET=local_ssl_record.o
JMK_O_OBJS=dtls1_bitmap.o rec_layer_d1.o rec_layer_s3.o ssl3_buffer.o \
  ssl3_record.o ssl3_record_tls13.o

$(call JMKE_INCLUDE_RULES)
