#ifndef __LANGSU_H_FEATURE
#define __LANGSU_H_FEATURE

#include <linux/types.h>
#include "uapi/feature.h" // IWYU pragma: keep

typedef int (*langsu_feature_get_t)(u64 *value);
typedef int (*langsu_feature_set_t)(u64 value);

struct langsu_feature_handler {
    u32 feature_id;
    const char *name;
    langsu_feature_get_t get_handler;
    langsu_feature_set_t set_handler;
};

int langsu_register_feature_handler(const struct langsu_feature_handler *handler);

int langsu_unregister_feature_handler(u32 feature_id);

int langsu_get_feature(u32 feature_id, u64 *value, bool *supported);

int langsu_set_feature(u32 feature_id, u64 value);

// Whether the feature is forced by the force_feature module param, in which
// case langsu_set_feature() refuses to change it.
bool langsu_feature_is_forced(u32 feature_id);

// Apply forced feature values not applied yet. Called at several boot stages
// since some handlers can only succeed late (e.g. selinux_hide needs the
// sepolicy backup).
void langsu_feature_apply_forced(void);

void langsu_feature_init(void);

void langsu_feature_exit(void);

#endif // __LANGSU_H_FEATURE
