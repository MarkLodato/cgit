/* repo-db.c - database-backed repolist provider
 * 
 * Copyright (C) 2010 Mark Lodato <lodatom@gmail.com>
 *
 * Licensed under GNU General Public License v2
 *   (see COPYING for full license text)
 */

#include "repo-db.h"
#include "cgit.h"
#include "cache.h"
#include "repo.h"
#include "ui-stats.h"

#ifdef HAVE_REPO_DB

#define IS_EMPTY(s) (!(s) || !(s)[0])

#define REQUIRE(var, name) do { \
	if (IS_EMPTY(var)) { \
		fprintf(stderr, "'%s' must be set\n", (name)); \
		return 0; \
	} \
} while (0)

static struct cgit_repo *repo;

static void finish_repo(struct cgit_repo *repo)
{
	if (IS_EMPTY(repo->path))
		repo->path = xstrdup(repo->url);
	if (!IS_EMPTY(ctx.cfg.db.base_path) && repo->path[0] != '/') {
		char *oldpath = repo->path;
		repo->path = xstrdup(fmt("%s/%s", ctx.cfg.db.base_path,
					 repo->path ? repo->path : repo->url));
		free(oldpath);
	}
	repo->snapshots &= ctx.cfg.snapshots;
	repo->enable_log_filecount *= ctx.cfg.enable_log_filecount;
	repo->enable_log_linecount *= ctx.cfg.enable_log_linecount;
}

#ifdef HAVE_SQLITE3
# include <sqlite3.h>
# define CALL(func, ...) do { \
	if (func(__VA_ARGS__) != SQLITE_OK) { \
		fprintf(stderr, #func " failed: %s\n", sqlite3_errmsg(db)); \
		goto end; \
	} \
} while (0)
static int do_sqlite3(struct cgit_config_db *cfg, const char *url, int single)
{
	sqlite3 *db = NULL;
	sqlite3_stmt *stmt = NULL;
	const char *value, *name;
	char *query = NULL;
	int count=0, url_column=-1, n_columns, i, r;

	REQUIRE(cfg->database, "db.database");
	REQUIRE(cfg->query_all, "db.query.all");

	if (!single)
		query = xstrdup(cfg->query_all);
	else if (!IS_EMPTY(cfg->query_single))
		query = xstrdup(cfg->query_single);
	else
		query = xstrdup(fmt("%s WHERE url=?", cfg->query_all));

	CALL(sqlite3_open, cfg->database, &db);

	CALL(sqlite3_prepare_v2, db, query, -1, &stmt, NULL);

	if (single)
		CALL(sqlite3_bind_text, stmt, 1, url, -1, SQLITE_STATIC);

	n_columns = sqlite3_column_count(stmt);
	for (i = 0; i < n_columns; i++) {
		if (!strcmp(sqlite3_column_name(stmt, i), "url")) {
			url_column = i;
			break;
		}
	}
	if (url_column < 0) {
		fprintf(stderr, "'url' column must be selected\n");
		goto end;
	}

	while ((r = sqlite3_step(stmt)) == SQLITE_ROW) {
		url = (const char*)sqlite3_column_text(stmt, url_column);
		if (!url)
			continue;
		repo = cgit_add_repo(url);
		for (i = 0; i < n_columns; i++) {
			if (i == url_column)
				continue;
			name = (const char*)sqlite3_column_name(stmt, i);
			value = (const char*)sqlite3_column_text(stmt, i);
			if (!name || !value)
				continue;
			if (!strcmp(name, "path"))
				repo->path = xstrdup(value);
			else
				cgit_repo_config(repo, name, value);
		}
		finish_repo(repo);
		count++;
		if (single)
			break;
	}
	if (r != SQLITE_DONE) {
		fprintf(stderr, "sqlite3_step failed: %s\n",
			sqlite3_errmsg(db));
		goto end;
	}

end:
	if (stmt)
		sqlite3_finalize(stmt);
	if (db)
		sqlite3_close(db);
	if (query)
		free(query);
	return count;
}
# undef CALL
#endif /* HAVE_SQLITE3 */

static int repo_db2(struct cgit_config *config, const char *url, int single)
{
	int rc;
	if (IS_EMPTY(config->db.driver))
		rc = 0;
#ifdef HAVE_SQLITE3
	else if (!strcmp(config->db.driver, "sqlite3"))
		rc = do_sqlite3(&config->db, url, single);
#endif
	else {
		fprintf(stderr, "invalid db.driver: '%s'\n",
			config->db.driver);
		rc = 0;
	}
	return rc;
}

int repo_db(struct cgit_config *config)
{
	return repo_db2(config, NULL, 0);
}

struct cgit_repo* repo_db_single(struct cgit_config *config, const char *url)
{
	repo = NULL;
	repo_db2(config, url, 1);
	return repo;
}

int repo_db_hash(struct cgit_config *config)
{
	int hash = 0;
	if (config->db.driver) {
		hash += hash_str("db backend enabled\n");
	}
	return hash;
}

#endif
