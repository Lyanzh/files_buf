#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>


#define DEVICE_NAME "ramblock"

static DEFINE_SPINLOCK(ramblock_lock);
#define RAMBLOCK_SIZE (1024*1024)

static unsigned int major;
static struct gendisk *ramblock_gendisk;
static struct request_queue *ramblock_queue;
static unsigned char *ramblock_buf;

static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	/* ÈÝÁ¿=heads*cylinders*sectors*512 */
	geo->heads     = 2;
	geo->cylinders = 32;
	geo->sectors   = RAMBLOCK_SIZE/2/32/512;
	return 0;
}

static struct block_device_operations ramblock_fops =
{
	.owner		= THIS_MODULE,
	//.open		= z2_open,
	//.release	= z2_release,
	.getgeo     = ramblock_getgeo,
};

static void do_ramblock_request(request_queue_t *q)
{
	struct request *req;
	while ((req = elv_next_request(q)) != NULL) {
		unsigned long start = req->sector << 9;
		unsigned long len  = req->current_nr_sectors << 9;

		if (start + len > RAMBLOCK_SIZE) {
			printk( KERN_ERR DEVICE_NAME ": bad access: block=%lu, count=%u\n",
				req->sector, req->current_nr_sectors);
			end_request(req, 0);
			continue;
		}
	
		if (rq_data_dir(req) == READ)
			memcpy(req->buffer, ramblock_buf+start, len);
		else
			memcpy(ramblock_buf+start, req->buffer, len);
		
		end_request(req, 1);
	}
}

static int __init ramblock_init(void)
{
	int ret;
	
	ret = -EBUSY;
	major = register_blkdev(0, DEVICE_NAME);
	if (major < 0)
		goto err;

	ret = -ENOMEM;
	ramblock_gendisk = alloc_disk(16);
	if (!ramblock_gendisk)
		goto out_disk;

	ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
	if (!ramblock_queue)
		goto out_queue;

	ramblock_gendisk->major = major;
	ramblock_gendisk->first_minor = 0;
	ramblock_gendisk->fops = &ramblock_fops;
	sprintf(ramblock_gendisk->disk_name, DEVICE_NAME);

	ramblock_gendisk->queue = ramblock_queue;

	set_capacity(ramblock_gendisk, RAMBLOCK_SIZE / 512);

	ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);
	
	add_disk(ramblock_gendisk);
	//blk_register_region(MKDEV(major, 0), Z2MINOR_COUNT, THIS_MODULE,
	//			z2_find, NULL, NULL);

	return 0;

out_queue:
	put_disk(ramblock_gendisk);
out_disk:
	unregister_blkdev(major, DEVICE_NAME);
err:
	return ret;
}

static void __exit ramblock_exit(void)
{
    //blk_unregister_region(MKDEV(Z2RAM_MAJOR, 0), 256);
    if ( unregister_blkdev(major, DEVICE_NAME) != 0 )
		printk( KERN_ERR DEVICE_NAME ": unregister of device failed\n");

    del_gendisk(ramblock_gendisk);
    put_disk(ramblock_gendisk);
    blk_cleanup_queue(ramblock_queue);
	kfree(ramblock_buf);
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");


