MODULE_NAME = amvdec_mmpeg12_v4l
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-amvdec-mmpeg12_v4l
endif
endif
obj-$(CONFIG_AMLOGIC_MEDIA_VDEC_MPEG2_MULTI) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += vmpeg12_multi.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
