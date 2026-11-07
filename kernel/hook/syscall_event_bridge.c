#include "linux/compiler.h"
#include "linux/cred.h"
#include "linux/jump_label.h"
#include "linux/printk.h"
#include "selinux/selinux.h"
#include <asm/syscall.h>
#include <linux/ptrace.h>
#include <linux/static_key.h>

#include "arch.h"
#include "klog.h" // IWYU pragma: keep
#include "hook/tp_marker.h"
#include "feature/sucompat.h"
#include "hook/setuid_hook.h"
#include "policy/app_profile.h"
#include "runtime/langsud.h"
#include "sulog/event.h"
#include "hook/syscall_hook.h"
#include "hook/syscall_event_bridge.h"
#include "feature/adb_root.h"

static int langsu_handle_init_mark_tracker(const char __user **filename_user)
{
    char path[64];
    unsigned long addr;
    const char __user *fn;
    long ret;

    if (unlikely(!filename_user))
        return 0;

    addr = untagged_addr((unsigned long)*filename_user);
    fn = (const char __user *)addr;
    ret = strncpy_from_user(path, fn, sizeof(path));
    if (ret < 0)
        return 0;

    path[sizeof(path) - 1] = '\0';
    if (unlikely(strcmp(path, LANGSUD_PATH) == 0)) {
        pr_info("hook_manager: escape to root for init executing langsud: %d\n", current->pid);
        escape_to_root_for_init();
    } else if (likely(strstr(path, "/app_process") == NULL && strstr(path, "/adbd") == NULL &&
                      strstr(path, "/stub_zygote") == NULL)) {
        pr_info("hook_manager: unmark %d exec %s\n", current->pid, path);
        langsu_clear_task_tracepoint_flag_if_needed(current);
    }

    return 0;
}

long __nocfi langsu_hook_newfstatat(int orig_nr, const struct pt_regs *regs)
{
    if (!langsu_su_compat_enabled)
        return langsu_syscall_table[orig_nr](regs);

    return langsu_handle_stat_sucompat(orig_nr, (struct pt_regs *)regs);
}

long __nocfi langsu_hook_faccessat(int orig_nr, const struct pt_regs *regs)
{
    if (!langsu_su_compat_enabled)
        return langsu_syscall_table[orig_nr](regs);

    return langsu_handle_faccessat_sucompat(orig_nr, (struct pt_regs *)regs);
}

DEFINE_STATIC_KEY_TRUE(langsud_execve_key);

void langsu_stop_langsud_execve_hook()
{
    static_branch_disable(&langsud_execve_key);
}

static long __nocfi langsu_hook_execve_common(int orig_nr, const struct pt_regs *regs, bool execveat)
{
    const char __user **filename_user =
        execveat ? (const char __user **)&PT_REGS_PARM2(regs) : (const char __user **)&PT_REGS_SYSCALL_PARM1(regs);
    const char __user *const __user *argv_user = execveat ? (const char __user *const __user *)PT_REGS_PARM3(regs) :
                                                            (const char __user *const __user *)PT_REGS_PARM2(regs);
    bool current_is_init = is_init(current_cred());
    struct langsu_sulog_pending_event *pending_root_execve = NULL;
    long ret;

    if (static_branch_unlikely(&langsud_execve_key)) {
        if (execveat) {
            langsu_execveat_hook_langsud(regs);
        } else {
            langsu_execve_hook_langsud(regs);
        }
    }

    if (current_euid().val == 0)
        pending_root_execve = langsu_sulog_capture_root_execve(*filename_user, argv_user, GFP_KERNEL);

    if (current->pid != 1 && current_is_init) {
        langsu_handle_init_mark_tracker(filename_user);
        ret = execveat ? langsu_adb_root_handle_execveat((struct pt_regs *)regs) :
                         langsu_adb_root_handle_execve((struct pt_regs *)regs);
        if (ret) {
            pr_err("adb root failed: %ld\n", ret);
        }
    } else if (langsu_su_compat_enabled) {
        ret = execveat ? langsu_handle_execveat_sucompat(filename_user, orig_nr, (struct pt_regs *)regs) :
                         langsu_handle_execve_sucompat(filename_user, orig_nr, (struct pt_regs *)regs);
        langsu_sulog_emit_pending(pending_root_execve, ret, GFP_KERNEL);
        return ret;
    }

    ret = langsu_syscall_table[orig_nr](regs);
    langsu_sulog_emit_pending(pending_root_execve, ret, GFP_KERNEL);
    return ret;
}

long __nocfi langsu_hook_execve(int orig_nr, const struct pt_regs *regs)
{
    return langsu_hook_execve_common(orig_nr, regs, false);
}

long __nocfi langsu_hook_execveat(int orig_nr, const struct pt_regs *regs)
{
    return langsu_hook_execve_common(orig_nr, regs, true);
}

long __nocfi langsu_hook_setresuid(int orig_nr, const struct pt_regs *regs)
{
    uid_t old_uid = current_uid().val;
    long ret = langsu_syscall_table[orig_nr](regs);

    if (ret < 0)
        return ret;

    langsu_handle_setresuid(old_uid, current_uid().val);
    return ret;
}
