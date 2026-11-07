#ifndef __LANGSU_H_SHELL_SU_ADB_ONLY
#define __LANGSU_H_SHELL_SU_ADB_ONLY

#include <linux/types.h>

// Extra check for current running as shell, it never grants root by itself:
// shell still has to be allowed by the allowlist. Always true when the feature
// is disabled; otherwise current must descend from adbd or a langsu domain process.
bool langsu_shell_su_allowed_for_current(void);

void langsu_shell_su_adb_only_init(void);

void langsu_shell_su_adb_only_exit(void);

#endif
