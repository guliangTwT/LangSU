#ifndef __LANGSU_H_HOOK_MANAGER
#define __LANGSU_H_HOOK_MANAGER

#include <asm/ptrace.h>

// Hook manager initialization and cleanup
void langsu_syscall_hook_manager_init(void);
void langsu_syscall_hook_manager_exit(void);

#endif
