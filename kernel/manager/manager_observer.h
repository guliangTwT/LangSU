#ifndef __LANGSU_H_MANAGER_OBSERVER
#define __LANGSU_H_MANAGER_OBSERVER

#ifdef CONFIG_LANGSU_DISABLE_MANAGER
static inline int langsu_observer_init(void)
{
    return 0;
}

static inline void langsu_observer_exit(void)
{
}
#else
int langsu_observer_init(void);
void langsu_observer_exit(void);
#endif

#endif // __LANGSU_H_MANAGER_OBSERVER
