#ifndef __LANGSU_H_MANAGER_IDENTITY
#define __LANGSU_H_MANAGER_IDENTITY

#include <linux/cred.h>
#include <linux/types.h>

#define LANGSU_INVALID_APPID -1
#define LANGSU_PER_USER_RANGE 100000

#ifdef CONFIG_LANGSU_DISABLE_MANAGER
static inline bool langsu_is_manager_appid_valid()
{
    return true;
}

static inline bool is_manager()
{
    return current_uid().val == 0;
}

static inline bool is_uid_manager(uid_t uid)
{
    return uid == 0;
}

static inline uid_t langsu_get_manager_appid()
{
    return 0;
}

static inline void langsu_set_manager_appid(uid_t appid)
{
    (void)appid;
}

static inline void langsu_invalidate_manager_uid()
{
}
#else
extern uid_t langsu_manager_appid; // DO NOT DIRECT USE

static inline bool langsu_is_manager_appid_valid()
{
    return langsu_manager_appid != LANGSU_INVALID_APPID;
}

static inline bool is_manager()
{
    return unlikely(langsu_manager_appid == current_uid().val % LANGSU_PER_USER_RANGE);
}

static inline bool is_uid_manager(uid_t uid)
{
    return unlikely(langsu_manager_appid == uid % LANGSU_PER_USER_RANGE);
}

static inline uid_t langsu_get_manager_appid()
{
    return langsu_manager_appid;
}

static inline void langsu_set_manager_appid(uid_t appid)
{
    langsu_manager_appid = appid;
}

static inline void langsu_invalidate_manager_uid()
{
    langsu_manager_appid = LANGSU_INVALID_APPID;
}
#endif

#endif // __LANGSU_H_MANAGER_IDENTITY
