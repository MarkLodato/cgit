#ifndef REPO_H
#define REPO_H

#include "cgit.h"

struct cgit_repolist {
	int length;
	int count;
	struct cgit_repo *repos;
};

extern struct cgit_repo *cgit_add_repo(const char *url);
extern struct cgit_repo *cgit_get_repoinfo(const char *url);
extern struct cgit_repolist *cgit_get_repolist(void);
extern void cgit_repolist_done(void);
extern int cgit_num_repos(void);
extern int cgit_repolist_hash(void);

#endif
