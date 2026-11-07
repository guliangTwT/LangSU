#ifndef __LANGSU_H_UID_OBSERVER
#define __LANGSU_H_UID_OBSERVER

#include <linux/types.h>
#ifdef CONFIG_LANGSU_DISABLE_MANAGER
static inline void langsu_throne_tracker_init()
{
}

static inline void langsu_throne_tracker_exit()
{
}

static inline void track_throne(bool prune_only)
{
    (void)prune_only;
}
#else
void langsu_throne_tracker_init();

void langsu_throne_tracker_exit();

void track_throne(bool prune_only);
#endif

#endif
