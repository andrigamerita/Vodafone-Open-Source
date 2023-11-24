ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS=curve448

JMK_INTERNAL_HEADERS=ec_local.h

JMK_O_OBJS=curve25519.o ec2_oct.o ec2_smpl.o ec_ameth.o ec_asn1.o ec_check.o \
  ec_curve.o ec_cvt.o ec_err.o ec_key.o ec_kmeth.o ec_lib.o ec_mult.o ec_oct.o \
  ec_pmeth.o ec_print.o ecdh_kdf.o ecdh_ossl.o ecdsa_ossl.o ecdsa_sign.o \
  ecdsa_vrf.o eck_prn.o ecp_mont.o ecp_nist.o ecp_nistp224.o ecp_nistp256.o \
  ecp_nistp521.o ecp_nistputil.o ecp_oct.o ecp_smpl.o ecx_meth.o 

$(call JMKE_INCLUDE_RULES)

