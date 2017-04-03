#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/uaccess.h>

// Creation of msg node in linked list
struct msg_421 {
    char* msg;
    //long n;
    //unsigned long key;

    struct list_head list;

};

// Creation of mailbox node in linked list. 
struct mbox_421 {
    long id;
    int enable_crypt;
    int lifo;

    struct msg_421 msgList;
    struct list_head list;

};

LIST_HEAD(mbox_421_list);

// create a new dynamically created mailbox
// syscall 377
asmlinkage long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {

    struct mbox_421 *aNewMailbox, *aMailbox, *tmp;

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            printk("Mailbox already exists with that ID! Please try a different ID\n");
            return EADDRINUSE;
        }
    }

    printk("Creating new mailbox with ID %ld\n", id);
    aNewMailbox = (struct mbox_421 *)kmalloc(sizeof(*aNewMailbox), GFP_KERNEL);
    aNewMailbox->id = id;
    aNewMailbox->enable_crypt = enable_crypt;
    aNewMailbox->lifo = lifo;
    INIT_LIST_HEAD(&aNewMailbox->list);
    INIT_LIST_HEAD(&aNewMailbox->msgList.list);
    list_add(&aNewMailbox->list, &mbox_421_list);

    

    return 0;
}

// syscall 378
asmlinkage long remove_mbox_421(unsigned long id){
    /* Placeholder to be defined later */
    //printk("Hello World!\n");

    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    //printk(KERN_INFO "kernel module unloaded.n");
    printk("Deleting the list using list_for_each_entry_safe\n");
    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                printk("Freeing msgs...\n");
                list_del(&aMailbox->msgList.list);
                kfree(aMsg);
            }
            printk("Freeing node %ld \n", aMailbox->id);
            list_del(&aMailbox->list);
            kfree(aMailbox);
        }
    }

    return 0;
}

// syscall 379
asmlinkage long count_mbox_421(void){
    
    struct mbox_421 *aMailbox, *tmp;
    long count_mboxes = 0;

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        count_mboxes += 1;
    }

    printk("There are %ld mailboxes \n", count_mboxes);
    return count_mboxes;
}

// syscall 380
asmlinkage long list_mbox_421(unsigned long *mbxes, long k){
    /* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

// syscall 381
asmlinkage long send_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aNewMsg;

    int strCpyCheck;


    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            printk("Saving message into mailbox!\n");
            aNewMsg = (struct msg_421 *)kmalloc(sizeof(*aNewMsg), GFP_KERNEL);
            aNewMsg->msg = kmalloc(sizeof(char)*n, GFP_KERNEL);
            // aNewMsg->msg = msg;
            strCpyCheck = copy_from_user(aNewMsg->msg, msg, n);
            if (strCpyCheck == -EFAULT){
                printk("Copying from user space failed!");
                return EFAULT;
            }
            else{
                printk("Message received...: \n");
                //printk(aNewMsg->msg);
            }

            if (aMailbox->lifo == 0){
                list_add_tail(&aNewMsg->list, &aMailbox->msgList.list);
            } else {
                list_add(&aNewMsg->list, &aMailbox->msgList.list);
            }

            return 0;
        }
    }

    return 1;
    
}

// syscall 382
asmlinkage long recv_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    /* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}

// syscall 383
asmlinkage long peek_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;


    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                //msg = "Does this work>";
                //strncpy(msg, "Lol?", n);
                copy_to_user(msg, aMsg->msg, n);
                break;
            }
        }
    }
    return 0;
}

// syscall 384
asmlinkage long count_msg_421(unsigned long id) {

    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    
    long countMsg = 0;

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                countMsg += 1;
            }
        }
    }

    return countMsg;
}

// syscall 385
asmlinkage long len_msg_421(unsigned long id) {
    /* Placeholder to be defined later */
    printk("Hello World!\n");
    return 0;
}