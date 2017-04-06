# Asynchronous Message Passing In Linux Kernel

### Overview
This project required us to come up with a way to create mailboxes and allow processes to send and receive messages to and from those mailboxes. To implement this in kernel space, I decided to make use of a linked list of linked lists. The first list would be the mailbox layer, with messages added to the given mailbox in the form of a list as well. In order to allow for messages to be stored in either a stack or queue (LIFO or FIFO), I made use the doubly linked list library <linux/list.h> in order to make my life a bit easier. This library allows you to add nodes to either the beginning or the end of an existing list, so creating the mailboxes as either stacks or queues was trivial. Various examples were pulled from different sources in order to implement the lists correctly. The class also posted questions on a group forum, Piazza, in order to better assist one another. 


### Core Functionality
The requirements for this project included defining a couple key functions for our mailboxes and messages:
1) `long create_mbox_421(unsigned long id, int enable_crypt, int lifo)`: creates a new empty mailbox with ID id, if it does not already exist, and returns 0. The queue should be flagged for encryption if the enable_crypt option is set to anything other than 0. If enable_crypt is set to zero, then the key parameter in any functions including it should be ignored. The lifo parameter controls what direction the messages are retrieved in. If this parameter is 0, then the messages should be stored/retrieved in FIFO order (as a queue). If it is non-zero, then the messages should be stored in LIFO order (as a stack).
2) `long remove_mbox_421(unsigned long id)`: removes mailbox with ID id, if it is empty, and returns 0. If the mailbox is not empty, this system call should return an appropriate error and not remove the mailbox.
3) `long count_mbox_421(void)`: returns the number of existing mailboxes.
4) `long list_mbox_421(unsigned long *mbxes, long k)`: returns a list of up to k mailbox IDs in the user-space variable mbxes. It returns the number of IDs written successfully to mbxes on success and an appropriate error code on failure.
5) `long send_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key)`: encrypts the message msg (if appropriate), adding it to the already existing mailbox identified. Returns the number of bytes stored (which should be equal to the message length n) on success, and an appropriate error code on failure. Messages with negative lengths shall be rejected as invalid and cause an appropriate error to be returned.
6) `long recv_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key)`: copies up to n characters from the next message in the mailbox id to the user-space buffer msg, decrypting with the specified key (if appropriate), and removes the entire message from the mailbox (even if only part of the message is copied out). Returns the number of bytes successfully copied (which should be the minimum of the length of the message that is stored and n) on success or an appropriate error code on failure.
7) `long peek_msg_421(unsigned long id, unsigned char *msg, long n, unsigned long key)`: performs the same operation as recv_msg_421() without removing the message from the mailbox.
8) `long count_msg_421(unsigned long id)`: returns the number of messages in the mailbox id on success or an appropriate error code on failure.
9) `long len_msg_421(unsigned long id)`: returns the lenth of the next message that would be returned by calling recv_msg_421() with the same id value (that is the number of bytes in the next message in the mailbox). If there are no messages in the mailbox, this should return an appropriate error value.

### Return Values
Throughout the project, various return values were needed to indicate success or error. If a success occurred, a nonnegative integer would be returned. For errors, we would return a specific error code outlined in <linux/errno.h>. There are used as such:
1) `EPERM` : Error sent back if user was not root
2) `EADDRINUSE` : Error sent back if a mailbox of a given ID has already been created
3) `EACCES` : Error sent back if an operation was not allowed, such as deleting a mailbox with messages still inside
4) `ENOENT` : Error sent back if a mailbox was not found
5) `EFAULT` : Error sent back if a NULL pointer was passed
6) `ENOBUFS` : Error sent back if message is of a negative length

### Locks
I wished to use a reader writer lock in my kernel program. The implementation I found was the Reader/Writer Semaphore which I was hinted at by Professor Sebald. This type of lock allows programs to read messages / count messages and mailboxes by as many processes as needed, but locks the entire structure down whenever a create or remove method is called by a process. This allows for safe handling of mailboxes and methods and prevents any overflow or pointer errors by preventing processes from creating / destroying when others are reading.

### Privileges
This capability was also fairly simple to implement. I found the linux function `capable()` to be quite handy as it allows a given function to determine who was the caller. When used correctly, we can return from the function if an user has the incorrect privileges.

### XOR Cipher
I have 2 additional functions in my file, one called bitXOR and one called convertArray. convertArray takes in msg, converts it to a byteArray, XORS the bits, and casts byteArray back to msg as a character array. When sending a message, I take in the message, and pass it to my convertArray function before storing it. For methods like peek message, I first call convertArray to unencrypt the stored msg, copy it, and then re-encrypt. While this method might not be the best option as the message is temporarily unencrypted, it is unencrypted in kernel space for a brief period in time before re-encrypting.

### References
- Class lectures
- Class slides
- Class Piazza
- http://www.makelinux.net/ldd3/chp-5-sect-3 (read / write semaphores)
- http://www.makelinux.net/ldd3/chp-6-sect-1 (capable function)
- http://www.makelinux.net/ldd3/chp-11-sect-5 (linked list)
- http://www.roman10.net/2011/07/28/linux-kernel-programminglinked-list/ (linked list)
- http://www.fsl.cs.sunysb.edu/kernel-api/re256.html (copy_to_user)
- http://www.fsl.cs.sunysb.edu/kernel-api/re257.html (copy_from_user)
