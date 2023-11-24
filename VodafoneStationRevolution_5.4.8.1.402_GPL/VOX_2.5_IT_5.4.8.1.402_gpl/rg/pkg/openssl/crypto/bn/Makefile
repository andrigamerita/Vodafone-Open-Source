ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_WARN2ERR=n

JMK_INTERNAL_HEADERS=bn_local.h

JMK_SUBDIRS=asm

JMK_O_OBJS=bn_add.o bn_blind.o bn_const.o bn_ctx.o bn_depr.o bn_dh.o bn_div.o \
  bn_err.o bn_exp.o bn_exp2.o bn_gcd.o bn_gf2m.o bn_intern.o bn_kron.o \
  bn_lib.o bn_mod.o bn_mont.o bn_mpi.o bn_mul.o bn_nist.o bn_prime.o \
  bn_print.o bn_rand.o bn_recp.o bn_shift.o bn_sqr.o bn_sqrt.o bn_srp.o \
  bn_word.o bn_x931p.o

JMK_CD_EXPORTED_FILES+=bn_prime.pl

JMK_ARCHCONFIG_JPKG_FIRST_TASKS+=$(JMKE_PWD_BUILD)/bn_prime.h

$(JMKE_PWD_BUILD)/bn_prime.h: $(JMKE_PWD_SRC)/bn_prime.pl
	$(PERL) $(JMKE_DEPS) > $@

$(call JMKE_INCLUDE_RULES)
