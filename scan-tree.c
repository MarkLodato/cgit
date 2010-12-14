/* scan-tree.c
 * 
 * Copyright (C) 2008-2009 Lars Hjemli
 * Copyright (C) 2010 Jason A. Donenfeld <Jason@zx2c4.com>
 *
 * Licensed under GNU General Public License v2
 *   (see COPYING for full license text)
 */

#include "scan-tree.h"
#include "configfile.h"
#include "html.h"
#include "repo.h"

#define MAX_PATH 4096

/* return 1 if path contains a objects/ directory and a HEAD file */
static int is_git_dir(const char *path)
{
	struct stat st;
	static char buf[MAX_PATH];

	if (snprintf(buf, MAX_PATH, "%s/objects", path) >= MAX_PATH) {
		fprintf(stderr, "Insanely long path: %s\n", path);
		return 0;
	}
	if (stat(buf, &st)) {
		if (errno != ENOENT)
			fprintf(stderr, "Error checking path %s: %s (%d)\n",
				path, strerror(errno), errno);
		return 0;
	}
	if (!S_ISDIR(st.st_mode))
		return 0;

	sprintf(buf, "%s/HEAD", path);
	if (stat(buf, &st)) {
		if (errno != ENOENT)
			fprintf(stderr, "Error checking path %s: %s (%d)\n",
				path, strerror(errno), errno);
		return 0;
	}
	if (!S_ISREG(st.st_mode))
		return 0;

	return 1;
}

static struct cgit_repo *repo;
static char *owner;

static void repo_config(const char *name, const char *value)
{
	cgit_repo_config(repo, name, value);
}

static int git_owner_config(const char *key, const char *value, void *cb)
{
	if (!strcmp(key, "gitweb.owner"))
		owner = xstrdup(value);
	return 0;
}

static char *xstrrchr(char *s, char *from, int c)
{
	while (from >= s && *from != c)
		from--;
	return from < s ? NULL : from;
}

static int add_repo(const char *base, const char *path)
{
	struct stat st;
	struct passwd *pwd;
	char *rel, *p, *slash;
	int n;
	size_t size;

	if (stat(path, &st)) {
		fprintf(stderr, "Error accessing %s: %s (%d)\n",
			path, strerror(errno), errno);
		return 0;
	}

	if (ctx.cfg.strict_export && stat(fmt("%s/%s", path, ctx.cfg.strict_export), &st))
		return 0;

	if (!stat(fmt("%s/noweb", path), &st))
		return 0;

	owner = NULL;
	if (ctx.cfg.enable_gitweb_owner)
		git_config_from_file(git_owner_config, fmt("%s/config", path), NULL);
	if (base == path)
		rel = xstrdup(fmt("%s", path));
	else
		rel = xstrdup(fmt("%s", path + strlen(base) + 1));

	if (!strcmp(rel + strlen(rel) - 5, "/.git"))
		rel[strlen(rel) - 5] = '\0';

	repo = cgit_add_repo(rel);
	if (ctx.cfg.remove_suffix)
		if ((p = strrchr(repo->url, '.')) && !strcmp(p, ".git"))
			*p = '\0';
	repo->name = repo->url;
	repo->path = xstrdup(path);
	while (!owner) {
		if ((pwd = getpwuid(st.st_uid)) == NULL) {
			fprintf(stderr, "Error reading owner-info for %s: %s (%d)\n",
				path, strerror(errno), errno);
			break;
		}
		if (pwd->pw_gecos)
			if ((p = strchr(pwd->pw_gecos, ',')))
				*p = '\0';
		owner = xstrdup(pwd->pw_gecos ? pwd->pw_gecos : pwd->pw_name);
	}
	repo->owner = owner;

	p = fmt("%s/description", path);
	if (!stat(p, &st))
		readfile(p, &repo->desc, &size);

	if (!repo->readme) {
		p = fmt("%s/README.html", path);
		if (!stat(p, &st))
			repo->readme = "README.html";
	}
	if (ctx.cfg.section_from_path) {
		n  = ctx.cfg.section_from_path;
		if (n > 0) {
			slash = rel;
			while (slash && n && (slash = strchr(slash, '/')))
				n--;
		} else {
			slash = rel + strlen(rel);
			while (slash && n && (slash = xstrrchr(rel, slash, '/')))
				n++;
		}
		if (slash && !n) {
			*slash = '\0';
			repo->section = xstrdup(rel);
			*slash = '/';
			if (!prefixcmp(repo->name, repo->section)) {
				repo->name += strlen(repo->section);
				if (*repo->name == '/')
					repo->name++;
			}
		}
	}

	p = fmt("%s/cgitrc", path);
	if (!stat(p, &st)) {
		parse_configfile(xstrdup(p), &repo_config);
	}

	return 1;
}

static int scan_path(const char *base, const char *path)
{
	DIR *dir = opendir(path);
	struct dirent *ent;
	char *buf;
	struct stat st;
	int found = 0;

	if (!dir) {
		fprintf(stderr, "Error opening directory %s: %s (%d)\n",
			path, strerror(errno), errno);
		return 0;
	}
	if (is_git_dir(path)) {
		found = add_repo(base, path);
		goto end;
	}
	if (is_git_dir(fmt("%s/.git", path))) {
		found = add_repo(base, fmt("%s/.git", path));
		goto end;
	}
	while((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.') {
			if (ent->d_name[1] == '\0')
				continue;
			if (ent->d_name[1] == '.' && ent->d_name[2] == '\0')
				continue;
			if (!ctx.cfg.scan_path_hidden)
				continue;
		}
		buf = malloc(strlen(path) + strlen(ent->d_name) + 2);
		if (!buf) {
			fprintf(stderr, "Alloc error on %s: %s (%d)\n",
				path, strerror(errno), errno);
			exit(1);
		}
		sprintf(buf, "%s/%s", path, ent->d_name);
		if (stat(buf, &st)) {
			fprintf(stderr, "Error checking path %s: %s (%d)\n",
				buf, strerror(errno), errno);
			free(buf);
			continue;
		}
		if (S_ISDIR(st.st_mode))
			found += scan_path(base, buf);
		free(buf);
	}
end:
	closedir(dir);
	return found;
}

#define lastc(s) s[strlen(s) - 1]

int scan_projects(const char *path, const char *projectsfile)
{
	char line[MAX_PATH * 2], *z;
	FILE *projects;
	int err, found = 0;
	
	projects = fopen(projectsfile, "r");
	if (!projects) {
		fprintf(stderr, "Error opening projectsfile %s: %s (%d)\n",
			projectsfile, strerror(errno), errno);
	}
	while (fgets(line, sizeof(line), projects) != NULL) {
		for (z = &lastc(line);
		     strlen(line) && strchr("\n\r", *z);
		     z = &lastc(line))
			*z = '\0';
		if (strlen(line))
			found += scan_path(path, fmt("%s/%s", path, line));
	}
	if ((err = ferror(projects))) {
		fprintf(stderr, "Error reading from projectsfile %s: %s (%d)\n",
			projectsfile, strerror(err), err);
	}
	fclose(projects);
	return found;
}

int scan_tree(const char *path)
{
	return scan_path(path, path);
}
