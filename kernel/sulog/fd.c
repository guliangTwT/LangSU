#include <linux/anon_inodes.h>
#include <linux/err.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include "infra/event_queue.h"
#include "klog.h" // IWYU pragma: keep
#include "sulog/event.h"

static DEFINE_MUTEX(langsu_sulog_fd_lock);
static bool langsu_sulog_fd_active;

static ssize_t langsu_sulog_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return langsu_event_queue_read(langsu_sulog_get_queue(), buf, count, file->f_flags);
}

static __poll_t langsu_sulog_poll(struct file *file, poll_table *wait)
{
    return langsu_event_queue_poll(langsu_sulog_get_queue(), file, wait);
}

static int langsu_sulog_release(struct inode *inode, struct file *file)
{
    mutex_lock(&langsu_sulog_fd_lock);
    langsu_sulog_fd_active = false;
    mutex_unlock(&langsu_sulog_fd_lock);

    pr_info("sulog: fd released\n");
    return 0;
}

static const struct file_operations langsu_sulog_fops = {
    .owner = THIS_MODULE,
    .read = langsu_sulog_read,
    .poll = langsu_sulog_poll,
    .release = langsu_sulog_release,
    .llseek = noop_llseek,
};

int langsu_install_sulog_fd(void)
{
    struct file *filp;
    int fd;

    mutex_lock(&langsu_sulog_fd_lock);

    if (langsu_sulog_fd_active) {
        fd = -EBUSY;
        goto out_unlock;
    }

    if (READ_ONCE(langsu_sulog_get_queue()->closed)) {
        fd = -EPIPE;
        goto out_unlock;
    }

    fd = get_unused_fd_flags(O_CLOEXEC);
    if (fd < 0)
        goto out_unlock;

    filp = anon_inode_getfile("[langsu_sulog]", &langsu_sulog_fops, NULL, O_RDONLY | O_CLOEXEC);
    if (IS_ERR(filp)) {
        put_unused_fd(fd);
        fd = PTR_ERR(filp);
        goto out_unlock;
    }

    langsu_sulog_fd_active = true;
    fd_install(fd, filp);
    pr_info("sulog: fd installed %d for pid %d\n", fd, current->pid);

out_unlock:
    mutex_unlock(&langsu_sulog_fd_lock);
    return fd;
}

void __init langsu_sulog_fd_init(void)
{
    mutex_lock(&langsu_sulog_fd_lock);
    langsu_sulog_fd_active = false;
    mutex_unlock(&langsu_sulog_fd_lock);
}

void __exit langsu_sulog_fd_exit(void)
{
    mutex_lock(&langsu_sulog_fd_lock);
    langsu_sulog_fd_active = false;
    mutex_unlock(&langsu_sulog_fd_lock);

    langsu_event_queue_close(langsu_sulog_get_queue());
}
