#ifndef __LANGSU_H_SELINUX
#define __LANGSU_H_SELINUX

#include <linux/types.h>
#include <linux/version.h>
#include <linux/cred.h>

#define LANGSU_DOMAIN "langsu"
#define LANGSU_FILE "langsu_file"

#define LANGSU_CONTEXT "u:r:" LANGSU_DOMAIN ":s0"
#define LANGSU_FILE_CONTEXT "u:object_r:" LANGSU_FILE ":s0"
#define ZYGOTE_CONTEXT "u:r:zygote:s0"
#define INIT_CONTEXT "u:r:init:s0"
#define ADBD_CONTEXT "u:r:adbd:s0"

void setup_selinux(const char *, struct cred *);

void setenforce(bool);

bool getenforce();

void cache_sid(void);

bool is_task_langsu_domain(const struct cred *cred);

bool is_langsu_domain();

bool is_zygote(const struct cred *cred);

bool is_init(const struct cred *cred);

bool is_adbd(const struct cred *cred);

void apply_langsu_rules();

int handle_sepolicy(void __user *user_data, u64 data_len);

void setup_langsu_cred();

void escape_to_root_for_adb_root();

extern u32 langsu_file_sid;

#endif
