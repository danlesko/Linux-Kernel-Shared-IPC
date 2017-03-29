#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

// Creation of msg node in linked list
struct msg_421 {
    long id;
    char* msg;
    long n;
    unsigned long key;

    struct list_head list;

};

// Creation of mailbox node in linked list. 
struct mbox_421 {
    int id;
    int enable_crypt;
    int lifo;

    struct msg_421 *nextMsg;
    struct list_head list;

};

// static struct msg_421 msg_421_list;
// LIST_HEAD_INIT(&msg_421.list);

static struct mbox_421 mbox_421_list;

// create a new dynamically created mailbox
asmlinkage long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {
    /* Placeholder to be defined later */
    //printk("Hello World!\n");
    struct mbox_421 *aNewMailbox;
    //struct mbox_421 *aMailbox;

    // will need to check if this has already been initialized?
    INIT_LIST_HEAD(&mbox_421_list.list);

    aNewMailbox = (struct mbox_421 *)kmalloc(sizeof(*aNewMailbox), GFP_KERNEL);
    aNewMailbox->id = id;
    aNewMailbox->enable_crypt = enable_crypt;
    aNewMailbox->lifo = lifo;
    INIT_LIST_HEAD(&aNewMailbox->list);

    list_add_tail(&(aNewMailbox->list), &(mbox_421_list.list));

    printk("I wonder if this will work... LOL!\n");

    return 0;
}

asmlinkage long remove_mbox_421(unsigned long id){
    /* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

asmlinkage void count_mbox_421(void){
    /* Placeholder to be defined later */
    printk("Hello World!\n");
    //return 0;
}

asmlinkage long list_mbox_421(unsigned long *mbxes, long k){
    /* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

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