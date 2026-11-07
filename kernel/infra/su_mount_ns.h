#ifndef __LANGSU_SU_MOUNT_NS_H
#define __LANGSU_SU_MOUNT_NS_H

#include <linux/types.h>

#define LANGSU_NS_INHERITED 0
#define LANGSU_NS_GLOBAL 1
#define LANGSU_NS_INDIVIDUAL 2

struct langsu_mns_tw {
    struct callback_head cb;
    int32_t ns_mode;
};

void setup_mount_ns(int32_t ns_mode);

#endif
