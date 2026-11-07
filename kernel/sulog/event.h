#ifndef __LANGSU_H_SULOG_EVENT
#define __LANGSU_H_SULOG_EVENT

#include <linux/compiler_types.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include "uapi/sulog.h" // IWYU pragma: keep

struct langsu_event_queue;
struct langsu_sulog_pending_event;

int langsu_sulog_events_init(void);
void langsu_sulog_events_exit(void);

struct langsu_sulog_pending_event *langsu_sulog_capture_root_execve(const char __user *filename_user,
                                                              const char __user *const __user *argv_user, gfp_t gfp);
struct langsu_sulog_pending_event *langsu_sulog_capture_sucompat(const char __user *filename_user,
                                                           const char __user *const __user *argv_user, gfp_t gfp);
void langsu_sulog_emit_pending(struct langsu_sulog_pending_event *pending, int retval, gfp_t gfp);
int langsu_sulog_emit_grant_root(int retval, __u32 uid, __u32 euid, gfp_t gfp);

struct langsu_event_queue *langsu_sulog_get_queue(void);

#endif
