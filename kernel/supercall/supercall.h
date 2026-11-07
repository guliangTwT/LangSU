#ifndef __LANGSU_H_SUPERCALL
#define __LANGSU_H_SUPERCALL

#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>

// IOCTL handler types
typedef int (*langsu_ioctl_handler_t)(void __user *arg);
typedef bool (*langsu_perm_check_t)(void);

// IOCTL command mapping
struct langsu_ioctl_cmd_map {
    unsigned int cmd;
    const char *name;
    langsu_ioctl_handler_t handler;
    langsu_perm_check_t perm_check; // Permission check function
    bool allow_su_session;
};

// Install LANGSU fd to current process
int langsu_install_fd(void);
// Install a LANGSU fd that authorizes operations required while starting su.
int langsu_install_su_fd(void);
bool langsu_is_su_session_fd(const struct file *filp);

void langsu_supercalls_init(void);
void langsu_supercalls_exit(void);
#endif // __LANGSU_H_SUPERCALL
