#ifndef __LANGSU_H_SULOG
#define __LANGSU_H_SULOG

#include <linux/types.h>

bool langsu_sulog_is_enabled(void);
void langsu_sulog_init(void);
void langsu_sulog_exit(void);

#endif
