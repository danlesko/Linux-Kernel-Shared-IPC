#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

#define __NR_create_mbox 377
#define __NR_delete_mbox 378
#define __NR_count_mbox 379
#define __NR_list_mbox 380
#define __NR_send_msg 381
#define __NR_recv_msg 382
#define __NR_peek_msg 383
#define __NR_count_msg 384
#define __NR_len_msg 385

long create_mbox_syscall(unsigned long id, int enable_crypt, int lifo){
	return syscall(__NR_create_mbox, id, enable_crypt, lifo);
}

long delete_mbox_syscall(unsigned long id){
	return syscall(__NR_delete_mbox, id);
}

long count_mbox_syscall(void){
	return syscall(__NR_count_mbox);
}

long list_mbox_syscall(unsigned long *mbxes, long k){
	return syscall(__NR_list_mbox, mbxes, k);
}

long send_msg_syscall(unsigned long id, unsigned char* msg, long n, unsigned long key){
	return syscall(__NR_send_msg, id, msg, n, key);
}

long count_msg_syscall(unsigned long id){
	return syscall(__NR_count_msg, id);
}

long peek_msg_syscall(unsigned long id, unsigned char * msg, long n, unsigned long key){
	return syscall(__NR_peek_msg, id, msg, n, key);
}

long recv_msg_syscall(unsigned long id, unsigned char * msg, long n, unsigned long key){
	return syscall(__NR_recv_msg, id, msg, n, key);
}

long len_msg_syscall(unsigned long id){
	return syscall(__NR_len_msg, id);
}

int main(int argc, char* argv[]){

	// syscall 377 example
	if(create_mbox_syscall(5, 1, 1) < 0){
		printf("Error creating mailbox!\n");
	}

	char * msg1 = "Hows it going yall?";
	char * msg2 = "Does this work?";
	long stringLength1 = strlen(msg1);
	long stringLength2 = strlen(msg2);
	unsigned long someKey1 = 2222;
	unsigned long someKey2 = 5555;


	// syscall 381 example
	send_msg_syscall(5, msg1, stringLength1, someKey1);
	send_msg_syscall(5, msg2, stringLength2, someKey2);


	char * oldMsg1 = malloc(stringLength1+1 * sizeof(char));
	char * oldMsg2 = malloc(stringLength1+1 * sizeof(char));

	// syscall 385
	// Will return 15, the number of chars in msg2
	int msg2Length = len_msg_syscall(5);
	if (msg2Length < 0){
		printf("Error grabbing message length!\n");
	}
	else{
		printf("First message length: %d\n", msg2Length);
	}

	// syscall 382
	recv_msg_syscall(5, oldMsg2, stringLength2, someKey2);

	printf("Printing message: \n");
	printf(oldMsg2);
	printf("\n");

	// syscall 384
	int numMsgs = count_msg_syscall(5);
	if (numMsgs < 0){
		numMsgs = 0;
	}
	printf("After receiving 1 message, there is %d message left\n", numMsgs);

	// syscall 383
	peek_msg_syscall(5, oldMsg1, stringLength1, someKey1);

	printf("Printing message: \n");
	printf(oldMsg1);
	printf("\n");

	// syscall 384
	numMsgs = count_msg_syscall(5);
	if (numMsgs < 0){
		numMsgs = 0;
	}
	printf("After peeking 1 message, there is %d message left\n", numMsgs);

	// syscall 382
	recv_msg_syscall(5, oldMsg1, stringLength1, someKey1);

	printf("Printing message: \n");
	printf(oldMsg1);
	printf("\n");

	// syscall 384
	numMsgs = count_msg_syscall(5);
	if (numMsgs < 0){
		numMsgs = 0;
	}
	printf("After receiving 1 message, there is %d message left\n", numMsgs);

	free(oldMsg1);
	free(oldMsg2);

	return 0;
}
