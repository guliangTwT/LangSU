#ifndef __LANGSU_H_LANGSUD
#define __LANGSU_H_LANGSUD

#include <asm/syscall.h>

#define LANGSUD_PATH "/data/adb/langsud"

void langsu_langsud_init();
void langsu_langsud_exit();

void langsu_execve_hook_langsud(const struct pt_regs *regs);
void langsu_execveat_hook_langsud(const struct pt_regs *regs);
void langsu_stop_input_hook_runtime(void);

#endif
