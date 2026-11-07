#ifndef __LANGSU_H_KERNEL_COMPAT
#define __LANGSU_H_KERNEL_COMPAT

#include <linux/fs.h>
#include <linux/version.h>

extern void langsu_seccomp_clear_cache(struct seccomp_filter *filter, int nr);
extern void langsu_seccomp_allow_cache(struct seccomp_filter *filter, int nr);

#endif
