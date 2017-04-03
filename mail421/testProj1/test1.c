#include <stdio.h>
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

long send_msg_syscall(unsigned long id, unsigned char* msg, long n, unsigned long key){
	return syscall(__NR_send_msg, id, msg, n, key);
}

long count_msg_syscall(unsigned long id){
	return syscall(__NR_count_msg, id);
}

long peek_msg_syscall(unsigned long id, unsigned char * msg, long n, unsigned long key){
	return syscall(__NR_peek_msg, id, msg, n, key);
}

int main(int argc, char* argv[]){
	create_mbox_syscall(1, 1, 1);
	count_mbox_syscall();

	char * msg1 = "Test Message";
	long stringLength = strlen(msg1);
	unsigned long someKey = 22;

	send_msg_syscall(1, msg1, stringLength, someKey);

	

	//create_mbox_syscall(2, 1, 1);
	
	//delete_mbox_syscall(2);

	//create_mbox_syscall(3, 1, 1);
	//create_mbox_syscall(4, 1, 1);

	//delete_mbox_syscall(4);

	



}

	// char * newMsg1 = "Does this work?";
	// long stringLength1 = strlen(newMsg1);
	// unsigned long somekey = 22;

	// send_msg_syscall(3, newMsg1, stringLength1, somekey);

	// long numMsgs3 = count_msg_syscall(3);

	// printf("There are %ld messages in that mailbox!\n", numMsgs3);

	// char * newMsg2 = "Does that work?";
	// long stringLength2 = strlen(newMsg2);
	// somekey = 22;

	// send_msg_syscall(3, newMsg2, stringLength2, somekey);

	// numMsgs3 = count_msg_syscall(3);

	// printf("There are %ld messages in that mailbox!\n", numMsgs3);

	// char * oldMsg;
	// peek_msg_syscall(3, oldMsg, stringLength2, somekey);

	// printf("The old message is: %s", oldMsg);