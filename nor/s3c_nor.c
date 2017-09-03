#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
//#include <linux/mtd/physmap.h>
#include <asm/io.h>

static const char *s3c_nor_rom_probe_types[] = {
	"cfi_probe",
	"jedec_probe",
	"map_rom",
	NULL
};

struct s3c_nor_flash_info {
	struct mtd_info		*mtd;
	struct map_info		map;
	struct resource		*res;
#ifdef CONFIG_MTD_PARTITIONS
	int			nr_parts;
	struct mtd_partition	*parts;
#endif
};

struct s3c_nor_flash_info *s3c_nor_info;

static int s3c_nor_flash_probe(struct platform_device *dev)
{
	const char **probe_type;
	int err;

	s3c_nor_info = kzalloc(sizeof(struct s3c_nor_flash_info), GFP_KERNEL);
	if (s3c_nor_info == NULL) {
		err = -ENOMEM;
		goto err_out0;
	}

	s3c_nor_info->map.name = "s3c_norflash";
	s3c_nor_info->map.phys = 0;
	s3c_nor_info->map.size = 0x1000000;
	s3c_nor_info->map.bankwidth = 2;
	//s3c_nor_info->map.set_vpp = physmap_data->set_vpp;

	s3c_nor_info->map.virt = ioremap(s3c_nor_info->map.phys, s3c_nor_info->map.size);
	if (s3c_nor_info->map.virt == NULL) {
		printk("Failed to ioremap flash region\n");
		err = EIO;
		goto err_out1;
	}

	simple_map_init(&s3c_nor_info->map);

	probe_type = s3c_nor_rom_probe_types;
	for (; s3c_nor_info->mtd == NULL && *probe_type != NULL; probe_type++)
		s3c_nor_info->mtd = do_map_probe(*probe_type, &s3c_nor_info->map);
	if (s3c_nor_info->mtd == NULL) {
		dev_err(&dev->dev, "map_probe failed\n");
		err = -ENXIO;
		goto err_out1;
	}
	s3c_nor_info->mtd->owner = THIS_MODULE;

#if 0
	err = parse_mtd_partitions(s3c_nor_info->mtd, s3c_nor_rom_probe_types, &s3c_nor_info->parts, 0);
	if (err > 0) {
		add_mtd_partitions(s3c_nor_info->mtd, s3c_nor_info->parts, err);
		return 0;
	}

	if (physmap_data->nr_parts) {
		printk(KERN_NOTICE "Using physmap partition information\n");
		add_mtd_partitions(s3c_nor_info->mtd, physmap_data->parts,
						physmap_data->nr_parts);
		return 0;
	}
#endif

	add_mtd_device(s3c_nor_info->mtd);
	return 0;

err_out1:
	//physmap_flash_remove(dev);
	iounmap(s3c_nor_info->map.virt);
err_out0:
	kfree(s3c_nor_info);
	return err;
}

static int s3c_nor_flash_remove(struct platform_device *dev)
{
	//del_mtd_partitions(s3c_nor_info->mtd);
	del_mtd_device(s3c_nor_info->mtd);
	map_destroy(s3c_nor_info->mtd);
	iounmap(s3c_nor_info->map.virt);
	kfree(s3c_nor_info);

	return 0;
}

static struct platform_driver s3c_nor_flash_driver = {
	.probe		= s3c_nor_flash_probe,
	.remove		= s3c_nor_flash_remove,
	.driver		= {
		.name	= "physmap-flash",
	},
};

static int __init s3c_nor_init(void)
{
	int err;

	err = platform_driver_register(&s3c_nor_flash_driver);

	return err;
}

static void __exit s3c_nor_exit(void)
{
	platform_driver_unregister(&s3c_nor_flash_driver);
}

module_init(s3c_nor_init);
module_exit(s3c_nor_exit);
MODULE_LICENSE("GPL");

