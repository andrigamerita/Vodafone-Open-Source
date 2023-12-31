/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 *
 * \brief Module Loader
 * 
 */

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 1.8.6.1 $")

#include "asterisk/module.h"
#include "asterisk/options.h"
#include "asterisk/config.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/term.h"
#include "asterisk/manager.h"
#include "asterisk/cdr.h"
#include "asterisk/enum.h"
#include "asterisk/rtp.h"
#include "asterisk/lock.h"
#ifdef DLFCNCOMPAT
#include "asterisk/dlfcn-compat.h"
#else
#include <dlfcn.h>
#endif
#include "asterisk/md5.h"

#ifndef RTLD_NOW
#define RTLD_NOW 0
#endif

#include "modules.h"

AST_MUTEX_DEFINE_STATIC(modlock);
AST_MUTEX_DEFINE_STATIC(reloadlock);

static struct module *module_list=NULL;
static int modlistver = 0;

static unsigned char expected_key[] =
{ 0x8e, 0x93, 0x22, 0x83, 0xf5, 0xc3, 0xc0, 0x75,
  0xff, 0x8b, 0xa9, 0xbe, 0x7c, 0x43, 0x74, 0x63 };

struct module {
	int (*load_module)(void);
	int (*unload_module)(void);
	int (*usecount)(void);
	char *(*description)(void);
	char *(*key)(void);
	int (*reload)(void);
	int (*reload_if_changed)(void);
	void *lib;
	char resource[256];
	struct module *next;
};

static struct loadupdate {
	int (*updater)(void);
	struct loadupdate *next;
} *updaters = NULL;

static int printdigest(unsigned char *d)
{
	int x;
	char buf[256];
	char buf2[16];
	snprintf(buf, sizeof(buf), "Unexpected signature:");
	for (x=0; x<16; x++) {
		snprintf(buf2, sizeof(buf2), " %02x", *(d++));
		strcat(buf, buf2);
	}
	strcat(buf, "\n");
	ast_log(LOG_DEBUG, "%s", buf);
	return 0;
}

static int key_matches(unsigned char *key1, unsigned char *key2)
{
	int match = 1;
	int x;
	for (x=0; x<16; x++) {
		match &= (key1[x] == key2[x]);
	}
	return match;
}

static int verify_key(unsigned char *key)
{
	struct MD5Context c;
	unsigned char digest[16];
	MD5Init(&c);
	MD5Update(&c, key, strlen((char *)key));
	MD5Final(digest, &c);
	if (key_matches(expected_key, digest))
		return 0;
	printdigest(digest);
	return -1;
}

int ast_unload_resource(const char *resource_name, int force)
{
	struct module *m, *ml = NULL;
	int res = -1;
	if (ast_mutex_lock(&modlock))
		ast_log(LOG_WARNING, "Failed to lock\n");
	m = module_list;
	while(m) {
		if (!strcasecmp(m->resource, resource_name)) {
			if ((res = m->usecount()) > 0)  {
				if (force) 
					ast_log(LOG_WARNING, "Warning:  Forcing removal of module %s with use count %d\n", resource_name, res);
				else {
					ast_log(LOG_WARNING, "Soft unload failed, '%s' has use count %d\n", resource_name, res);
					ast_mutex_unlock(&modlock);
					return -1;
				}
			}
			res = m->unload_module();
			if (res) {
				ast_log(LOG_WARNING, "Firm unload failed for %s\n", resource_name);
				if (force <= AST_FORCE_FIRM) {
					ast_mutex_unlock(&modlock);
					return -1;
				} else
					ast_log(LOG_WARNING, "** Dangerous **: Unloading resource anyway, at user request\n");
			}
			if (ml)
				ml->next = m->next;
			else
				module_list = m->next;
			free(m);
			break;
		}
		ml = m;
		m = m->next;
	}
	modlistver = rand();
	ast_mutex_unlock(&modlock);
	ast_update_use_count();
	return res;
}

char *ast_module_helper(char *line, char *word, int pos, int state, int rpos, int needsreload)
{
	struct module *m;
	int which=0;
	char *ret;

	if (pos != rpos)
		return NULL;
	ast_mutex_lock(&modlock);
	m = module_list;
	while(m) {
		if (!strncasecmp(word, m->resource, strlen(word)) && (m->reload || !needsreload)) {
			if (++which > state)
				break;
		}
		m = m->next;
	}
	if (m) {
		ret = strdup(m->resource);
	} else {
		ret = NULL;
		if (!strncasecmp(word, "extconfig", strlen(word))) {
			if (++which > state)
				ret = strdup("extconfig");
		} else if (!strncasecmp(word, "manager", strlen(word))) {
			if (++which > state)
				ret = strdup("manager");
		} else if (!strncasecmp(word, "enum", strlen(word))) {
			if (++which > state)
				ret = strdup("enum");
		} else if (!strncasecmp(word, "rtp", strlen(word))) {
			if (++which > state)
				ret = strdup("rtp");
		}
			
	}
	ast_mutex_unlock(&modlock);
	return ret;
}

int ast_module_reload(const char *name, int check_conf_file)
{
	struct module *m;
	int reloaded = 0;
	int oldversion;
	int (*reload)(void);
	/* We'll do the logger and manager the favor of calling its reload here first */

	if (ast_mutex_trylock(&reloadlock)) {
		ast_verbose("The previous reload command didn't finish yet\n");
		return -1;
	}
	if (!name || !strcasecmp(name, "extconfig")) {
		read_config_maps();
		reloaded = 2;
	}
	if (!name || !strcasecmp(name, "manager")) {
		reload_manager(check_conf_file);
		reloaded = 2;
	}
	if (!name || !strcasecmp(name, "cdr")) {
		ast_cdr_engine_reload();
		reloaded = 2;
	}
	if (!name || !strcasecmp(name, "enum")) {
		ast_enum_reload();
		reloaded = 2;
	}
	if (!name || !strcasecmp(name, "rtp")) {
		ast_rtp_reload(check_conf_file);
		reloaded = 2;
	}
	if (!name || !strcasecmp(name, "dnsmgr")) {
		dnsmgr_reload();
		reloaded = 2;
	}
	time(&ast_lastreloadtime);

	ast_mutex_lock(&modlock);
	oldversion = modlistver;
	m = module_list;
	while(m) {
		if (!name || !strcasecmp(name, m->resource)) {
			if (reloaded < 1)
				reloaded = 1;
			if (check_conf_file && m->reload_if_changed)
				reload = m->reload_if_changed;
			else
				reload = m->reload;
			ast_mutex_unlock(&modlock);
			if (reload) {
				reloaded = 2;
				if (option_verbose > 2) 
					ast_verbose(VERBOSE_PREFIX_3 "Reloading module '%s' (%s)\n", m->resource, m->description());
				reload();
			}
			ast_mutex_lock(&modlock);
			if (oldversion != modlistver)
				break;
		}
		m = m->next;
	}
	ast_mutex_unlock(&modlock);
	ast_mutex_unlock(&reloadlock);
	return reloaded;
}

static int __load_resource(const char *resource_name, const struct ast_config *cfg)
{
	static char fn[256];
	int errors=0;
	int res;
	struct module *m;
	int flags=RTLD_NOW;
#ifdef RTLD_GLOBAL
	char *val;
#endif
	unsigned char *key;
	char tmp[80];
	module_functions_t *module;

	if (strncasecmp(resource_name, "res_", 4)) {
#ifdef RTLD_GLOBAL
		if (cfg) {
			if ((val = ast_variable_retrieve(cfg, "global", resource_name))
					&& ast_true(val))
				flags |= RTLD_GLOBAL;
		}
#endif
	} else {
		/* Resource modules are always loaded global and lazy */
#ifdef RTLD_GLOBAL
		flags = (RTLD_GLOBAL | RTLD_LAZY);
#else
		flags = RTLD_LAZY;
#endif
	}
	
	if (ast_mutex_lock(&modlock))
		ast_log(LOG_WARNING, "Failed to lock\n");
	m = module_list;
	while(m) {
		if (!strcasecmp(m->resource, resource_name)) {
			ast_log(LOG_WARNING, "Module '%s' already exists\n", resource_name);
			ast_mutex_unlock(&modlock);
			return -1;
		}
		m = m->next;
	}
	m = malloc(sizeof(struct module));	
	if (!m) {
		ast_log(LOG_WARNING, "Out of memory\n");
		ast_mutex_unlock(&modlock);
		return -1;
	}
	strncpy(m->resource, resource_name, sizeof(m->resource)-1);
	if (resource_name[0] == '/') {
		strncpy(fn, resource_name, sizeof(fn)-1);
	} else {
		snprintf(fn, sizeof(fn), "%s/%s", (char *)ast_config_AST_MODULE_DIR, resource_name);
	}
	
	/* find the relevant module, by name, from the module array */
	for (module = modules; strcmp(module->name, resource_name); module++);

	m->load_module = module->load_module;
	if (!m->load_module) {
		ast_log(LOG_WARNING, "No load_module in module %s\n", fn);
		errors++;
	}
	m->unload_module = module->unload_module;
	if (!m->unload_module) {
		ast_log(LOG_WARNING, "No unload_module in module %s\n", fn);
		errors++;
	}
	m->usecount = module->usecount;
	if (!m->usecount) {
		ast_log(LOG_WARNING, "No usecount in module %s\n", fn);
		errors++;
	}
	m->description = module->description;
	if (!m->description) {
		ast_log(LOG_WARNING, "No description in module %s\n", fn);
		errors++;
	}
	m->key = module->key;
	if (!m->key) {
		ast_log(LOG_WARNING, "No key routine in module %s\n", fn);
		errors++;
	}

	m->reload = module->reload;

	m->reload_if_changed = module->reload_if_changed;

	if (!m->key || !(key = (unsigned char *) m->key())) {
		ast_log(LOG_WARNING, "Key routine returned NULL in module %s\n", fn);
		key = NULL;
		errors++;
	}
	if (key && verify_key(key)) {
		ast_log(LOG_WARNING, "Unexpected key returned by module %s\n", fn);
		errors++;
	}
	if (errors) {
		ast_log(LOG_WARNING, "%d error%s loading module %s, aborted\n", errors, (errors != 1) ? "s" : "", fn);
		free(m);
		ast_mutex_unlock(&modlock);
		return -1;
	}
	if (!fully_booted) {
		if (option_verbose) 
			ast_verbose( " => (%s)\n", term_color(tmp, m->description(), COLOR_BROWN, COLOR_BLACK, sizeof(tmp)));
		if (option_console && !option_verbose)
			ast_verbose( ".");
	} else {
		if (option_verbose)
			ast_verbose(VERBOSE_PREFIX_1 "Loaded %s => (%s)\n", fn, m->description());
	}

	/* add module 'm' to end of module_list chain
  	   so reload commands will be issued in same order modules were loaded */
	m->next = NULL;
	if (module_list == NULL) {
		/* empty list so far, add at front */
		module_list = m;
	}
	else {
		struct module *i;
		/* find end of chain, and add there */
		for (i = module_list; i->next; i = i->next)
			;
		i->next = m;
	}
	
	modlistver = rand();
	ast_mutex_unlock(&modlock);
	if ((res = m->load_module())) {
		ast_log(LOG_WARNING, "%s: load_module failed, returning %d\n", m->resource, res);
		ast_unload_resource(resource_name, 0);
		return -1;
	}
	ast_update_use_count();
	return 0;
}

int ast_load_resource(const char *resource_name)
{
	int o;
	struct ast_config *cfg = NULL;
	int res;

	/* Keep the module file parsing silent */
	o = option_verbose;
	option_verbose = 0;
	cfg = ast_config_load(AST_MODULE_CONFIG);
	option_verbose = o;
	res = __load_resource(resource_name, cfg);
	if (cfg)
		ast_config_destroy(cfg);
	return res;
}	

static int ast_resource_exists(char *resource)
{
	struct module *m;
	if (ast_mutex_lock(&modlock))
		ast_log(LOG_WARNING, "Failed to lock\n");
	m = module_list;
	while(m) {
		if (!strcasecmp(resource, m->resource))
			break;
		m = m->next;
	}
	ast_mutex_unlock(&modlock);
	if (m)
		return -1;
	else
		return 0;
}

static const char *loadorder[] =
{
	"res_",
	"pbx_",
	"chan_",
	NULL,
};

int load_modules(const int preload_only)
{
	struct ast_config *cfg;
	struct ast_variable *v;
	char tmp[80];

	if (option_verbose) {
		if (preload_only)
			ast_verbose("Asterisk Dynamic Loader loading preload modules:\n");
		else
			ast_verbose("Asterisk Dynamic Loader Starting:\n");
	}

	cfg = ast_config_load(AST_MODULE_CONFIG);
	if (cfg) {
		int doload;

		/* Load explicitly defined modules */
		for (v = ast_variable_browse(cfg, "modules"); v; v = v->next) {
			doload = 0;

			if (preload_only)
				doload = !strcasecmp(v->name, "preload");
			else
				doload = !strcasecmp(v->name, "load");

		       if (doload) {
				if (option_debug && !option_verbose)
					ast_log(LOG_DEBUG, "Loading module %s\n", v->value);
				if (option_verbose) {
					ast_verbose(VERBOSE_PREFIX_1 "[%s]", term_color(tmp, v->value, COLOR_BRWHITE, 0, sizeof(tmp)));
					fflush(stdout);
				}
				if (__load_resource(v->value, cfg)) {
					ast_log(LOG_WARNING, "Loading module %s failed!\n", v->value);
					ast_config_destroy(cfg);
					return -1;
				}
			}
		}
	}

	if (preload_only) {
		ast_config_destroy(cfg);
		return 0;
	}

	if (!cfg || ast_true(ast_variable_retrieve(cfg, "modules", "autoload"))) {
		/* Load all modules */
		int x;
		module_functions_t  *d;

		/* Loop through each order */
		for (x=0; x<sizeof(loadorder) / sizeof(loadorder[0]); x++) {
			/* else part related to case where diropen fails - not
			 * relevant (we have no shared libraries directory)
			 */
			if (1) {
				for (d = modules; d->name; d++) {
					/* Must end in .so to load it.  */
					if ((strlen(d->name) > 3) && 
					    (!loadorder[x] || !strncasecmp(d->name, loadorder[x], strlen(loadorder[x]))) 
					    &&
						!ast_resource_exists(d->name)) {
						/* It's a shared library -- Just be sure we're allowed to load it -- kinda
						   an inefficient way to do it, but oh well. */
						if (cfg) {
							v = ast_variable_browse(cfg, "modules");
							while(v) {
								if (!strcasecmp(v->name, "noload") &&
								    !strcasecmp(v->value, d->name)) 
									break;
								v = v->next;
							}
							if (v) {
								if (option_verbose) {
									ast_verbose( VERBOSE_PREFIX_1 "[skipping %s]\n", d->name);
									fflush(stdout);
								}
								continue;
							}
							
						}
						if (option_debug && !option_verbose)
							ast_log(LOG_DEBUG, "Loading module %s\n", d->name);
						if (option_verbose) {
							ast_verbose( VERBOSE_PREFIX_1 "[%s]", term_color(tmp, d->name, COLOR_BRWHITE, 0, sizeof(tmp)));
							fflush(stdout);
						}
						if (__load_resource(d->name, cfg)) {
							ast_log(LOG_WARNING, "Loading module %s failed!\n", d->name);
							if (cfg)
								ast_config_destroy(cfg);
							return -1;
						}
					}
				}
			} else {
				if (!option_quiet)
					ast_log(LOG_WARNING, "Unable to open modules directory %s.\n", (char *)ast_config_AST_MODULE_DIR);
			}
		}
	} 
	ast_config_destroy(cfg);
	return 0;
}

void ast_update_use_count(void)
{
	/* Notify any module monitors that the use count for a 
	   resource has changed */
	struct loadupdate *m;
	if (ast_mutex_lock(&modlock))
		ast_log(LOG_WARNING, "Failed to lock\n");
	m = updaters;
	while(m) {
		m->updater();
		m = m->next;
	}
	ast_mutex_unlock(&modlock);
	
}

int ast_update_module_list(int (*modentry)(const char *module, const char *description, int usecnt, const char *like),
			   const char *like)
{
	struct module *m;
	int unlock = -1;
	int total_mod_loaded = 0;

	if (ast_mutex_trylock(&modlock))
		unlock = 0;
	m = module_list;
	while (m) {
		total_mod_loaded += modentry(m->resource, m->description(), m->usecount(), like);
		m = m->next;
	}
	if (unlock)
		ast_mutex_unlock(&modlock);

	return total_mod_loaded;
}

int ast_loader_register(int (*v)(void)) 
{
	struct loadupdate *tmp;
	/* XXX Should be more flexible here, taking > 1 verboser XXX */
	if ((tmp = malloc(sizeof (struct loadupdate)))) {
		tmp->updater = v;
		if (ast_mutex_lock(&modlock))
			ast_log(LOG_WARNING, "Failed to lock\n");
		tmp->next = updaters;
		updaters = tmp;
		ast_mutex_unlock(&modlock);
		return 0;
	}
	return -1;
}

int ast_loader_unregister(int (*v)(void))
{
	int res = -1;
	struct loadupdate *tmp, *tmpl=NULL;
	if (ast_mutex_lock(&modlock))
		ast_log(LOG_WARNING, "Failed to lock\n");
	tmp = updaters;
	while(tmp) {
		if (tmp->updater == v)	{
			if (tmpl)
				tmpl->next = tmp->next;
			else
				updaters = tmp->next;
			break;
		}
		tmpl = tmp;
		tmp = tmp->next;
	}
	if (tmp)
		res = 0;
	ast_mutex_unlock(&modlock);
	return res;
}
