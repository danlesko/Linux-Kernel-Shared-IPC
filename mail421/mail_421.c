#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/rwsem.h>

// Creation of msg node in linked list
struct msg_421 {
    char* msg;

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

// www.makelinux.net/ldd3/chp-5-sect-3
static DECLARE_RWSEM(lock);

long bitXOR(long x, long y){
    return x ^ y;
}

// works as intended
// memory is freed correctly as well
// function done
void convertArray(char * msg, unsigned long key){
    int newN = strlen(msg);
    int i = 0;
    int leftover;

    long * byteArray = kmalloc(newN, GFP_KERNEL);
    memcpy(byteArray, msg, newN);
    //byteArray = (long *)msg;

    printk("In convertArray printing message1: \n%s\n", msg);

    if(newN % 4 != 0){
        leftover = 4 - newN % 4;
        newN += leftover;
    }

    for (i = 0; i<(newN/sizeof(long)); i++){
        byteArray[i] = bitXOR(byteArray[i], key);
    }

    memcpy(msg, byteArray, newN);
    //msg = (char *)byteArray;
    printk("In convertArray printing message2: \n%s\n", msg);

    
    kfree(byteArray);

}

// create a new dynamically created mailbox
// syscall 377
// works as intended thus far
// returns error of ID already in use, otherwise returns 0
asmlinkage long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {

    struct mbox_421 *aNewMailbox, *aMailbox, *tmp;

    down_write(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            printk("Mailbox already exists with that ID! Please try a different ID\n");

            up_write(&lock);

            // We already have a mailbox by that id, return error
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

    
    up_write(&lock);
    return 0;
}

// syscall 378
// works as intended thus far
// if there are messages still in the mailbox, return error
asmlinkage long remove_mbox_421(unsigned long id){

    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    int counter = 0;

    down_write(&lock);

    //printk(KERN_INFO "kernel module unloaded.n");
    printk("Deleting the list using list_for_each_entry_safe\n");
    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){

                if(counter >= 1){
                    up_write(&lock);
                    return EPERM;
                }

                // printk("Freeing msgs...\n");
                // list_del(&aMsg->list);
                // kfree(aMsg);
            }
            printk("Freeing node %ld \n", aMailbox->id);
            list_del(&aMailbox->list);
            kfree(aMailbox);

        }
    }

    // return normally
    up_write(&lock);
    return 0;
}

// syscall 379
// works as intended thus far
asmlinkage long count_mbox_421(void){
    
    struct mbox_421 *aMailbox, *tmp;
    long count_mboxes = 0;

    down_read(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        count_mboxes += 1;
    }

    printk("There are %ld mailboxes \n", count_mboxes);

    up_read(&lock);
    return count_mboxes;
}

// syscall 380
// works but will return 0 if ID not found
asmlinkage long list_mbox_421(unsigned long *mbxes, long k){

    struct mbox_421 *aMailbox, *tmp;
    long counter = 0;

    down_read(&lock);

    //printk(KERN_INFO "kernel module unloaded.n");
    printk("Iterating through mailboxes to return up to %ld IDs...\n", k);
    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (counter < k){
            mbxes[counter] = aMailbox->id;
        }
        counter += 1;
    }

    
    up_read(&lock);
    if (counter > 0){
        return counter;
    }

    // if no mboxes found return error
    return ENOENT;
}

// syscall 381
// works as intended thus far
asmlinkage long send_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aNewMsg;

    int strCpyCheck;

    down_write(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        long newN = n;
        long leftover = 0;

        if (aMailbox->id == id){
            printk("Saving message into mailbox!\n");
            aNewMsg = (struct msg_421 *)kmalloc(sizeof(*aNewMsg), GFP_KERNEL);

            if(newN % 4 != 0){
                leftover = 4 - newN % 4;
                newN += leftover;
            }

            aNewMsg->msg = kmalloc(sizeof(char)*newN, GFP_KERNEL);
            // aNewMsg->msg = msg;
            strCpyCheck = copy_from_user(aNewMsg->msg, msg, n);
            if (strCpyCheck == -EFAULT){
                printk("Copying from user space failed!");
                return EFAULT;
            }
            else{
                printk("Message received...printing the message: \n");
                printk(aNewMsg->msg);
                printk("\n");
                if(aMailbox->enable_crypt == 1){
                    convertArray(aNewMsg->msg,key);
                }
            }

            if (aMailbox->lifo == 0){
                list_add_tail(&aNewMsg->list, &aMailbox->msgList.list);
            } else {
                list_add(&aNewMsg->list, &aMailbox->msgList.list);
            }

            up_write(&lock);
            return 0;
        }
    }

    up_write(&lock);
    // no such mailbox found
    return ENOENT;
    
}

// syscall 382
// works as intended
asmlinkage long recv_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    int cpyToUserCheck;

    down_write(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                //msg = "Does this work>";
                //strncpy(msg, "Lol?", n);
                if(aMailbox->enable_crypt == 1){
                    convertArray(aMsg->msg,key);
                }

                cpyToUserCheck = copy_to_user(msg, aMsg->msg, n);

                if (cpyToUserCheck == -EFAULT){
                    printk("Copying from kernel space failed!");
                    up_write(&lock);
                    return EFAULT;
                }
                printk("Receiving message... Deleting message... \n");
                list_del(&aMsg->list);
                kfree(aMsg);
                break;
            }
        }
    }

    up_write(&lock);
    // even if no bytes are returned, return 0 because function exited successfully
    return 0;
}

// syscall 383
// works as intended thus far
asmlinkage long peek_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    int cpyToUserCheck;

    down_read(&lock);


    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                //msg = "Does this work>";
                //strncpy(msg, "Lol?", n);
                if(aMailbox->enable_crypt == 1){
                    convertArray(aMsg->msg,key);
                }
                cpyToUserCheck = copy_to_user(msg, aMsg->msg, n);
                if (cpyToUserCheck == -EFAULT){
                    printk("Copying from kernel space failed!");
                    if(aMailbox->enable_crypt == 1){
                        convertArray(aMsg->msg,key);
                    }
                    up_read(&lock);
                    return EFAULT;
                }
                if(aMailbox->enable_crypt == 1){
                    convertArray(aMsg->msg,key);
                }
                break;
            }
        }
    }
    
    up_read(&lock);
    // even if no bytes are returned, return 0 because function exited successfully
    return 0;
}

// syscall 384
// works as intended for now
asmlinkage long count_msg_421(unsigned long id) {

    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    
    long countMsg = 0;

    down_read(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                countMsg += 1;
            }
        }
    }

    up_read(&lock);
    if (countMsg > 0){
        return countMsg;
    }

    // if no messages found return error
    return ENOENT;
}

// syscall 385
// works as intended
asmlinkage long len_msg_421(unsigned long id) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    //int cpyToUserCheck;

    down_read(&lock);


    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
                //msg = "Does this work>";
                //strncpy(msg, "Lol?", n);
                up_read(&lock);
                return strlen(aMsg->msg);
            }
        }
    }
    up_read(&lock);
    // return that there is no message
    return ENOENT;
}

