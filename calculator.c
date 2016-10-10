/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

/*  
 *  Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);

static ssize_t device_read(struct file *, char *, size_t, loff_t *);

static ssize_t proc_first_write(struct file *, const char *, size_t, loff_t *);
static ssize_t proc_second_write(struct file *, const char *, size_t, loff_t *);
static ssize_t proc_operand_write(struct file *, const char *, size_t, loff_t *);


#define DEVICE_NAME "result"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

static int Major;		/* Major number assigned to our device driver */

static char result[BUF_LEN];
static char first[BUF_LEN];
static char second[BUF_LEN];
static char operand[BUF_LEN];


static struct file_operations fops = {
	.read = device_read,
};

static struct file_operations proc_first_fops = {
	.write = proc_first_write,
};

static struct file_operations proc_second_fops = {
	.write = proc_second_write,
};

static struct file_operations proc_operand_fops = {
	.write = proc_operand_write,
};

static struct proc_dir_entry *proc_entry_first;
static struct proc_dir_entry *proc_entry_second;
static struct proc_dir_entry *proc_entry_operand;

int init_module(void)
{
    memset(result, '0', 3);
    proc_entry_first = proc_create( "first", 0, NULL, &proc_first_fops);
    proc_entry_second = proc_create( "second", 0, NULL, &proc_second_fops);
    proc_entry_operand = proc_create( "operand", 0, NULL, &proc_operand_fops);

    Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return 0;
}

void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);

    remove_proc_entry("first", NULL);
    remove_proc_entry("second", NULL);
    remove_proc_entry("operand", NULL);
}

static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
    ssize_t cnt = strlen(result), ret;
    int nfirst;
    int nsecond;
    int nresult;
    sscanf(first, "%d", &nfirst);
    sscanf(second, "%d", &nsecond);

    if(operand[0] == 'p') {
        nresult = nfirst * nsecond;
        sprintf(result, "%3d", nresult);
    }
    else if(operand[0] == '/') {
        nresult = nfirst / nsecond;
        sprintf(result, "%3d", nresult);
    }
    else if(operand[0] == '-') {
        nresult = nfirst - nsecond;
        sprintf(result, "%3d", nresult);
    }
    else if(operand[0] == '+') {
        nresult = nfirst + nsecond;
        sprintf(result, "%3d", nresult);
    }

    ret = copy_to_user(buffer, result, cnt);
        *offset += cnt - ret;

    if (*offset > cnt)
        return 0;
    else
        return cnt;
}

static ssize_t proc_first_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    if ( copy_from_user(first, buff, len) ) {
        return -EFAULT;
    }
    return len;
}

static ssize_t proc_second_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    if ( copy_from_user(second, buff, len) ) {
        return -EFAULT;
    }
    return len;
}

static ssize_t proc_operand_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    if ( copy_from_user(operand, buff, len) ) {
        return -EFAULT;
    }
    return len;
}

