#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <asm/current.h>
#include <asm/errno.h>
#include <asm/uaccess.h>

unsigned long **sys_call_table;
asmlinkage long (*ref_sys_cs3013_syscall1)(void);
//smite
asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_uid,int *num_pids_smited, int *smited_pids, long *pid_states);
//unsmite
asmlinkage long (*ref_sys_cs3013_syscall3)(int *num_pids_smited, int *smited_pids, long *pid_states);

static LIST_HEAD(pidList);
//example
asmlinkage long new_sys_cs3013_syscall1(void) {
	printk(KERN_INFO "\"’Hello world?!’ More like ’Goodbye, world!’ EXTERMINATE!\" -- Dalek");
	return 0;
}

//new smite
asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_uid,int *num_pids_smited, int *smited_pids, long *pid_states) {
	int counter=0;
	long tempstate[100];
	int temppid[100];
	unsigned short temptarget;
	struct task_struct *p;
	if(copy_from_user(&temptarget,target_uid,sizeof(unsigned short))){
		return EFAULT;
	}

	if( temptarget!=0){//executed by root but not smiting root processes
		printk(KERN_INFO "\ttarget is %d\n",temptarget);


		for_each_process(p){//looping thru process list
			if(counter<100){//only for the first 100 processes found
				//printk(KERN_INFO "\tProcess %hu, state %ld\n",p->real_cred->uid.val,p->state);
				if(p->real_cred->uid.val==temptarget && p->state==0){//check uid and runnable state
					temppid[counter]=p->pid;
					tempstate[counter]=p->state;
					p->state=TASK_UNINTERRUPTIBLE; //change the state of runnable to unrunnable
					counter++;
					printk(KERN_INFO "\tProcess %d, %s was altered by user %hu",p->pid,p->comm,p->real_cred->uid.val);
				}//end of uid check
			}//end of if counter<100
			else{
				break;
			}

		}//end of for each process
		if(copy_to_user(smited_pids,temppid,100*sizeof(int))){
			printk(KERN_INFO"\t\tcopy error\n");
			return EFAULT;
		} //store smited pi
		if(copy_to_user(pid_states,tempstate,100*sizeof(long))){
			printk(KERN_INFO"\t\tcopy error\n");
			return EFAULT;
		} //store previous state of smited pid

		if(copy_to_user(num_pids_smited,&counter,sizeof(int))){
			printk(KERN_INFO"\t\tcopy error\n");
			return EFAULT;
		}//stores num pids smited
		return 0;

		
	}//end of root checkt	
	else{
		return EFAULT;
	}
	
}//end of syscall2



//new unsmite
asmlinkage long new_sys_cs3013_syscall3(int *num_pids_smited, int *smited_pids, long *pid_states) {
	long tempstate[100];//temperate variable to store states
	int temppid[100];
	int counter=0;
	int num_processes;
	struct task_struct *p;


	if(copy_from_user(&num_processes,num_pids_smited,sizeof(int))){
		return EFAULT;
	}

	if(num_processes!=0){
		if(copy_from_user(tempstate,pid_states,100*sizeof(long))){
			return EFAULT;
		}


		if(copy_from_user(temppid,smited_pids,100*sizeof(int))){
			printk(KERN_INFO"\t\tcopy error\n");
			return EFAULT;
		} //store smited pid


		for_each_process(p){//looping thru process list
			if(counter<=num_processes){//only for the passed in number of processes 
				if(p->pid==temppid[counter]){//find the smited pid in the processes list
					wake_up_process(p);
					p->state=tempstate[counter]; //change the state to previous state
					counter++;
					printk(KERN_INFO "\tProcess %d, %s was altered by user %hu",p->pid,p->comm,current_uid().val);
				}//end of pid check
			}//end of if counter check
			else{
				break;
			}
		}//end of for each process
		//kfree(p);
		return 0;

	}	
	else{
		return EFAULT;
	}
}


static unsigned long **find_sys_call_table(void) {
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;
	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;
		if (sct[__NR_close] == (unsigned long *) sys_close) {
			printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX\n",
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

	printk(KERN_INFO "before ref=sys\n");
	ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
	ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
	ref_sys_cs3013_syscall3 = (void *)sys_call_table[__NR_cs3013_syscall3];
	disable_page_protection();
	printk(KERN_INFO "after disable\n");
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
	sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)new_sys_cs3013_syscall3;
	enable_page_protection();
	printk(KERN_INFO "Loaded interceptor!");
	return 0;
}

static void __exit interceptor_end(void) {
	if(!sys_call_table)
	return;
	disable_page_protection();
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
	sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)ref_sys_cs3013_syscall3;
	enable_page_protection();
	printk(KERN_INFO "Unloaded interceptor!");
}


MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);