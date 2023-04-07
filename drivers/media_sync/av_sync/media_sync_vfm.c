#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/semaphore.h>
#include <linux/unistd.h>
#include <linux/times.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/amlogic/media/vfm/vframe.h>
#include <linux/amlogic/media/vfm/vframe_provider.h>
#include <linux/amlogic/media/vfm/vframe_receiver.h>
#include <linux/amlogic/media/di/di.h>
#include <linux/amlogic/media/frame_sync/ptsserv.h>

#include "media_sync_vfm.h"


static u32 media_sync_vf_debug_level = 0;
#define mediasync_pr_info(dbg_level,inst,fmt,args...) if (dbg_level <= media_sync_vf_debug_level) {pr_info("[media_sync:%d][%d] " fmt,__LINE__, inst,##args);}


static int mediasync_core_thread(void* param)
{
	mediasync_vf_dev *dev = (mediasync_vf_dev *)param;
	struct vframe_s* vf = NULL;
	mediasync_pr_info(0,dev->inst,"mediasync_core_thread in\n");
	while (dev->running) {
		//mediasync_pr_info(0,dev->inst,"down_interruptible in\n");
		//if (down_interruptible(&dev->sem)) {
		//	mediasync_pr_info(0,dev->inst,"thread down_interruptible break\n")
		//	break;
		//}
		//mediasync_pr_info(0,dev->inst,"down_interruptible out\n");
		//while(1) {
		if (kthread_should_stop()) {
			break;
		}
		vf = vf_peek(dev->vf_receiver_name);
		if (vf == NULL) {
			//mediasync_pr_info(0,dev->inst,"mediasync_core_thread vf_peek NULL\n");
			msleep(1);
			continue;
			//break;
		}
		msleep(20);
		dev->frmeStatus = 1;
		#if 0
		VideoProcess(mediasync_id,vf->pts,&vsyncPolicy);
		if (videopolicy == NORMAL_OUTPUT)
			vf = vf_get(dev->vf_receiver_name);
			vfq_push(&dev->q_ready, vf);
			vf_notify_receiver(dev->vf_provider_name,
				VFRAME_EVENT_PROVIDER_VFRAME_READY,NULL);
		} else if (videopolicy == HOLD) {
			usleep(hold time);
			vf = vf_get(dev->vf_receiver_name);
			vfq_push(&dev->q_ready, vf);
			vf_notify_receiver(dev->vf_provider_name,
				VFRAME_EVENT_PROVIDER_VFRAME_READY,NULL);
		} else if (videopolicy == DROP) {
			vf = vf_get(dev->vf_receiver_name);
			vf_put(vf, dev->vf_receiver_name);
			vf_notify_provider(dev->vf_receiver_name,
			VFRAME_EVENT_RECEIVER_PUT,NULL);
		}
		#else
		//msleep(10);
		//vf = vf_get(dev->vf_receiver_name);
		//kfifo_put(&dev->q_ready, vf);
		//mediasync_pr_info(0,dev->inst,"mediasync_core_thread vf->pts : %lld list_free:%d\n",vf->pts_us64,kfifo_avail(&(dev->q_ready)));
		//vf_notify_receiver(dev->vf_provider_name,
		//	VFRAME_EVENT_PROVIDER_VFRAME_READY,NULL);
		#endif

		//}
		//if (kthread_should_stop()) {
		//	mediasync_pr_info(0,dev->inst,"kthread_should_stop break !\n");
		//	break;
		//}
	}


	mediasync_pr_info(0,dev->inst,"mediasync_core_thread while(running) break\n");
	while (!kthread_should_stop()) {
		usleep_range(1000, 2000);
	}
	mediasync_pr_info(0,dev->inst,"mediasync_core_thread out\n");
	return 0;
}




/* -----------------------------------------------------------------
 *           provider operations
 * -----------------------------------------------------------------
 */
static struct vframe_s *mediasync_vf_peek(void *op_arg)
{
	mediasync_vf_dev *dev = (mediasync_vf_dev *)op_arg;
	struct vframe_s *vf = NULL;
	if (dev == NULL) {
		return NULL;
	}
	if (dev->frmeStatus == 0) {
		return NULL;
	}
	vf = vf_peek(dev->vf_receiver_name);
	if (vf == NULL) {
		return NULL;
	}
	return vf;
}

static struct vframe_s *mediasync_vf_get(void *op_arg)
{
	struct vframe_s *vf = NULL;
	mediasync_vf_dev *dev = (mediasync_vf_dev *)op_arg;
	if (dev == NULL) {
		return NULL;
	}
	if (dev->frmeStatus == 0) {
		return NULL;
	}
	vf = vf_get(dev->vf_receiver_name);
	if (vf == NULL) {
		return NULL;
	}
	dev->frmeStatus = 0;
	return vf;
}

static void mediasync_vf_put(struct vframe_s *vf, void *op_arg)
{
	mediasync_vf_dev *dev = (mediasync_vf_dev *)op_arg;
	int ret = 0;
	bool is_di_pw = false;

	if (vf->type & VIDTYPE_DI_PW) {
		is_di_pw = true; //is di buffer
	}

	ret = vf_put(vf, dev->vf_receiver_name);
	if (ret < 0) {
		if (is_di_pw) {
			dim_post_keep_cmd_release2(vf);
			mediasync_pr_info(0,dev->inst,"mediasync_vf_put dim release \n");
		}
		return;
	}
	ret = vf_notify_provider(dev->vf_receiver_name, VFRAME_EVENT_RECEIVER_PUT,
				NULL);
	if (ret < 0) {
		mediasync_pr_info(0,dev->inst,"mediasync_vf_put vf_notify_provider error\n");
	}
	dev->frmeStatus = 0;
}

static int mediasync_event_cb(int type, void *data, void *private_data)
{
	mediasync_vf_dev *dev = (mediasync_vf_dev *)private_data;
	//mediasync_pr_info(0,dev->inst,"mediasync_event_cb type:0x%x",type);
	/* printk("ionvideo_event_cb_type=%d\n",type); */
	if (type & VFRAME_EVENT_RECEIVER_PUT) {
		/* printk("video put, avail=%d\n", vfq_level(&q_ready) ); */
	} else if (type & VFRAME_EVENT_RECEIVER_GET) {
		/* printk("video get, avail=%d\n", vfq_level(&q_ready) ); */
	} else if (type & VFRAME_EVENT_RECEIVER_FRAME_WAIT) {
		/* up(&thread_sem); */
		/* printk("receiver is waiting\n"); */
	} else if (type & VFRAME_EVENT_RECEIVER_FRAME_WAIT) {
		/* printk("frame wait\n"); */
	}
	vf_notify_provider(dev->vf_receiver_name, type, NULL);
	return 0;
}

static int mediasync_vf_states(struct vframe_states *states, void *op_arg)
{
	/* unsigned long flags; */
	/* spin_lock_irqsave(&lock, flags); */
	mediasync_vf_dev *dev = (mediasync_vf_dev *)op_arg;
	if (dev == NULL) {
		return 0;
	}
	states->vf_pool_size = 0;
	states->buf_recycle_num = 0;
	states->buf_free_num = 0;
	states->buf_avail_num = 0;
	/* spin_unlock_irqrestore(&lock, flags); */
	return 0;
}

static const struct vframe_operations_s mediasync_vf_provider = {
	.peek = mediasync_vf_peek,
	.get = mediasync_vf_get,
	.put = mediasync_vf_put,
	.event_cb = mediasync_event_cb,
	.vf_states = mediasync_vf_states,
};


static int mediasync_receiver_event_fun(int type, void *data, void *private_data)
{
	//struct vframe_states states;
	mediasync_vf_dev *dev = (mediasync_vf_dev *)private_data;
	struct vframe_provider_s * vframeName_receiver = NULL;
	struct vframe_receiver_s * vframeName_provider = NULL;
	//struct vframe_s *vf = NULL;
	//mediasync_pr_info(0,dev->inst,"mediasync_receiver_event_fun type:%d \n",type);
	if (type == VFRAME_EVENT_PROVIDER_UNREG) {
		mediasync_pr_info(0,dev->inst,"VFRAME_EVENT_PROVIDER_UNREG in \n");

		if (dev->thread != NULL) {
			dev->running = false;
			up(&dev->sem);
			kthread_stop(dev->thread);
			dev->thread = NULL;
		}

		vframeName_receiver = vf_get_provider(dev->vf_receiver_name);
		if (vframeName_receiver) {
			mediasync_pr_info(0,dev->inst,"unreg-> provider:%s \n",vframeName_receiver->name);
		} else {
			mediasync_pr_info(0,dev->inst,"unreg-> provider:NULL \n");
		}

		vf_unreg_provider(&dev->mediasync_vf_prov);

		pts_stop(PTS_TYPE_VIDEO);
		mediasync_pr_info(0,dev->inst,"VFRAME_EVENT_PROVIDER_UNREG out \n");

		//mutex_unlock(&dev->vf_mutex);
	} else if (type == VFRAME_EVENT_PROVIDER_REG) {

		//mutex_lock(&dev->vf_mutex);
		int ret = 0;
		mediasync_pr_info(0,dev->inst,"VFRAME_EVENT_PROVIDER_REG in\n");

		vf_reg_provider(&dev->mediasync_vf_prov);

		dev->frmeStatus = 0;
		ret = pts_start(PTS_TYPE_VIDEO);
		if (ret < 0) {
			mediasync_pr_info(0,dev->inst,"pts_start failed, retry\n");
			ret = pts_stop(PTS_TYPE_VIDEO);
			if (ret < 0)
				mediasync_pr_info(0,dev->inst,"pts_stop failed when retrying...");
			ret = pts_start(PTS_TYPE_VIDEO);
			if (ret < 0)
				mediasync_pr_info(0,dev->inst,"pts_start failed\n");
		}
		mediasync_pr_info(0,dev->inst,"VFRAME_EVENT_PROVIDER_REG out\n");
		//mutex_unlock(&dev->vf_mutex);
	} else if (type == VFRAME_EVENT_PROVIDER_QUREY_STATE) {
		//mediasync_vf_states(&states, dev);
		//if (states.buf_avail_num > 0)
		//	return RECEIVER_ACTIVE;

		if (vf_notify_receiver(dev->vf_provider_name,
				 VFRAME_EVENT_PROVIDER_QUREY_STATE,
				 NULL) == RECEIVER_ACTIVE) {
			return RECEIVER_ACTIVE;
		}
		return RECEIVER_INACTIVE;
	/*break;*/
	} else if (type == VFRAME_EVENT_PROVIDER_START) {
		mediasync_pr_info(0,dev->inst,"VFRAME_EVENT_PROVIDER_START\n");
		//mutex_lock(&dev->vf_mutex);
		vframeName_provider = vf_get_receiver(dev->vf_provider_name);
		if (vframeName_provider) {
			mediasync_pr_info(0,dev->inst,"provider name=%s", vframeName_provider->name);

			vframeName_receiver = vf_get_provider(dev->vf_receiver_name);
			if (vframeName_receiver) {
				mediasync_pr_info(0,dev->inst,"START mediasync receiver:%s \n",vframeName_receiver->name);
			}

			vf_notify_receiver(dev->vf_provider_name,
								VFRAME_EVENT_PROVIDER_START,
								NULL);

			sema_init(&dev->sem, 0);
			dev->running = true;
			dev->thread = kthread_run(mediasync_core_thread, dev,
						"%s", "mediasync-core");
			if (IS_ERR(dev->thread)) {
				mediasync_pr_info(0,dev->inst,"kthread_run error ");
			}
		}
		//mutex_unlock(&dev->vf_mutex);
	} else if (type == VFRAME_EVENT_PROVIDER_FR_HINT) {
		vf_notify_receiver(dev->vf_provider_name,
							VFRAME_EVENT_PROVIDER_FR_HINT,
							data);
	} else if (type == VFRAME_EVENT_PROVIDER_FR_END_HINT) {
		vf_notify_receiver(dev->vf_provider_name,
							VFRAME_EVENT_PROVIDER_FR_END_HINT,
							data);
	} else if (type == VFRAME_EVENT_PROVIDER_RESET) {
		vf_notify_receiver(dev->vf_provider_name,
							VFRAME_EVENT_PROVIDER_RESET,
							data);
	} else if (type == VFRAME_EVENT_PROVIDER_VFRAME_READY) {
		//if (vf_peek(dev->vf_receiver_name)) {
			//wake_up_interruptible(&dev->wq);
			//mediasync_pr_info(0,dev->inst,"VFRAME_EVENT_PROVIDER_VFRAME_READY \n");
			up(&dev->sem);
		//}
	}
	return 0;
}

static const struct vframe_receiver_op_s mediasync_vf_receiver = {
	.event_cb = mediasync_receiver_event_fun
};


static int mediasync_create_vf_instance(int inst)
{

	mediasync_vf_dev *dev;
	//int ret;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/*init */
	dev->inst = inst;

	init_waitqueue_head(&dev->wq);
	sema_init(&dev->sem, 0);


	snprintf(dev->vf_receiver_name, MEDIASYNC_VF_NAME_SIZE,
			 RECEIVER_NAME ".%x", inst & 0xff);

	snprintf(dev->vf_provider_name, MEDIASYNC_VF_NAME_SIZE,
			 PROVIDER_NAME ".%x", inst & 0xff);

	vf_receiver_init(&dev->mediasync_vf_recv,
			 dev->vf_receiver_name,
			 &mediasync_vf_receiver, dev);

	vf_reg_receiver(&dev->mediasync_vf_recv);

	vf_provider_init(&dev->mediasync_vf_prov,
					dev->vf_provider_name,
					&mediasync_vf_provider,
					dev);

	/* Now that everything is fine, let's add it to device list */
	list_add_tail(&dev->mediasync_vf_devlist, &mediasync_vf_devlist);

	return 0;

//free_dev:
//	kfree(dev);
//	return ret;
}

int mediasync_vf_release(void)
{
	mediasync_vf_dev *dev;
	struct list_head *list;

	while (!list_empty(&mediasync_vf_devlist)) {
		list = mediasync_vf_devlist.next;
		list_del(list);
		dev = list_entry(list, mediasync_vf_dev, mediasync_vf_devlist);
		kfree(dev);
	}

	return 0;
}



int mediasync_vf_init(void)
{
	int inst = 0;
	mediasync_create_vf_instance(inst);
	return 0;
}



