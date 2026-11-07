#ifndef __LANGSU_UAPI_SUPERCALL_H
#define __LANGSU_UAPI_SUPERCALL_H

#include <linux/ioctl.h>
#include <linux/types.h>

#include "uapi/app_profile.h"

// 2: allowlist v4 root profile flags
// 3: scoped su-session driver fd
// 4: add LANGSU_GET_INFO_FLAG_BUNDLED
// 5: add EVENT_SERVICES with a start/skip result
static const __u32 LANGSU_UAPI_VERSION = 5;

/* Magic numbers for reboot hook to install fd.
 * LangSU uses its own values so scanners matching the classic
 * 0xDEADBEEF / 0xCAFEBABE pair cannot recognize the hook. */
static const __u32 LANGSU_INSTALL_MAGIC1 = 0x4C414E47; /* "LANG" */
static const __u32 LANGSU_INSTALL_MAGIC2 = 0x5355A5A5;

struct langsu_become_daemon_cmd {
    __u8 token[65]; /* Input: daemon token (null-terminated) */
};

static const __u32 EVENT_POST_FS_DATA = 1;
static const __u32 EVENT_BOOT_COMPLETED = 2;
static const __u32 EVENT_MODULE_MOUNTED = 3;
static const __u32 EVENT_SERVICES = 4;

static const __u32 LANGSU_GET_INFO_FLAG_LKM = (1U << 0);
static const __u32 LANGSU_GET_INFO_FLAG_MANAGER = (1U << 1);
static const __u32 LANGSU_GET_INFO_FLAG_LATE_LOAD = (1U << 2);
static const __u32 LANGSU_GET_INFO_FLAG_PR_BUILD = (1U << 3);
static const __u32 LANGSU_GET_INFO_FLAG_BUNDLED = (1U << 4);

struct langsu_get_info_cmd {
    __u32 version; /* Output: LANGSU_KERNEL_VERSION */
    __u32 flags; /* Output: LANGSU_GET_INFO_FLAG_* bits */
    __u32 features; /* Output: max feature ID supported */
    __u32 uapi_version; /* Output: LANGSU_UAPI_VERSION */
};

struct langsu_get_info_legacy_cmd {
    __u32 version; /* Output: LANGSU_KERNEL_VERSION */
    __u32 flags; /* Output: LANGSU_GET_INFO_FLAG_* bits */
    __u32 features; /* Output: max feature ID supported */
};

struct langsu_report_event_cmd {
    __u32 event; /* Input: EVENT_POST_FS_DATA, EVENT_BOOT_COMPLETED, etc. */
};

struct langsu_set_sepolicy_cmd {
    __u64 data_len; /* Input: bytes of serialized command payload */
    __aligned_u64 data; /* Input: pointer to serialized payload */
};

struct langsu_sepolicy_cmd_hdr {
    __u32 cmd; /* Input: command type, CMD_* */
    __u32 subcmd; /* Input: command subtype */
};
/*
 * After each langsu_sepolicy_cmd_hdr, command arguments are encoded sequentially as:
 * [u32 len][len bytes][\0], where len excludes the trailing '\0'.
 * len == 0 represents ALL.
 * Argument count is derived from cmd:
 * LANGSU_SEPOLICY_CMD_NORMAL_PERM=4, LANGSU_SEPOLICY_CMD_XPERM=5,
 * LANGSU_SEPOLICY_CMD_TYPE_STATE=1, LANGSU_SEPOLICY_CMD_TYPE=2,
 * LANGSU_SEPOLICY_CMD_TYPE_ATTR=2, LANGSU_SEPOLICY_CMD_ATTR=1,
 * LANGSU_SEPOLICY_CMD_TYPE_TRANSITION=5, LANGSU_SEPOLICY_CMD_TYPE_CHANGE=4,
 * LANGSU_SEPOLICY_CMD_GENFSCON=3.
 */

struct langsu_check_safemode_cmd {
    __u8 in_safe_mode; /* Output: true if in safe mode, false otherwise */
};

/* deprecated */
struct langsu_get_allow_list_cmd {
    __u32 uids[128]; /* Output: array of allowed/denied UIDs */
    __u32 count; /* Output: number of UIDs in array */
    __u8 allow; /* Input: true for allow list, false for deny list */
};

struct langsu_new_get_allow_list_cmd {
    __u16 count; /* Input / Output: number of UIDs in array */
    __u16 total_count; /* Output: total number of UIDs in requested list */
    __u32 uids[0]; /* Output: array of allowed/denied UIDs */
};

struct langsu_uid_granted_root_cmd {
    __u32 uid; /* Input: target UID to check */
    __u8 granted; /* Output: true if granted, false otherwise */
};

struct langsu_uid_should_umount_cmd {
    __u32 uid; /* Input: target UID to check */
    __u8 should_umount; /* Output: true if should umount, false otherwise */
};

struct langsu_get_manager_appid_cmd {
    __u32 appid; /* Output: manager app id */
};

struct langsu_get_app_profile_cmd {
    struct app_profile profile; /* Input/Output: app profile structure */
};

struct langsu_set_app_profile_cmd {
    struct app_profile profile; /* Input: app profile structure */
};

struct langsu_get_feature_cmd {
    __u32 feature_id; /* Input: feature ID (enum langsu_feature_id) */
    __u64 value; /* Output: feature value/state */
    __u8 supported; /* Output: LANGSU_FEATURE_FLAG_* bits, 0 if feature is not supported */
};

#define LANGSU_FEATURE_FLAG_SUPPORTED (1 << 0)
/* Value is forced by boot config (force_feature param), SET_FEATURE is refused */
#define LANGSU_FEATURE_FLAG_FORCED (1 << 1)

struct langsu_set_feature_cmd {
    __u32 feature_id; /* Input: feature ID (enum langsu_feature_id) */
    __u64 value; /* Input: feature value/state to set */
};

struct langsu_get_wrapper_fd_cmd {
    __u32 fd; /* Input: userspace fd */
    __u32 flags; /* Input: flags of userspace fd */
};

struct langsu_manage_mark_cmd {
    __u32 operation; /* Input: LANGSU_MARK_* */
    __s32 pid; /* Input: target pid (0 for all processes) */
    __u32 result; /* Output: for get operation - mark status or reg_count */
};

static const __u32 LANGSU_MARK_GET = 1;
static const __u32 LANGSU_MARK_MARK = 2;
static const __u32 LANGSU_MARK_UNMARK = 3;
static const __u32 LANGSU_MARK_REFRESH = 4;

struct langsu_nuke_ext4_sysfs_cmd {
    __aligned_u64 arg; /* Input: mnt pointer */
};

struct langsu_add_try_umount_cmd {
    __aligned_u64 arg; /* char ptr, this is the mountpoint */
    __u32 flags; /* this is the flag we use for it */
    __u8 mode; /* denotes what to do with it 0:wipe_list 1:add_to_list 2:delete_entry */
};

struct langsu_get_sulog_fd_cmd {
    __u32 flags; /* Input: reserved for future use, must be 0 */
};

static const __u8 LANGSU_UMOUNT_WIPE = 0; /* ignore everything and wipe list */
static const __u8 LANGSU_UMOUNT_ADD = 1; /* add entry (path + flags) */
static const __u8 LANGSU_UMOUNT_DEL = 2; /* delete entry, strcmp */

/* IOCTL command definitions */
static const __u32 LANGSU_IOCTL_GRANT_ROOT = _IOC(_IOC_NONE, 'K', 1, 0);
static const __u32 LANGSU_IOCTL_GET_INFO = _IOR('K', 2, struct langsu_get_info_cmd);
/* deprecated */
static const __u32 LANGSU_IOCTL_GET_INFO_LEGACY = _IOC(_IOC_READ, 'K', 2, 0);
static const __u32 LANGSU_IOCTL_REPORT_EVENT = _IOC(_IOC_WRITE, 'K', 3, 0);
static const __u32 LANGSU_IOCTL_SET_SEPOLICY = _IOC(_IOC_READ | _IOC_WRITE, 'K', 4, 0);
static const __u32 LANGSU_IOCTL_CHECK_SAFEMODE = _IOC(_IOC_READ, 'K', 5, 0);
/* deprecated */
static const __u32 LANGSU_IOCTL_GET_ALLOW_LIST = _IOC(_IOC_READ | _IOC_WRITE, 'K', 6, 0);
/* deprecated */
static const __u32 LANGSU_IOCTL_GET_DENY_LIST = _IOC(_IOC_READ | _IOC_WRITE, 'K', 7, 0);
static const __u32 LANGSU_IOCTL_NEW_GET_ALLOW_LIST = _IOWR('K', 6, struct langsu_new_get_allow_list_cmd);
static const __u32 LANGSU_IOCTL_NEW_GET_DENY_LIST = _IOWR('K', 7, struct langsu_new_get_allow_list_cmd);
static const __u32 LANGSU_IOCTL_UID_GRANTED_ROOT = _IOC(_IOC_READ | _IOC_WRITE, 'K', 8, 0);
static const __u32 LANGSU_IOCTL_UID_SHOULD_UMOUNT = _IOC(_IOC_READ | _IOC_WRITE, 'K', 9, 0);
static const __u32 LANGSU_IOCTL_GET_MANAGER_APPID = _IOC(_IOC_READ, 'K', 10, 0);
static const __u32 LANGSU_IOCTL_GET_APP_PROFILE = _IOC(_IOC_READ | _IOC_WRITE, 'K', 11, 0);
static const __u32 LANGSU_IOCTL_SET_APP_PROFILE = _IOC(_IOC_WRITE, 'K', 12, 0);
static const __u32 LANGSU_IOCTL_GET_FEATURE = _IOC(_IOC_READ | _IOC_WRITE, 'K', 13, 0);
static const __u32 LANGSU_IOCTL_SET_FEATURE = _IOC(_IOC_WRITE, 'K', 14, 0);
static const __u32 LANGSU_IOCTL_GET_WRAPPER_FD = _IOC(_IOC_WRITE, 'K', 15, 0);
static const __u32 LANGSU_IOCTL_MANAGE_MARK = _IOC(_IOC_READ | _IOC_WRITE, 'K', 16, 0);
static const __u32 LANGSU_IOCTL_NUKE_EXT4_SYSFS = _IOC(_IOC_WRITE, 'K', 17, 0);
static const __u32 LANGSU_IOCTL_ADD_TRY_UMOUNT = _IOC(_IOC_WRITE, 'K', 18, 0);
static const __u32 LANGSU_IOCTL_SET_INIT_PGRP = _IO('K', 19);
static const __u32 LANGSU_IOCTL_GET_SULOG_FD = _IOW('K', 20, struct langsu_get_sulog_fd_cmd);
static const __u32 LANGSU_IOCTL_DISABLE_ESCAPE_TO_ROOT = _IO('K', 21);

#endif
