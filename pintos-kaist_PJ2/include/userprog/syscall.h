#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

struct lock filesys_lock;

void syscall_init (void);

#endif /* userprog/syscall.h */

#ifndef VM
void check_address(void *addr);
#else
struct page* check_address(void *addr);
#endif

