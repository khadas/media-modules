MODULE_NAME = amlogic-stream-input
obj-m	+=	 ${MODULE_NAME}.o

${MODULE_NAME}-objs	+=	amports/amstream.o
${MODULE_NAME}-objs	+=	amports/adec.o
${MODULE_NAME}-objs	+=	amports/thread_rw.o
${MODULE_NAME}-objs	+=	amports/streambuf.o
${MODULE_NAME}-objs	+=	amports/stream_buffer_base.o
${MODULE_NAME}-objs	+=	amports/stream_buffer_interface.o

${MODULE_NAME}-objs	+=	parser/esparser.o
${MODULE_NAME}-objs	+=	parser/tsdemux.o
${MODULE_NAME}-objs	+=	parser/psparser.o
${MODULE_NAME}-objs	+=	parser/rmparser.o
${MODULE_NAME}-objs	+=	subtitle/subtitle.o

${MODULE_NAME}-objs	+=	parser/dvb_common.o
obj-$(CONFIG_AMLOGIC_DVB)   += parser/hw_demux/
obj-$(CONFIG_AMLOGIC_DVB)   += parser/dvb_ci/

ccflags-y += -I.
ccflags-y += -I$(srctree)/include/media

#obj-y 	+= 	tv_frontend/
# obj-y	+=	box-frontend/avl6211/
# obj-y	+=	box-frontend/atbm8881/
# obj-y	+=	box-frontend/avl68xx/
