ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

LIBGMP_BUILD_DIR=$(JMKE_BUILDDIR)/pkg/gmp/
LIBGMP=$(LIBGMP_BUILD_DIR)libgmp.so.10.4.0

ifdef CONFIG_DYN_LINK
  JMK_OTHER_TARGETS+=$(LIBGMP)
  JMK_RAMDISK_LIB_FILES+=$(LIBGMP)
endif

JMK_EXPORT_HEADERS_DIR=gmp
JMK_EXPORT_HEADERS+=gmp.h

CONFIG_FLAGS = --host=$(TARGET_MACHINE) --target=$(TARGET_MACHINE)
ifeq ($(ARCH),arm)
  CONFIG_FLAGS = --host=arm-linux --target=arm-linux --build=i386-linux
endif
ifeq ($(ARCH),mips)
  ifdef CONFIG_64BIT
    CONFIG_FLAGS = --host=mips64-unknown-linux --target=mips64-unknown-linux --build=i386-linux
  else
    CONFIG_FLAGS = --host=mips-linux --target=mips-linux --build=i386-linux
  endif
endif
ifeq ($(ARCH),i386)
  CONFIG_FLAGS = --host=i386-linux --target=i386-linux --build=i386-linux
endif
ifeq ($(ARCH),um)
  CONFIG_FLAGS = --host=i386-linux --target=i386-linux --build=i386-linux
endif
ifeq ($(ARCH),ppc)
  CONFIG_FLAGS = --host=powerpc-linux --target=powerpc-linux --build=powerpc-linux
endif

$(LIBGMP): .configure
	$(JMKE_LN) $(LIBGMP_BUILD_DIR).libs/libgmp.so.10.4.0 \
	  $(LIBGMP_BUILD_DIR)libgmp.so.10.4.0
	$(JMKE_LN) $(LIBGMP_BUILD_DIR).libs/libgmp.a \
	  $(LIBGMP_BUILD_DIR)libgmp.a
	if test -f $(LIBGMP_BUILD_DIR)Makefile; then \
	$(MAKE) -C $(LIBGMP_BUILD_DIR) -f Makefile;\
	fi

.configure: 
ifndef DIST
	@echo 'error - give param DIST' && false
endif	
	$(JMKE_LN) $(LIBGMP_BUILD_DIR)gmp.h \
	  $(JMKE_BUILDDIR)/pkg/include/gmp/gmp.h
	@cd $(LIBGMP_BUILD_DIR) && \
	./configure $(CONFIG_FLAGS) CFLAGS="$(CFLAGS_ENVIR) -std=gnu99" \
	  LDFLAGS="$(LDFLAGS_ENVIR)" AR=$(AR) RANLIB=$(RANLIB) && \
	touch .configure

$(call JMKE_INCLUDE_RULES)
