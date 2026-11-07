#ifndef __LANGSU_H_LANGSU
#define __LANGSU_H_LANGSU

#include <linux/types.h>
#include <linux/cred.h>
#include <linux/workqueue.h>

#define LANGSU_KERNEL_VERSION LANGSU_VERSION

extern struct cred *langsu_cred;
extern bool langsu_late_loaded;
extern bool allow_shell;
#ifdef MODULE
extern bool langsu_bundled;
#endif
extern struct selinux_policy *backup_sepolicy;
extern bool langsu_no_custom_rc;

static inline int startswith(char *s, char *prefix)
{
    return strncmp(s, prefix, strlen(prefix));
}

static inline int endswith(const char *s, const char *t)
{
    size_t slen = strlen(s);
    size_t tlen = strlen(t);
    if (tlen > slen)
        return 1;
    return strcmp(s + slen - tlen, t);
}

#endif
