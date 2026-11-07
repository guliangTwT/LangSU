#ifndef __LANGSU_H_ADB_ROOT
#define __LANGSU_H_ADB_ROOT
#include <asm/ptrace.h>

long langsu_adb_root_handle_execve(struct pt_regs *regs);
long langsu_adb_root_handle_execveat(struct pt_regs *regs);

void langsu_adb_root_init(void);

void langsu_adb_root_exit(void);

#endif
