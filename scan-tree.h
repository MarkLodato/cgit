#ifndef SCAN_TREE_H
#define SCAN_TREE_H

#include "cgit.h"

extern int scan_tree(struct cgit_config *config);
extern struct cgit_repo* scan_tree_single(struct cgit_config *config,
					  const char *url);
extern int scan_tree_hash(struct cgit_config *config);

#endif
