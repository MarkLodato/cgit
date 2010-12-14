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

static struct cgit_repolist cgit_repolist;

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
	return NULL;
}

struct cgit_repolist *cgit_get_repolist(void)
{
	return &cgit_repolist;
}
