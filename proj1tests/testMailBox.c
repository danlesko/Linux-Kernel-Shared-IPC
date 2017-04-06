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

	// Syscall 377 example
	// Attempt to create 4 mailboxes with IDs 1, 2, 2, and 3
	if(create_mbox_syscall(1, 0, 0) < 0){
		printf("Error creating mailbox!\n");
	}
	if(create_mbox_syscall(2, 1, 1) < 0){
		printf("Error creating mailbox!\n");
	}
	if(create_mbox_syscall(2, 0, 0) < 0){
		printf("Error creating mailbox!\n");
	}
	if(create_mbox_syscall(3, 0, 0) < 0){
		printf("Error creating mailbox!\n");
	}

	// Syscall 378 example
	// Delete Mailbox at with ID 3
	if(delete_mbox_syscall(3) < 0){
		printf("Error deleting mailbox!\n");
	}

	// Get list of mailboxes
	// Should only return 1 and 2
	unsigned long *mbxes;

	// Syscall 379
	long k = count_mbox_syscall();
	mbxes = malloc(k * sizeof(unsigned long));

	// Set all values to a negative value initially
	int i = 0;
	for (i = 0; i< k; i++){
		mbxes[i] = -1;
	}

	// Syscall 380 example
	if(list_mbox_syscall(mbxes, k) < 0){
		printf("Error obtaining list of mailboxes!\n");
	}

	// Only print out values that do not have a negative value, in this case 2 and 1
	for (i = 0; i< k; i++){
		if(mbxes[i] != -1){
			printf("ID Found: %ld \n", mbxes[i]);
		}
	}

	return 0;
}
