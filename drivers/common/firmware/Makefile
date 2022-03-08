MODULE_NAME = amlogic-firmware
obj-m = $(MODULE_NAME).o
$(MODULE_NAME)-y += firmware_drv.o
$(MODULE_NAME)-y += firmware_type.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)= \"[$(PR_FMT)]: \" fmt"
ccflags-y += $(PR_FMT_DEFINE)