#ifndef __LANGSU_UAPI_FEATURE_H
#define __LANGSU_UAPI_FEATURE_H

enum langsu_feature_id {
    LANGSU_FEATURE_SU_COMPAT = 0,
    LANGSU_FEATURE_KERNEL_UMOUNT = 1,
    LANGSU_FEATURE_SULOG = 2,
    LANGSU_FEATURE_ADB_ROOT = 3,
    LANGSU_FEATURE_SELINUX_HIDE = 4,
    // 5 was webview_zygote_umount (v3.3.0), keep it unused so stale configs don't apply
    LANGSU_FEATURE_SHELL_SU_ADB_ONLY = 6,

    LANGSU_FEATURE_MAX
};

#endif
