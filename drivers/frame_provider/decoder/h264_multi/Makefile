MODULE_NAME = amvdec_mh264
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-amvdec-mh264
endif
endif
obj-$(CONFIG_AMLOGIC_MEDIA_VDEC_H264_MULTI) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += vmh264.o h264_dpb.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
