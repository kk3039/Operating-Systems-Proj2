#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

//int uid;
//kuid_t reg_u=1000;
unsigned long **sys_call_table;
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_open)(const char __user *filename,int flags, umode_t mode);
asmlinkage long (*ref_sys_close)(int fildes);
asmlinkage long (*ref_sys_read)(int fd, void *buf, size_t count);

//example
asmlinkage long new_sys_cs3013_syscall1(void) {
	printk(KERN_INFO "\"’Hello world?!’ More like ’Goodbye, world!’ EXTERMINATE!\" -- Dalek");
	return 0;
}

//new read
asmlinkage long new_sys_read(int fd, void *buf, size_t count) {
	int uid=current_uid().val;
	int ret;
	char * virus="virus";
	ret=ref_sys_read(fd,buf,count);
	if(ret>0){
		void * str_ret=strstr((char *)buf,virus);
		if(str_ret){
			printk(KERN_INFO "User is %d reading from file descriptor %d, but the read contains a virus!",uid,fd);
		}
	}
	return ret;
}

//new close
asmlinkage long new_sys_close(int fildes) {
int uid=current_uid().val;
	if(uid>=1000){
		printk(KERN_INFO "user %d is closing file descriptor: %d",uid,fildes);
	}
		return ref_sys_close(fildes);

}

//new open
asmlinkage long new_sys_open(const char __user *filename,int flags, umode_t mode) {
	int uid=current_uid().val;
	if(uid>=1000){
		printk(KERN_INFO "user %d is openning file: %s",uid,filename);
	}
		return ref_sys_open(filename, flags,mode);

}

static unsigned long **find_sys_call_table(void) {
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;
	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;
		if (sct[__NR_close] == (unsigned long *) sys_close) {
			printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX",
			(unsigned long) sct);
			return sct;
		}
		offset += sizeof(void *);
	}
	return NULL;
}

static void disable_page_protection(void) {

	write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
	write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
	

	if(!(sys_call_table = find_sys_call_table())) {
		return -1;
	}
	ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
	ref_sys_open = (void *)sys_call_table[__NR_open];
	ref_sys_close = (void *)sys_call_table[__NR_close];
	ref_sys_read = (void *)sys_call_table[__NR_read];
	disable_page_protection();
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
	sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)new_sys_close;
	sys_call_table[__NR_read] = (unsigned long *)new_sys_read;
	enable_page_protection();
	printk(KERN_INFO "Loaded interceptor!");
	return 0;
}

static void __exit interceptor_end(void) {
	if(!sys_call_table)
	return;
	disable_page_protection();
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
	sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
	sys_call_table[__NR_read] = (unsigned long *)ref_sys_read;
	enable_page_protection();
	printk(KERN_INFO "Unloaded interceptor!");
}


MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);