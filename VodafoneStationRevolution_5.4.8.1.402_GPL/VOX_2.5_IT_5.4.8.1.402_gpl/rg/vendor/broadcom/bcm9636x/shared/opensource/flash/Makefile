JMK_ROOT=../../../../../..
include $(JMK_ROOT)/envir.mak

ifeq ($(CONFIG_MIPS_BRCM),y)

# Linux

obj-y		+= flash_api.o flash_common.o

# By default, assume neitehr flash driver is in use
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=0

ifeq ($(strip $(BRCM_CHIP)),6368)
INC_CFI_FLASH_DRIVER=1
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),6816)
INC_CFI_FLASH_DRIVER=1
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),6818)
INC_CFI_FLASH_DRIVER=1
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),6362)
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),6328)
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),6318)
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),63268)
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=1
endif
ifeq ($(strip $(BRCM_CHIP)),6828)
INC_CFI_FLASH_DRIVER=0
INC_SPI_FLASH_DRIVER=1
endif

ifeq ($(strip $(INC_CFI_FLASH_DRIVER)),1)
obj-y		+= cfiflash.o
endif
ifeq ($(strip $(INC_SPI_FLASH_DRIVER)),1)
obj-y		+= spiflash.o
endif

EXTRA_CFLAGS += -DCONFIG_BCM9$(BRCM_CHIP) -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD) -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) -DINC_CFI_FLASH_DRIVER=$(INC_CFI_FLASH_DRIVER) -DINC_SPI_FLASH_DRIVER=$(INC_SPI_FLASH_DRIVER)

endif

JMK_MOD_TARGET+=bcmflash_mod.o
JMK_MOD_2_STAT+=bcmflash_mod.o
JMK_O_OBJS+=$(obj-y)
JMK_MOD_CFLAGS+=$(EXTRA_CFLAGS)

include $(JMKE_MK)

