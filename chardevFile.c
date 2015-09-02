#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/segment.h>

#define MODULE_NAME	"chardevFile"

#define FILE_PATH	"/tmp/chardevFile"
#define FILE_FLAG	(O_RDWR|O_CREAT)
#define FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#if 1
#define debug(str) \
	printk("%s info, %s: "str"\n", MODULE_NAME, __func__)
#define debugP(fmt, ...) \
	printk("%s info, %s: "fmt"\n", MODULE_NAME, __func__, __VA_ARGS__)
#else
  #define debug(str)
  #define debugP(fmt, ...)
#endif

#define error(str) \
	printk("%s err , %s: "str"\n", MODULE_NAME, __func__)

#define errorP(fmt, ...) \
	printk("%s err , %s: "fmt"\n", MODULE_NAME, __func__, __VA_ARGS__)

static int major;
static struct file *filp = NULL;

/********************************************************************/
/* operators */

ssize_t dev_read(char* buf, size_t len)
{
	int ret = -1;
	char* tmp;
	mm_segment_t oldfs;
	
	tmp = vmalloc(sizeof(char)*len);
	memset(tmp, 0, sizeof(char)*len);

	oldfs = get_fs(); 
	set_fs(get_ds());

	ret = filp->f_op->read (filp, buf, len, &filp->f_pos);
	debugP("read %d bytes", ret);

	set_fs(oldfs); 

	if( copy_to_user( buf, tmp, len)!=0)
	{
		error("copy_to_user fault");
		return 0; 
	}
	
	vfree(tmp);

	return ret;
}


ssize_t dev_write(const char* buf, size_t len)
{	
	int ret = -1;
	char* tmp;
	mm_segment_t oldfs;

	tmp = vmalloc(sizeof(char)*len);
	memset(tmp, 0, sizeof(char)*len);

	if( copy_from_user( tmp, buf, len)!=0)
	{
		error("copy_from_user fault");
		return 0; 
	}
	
	oldfs = get_fs(); 
	set_fs(get_ds());

	ret = filp->f_op->write(filp, buf, len, &filp->f_pos);
	debugP("write %d bytes", ret);
	
	set_fs(oldfs); 
	vfree(tmp);

	return ret;
}


// fopen
int dev_open(void)
{

	if( filp == NULL )
	{
		filp = filp_open(FILE_PATH, FILE_FLAG, FILE_MODE);
	}

	if( IS_ERR(filp) )
	{
		error("file open failed");
		return -1;
	}

	debug("device open");
	return 0;
}

// fclose
int dev_close(void)
{
	if( filp != NULL )
	{
		filp_close(filp, NULL);
	}

	debug("device close");
	return 0;
}


/********************************************************************/

static int oper_open(struct inode *inode, struct file *filp)
{
	return dev_open();
}

static int oper_release(struct inode *inode, struct file *filp)
{
	return dev_close();
}

static ssize_t oper_write(struct file *filp, const char *buf, size_t len, loff_t *ppos)
{
	return dev_write(buf, len);
}

static ssize_t oper_read(struct file *filp, char *buf, size_t len, loff_t *ppos)
{
	return dev_read(buf, len);
}

struct file_operations fopers = 
{
	read: 	oper_read,
	write: 	oper_write,
	open: 	oper_open,
	release:oper_release
};

/********************************************************************/

static int chardevFile_init(void) {

	if( dev_open() )
	{
		error("Open fail");
		goto open_fail;
	}

	major = register_chrdev(0, MODULE_NAME, &fopers);
	if( major < 0 )
	{
		errorP("Register fail, major %d", major);
		goto reg_fail;
	}

	debug("Module initialize success");
	return 0;

open_fail:
reg_fail:
	dev_close();
	return (-EBUSY);
}


static void chardevFile_exit(void) {
	dev_close();
	unregister_chrdev(major, MODULE_NAME);
	debug("Module destruct success");
}

module_init(chardevFile_init);
module_exit(chardevFile_exit);
