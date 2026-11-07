#ifndef __LANGSU_H_SELINUX_HIDE
#define __LANGSU_H_SELINUX_HIDE

void langsu_selinux_hide_init();
void langsu_selinux_hide_exit();
void langsu_selinux_hide_drop_backup_if_unused();
void langsu_selinux_hide_handle_second_stage();
void langsu_selinux_hide_handle_post_fs_data();

#endif
