MODULE_NAME = amvdec_h265
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-amvdec-h265
endif
endif
obj-$(CONFIG_AMLOGIC_MEDIA_VDEC_H265) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += vh265.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
