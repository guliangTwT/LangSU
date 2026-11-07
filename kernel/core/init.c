#include <linux/export.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/moduleparam.h>

#include "policy/allowlist.h"
#include "policy/app_profile.h"
#include "policy/feature.h"
#include "klog.h" // IWYU pragma: keep
#include "manager/manager_observer.h"
#include "manager/throne_tracker.h"
#include "hook/syscall_hook_manager.h"
#include "hook/lsm_hook.h"
#include "runtime/langsud.h"
#include "runtime/langsud_boot.h"
#include "feature/sulog.h"
#include "supercall/supercall.h"
#include "langsu.h"
#include "infra/file_wrapper.h"
#include "selinux/selinux.h"
#include "hook/syscall_hook.h"
#include "feature/adb_root.h"
#include "feature/selinux_hide.h"
#include "feature/module_blacklist.h"
#include "feature/shell_su_adb_only.h"
#include "infra/symbol_resolver.h"

#if defined(__x86_64__) && !defined(CONFIG_LANGSU_X86_PATCH_SYSCALL_DISPATCHER)
#include <asm/cpufeature.h>
#include <linux/version.h>
#ifndef X86_FEATURE_INDIRECT_SAFE
#error "FATAL: Your kernel is missing the indirect syscall bypass patches!"
#endif
#endif

// workaround for A12-5.10 kernel
// Some third-party kernel (e.g. linegaeOS) uses wrong toolchain, which supports
// CC_HAVE_STACKPROTECTOR_SYSREG while gki's toolchain doesn't.
// Therefore, langsu lkm, which uses gki toolchain, requires this __stack_chk_guard,
// while those third-party kernel can't provide.
// Thus, we manually provide it instead of using kernel's
#if defined(CONFIG_STACKPROTECTOR) &&                                                                                  \
    (defined(CONFIG_ARM64) && defined(MODULE) && !defined(CONFIG_STACKPROTECTOR_PER_TASK))
#include <linux/stackprotector.h>
#include <linux/random.h>
unsigned long __stack_chk_guard __ro_after_init __attribute__((visibility("hidden")));

__attribute__((no_stack_protector)) void __init langsu_setup_stack_chk_guard()
{
    unsigned long canary;

    /* Try to get a semi random initial value. */
    get_random_bytes(&canary, sizeof(canary));
    canary ^= LINUX_VERSION_CODE;
    canary &= CANARY_MASK;
    __stack_chk_guard = canary;
}

__attribute__((naked)) int __init langsu_init_early(void)
{
    asm("mov x19, x30;\n"
        "bl langsu_setup_stack_chk_guard;\n"
        "mov x30, x19;\n"
        "b langsu_init;\n");
}
#define NEED_OWN_STACKPROTECTOR 1
#else
#define NEED_OWN_STACKPROTECTOR 0
#endif

struct cred *langsu_cred;
bool langsu_late_loaded;

#ifdef CONFIG_LANGSU_DEBUG
bool allow_shell = true;
#else
bool allow_shell = false;
#endif
module_param(allow_shell, bool, 0);

bool langsu_no_custom_rc = false;
module_param_named(norc, langsu_no_custom_rc, bool, 0);

#ifdef MODULE
bool langsu_bundled = false;
module_param_named(bundled, langsu_bundled, bool, 0);
#endif

int __init langsu_init(void)
{
    pr_info("welcome to LangSU version " __stringify(LANGSU_KERNEL_VERSION) ", package name " LANGSU_PACKAGE_NAME "\n");
#if defined(__x86_64__) && !defined(CONFIG_LANGSU_X86_PATCH_SYSCALL_DISPATCHER)
    // If the kernel has the hardening patch, X86_FEATURE_INDIRECT_SAFE must be set
    if (!boot_cpu_has(X86_FEATURE_INDIRECT_SAFE)) {
        pr_alert("*************************************************************");
        pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
        pr_alert("**                                                         **");
        pr_alert("**        X86_FEATURE_INDIRECT_SAFE is not enabled!        **");
        pr_alert("**      LangSU will abort initialization to prevent      **");
        pr_alert("**                     kernel panic.                       **");
        pr_alert("**                                                         **");
        pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
        pr_alert("*************************************************************");
        return -ENOSYS;
    }
#endif

#ifdef MODULE
    langsu_late_loaded = (current->pid != 1);
#else
    langsu_late_loaded = false;
#endif

#ifdef CONFIG_LANGSU_DEBUG
    pr_alert("*************************************************************");
    pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
    pr_alert("**                                                         **");
    pr_alert("**         You are running LangSU in DEBUG mode          **");
    pr_alert("**                                                         **");
    pr_alert("**     NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE NOTICE    **");
    pr_alert("*************************************************************");
#endif
    if (allow_shell) {
        pr_alert("shell is allowed at init!");
    }

    langsu_cred = prepare_creds();
    if (!langsu_cred) {
        pr_err("prepare cred failed!\n");
        return -ENOSYS;
    }

    langsu_init_symbol_resolver();
    langsu_syscall_hook_init();

    // Block modules early, before vendor modules are loaded in second stage.
    langsu_module_blacklist_init();

    langsu_feature_init();
    langsu_sulog_init();
    langsu_adb_root_init();
    langsu_lsm_hook_init();
    langsu_selinux_hide_init();
    langsu_shell_su_adb_only_init();

    langsu_supercalls_init();
    langsu_app_profile_init();

    if (langsu_late_loaded) {
        pr_info("late load mode, skipping kprobe hooks\n");

        apply_langsu_rules();
        cache_sid();
        setup_langsu_cred();

        // Grant current process (langsud late-load) root
        // with LANGSU SELinux domain before enforcing SELinux, so it
        // can continue to access /data/app etc. after enforcement.
        escape_to_root_for_init();

        langsu_allowlist_init();
        langsu_load_allow_list();

        langsu_syscall_hook_manager_init();

        langsu_throne_tracker_init();
        langsu_observer_init();
        langsu_file_wrapper_init();

        langsu_boot_completed = true;
        track_throne(false);

        langsu_feature_apply_forced();

        if (!getenforce()) {
            pr_info("Permissive SELinux, enforcing\n");
            setenforce(true);
        }

    } else {
        langsu_syscall_hook_manager_init();

        langsu_allowlist_init();

        langsu_throne_tracker_init();

        langsu_langsud_init();

        langsu_file_wrapper_init();
    }

#ifdef MODULE
#ifndef CONFIG_LANGSU_DEBUG
    kobject_del(&THIS_MODULE->mkobj.kobj);
#endif
#endif
    return 0;
}

void __exit langsu_exit(void)
{
    // Phase 1: Stop all hooks first to prevent new callbacks
    langsu_syscall_hook_manager_exit();

    langsu_supercalls_exit();

    if (!langsu_late_loaded)
        langsu_langsud_exit();

    // Wait for any in-flight RCU readers (e.g. handler traversing allow_list)
    synchronize_rcu();

    // Phase 2: Now safe to release data structures
    langsu_observer_exit();

    langsu_throne_tracker_exit();

    langsu_allowlist_exit();

    langsu_shell_su_adb_only_exit();
    langsu_selinux_hide_exit();
    langsu_lsm_hook_exit();
    langsu_adb_root_exit();
    langsu_sulog_exit();
    langsu_feature_exit();
    langsu_module_blacklist_exit();

    put_cred(langsu_cred);
}

#if NEED_OWN_STACKPROTECTOR
module_init(langsu_init_early);
#else
module_init(langsu_init);
#endif
module_exit(langsu_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LangSU");
MODULE_DESCRIPTION("Android LangSU");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
MODULE_IMPORT_NS("VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver");
#else
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif
