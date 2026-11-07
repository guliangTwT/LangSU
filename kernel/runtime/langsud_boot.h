#ifndef __LANGSU_H_LANGSUD_BOOT
#define __LANGSU_H_LANGSUD_BOOT

#include <linux/types.h>

void on_post_fs_data(void);
void on_module_mounted(void);
void on_boot_completed(void);

bool langsu_is_safe_mode(void);

int nuke_ext4_sysfs(const char *mnt);

extern bool langsu_module_mounted;
extern bool langsu_boot_completed;

#endif // __LANGSU_H_LANGSUD_BOOT
