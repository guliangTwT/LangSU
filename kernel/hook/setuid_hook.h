#ifndef __LANGSU_H_LANGSU_CORE
#define __LANGSU_H_LANGSU_CORE

#include <linux/init.h>
#include <linux/types.h>

void langsu_setuid_hook_init(void);
void langsu_setuid_hook_exit(void);

// Handler functions for hook_manager
int langsu_handle_setresuid(uid_t old_uid, uid_t new_uid);

#endif
