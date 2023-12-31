OPENSSL_DIR=$(JMK_ROOT)/pkg/openssl

JMK_CFLAGS_REMOVE+=-Os

OPTIMIZE_FLAGS+=-O3
INCLUDES=-I$(OPENSSL_DIR) -I$(OPENSSL_DIR)/crypto -I$(OPENSSL_DIR)/crypto/asn1 \
	 -I$(OPENSSL_DIR)/crypto/evp -I$(OPENSSL_DIR)/crypto/modes \
	 -I$(OPENSSL_DIR)/include
JMK_CFLAGS+=$(INCLUDES) $(OPTIMIZE_FLAGS)
JMK_LOCAL_CFLAGS+=$(INCLUDES)

# libcrypto.so is generated from libcrypto.a - this bypasses the standard
# JMK_SO_TARGET rules, so the -fpic flag must be added manually 
JMK_CFLAGS+=-fpic

ifneq ($(CONFIG_RG_GPL)-$(CONFIG_RG_CRYPTO),y-)
  JMK_A_TARGET+=$(JMKE_BUILDDIR)/pkg/openssl/crypto/libcrypto.a
  JMK_LOCAL_A_TARGET+=$(JMKE_BUILDDIR)/pkg/openssl/crypto/liblocal_crypto.a
endif

