#ifndef __LANGSU_H_SEPOLICY
#define __LANGSU_H_SEPOLICY

#include <linux/types.h>

#include "ss/policydb.h"

struct selinux_policy *langsu_dup_sepolicy(struct selinux_policy *old_pol);

void langsu_destroy_sepolicy(struct selinux_policy *orig);

// Operation on types
bool langsu_type(struct policydb *db, const char *name, const char *attr);
bool langsu_attribute(struct policydb *db, const char *name);
bool langsu_permissive(struct policydb *db, const char *type);
bool langsu_enforce(struct policydb *db, const char *type);
bool langsu_typeattribute(struct policydb *db, const char *type, const char *attr);
bool langsu_exists(struct policydb *db, const char *type);

// Access vector rules
bool langsu_allow(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);
bool langsu_deny(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);
bool langsu_auditallow(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);
bool langsu_dontaudit(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *perm);

// Extended permissions access vector rules
bool langsu_allowxperm(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *range);
bool langsu_auditallowxperm(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *range);
bool langsu_dontauditxperm(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *range);

// Type rules
bool langsu_type_transition(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *def,
                         const char *obj);
bool langsu_type_change(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *def);
bool langsu_type_member(struct policydb *db, const char *src, const char *tgt, const char *cls, const char *def);

// File system labeling
bool langsu_genfscon(struct policydb *db, const char *fs_name, const char *path, const char *ctx);

#endif
