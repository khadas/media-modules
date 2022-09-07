MODULE_NAME = amvdec_av1_v4l
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-amvdec-av1_v4l
endif
endif
obj-$(CONFIG_AMLOGIC_MEDIA_VDEC_AV1) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += vav1.o av1_bufmgr.o av1_film_grain.o

PR_FMT = "av1"
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
