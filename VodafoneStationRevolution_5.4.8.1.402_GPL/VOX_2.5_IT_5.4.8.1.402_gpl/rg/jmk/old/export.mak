# Autogenerated sources and links should not be exported nor their objects
JMK_DONT_EXPORT+=$(foreach f,$(JMK_LINKS) $(JMK_AUTOGEN_SRC),$f $(f:%.c=%.o) \
  $(f:%.c=local_%.o) $(f:%.c=%_pic.o) $(f:%.S=%.o) $(f:%.S=local_%.o) \
  $(f:%.S=%_pic.o))

# This functions handles files in the format of <src>__<target>

# Escape/unescape unusual file names with [set of] special symbols:
# 1) Avoid __ that is used for <src>__<target> separation
# n) More escaping rules go here.
JMKF_ESCAPE_FILE_NAME=$(subst _,\_,$1)
JMKF_UNESCAPE_FILE_NAME=$(subst \_,_,$1)

# $1 - files in the <src>__<target> format
# $2 - 1 - return the src, 2 return the target
IS_FROM_TO=$(findstring __,$1)
GET_FILE_FROM_TO=$(foreach f,$1,$(call JMKF_UNESCAPE_FILE_NAME,$(if $(call IS_FROM_TO,$f),$(word $2,$(subst __, ,$f)),$f)))
GET_FILE_FROM=$(call GET_FILE_FROM_TO,$1,1)
GET_FILE_TO=$(call GET_FILE_FROM_TO,$1,2)

# We create the target directory because it might not exist yet.
# We check whether to create the link to the SRC or the BUILD dir, usually the
# target of the link is in the source directory, but autogenerated files are in
# BUILD directory.
EXPORT_FILE=$(MKDIR) $(dir $2) && \
  $(JMKE_LN) $(if $(wildcard $(JMKE_PWD_SRC)/$1),$(JMKE_PWD_SRC),$(CUR_DIR))/$1 $2

EXPORT_FILE_CP=$(MKDIR) $(dir $2) && \
  cp -fs $(if $(wildcard $(JMKE_PWD_SRC)/$1),$(JMKE_PWD_SRC),$(CUR_DIR))/$1 $2

export_headers:
	$(foreach f,$(JMK_EXPORT_HEADERS),\
	$(call EXPORT_FILE,$(call GET_FILE_FROM,$f),$(RG_INCLUDE_DIR)/$(JMK_EXPORT_HEADERS_DIR)/$(call GET_FILE_TO,$f)) &&) true 

export_headers_flat:
	$(foreach f,$(JMK_EXPORT_HEADERS_FLAT),\
	$(call EXPORT_FILE_CP,$(call GET_FILE_FROM,$f),$(RG_INCLUDE_DIR)/$(JMK_EXPORT_HEADERS_DIR)/) &&) true 

export_rg_linux_headers:
	$(foreach f,$(JMK_EXPORT_RG_LINUX_HEADERS),\
	$(call EXPORT_FILE,$(call GET_FILE_FROM,$f),$(RG_INCLUDE_RG_LINUX_DIR)/$(JMK_EXPORT_HEADERS_DIR)/$(call GET_FILE_TO,$f)) &&) true

export_libs:
ifdef JMK_EXPORT_LIBS
	$(foreach f,$(JMK_EXPORT_LIBS),$(call EXPORT_FILE,$f,$(JMKE_BUILDDIR)/pkg/lib/$f) &&) true
endif

ln_internal_headers:
	$(RG_SHELL_FUNCS) && \
	  $(foreach f,\
	  $(strip $(JMK_INTERNAL_HEADERS) $(JMK_CD_EXPORTED_FILES) \
	     $(JMK_JS_FILES) $(JMK_CSS_FILES) $(JMK_SQL_FILES) \
	     $(foreach eh,$(JMK_EXPORT_HEADERS),$(call GET_FILE_FROM,$(eh)))),\
	  $(if $(MAKEDEBUG),echo ln_internal_headers: copying $f &&,)\
	  DO_LINK=1 rg_vpath_cp $f $(JMKE_PWD_BUILD)/$f &&) \
	true

