#ifndef SCAN_TREE_H
#define SCAN_TREE_H

#include "cgit.h"

extern void scan_projects(const char *path, const char *projectsfile, repo_config_fn fn);
extern void scan_tree(const char *path, repo_config_fn fn);

#endif
