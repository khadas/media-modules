MODULE_NAME = amvdec_mmpeg4
ifeq (${VERSION},5)
ifeq (${PATCHLEVEL},15)
	MODULE_NAME = amlogic-amvdec-mmpeg4
endif
endif
obj-$(CONFIG_AMLOGIC_MEDIA_VDEC_MPEG4_MULTI) += ${MODULE_NAME}.o
${MODULE_NAME}-objs += vmpeg4_multi.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
