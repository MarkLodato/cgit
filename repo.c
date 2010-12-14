/* repo.c - repository list interface
 *
 * Copyright (C) 2008-2009 Lars Hjemli
 * Copyright (C) 2010 Jason A. Donenfeld <Jason@zx2c4.com>
 *
 * Licensed under GNU General Public License v2
 *   (see COPYING for full license text)
 */

#include "repo.h"
#include "cgit.h"
#include "ui-stats.h"

#include "scan-tree.h"
#include "repo-db.h"

static struct cgit_repolist cgit_repolist;

struct provider {
	/* name of the provider */
	char *name;

	/* return a single repo with the given URL, or NULL if not found */
	struct cgit_repo* (*load)(struct cgit_config *config, const char *url);

	/* add all repos; return number added */
	int (*scan)(struct cgit_config *config);

	/* non-zero hash of relevant configuration values, or 0 if disabled */
	int (*hash)(struct cgit_config *config);
};

static struct provider providers[] = {
#ifdef HAVE_REPO_DB
	{"db", repo_db_single, repo_db, repo_db_hash},
#endif
	{"scan-tree", scan_tree_single, scan_tree, scan_tree_hash}
};
static int num_providers = sizeof(providers) / sizeof(*providers);
static int providers_done;

struct cgit_repo *cgit_add_repo(const char *url)
{
	struct cgit_repo *ret;

	if (!url || !url[0])
		die("repo url must be non-blank");

	if (++cgit_repolist.count > cgit_repolist.length) {
		if (cgit_repolist.length == 0)
			cgit_repolist.length = 8;
		else
			cgit_repolist.length *= 2;
		cgit_repolist.repos = xrealloc(cgit_repolist.repos,
					       cgit_repolist.length *
					       sizeof(struct cgit_repo));
	}

	ret = &cgit_repolist.repos[cgit_repolist.count-1];
	memset(ret, 0, sizeof(struct cgit_repo));
	ret->url = trim_end(url, '/');
	ret->name = ret->url;
	ret->path = NULL;
	ret->desc = "[no description]";
	ret->owner = NULL;
	ret->section = ctx.cfg.section;
	ret->defbranch = "master";
	ret->snapshots = ctx.cfg.snapshots;
	ret->enable_log_filecount = ctx.cfg.enable_log_filecount;
	ret->enable_log_linecount = ctx.cfg.enable_log_linecount;
	ret->enable_remote_branches = ctx.cfg.enable_remote_branches;
	ret->enable_subject_links = ctx.cfg.enable_subject_links;
	ret->max_stats = ctx.cfg.max_stats;
	ret->module_link = ctx.cfg.module_link;
	ret->readme = ctx.cfg.readme;
	ret->mtime = -1;
	ret->about_filter = ctx.cfg.about_filter;
	ret->commit_filter = ctx.cfg.commit_filter;
	ret->source_filter = ctx.cfg.source_filter;
	return ret;
}

struct cgit_repo *cgit_get_repoinfo(const char *url)
{
	int i;
	struct cgit_repo *repo;

	for (i=0; i<cgit_repolist.count; i++) {
		repo = &cgit_repolist.repos[i];
		if (!strcmp(repo->url, url))
			return repo;
	}

	for (i=0; i<num_providers; i++) {
		repo = providers[i].load(&ctx.cfg, url);
		if (repo)
			return repo;
	}

	return NULL;
}

struct cgit_repolist *cgit_get_repolist(void)
{
	int i;
	if (!providers_done) {
		for (i=0; i<num_providers; i++)
			providers[i].scan(&ctx.cfg);
		providers_done = 1;
	}
	return &cgit_repolist;
}

void cgit_repolist_done(void)
{
	providers_done = 1;
}

int cgit_num_repos(void)
{
	return cgit_repolist.count;
}

int cgit_repolist_hash(void)
{
	int i, tmp, valid = 0, hash = 0;
	for (i = 0; i < num_providers; i++) {
		tmp = providers[i].hash(&ctx.cfg);
		if (tmp) {
			valid = 1;
			hash += tmp;
		}
	}
	if (valid && !hash)
		hash = 1;
	return hash;
}
