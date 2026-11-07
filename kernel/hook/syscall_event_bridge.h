#ifndef __LANGSU_H_SYSCALL_EVENT_BRIDGE
#define __LANGSU_H_SYSCALL_EVENT_BRIDGE

#include <asm/ptrace.h>

long langsu_hook_newfstatat(int orig_nr, const struct pt_regs *regs);
long langsu_hook_faccessat(int orig_nr, const struct pt_regs *regs);
long langsu_hook_execve(int orig_nr, const struct pt_regs *regs);
long langsu_hook_execveat(int orig_nr, const struct pt_regs *regs);
long langsu_hook_setresuid(int orig_nr, const struct pt_regs *regs);

void langsu_stop_langsud_execve_hook(void);

#endif // __LANGSU_H_SYSCALL_EVENT_BRIDGE
