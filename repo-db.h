#ifndef REPO_DB_H
#define REPO_DB_H

#include "cgit.h"

#if defined(HAVE_SQLITE3) || defined(HAVE_MYSQL)
# define HAVE_REPO_DB
extern int repo_db(struct cgit_config *config);
extern struct cgit_repo* repo_db_single(struct cgit_config *config, const char *url);
extern int repo_db_hash(struct cgit_config *config);
#endif

#ifdef HAVE_MYSQL
# define MYSQL_FEATURES "+mysql"
#else
# define MYSQL_FEATURES "-mysql"
#endif

#ifdef HAVE_SQLITE3
# define SQLITE3_FEATURES "+sqlite3"
#else
# define SQLITE3_FEATURES "-sqlite3"
#endif

#define DB_FEATURES MYSQL_FEATURES SQLITE3_FEATURES

#endif
