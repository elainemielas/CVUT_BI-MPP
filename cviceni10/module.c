#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define ZMENA          100

int major;
char* kbuf;

static int mpp_open(struct inode *inode, struct file *filp)
{
	printk ("mpp_open called\n");
	return 0;
}
static int mpp_release(struct inode *inode, struct file *f)
{
	printk ("mpp_release called\n");
	return 0;
}
static int mpp_read(struct file *f, char *buf, size_t size, loff_t *offset)
{
	printk ("mpp_read called\n");
	copy_to_user (buf, kbuf, size);
	return 0;
}
static int mpp_write(struct file *f, const char *buf, size_t size, loff_t *offset)
{
	printk ("mpp_write called\n");
	copy_from_user (kbuf, buf, size);
	return size; 
}
static long mpp_ioctl(struct file *f, const char *buf, unsigned int request, unsigned long param)
{
	printk ("mpp_ioctl called\n");
	int res = 0, i;
	char* tmpbuf;
	tmpbuf = kmalloc(1000, GFP_KERNEL);
	switch(request) {
		case ZMENA:
		     copy_from_user (tmpbuf, buf, strlen(buf)+1);
                     for (i=0;i<strlen(buf);i++) {
			if(tmpbuf[i] >= 'a' && tmpbuf[i] <= 'z') tmpbuf[i]=('A'+ tmpbuf[i]-'a');
		     }
		     copy_to_user (buf, tmpbuf, strlen(buf)+1);
		     res = 11;
                     break;
		default:
                     res = -ENOTTY;
                     break;
	}
	printk ("res: %d\n", res);
	kfree(tmpbuf);
	return res;
}

struct file_operations mpp_fops = {
    .owner = THIS_MODULE,
    .llseek = NULL,
    .read = mpp_read,
    .write = mpp_write,
        .aio_read = NULL,
        .aio_write = NULL,
        .readdir = NULL,
        .poll = NULL,
        .unlocked_ioctl = mpp_ioctl,
    .compat_ioctl = NULL,
        .mmap = NULL,
    .open = mpp_open,
        .flush = NULL,
    .release = mpp_release,
        .fsync = NULL,
        .aio_fsync = NULL,
        .fasync = NULL,
        .lock = NULL,
        .sendpage = NULL,
        .get_unmapped_area = NULL,
        .check_flags = NULL,
        .flock = NULL,
        .splice_write = NULL,
        .splice_read = NULL,
        .setlease = NULL,
        .fallocate = NULL
};


static int __init mpp_module_init(void)
{
	int size = 1000;
	major = register_chrdev(0, "mpp", &mpp_fops);
	kbuf = kmalloc(size, GFP_KERNEL);
        printk ("MPP module is loaded\n");
        return 0;
}

static void __exit mpp_module_exit(void)
{
	unregister_chrdev(major, "mpp");
	kfree(kbuf);
	printk ("MPP module is unloaded\n");
	return;
}


module_init(mpp_module_init);
module_exit(mpp_module_exit);


MODULE_LICENSE("GPL");
