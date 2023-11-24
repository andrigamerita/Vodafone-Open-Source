ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_WARN2ERR=n

JMK_SUBDIRS=record statem

JMK_O_OBJS=bio_ssl.o d1_lib.o d1_msg.o d1_srtp.o methods.o packet.o pqueue.o \
  s3_cbc.o s3_enc.o s3_lib.o s3_msg.o ssl_asn1.o ssl_cert.o ssl_ciph.o \
  ssl_conf.o ssl_err.o ssl_init.o ssl_lib.o ssl_mcnf.o ssl_rsa.o ssl_sess.o \
  ssl_stat.o ssl_txt.o ssl_utst.o t1_enc.o t1_lib.o t1_trce.o tls13_enc.o \
  tls_srp.o record/ssl_record.o statem/ssl_statem.o

JMK_INTERNAL_HEADERS=packet_local.h ssl_cert_table.h ssl_local.h

ifdef CONFIG_DYN_LINK
  JMK_SO_TARGET=libssl.so
  JMK_EXPORT_LIBS=$(JMK_SO_TARGET)
  JMK_RAMDISK_LIB_FILES+=$(JMK_SO_TARGET)
else
  JMK_A_TARGET=libssl.a
  JMK_EXPORT_LIBS=$(JMK_A_TARGET)
endif
JMK_LOCAL_A_TARGET=liblocal_ssl.a
JMK_EXPORT_LIBS+=$(JMK_LOCAL_A_TARGET)

$(call JMKE_INCLUDE_RULES)
