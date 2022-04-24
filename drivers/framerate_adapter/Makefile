MODULE_NAME = amlogic-video-framerate-adapter
obj-m	+=	 ${MODULE_NAME}.o

${MODULE_NAME}-objs	+=	video_framerate_adapter.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
