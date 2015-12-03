/*Ying LU ylu6@wpi.edu
* Qiaoyu LIAO qliao@wpi.edu
*/

#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
// These values MUST match the unistd_32.h modifications:
#define __NR_cs3013_syscall1 355
#define __NR_cs3013_syscall2 356
#define __NR_cs3013_syscall3 357

long testCall1 ( void) {
	return (long) syscall(__NR_cs3013_syscall1);
}

long testCall2 ( unsigned short *target_uid,int *num_pids_smited, int *smited_pids, long *pid_states) {
	return (long) syscall(__NR_cs3013_syscall2,target_uid,num_pids_smited, smited_pids,pid_states);
}

long testCall3 ( int *num_pids_smited, int *smited_pids, long *pid_states) {
	return (long) syscall(__NR_cs3013_syscall3,num_pids_smited,smited_pids,pid_states);
}


int main(){

	int numSmited2=0;
	int smitedPids2[100];
	long pidState2[100];
	int index;
	scanf("%d ",&numSmited2);
	if(numSmited2>0){


		for(index=0;index<numSmited2;index++){
			scanf("%d %ld ",smitedPids2+index,pidState2+index);
			//printf("at %d pid is %d, state is %ld\n",index,smitedPids2[index],pidState2[index]);

		}
	}

	printf("\nfor target 1001 The return values of the unsmite are:\n");
	// printf("\tcs3013_syscall1: %ld\n", testCall1());
	// test2=testCall2(&target2,&numSmited2,smitedPids2,pidState2);
	// printf("\tcs3013_syscall2: %ld\n", test2);
	// // if(test2==0){
	printf("\tcs3013_syscall3: %ld\n", testCall3(&numSmited2,smitedPids2,pidState2));


	
	return 0;
}
