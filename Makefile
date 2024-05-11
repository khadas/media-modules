mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
MEDIA_MODULE_PATH := $(dir $(mkfile_path))
VERSION_CONTROL_CFLAGS := $(shell ${MEDIA_MODULE_PATH}/version_control.sh)

ifeq (${wildcard ${PRODUCT_FULL_DIR}/media_modules.build.config.trunk.mk},)
${info "media_modules use default config"}
CONFIGS := CONFIG_AMLOGIC_MEDIA_VDEC_MPEG2_MULTI=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_MPEG4_MULTI=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_VC1=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_H264_MULTI=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_H264_MVC=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_H265=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_VP9=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_MJPEG_MULTI=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AVS_MULTI=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AVS2=m \
	CONFIG_AMLOGIC_MEDIA_VENC_H264=m \
	CONFIG_AMLOGIC_MEDIA_VENC_H265=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AV1=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AVS3=m \
	CONFIG_AMLOGIC_MEDIA_ENHANCEMENT_DOLBYVISION=y \
	CONFIG_AMLOGIC_MEDIA_GE2D=y \
	CONFIG_AMLOGIC_MEDIA_VENC_MULTI=m \
	CONFIG_AMLOGIC_MEDIA_VENC_JPEG=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_VP9_FB=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_H265_FB=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AV1_FB=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AV1_T5D=m \
	CONFIG_AMLOGIC_MEDIA_VDEC_AVS2_FB=m \
	CONFIG_AMLOGIC_MEDIA_VENC_MEMALLOC=m \
	CONFIG_AMLOGIC_MEDIA_VENC_VCENC=m \
	CONFIG_AMLOGIC_MEDIA_VENC_COMMON=m \
	CONFIG_AMLOGIC_HW_DEMUX=m \
	CONFIG_AMLOGIC_MEDIA_V4L_DEC=y

MEDIA_MODULES_CFLAGS = ""
else
${info "media_modules use config in ${PRODUCT_FULL_DIR}"}
include ${PRODUCT_FULL_DIR}/media_modules.build.config.trunk.mk
endif

EXTRA_INCLUDE := -I$(KERNEL_SRC)/$(M)/drivers/include

CONFIGS_BUILD := -Wno-parentheses-equality -Wno-pointer-bool-conversion \
				-Wno-unused-const-variable -Wno-typedef-redefinition \
				-Wno-logical-not-parentheses -Wno-sometimes-uninitialized \
				-Wno-frame-larger-than=

KBUILD_CFLAGS_MODULE += $(GKI_EXT_MODULE_PREDEFINE)

ifeq ($(O),)
out_dir := .
else
out_dir := $(O)
endif
include $(out_dir)/include/config/auto.conf

modules:
	$(MAKE) -C  $(KERNEL_SRC) M=$(M)/drivers modules "EXTRA_CFLAGS+=-I$(INCLUDE) -Wno-error $(CONFIGS_BUILD) $(EXTRA_INCLUDE) $(KBUILD_CFLAGS_MODULE) ${VERSION_CONTROL_CFLAGS} ${MEDIA_MODULES_CFLAGS}" $(CONFIGS)

all: modules

modules_install:
	$(MAKE) INSTALL_MOD_STRIP=1 M=$(M)/drivers -C $(KERNEL_SRC) modules_install
	$(Q)mkdir -p ${out_dir}/../vendor_lib/modules
	$(Q)mkdir -p ${out_dir}/../vendor_lib/firmware/video
	$(Q)cp $(KERNEL_SRC)/$(M)/firmware/* ${out_dir}/../vendor_lib/firmware/video/ -rf
	$(Q)if [ -z "$(CONFIG_AMLOGIC_KERNEL_VERSION)" ]; then \
		cd ${out_dir}/$(M)/; find -name "*.ko" -exec cp {} ${out_dir}/../vendor_lib/modules/ \; ; \
	else \
		find $(INSTALL_MOD_PATH)/lib/modules/*/$(INSTALL_MOD_DIR) -name "*.ko" -exec cp {} ${out_dir}/../vendor_lib/modules \; ; \
	fi;

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M)  clean
