ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS=include crypto $(if $(CONFIG_RG_OPENSSL),ssl)

JMK_EXPORT_HEADERS+=e_os.h
JMK_CD_EXPORTED_FILES=LICENSE

# go throu all directories just to export header files.
JMK_ARCHCONFIG_SUBDIRS=crypto ssl include

$(call JMKE_INCLUDE_RULES)

