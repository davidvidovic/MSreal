#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#define BUFF_SIZE 20

// MAX SIZE OF LIFO BUFFER
#define BUFFER_SIZE 10

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

int LIFO_buffer[BUFFER_SIZE];
int tail = 0;
int endRead = 0;

int LIFO_buffer_open(struct inode *pinode, struct file *pfile);
int LIFO_buffer_close(struct inode *pinode, struct file *pfile);
ssize_t LIFO_buffer_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t LIFO_buffer_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = LIFO_buffer_open,
	.read = LIFO_buffer_read,
	.write = LIFO_buffer_write,
	.release = LIFO_buffer_close,
};


int LIFO_buffer_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int LIFO_buffer_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t LIFO_buffer_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	long int len;
	int j;
	
	if(tail == 0){
		printk(KERN_INFO "Buffer is empty\n");
		return 0;
	}
	
	if(endRead){
		endRead = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	
	len = scnprintf(buff,BUFF_SIZE , "%d ", LIFO_buffer[0]);
	ret = copy_to_user(buffer, buff, len);
	if(ret)
		return -EFAULT;

	endRead = 1;

	for(j = 0; j < tail-1; j++){
		LIFO_buffer[j] = LIFO_buffer[j+1];
	}
	
	tail--;
	LIFO_buffer[tail] = '\0';
	
	return len;
}

ssize_t LIFO_buffer_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int value;
	int ret;
	int j;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	buff[length-1] = '\0';

	ret = sscanf(buff,"%d",&value);

	if(ret==1)//one parameter parsed in sscanf
	{
		if(tail >=0 && tail < BUFFER_SIZE)
		{
			tail++;
			LIFO_buffer[tail] = '\0';
			
			for(j = tail-1; j > 0; j--){
				LIFO_buffer[j] = LIFO_buffer[j-1];
			}
			
			LIFO_buffer[0] = value; 
			printk(KERN_INFO "Succesfully wrote value %d, buffer has %d elements", value, tail); 
		}
		else
		{
			printk(KERN_WARNING "Buffer is full!\n"); 
		}
	}
	else
	{
		printk(KERN_WARNING "Wrong command format\nexpected: m\ntm-value\n");
	}

	return length;
}

static int __init LIFO_buffer_init(void)
{
   int ret = 0;

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "LIFO_buffer");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "LIFO_buffer_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "LIFO_buffer");
   if (my_device == NULL){
      printk(KERN_ERR "failed to create device\n");
      goto fail_1;
   }
   printk(KERN_INFO "device created\n");

	my_cdev = cdev_alloc();	
	my_cdev->ops = &my_fops;
	my_cdev->owner = THIS_MODULE;
	ret = cdev_add(my_cdev, my_dev_id, 1);
	if (ret)
	{
      printk(KERN_ERR "failed to add cdev\n");
		goto fail_2;
	}
   printk(KERN_INFO "cdev added\n");
   printk(KERN_INFO "Hello world\n");

   return 0;

   fail_2:
      device_destroy(my_class, my_dev_id);
   fail_1:
      class_destroy(my_class);
   fail_0:
      unregister_chrdev_region(my_dev_id, 1);
   return -1;
}

static void __exit LIFO_buffer_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(LIFO_buffer_init);
module_exit(LIFO_buffer_exit);