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

// Function takes in msg, copies it to an array of bytes and does XOR operation, and then copies it back to msg
void convertArray(char * msg, unsigned long key){
    int newN = strlen(msg);
    int i = 0;
    int leftover;

    long * byteArray = kmalloc(newN, GFP_KERNEL);
    memcpy(byteArray, msg, newN);

    printk("In convertArray printing message1: \n%s\n", msg);

    if(newN % 4 != 0){
        leftover = 4 - newN % 4;
        newN += leftover;
    }

    for (i = 0; i<(newN/sizeof(long)); i++){
        byteArray[i] = bitXOR(byteArray[i], key);
    }

    memcpy(msg, byteArray, newN);
    printk("In convertArray printing message2: \n%s\n", msg);
    
    kfree(byteArray);

}

// Syscall 377
// Creates a new empty mailbox with ID id, if it does not already exist, and returns 0.
// The queue should be flagged for encryption if the enable_crypt option is set to anything other than 0.
// If enable_crypt is set to zero, then the key parameter in any functions including it should be ignored.
// The lifo parameter controls what direction the messages are retrieved in. If this parameter is 0, then the messages should be stored/retrieved in FIFO order (as a queue).
// If it is non-zero, then the messages should be stored in LIFO order (as a stack).
asmlinkage long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {

    struct mbox_421 *aNewMailbox, *aMailbox, *tmp;

    // checks to see if user is root
    if(!capable(CAP_SYS_ADMIN)){
        printk("You must be root to do that!\n");
        return -EPERM;
    }

    down_write(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            printk("Mailbox already exists with that ID! Please try a different ID\n");

            up_write(&lock);

            // We already have a mailbox by that id, return error
            return -EADDRINUSE;
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

// Syscall 378
// Removes mailbox with ID id, if it is empty, and returns 0. 
// If the mailbox is not empty, this system call should return an appropriate error and not remove the mailbox.
asmlinkage long remove_mbox_421(unsigned long id){

    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    int counter = 0;

    // checks to see if user is root
    if(!capable(CAP_SYS_ADMIN)){
        printk("You must be root to do that!");
        return -EPERM;
    }

    down_write(&lock);

    printk("Deleting the list using list_for_each_entry_safe\n");
    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){

                // if we find a message, return error
                if(counter >= 1){
                    up_write(&lock);
                    return -EACCES;
                }

                counter += 1;
                // This was code to delete all messages in mailbox
                // printk("Freeing msgs...\n");
                // list_del(&aMsg->list);
                // kfree(aMsg);
            }
            printk("Freeing node %ld \n", aMailbox->id);
            list_del(&aMailbox->list);
            kfree(aMailbox);

            // return normally
            up_write(&lock);
            return 0;
        }
    }

    // no mailboxes found
    up_write(&lock);
    return -ENOENT;
}

// Syscall 379
// Returns the number of existing mailboxes.
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

// Syscall 380
// Returns a list of up to k mailbox IDs in the user-space variable mbxes. 
// It returns the number of IDs written successfully to mbxes on success and an appropriate error code on failure.
asmlinkage long list_mbox_421(unsigned long *mbxes, long k){

    struct mbox_421 *aMailbox, *tmp;
    long counter = 0;

    if (mbxes == NULL){
        return -EFAULT;
    }

    down_read(&lock);

    printk("Iterating through mailboxes to return up to %ld IDs...\n", k);
    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (counter < k){
            mbxes[counter] = aMailbox->id;
        }
        counter += 1;
    }

    
    up_read(&lock);

    // if mailbox found, return success
    if (counter > 0){
        return counter;
    }

    // if no mboxes found return error
    return -ENOENT;
}

// Syscall 381
// Encrypts the message msg (if appropriate), adding it to the already existing mailbox identified.
// Returns the number of bytes stored (which should be equal to the message length n) on success, and an appropriate error code on failure. 
// Messages with negative lengths shall be rejected as invalid and cause an appropriate error to be returned.
asmlinkage long send_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aNewMsg;

    int strCpyCheck;

    // return buffer error if message is of negative length
    if (n < 0){
        return -ENOBUFS;
    }

    down_write(&lock);

    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        long newN = n;
        long leftover = 0;

        if (aMailbox->id == id){
            printk("Saving message into mailbox!\n");
            aNewMsg = (struct msg_421 *)kmalloc(sizeof(*aNewMsg), GFP_KERNEL);

            // create buffer divisible by 4
            if(newN % 4 != 0){
                leftover = 4 - newN % 4;
                newN += leftover;
            }

            aNewMsg->msg = kmalloc(sizeof(char)*newN, GFP_KERNEL);
            
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

            // lifo == 0 then create queue
            if (aMailbox->lifo == 0){
                list_add_tail(&aNewMsg->list, &aMailbox->msgList.list);
            } 
            // otherwise create stack
            else {
                list_add(&aNewMsg->list, &aMailbox->msgList.list);
            }

            up_write(&lock);
            return 0;
        }
    }

    up_write(&lock);
    // no such mailbox found
    return -ENOENT;
    
}

// Syscall 382
// Copies up to n characters from the next message in the mailbox id to the user-space buffer msg,
// decrypting with the specified key (if appropriate), and removes the entire message from the mailbox (even if only part of the message is copied out).
// Returns the number of bytes successfully copied (which should be the minimum of the length of the message that is stored and n) on success 
// or an appropriate error code on failure.
asmlinkage long recv_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    int cpyToUserCheck;

    if (n <= 0){
        return -ENOBUFS;
    }

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
                    return -EFAULT;
                }
                printk("Receiving message... Deleting message... \n");
                list_del(&aMsg->list);
                kfree(aMsg);
                break;
            }
        }
    }

    up_write(&lock);
    // return number of bytes successfully copied, which will be N if we get here
    return n;
}

// Syscall 383
// Performs the same operation as recv_msg_421() without removing the message from the mailbox.
asmlinkage long peek_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;
    int cpyToUserCheck;

    if (n <= 0){
        return -ENOBUFS;
    }

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
                    return -EFAULT;
                }
                if(aMailbox->enable_crypt == 1){
                    convertArray(aMsg->msg,key);
                }
                break;
            }
        }
    }
    
    up_read(&lock);
    // return number of bytes successfully read, which will be n if we get here
    return n;
}

// Syscall 384
// Returns the number of messages in the mailbox id on success or an appropriate error code on failure.
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
    return -ENOENT;
}

// Syscall 385
// Returns the lenth of the next message that would be returned by calling recv_msg_421() with the same id value (that is the number of bytes in the next message in the mailbox).
// If there are no messages in the mailbox, this should return an appropriate error value.
asmlinkage long len_msg_421(unsigned long id) {
    struct mbox_421 *aMailbox, *tmp;
    struct msg_421 *aMsg, *tmpMsg;

    down_read(&lock);


    list_for_each_entry_safe(aMailbox, tmp, &mbox_421_list, list){
        if (aMailbox->id == id){
            list_for_each_entry_safe(aMsg, tmpMsg, &aMailbox->msgList.list, list){
            
                
                up_read(&lock);
                return strlen(aMsg->msg);
            }
        }
    }
    up_read(&lock);
    // return that there is no message
    return -ENOENT;
}

