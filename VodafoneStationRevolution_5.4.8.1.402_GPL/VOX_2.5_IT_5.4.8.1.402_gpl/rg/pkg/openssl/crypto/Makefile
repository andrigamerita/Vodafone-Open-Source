ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

COMMON_SUBDIRS+=$(if $(CONFIG_RG_OPENSSL_MD5),md5)
COMMON_SUBDIRS+=$(if $(CONFIG_RG_OPENSSL_SHA),sha)
COMMON_SUBDIRS+=$(if $(CONFIG_RG_OPENSSL_DES),des)
COMMON_SUBDIRS+=$(if $(CONFIG_RG_OPENSSL_MD4),md4)

CRYPTO_LIB_SUBDIRS=aes asn1 async $(if $(CONFIG_RG_OPENSSL_BLOWFISH),bf) \
  bio bn buffer $(if $(CONFIG_RG_OPENSSL_CAST),cast) cmac cms \
  $(if $(CONFIG_RG_OPENSSL_COMPRESSION),comp) conf ct dh dsa dso \
  $(if $(CONFIG_RG_OPENSSL_ECDH),ec) err evp hmac kdf lhash modes objects ocsp \
  pem pkcs12 pkcs7 rand rc2 rc4 ripemd rsa stack store ts txt_db ui x509 x509v3

# all directories are entered just to export their headers
JMK_ARCHCONFIG_SUBDIRS=$(COMMON_SUBDIRS) $(CRYPTO_LIB_SUBDIRS)

JMK_A_TARGET:=$(notdir $(JMK_A_TARGET))
JMK_LOCAL_A_TARGET:=$(notdir $(JMK_LOCAL_A_TARGET))

ifdef CONFIG_RG_CRYPTO
ifeq ($(CONFIG_MIPS), y)
  JMK_O_OBJS_$(JMK_A_TARGET)+=bn/asm/mips.o
else
  JMK_O_OBJS_$(JMK_A_TARGET)+=bn/bn_asm.o
endif

JMK_O_OBJS_$(JMK_LOCAL_A_TARGET)+=bn/bn_asm.o
endif

ifndef CONFIG_RG_CRYPTO
  JMK_SUBDIRS=$(COMMON_SUBDIRS)
  JMK_O_OBJS=crypto_stubs.o mem_clr.o
else
  JMK_SUBDIRS=$(COMMON_SUBDIRS) $(CRYPTO_LIB_SUBDIRS)
  JMK_O_OBJS=cryptlib.o mem.o mem_clr.o mem_dbg.o cversion.o ex_data.o \
    cpt_err.o ebcdic.o uid.o o_time.o o_str.o o_dir.o o_fips.o o_init.o \
    getenv.o ctype.o threads_none.o o_fopen.o mem_sec.o init.o
endif

ifdef CONFIG_DYN_LINK
  JMK_SO_TARGET=$(JMK_A_TARGET:%.a=%.so)
  JMK_LOCAL_SO_TARGET=$(JMK_LOCAL_A_TARGET:%.a=%.so)
  JMK_RAMDISK_LIB_FILES=$(JMK_SO_TARGET)
  JMK_L_OBJS_$(JMK_SO_TARGET)=$(JMK_A_TARGET)
  JMK_L_OBJS_$(JMK_LOCAL_SO_TARGET)=$(JMK_LOCAL_A_TARGET)
  JMK_SO_CFLAGS=-Wl,--whole-archive
endif

JMK_INTERNAL_HEADERS=crypto_stubs.c LPdir_unix.c bn_int.h

JMK_EXPORT_LIBS=liblocal_crypto.a
ifdef CONFIG_DYN_LINK
  JMK_EXPORT_LIBS+=$(JMK_SO_TARGET)
else
  JMK_EXPORT_LIBS+=$(JMK_A_TARGET)
endif

JMK_ARCHCONFIG_JPKG_FIRST_TASKS+=$(JMKE_PWD_BUILD)/buildinf.h

$(JMKE_PWD_BUILD)/buildinf.h: $(JMKE_PWD_SRC)/../util/mkbuildinf.pl $(JMKE_PWD_SRC)/Makefile
	$(PERL) $< "$(CC) $(CFLAGS)" "$(PLATFORM)" > $@

$(call JMKE_INCLUDE_RULES)
