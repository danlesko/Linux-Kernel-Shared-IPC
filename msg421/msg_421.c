#include <linux/kernel.h>

asmlinkage long send_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
	/* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

asmlinkage long recv_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
	/* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

asmlinkage long peek_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
	/* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

asmlinkage long count_msg_421(unsigned long id) {
	/* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

asmlinkage long len_msg_421(unsigned long id) {
	/* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}