MODULE_NAME = amlogic-media-sync
obj-m	+=	 ${MODULE_NAME}.o

${MODULE_NAME}-objs	+=	media_sync_dev.o
${MODULE_NAME}-objs	+=	media_sync_core.o

PR_FMT = $(subst amlogic-,,$(MODULE_NAME))
PR_FMT_DEFINE="-Dpr_fmt(fmt)=\"[$(PR_FMT)]:\" fmt"
ccflags-y += $(PR_FMT_DEFINE)
