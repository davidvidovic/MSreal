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
#include <linux/wait.h>
#include <linux/semaphore.h>

#define BUFF_SIZE 20

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

struct semaphore sem;
// DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);


int regA = 256, regB = 256, regC = 256, regD = 256;
int result = 256;
unsigned char carry;

unsigned char flag_hex = 0;
unsigned char flag_bin = 0;
unsigned char flag_BLOKIRAJ_UPIS = 0;

int endRead = 0;

int alu_open(struct inode *pinode, struct file *pfile);
int alu_close(struct inode *pinode, struct file *pfile);
ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.open = alu_open,
	.read = alu_read,
	.write = alu_write,
	.release = alu_close,
};


int alu_open(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully opened file\n");
		return 0;
}

int alu_close(struct inode *pinode, struct file *pfile) 
{
		printk(KERN_INFO "Succesfully closed file\n");
		return 0;
}

ssize_t alu_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
	int ret;
	char buff[BUFF_SIZE];
	char temp_buff[BUFF_SIZE];
	int len;
	int temp_res;
	int i;
	int j;

	if (endRead){
		endRead = 0;
		printk(KERN_INFO "Succesfully read from file\n");
		return 0;
	}
	
	
	// ZAUZIMAM SEMAFOR
	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	
	
	if(result == 256)
	{
		endRead = 0;
		printk(KERN_INFO "Nema rezultata za prikazati!\n");
		
		return 0;
	}
	else
	{
		flag_BLOKIRAJ_UPIS = 0;
		// prebacujem result u string
		
		if(flag_bin || flag_hex)
		{
			if(flag_bin)
			{
				temp_res = result;
				i = 0;

				while(temp_res & 0x00ff)
				{
					if(temp_res % 2 == 0)
					{
						temp_buff[i++] = '0';
					}
					else
					{
						temp_buff[i++] = '1';
					}
					
					temp_res = temp_res / 2;
				}
				
				buff[i] = '\0';
				temp_buff[i] = '\0';
				

				for(j = i; j > 0; j--) {
					buff[i-j] = temp_buff[j-1];	
				}

				len = i;
				flag_bin = 0;
			}

			if(flag_hex)
			{
				flag_hex = 0;
				len = scnprintf(buff, BUFF_SIZE, "%x %d", result, carry);
			}
		}
		else
		{
			len = scnprintf(buff, BUFF_SIZE, "%d %d", result, carry);
			// printk(KERN_DEBUG "len=%d\n", len);
		}
		
		
		ret = copy_to_user(buffer, buff, len);
		if(ret)
			return -EFAULT;
		
		endRead = 1;
		
		wake_up_interruptible(&writeQ);
	}

	// VRACAM SEMAFOR
	up(&sem);
	
	return len;
}

ssize_t alu_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
	char buff[BUFF_SIZE];
	int operand1, operand2;
	int cifra;
	int ret;
	char oznaka;

	ret = copy_from_user(buff, buffer, length);
	if(ret)
		return -EFAULT;
	
	// ZAUZIMAM SEMAFOR
	if(down_interruptible(&sem))
		return -ERESTARTSYS;
	
	
	if(buff[4] == '=')	// upis u registar
	{		
		ret = sscanf(buff, "reg%c=%d", &oznaka, &cifra);
			
		if(ret == 2){
			if(cifra < 0 || cifra > 255) 
			{
				printk(KERN_WARNING "Cifra van opsega! Dozvoljen opseg: 0 - 255\n");
			}
			else 
			{
				switch(buff[3])
				{
					case 'A':
						regA = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regA\n", cifra);
					break;
					
					
					case 'B':
						regB = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regB\n", cifra);
					break;
					
					
					case 'C':
						regC = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regC\n", cifra);
					break;
					
					
					case 'D':
						regD = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regD\n", cifra);
					break;
					
					
					case 'a':
						regA = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regA\n", cifra);
					break;
					
					
					case 'b':
						regB = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regB\n", cifra);
					break;
					
					
					case 'c':
						regC = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regC\n", cifra);
					break;
					
					
					case 'd':
						regD = cifra;
						printk(KERN_WARNING "Uspjesno upisan broj %d u regD\n", cifra);
					break;
					
					default:
						printk(KERN_WARNING "Nije pronadjen trazeni registar! Postojeci registri: regA, regB, regC, regD. \n");
					break;
				}
			}
		}
		else {
			printk(KERN_INFO "Pogresan format!\n");
		}
	}
	
	if(buff[4] == ' ') // upis operacije
	{
		printk(KERN_DEBUG "Upis operacije\n");
		
		if(buff[5] == '+' ||
		   buff[5] == '-' ||
		   buff[5] == '*' ||
		   buff[5] == '/')
		{
			printk(KERN_DEBUG "Ispravan format!\n");
			printk(KERN_DEBUG "Operacija je %c\n", buff[5]);
			
			
			while(flag_BLOKIRAJ_UPIS)
			{
				up(&sem);
				if(wait_event_interruptible(writeQ, !flag_BLOKIRAJ_UPIS))
					return -ERESTARTSYS;
				if(down_interruptible(&sem))
					return -ERESTARTSYS;
			}
			
			
			if(!flag_BLOKIRAJ_UPIS)
			{
				printk(KERN_INFO "Dozvoljen upis!\n");
				flag_BLOKIRAJ_UPIS = 1;
				
				// skeniranje prvog operanda sa indeksa 3
				switch(buff[3])
				{
						case 'A':
							operand1 = regA;
						break;
						
						
						case 'B':
							operand1 = regB;
						break;
						
						
						case 'C':
							operand1 = regC;
						break;
						
						
						case 'D':
							operand1 = regD;
						break;
						
						
						case 'a':
							operand1 = regA;
						break;
						
						
						case 'b':
							operand1 = regB;
						break;
						
						
						case 'c':
							operand1 = regC;
						break;
						
						
						case 'd':
							operand1 = regD;
						break;
						
						default:
							printk(KERN_WARNING "Nije pronadjen trazeni registar! Postojeci registri: regA, regB, regC, regD. \n");
							flag_BLOKIRAJ_UPIS = 0;
						break;
				}
				
				// skeniranje drugog operanda sa indeksa 10
				switch(buff[10])
				{
						case 'A':
							operand2 = regA;
						break;
						
						
						case 'B':
							operand2 = regB;
						break;
						
						
						case 'C':
							operand2 = regC;
						break;
						
						
						case 'D':
							operand2 = regD;
						break;
						
						
						case 'a':
							operand2 = regA;
						break;
						
						
						case 'b':
							operand2 = regB;
						break;
						
						
						case 'c':
							operand2 = regC;
						break;
						
						
						case 'd':
							operand2 = regD;
						break;
						
						default:
							printk(KERN_WARNING "Nije pronadjen trazeni registar! Postojeci registri: regA, regB, regC, regD. \n");
							flag_BLOKIRAJ_UPIS = 0;
						break;
				}
				
				printk(KERN_DEBUG "Operandi su %d i %d\n", operand1, operand2);
				
				if(flag_BLOKIRAJ_UPIS) 
				{
					if(operand1 == 256 || operand2 == 256)
					{
						printk(KERN_WARNING "Registri nisu popunjeni!\n");
						flag_BLOKIRAJ_UPIS = 0;
					}
					else
					{
					   if(buff[5] == '+') 
					   {
						   printk(KERN_INFO "Operacija sabiranja %d + %d!\n", operand1, operand2);
							result = operand1 + operand2;
							if(result > 255)
							{
								carry = 1;
								result = result - 256; 
							}
							else
							{
								carry = 0;
							}
						}
						
						if(buff[5] == '-') 
						{
							printk(KERN_INFO "Operacija oduzimanja %d - %d!\n", operand1, operand2);
							result = operand1 - operand2;
							if(result < 0)
							{
								carry = 1;
								result = result + 256; 
							}
							else
							{
								carry = 0;
							}
						}
						
						if(buff[5] == '*') 
						{
							printk(KERN_INFO "Operacija mnozenja %d * %d!\n", operand1, operand2);
							result = operand1 * operand2;
							if(result > 255)
							{
								carry = 1;
								while(result > 255)
								{
									result = result - 256; 
								}
							}
							else
							{
								carry = 0;
							}
						}
						
						
						if(buff[5] == '/') 
						{
							printk(KERN_INFO "Operacija deljenja %d / %d!\n", operand1, operand2);
							result = operand1 / operand2;
							carry = 0;
						}
					}
				}
			}
			else
			{
				printk(KERN_INFO "Trenutno nije dozvoljen upis! Prvo procitaj rezultat prethodne operacije!\n");
			}
		}
		else
		{
		   printk(KERN_INFO "Pogresan format!\n"); 
		}
	}

	if(buff[0] == 'f') // pomocna naredba
	{
		//flag_BLOKIRAJ_UPIS = 1;
		if(buff[8] == 'h' && buff[9] == 'e' && buff[10] == 'x') 
		{
			printk(KERN_INFO "HEX format!\n");
			flag_hex = 1;
			flag_bin = 0;
		}
		else if(buff[8] == 'b' && buff[9] == 'i' && buff[10] == 'n') 
		{
			printk(KERN_INFO "BIN format!\n");
			flag_hex = 0;
			flag_bin = 1;
		}
		else if(buff[8] == 'd' && buff[9] == 'e' && buff[10] == 'c')
		{
			printk(KERN_INFO "DEC format!\n");
			flag_hex = 0;
			flag_bin = 0;
		}
		else printk(KERN_INFO "Pogresan format!\n"); 
	}
	
	// VRACAM SEMAFOR
	up(&sem);
	
	
	return length;
}

static int __init alu_init(void)
{
   int ret = 0;

	sema_init(&sem,1);

   ret = alloc_chrdev_region(&my_dev_id, 0, 1, "alu");
   if (ret){
      printk(KERN_ERR "failed to register char device\n");
      return ret;
   }
   printk(KERN_INFO "char device region allocated\n");

   my_class = class_create(THIS_MODULE, "alu_class");
   if (my_class == NULL){
      printk(KERN_ERR "failed to create class\n");
      goto fail_0;
   }
   printk(KERN_INFO "class created\n");
   
   my_device = device_create(my_class, NULL, my_dev_id, NULL, "alu");
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

static void __exit alu_exit(void)
{
   cdev_del(my_cdev);
   device_destroy(my_class, my_dev_id);
   class_destroy(my_class);
   unregister_chrdev_region(my_dev_id,1);
   printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(alu_init);
module_exit(alu_exit);