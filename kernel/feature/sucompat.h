#ifndef __LANGSU_H_SUCOMPAT
#define __LANGSU_H_SUCOMPAT
#include <asm/ptrace.h>
#include <linux/types.h>

extern bool langsu_su_compat_enabled;

void langsu_sucompat_init(void);
void langsu_sucompat_exit(void);

// Handler functions exported for hook_manager
long langsu_handle_faccessat_sucompat(int orig_nr, struct pt_regs *regs);
long langsu_handle_stat_sucompat(int orig_nr, struct pt_regs *regs);
long langsu_handle_execve_sucompat(const char __user **filename_user, int orig_nr, struct pt_regs *regs);
long langsu_handle_execveat_sucompat(const char __user **filename_user, int orig_nr, struct pt_regs *regs);

#endif
