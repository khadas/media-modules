MODULE_NAME = amvdec_avs2_v4l
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-amvdec-avs2_v4l
endif
endif
obj-$(CONFIG_AMLOGIC_MEDIA_VDEC_AVS2) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += vavs2.o avs2_bufmgr.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
