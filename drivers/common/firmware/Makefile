MODULE_NAME = firmware
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-firmware
endif
endif
obj-m = $(MODULE_NAME).o
$(MODULE_NAME)-y += firmware_drv.o
$(MODULE_NAME)-y += firmware_type.o
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	$(MODULE_NAME)-y += ../media_utils/media_utils_dummy.o
ifeq (${CONFIG_AMLOGIC_V4L2},y)
	$(MODULE_NAME)-y += ../media_utils/vendor_hooks.o
endif
else
	$(MODULE_NAME)-y += ../media_utils/media_utils.o
endif
endif
PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)= \"[$(PR_FMT)]: \" fmt"
ccflags-y += $(PR_FMT_DEFINE)
