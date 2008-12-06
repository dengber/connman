/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2007-2008  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __CONNMAN_PLUGIN_H
#define __CONNMAN_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SECTION:plugin
 * @title: Plugin premitives
 * @short_description: Functions for declaring plugins
 */

struct connman_plugin_desc {
	const char *name;
	const char *description;
	const char *version;
	int (*init) (void);
	void (*exit) (void);
};

/**
 * CONNMAN_PLUGIN_DEFINE:
 * @name: plugin name
 * @description: plugin description
 * @version: plugin version string
 * @init: init function called on plugin loading
 * @exit: exit function called on plugin removal
 *
 * Macro for defining a plugin descriptor
 */
#define CONNMAN_PLUGIN_DEFINE(name, description, version, init, exit) \
		struct connman_plugin_desc connman_plugin_desc = { \
			#name, description, version, init, exit \
		};

#ifdef __cplusplus
}
#endif

#endif /* __CONNMAN_PLUGIN_H */
