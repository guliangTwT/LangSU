#ifndef __LANGSU_H_KLOG
#define __LANGSU_H_KLOG

#include <linux/printk.h>

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) "LangSU: " fmt
#endif

#endif
