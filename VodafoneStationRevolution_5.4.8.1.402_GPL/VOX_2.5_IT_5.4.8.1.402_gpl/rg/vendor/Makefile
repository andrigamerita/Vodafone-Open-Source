ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS+=$(if $(CONFIG_CONEXANT_COMMON),conexant)
JMK_SUBDIRS+=$(if $(CONFIG_MINDSPEED_COMMON),mindspeed)
JMK_SUBDIRS+=$(if $(CONFIG_ZOOM_CM),zoom)
JMK_SUBDIRS+=$(if $(CONFIG_RG_VOATM_ELCP),tdsoft)
JMK_SUBDIRS+=$(if $(CONFIG_KINPO_COMMON),kinpo)
JMK_SUBDIRS+=$(if $(CONFIG_ARCH_SN8947),samsung)
JMK_SUBDIRS+=$(if $(CONFIG_MIPS_BRCM)$(CONFIG_BROADCOM_9636X_3X),broadcom)
JMK_SUBDIRS+=$(if $(CONFIG_SIBYTE_SWARM),broadcom/bcm91125e)
JMK_SUBDIRS+=$(if $(CONFIG_SIBYTE_SENTOSA),broadcom/bcm91125e)
JMK_SUBDIRS+=$(if $(CONFIG_SIBYTE_COLORADO),broadcom/bcm91125e)
JMK_SUBDIRS+=$(if $(CONFIG_SL2312_COMMON),storlink)
JMK_SUBDIRS+=$(if $(CONFIG_INTERSIL_COMMON),intersil)
JMK_SUBDIRS+=$(if $(CONFIG_ZCOM_XG1000),zcom)
JMK_SUBDIRS+=$(if $(CONFIG_VINETIC),infineon/vinetic)
JMK_SUBDIRS+=$(if $(CONFIG_RG_DEV_IF_AR8316_HW_SWITCH)$(CONFIG_RG_ATHEROS),atheros)
JMK_SUBDIRS+=$(if $(CONFIG_RG_VOIP_RV),radvision)
JMK_JPKG_EXPORTED_DIR_SRC+=$(if $(CONFIG_RG_VOIP_RV),radvision)
JMK_LINK_DIRS+=$(if $(CONFIG_RG_VOIP_RV),radvision)
JMK_SUBDIRS+=$(if $(CONFIG_RG_UML),jungo)
JMK_SUBDIRS+=$(if $(CONFIG_RG_VSAF),cisco)
JMK_SUBDIRS+=$(if $(CONFIG_RALINK_RT2560),ralink)
JMK_SUBDIRS+=$(if $(CONFIG_RALINK_RT2561),ralink)
JMK_SUBDIRS+=$(if $(CONFIG_RALINK_RT2860),ralink)
JMK_SUBDIRS+=$(if $(CONFIG_RALINK_RT2880),ralink)
JMK_SUBDIRS+=$(if $(CONFIG_RALINK_RT3883),ralink)
JMK_SUBDIRS+=$(if $(CONFIG_AGN100),airgo)
JMK_SUBDIRS+=$(if $(CONFIG_MPC8272ADS)$(CONFIG_EP8248)$(CONFIG_MPC8349_ITX),freescale)
JMK_SUBDIRS+=$(if $(CONFIG_HW_ST_20190),st)
JMK_SUBDIRS+=$(if $(CONFIG_CPU_LX4189),lexra/lx4189)
JMK_SUBDIRS+=$(if $(CONFIG_ADI_6843),adi/ad6843)
JMK_SUBDIRS+=$(if $(CONFIG_ZSP400),lsi)
JMK_SUBDIRS+=$(if $(CONFIG_KS8695),micrel)
JMK_SUBDIRS+=$(if $(CONFIG_HW_SWITCH_BCM53XX),broadcom/bcm53xx)
JMK_SUBDIRS+=$(if $(CONFIG_RG_IKANOS_VDSLD)$(CONFIG_FUSIV_VX160)$(CONFIG_FUSIV_VX180),ikanos)
JMK_SUBDIRS+=$(if $(CONFIG_RG_DEV_IF_ADM6996_HW_SWITCH),infineon/adm6996)
JMK_SUBDIRS+=$(if $(CONFIG_RG_DEV_IF_PSB6973_HW_SWITCH),infineon/psb6973)
JMK_SUBDIRS+=$(if $(CONFIG_DANUBE),infineon/danube)
JMK_SUBDIRS+=$(if $(CONFIG_IFX_ATM_OAM),infineon/oam)
JMK_SUBDIRS+=$(if $(CONFIG_RG_VINAX_VDSL),infineon/vinax)
JMK_SUBDIRS+=$(if $(CONFIG_P400_REF),centillium)
JMK_SUBDIRS+=$(if $(CONFIG_CAVIUM_OCTEON),cavium/octeon)
JMK_SUBDIRS+=$(if $(CONFIG_ARCH_AVALANCHE),ti/avalanche)
JMK_SUBDIRS+=$(if $(CONFIG_TNETC550W_CM),ti/tnetc550w)
JMK_SUBDIRS+=$(if $(CONFIG_TI_DOCSIS),ti/docsis_30)
JMK_SUBDIRS+=$(if $(CONFIG_TI_DOCSIS),ti/rg)
JMK_SUBDIRS+=$(if $(CONFIG_MACH_PUMA5_MVG3420N),mototech/mvg3420n)
JMK_SUBDIRS+=$(if $(CONFIG_RG_PROSYST_MBS),prosyst/mbs_prof)
JMK_SUBDIRS+=$(if $(CONFIG_VTECH_DECT),vtech/dect)
JMK_SUBDIRS+=$(if $(CONFIG_RG_HW_IAD303),vtech)
JMK_SUBDIRS+=$(if $(CONFIG_HITRON_BOARD),hitron)
JMK_SUBDIRS+=$(if $(CONFIG_LANTIQ_XWAY),lantiq/xway)
JMK_SUBDIRS+=$(if $(CONFIG_HW_80211N_LANTIQ_WAVE300),lantiq/wave300)
JMK_SUBDIRS+=$(if $(CONFIG_LANTIQ_IPACX),lantiq/ipac-x/rg/modules)
JMK_SUBDIRS+=$(if $(CONFIG_LANTIQ_TSMINT),lantiq/t-smint/rg/modules)
JMK_SUBDIRS+=$(if $(CONFIG_RG_VODAFONE)$(CONFIG_RG_VODAFONE_LCD_LOGO),vodafone)
JMK_SUBDIRS+=$(if $(CONFIG_HW_YWZ00B)$(CONFIG_HW_ADA00X)$(CONFIG_HW_ADA00X_INT)$(CONFIG_HW_AD900X)$(CONFIG_HW_BCM963168_AUQ00X),sercomm)
JMK_SUBDIRS+=$(if $(CONFIG_HW_HG558)$(CONFIG_HW_BCM963168_HUAWEI),huawei)
JMK_SUBDIRS+=$(if $(CONFIG_HUAWEI_CDC_ETHER),huawei/modules/cdc_ether)
JMK_SUBDIRS+=$(if $(CONFIG_RG_HW_SWITCH_RTL836X),realtek/rtl836x)
JMK_SUBDIRS+=$(if $(CONFIG_COLOGNECHIP_XHFC),colognechip/xhfc/rg/modules)
JMK_SUBDIRS+=$(if $(CONFIG_NETBRICKS_ISDN_STACK),netbricks/isdn-bricks)
JMK_SUBDIRS+=$(if $(CONFIG_NETBRICKS_ISDN_STACK),netbricks/isdnd)
JMK_SUBDIRS+=$(if $(CONFIG_HW_GCT_LTE_GDM7240),gct/lte)
JMK_SUBDIRS+=$(if $(CONFIG_HW_QUANTA_LTE_1KG),quanta/lte)
JMK_SUBDIRS+=$(if $(CONFIG_HW_A000J)$(CONFIG_HW_C000J), arcadyan)
JMK_SUBDIRS+=$(if $(CONFIG_RG_EMAZE_HTTP_PROXY),emaze)
# Workaround to fix JPKG build, config is used for VOX1.5
#JMK_SUBDIRS+=$(if $(CONFIG_FB_HIMAX_LCD), himax)
JMK_SUBDIRS+=$(if $(CONFIG_HW_BCM963168_ADB), adb/vox25_it)
JMK_SUBDIRS+=$(if $(CONFIG_BCM63XX),linux/bcm63xx)

ifdef JMKE_DOING_MAKE_CONFIG
  # JMK_ARCHCONFIG_FIRST_TASKS is too late for us
  # We need the Makefile to exist before the JMK mechanism enters the SUBDIRS
  vendor_git_sub_to_init:=\
    $(shell git submodule status $(JMK_ROOT)/vendor | grep ^- | cut -d' ' -f2)

  vendor_subdirs:=$(foreach d,$(JMK_SUBDIRS),$(word 1,$(subst /, ,$d)))
  vendor_git_sub_needed:=$(filter $(addprefix vendor/,$(vendor_subdirs)), \
    $(vendor_git_sub_to_init))

  $(if $(vendor_git_sub_needed), \
    $(info Init git submodules: $(vendor_git_sub_needed)) \
    $(shell for m in $(vendor_git_sub_needed); \
      do git submodule update --init $$m ; done > /dev/null))
endif

$(call JMKE_INCLUDE_RULES)
