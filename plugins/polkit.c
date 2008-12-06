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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include <glib.h>
#include <polkit-dbus/polkit-dbus.h>

#include <connman/plugin.h>
#include <connman/security.h>
#include <connman/log.h>

#define ACTION "org.moblin.connman.modify"

static DBusConnection *connection;
static PolKitContext *polkit_context;

static int polkit_authorize(const char *sender)
{
	DBusError error;
	PolKitCaller *caller;
	PolKitAction *action;
	PolKitResult result;

	DBG("sender %s", sender);

	dbus_error_init(&error);

	caller = polkit_caller_new_from_dbus_name(connection, sender, &error);
	if (caller == NULL) {
		if (dbus_error_is_set(&error) == TRUE) {
			connman_error("%s", error.message);
			dbus_error_free(&error);
		} else
			connman_error("Failed to get caller information");
		return -EIO;
	}

	action = polkit_action_new();
	polkit_action_set_action_id(action, ACTION);

	result = polkit_context_is_caller_authorized(polkit_context,
						action, caller, TRUE, NULL);

	polkit_action_unref(action);
	polkit_caller_unref(caller);

	DBG("result %s", polkit_result_to_string_representation(result));

	if (result == POLKIT_RESULT_NO)
		return -EPERM;

	return 0;
}

static struct connman_security polkit_security = {
	.name			= "polkit",
	.authorize_sender	= polkit_authorize,
};

static gboolean watch_event(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	PolKitContext *context = user_data;
	int fd;

	DBG("context %p", context);

	fd = g_io_channel_unix_get_fd(channel);

	polkit_context_io_func(context, fd);

	return TRUE;
}

static int add_watch(PolKitContext *context, int fd)
{
	GIOChannel *channel;
	guint id = 0;

	DBG("context %p", context);

	channel = g_io_channel_unix_new(fd);
	if (channel == NULL)
		return 0;

	id = g_io_add_watch(channel, G_IO_IN, watch_event, context);

	g_io_channel_unref(channel);

	return id;
}

static void remove_watch(PolKitContext *context, int id)
{
	DBG("context %p", context);

	g_source_remove(id);
}

static int polkit_init(void)
{
	int err;

	connection = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (connection == NULL)
		return -EIO;

	polkit_context = polkit_context_new();

	polkit_context_set_io_watch_functions(polkit_context,
						add_watch, remove_watch);

	if (polkit_context_init(polkit_context, NULL) == FALSE) {
		connman_error("Can't initialize PolicyKit");
		polkit_context_unref(polkit_context);
		dbus_connection_unref(connection);
		return -EIO;
	}

	err = connman_security_register(&polkit_security);
	if (err < 0) {
		polkit_context_unref(polkit_context);
		dbus_connection_unref(connection);
		return err;
	}

	return 0;
}

static void polkit_exit(void)
{
	connman_security_unregister(&polkit_security);

	polkit_context_unref(polkit_context);

	dbus_connection_unref(connection);
}

CONNMAN_PLUGIN_DEFINE(polkit, "PolicyKit authorization plugin", VERSION,
						polkit_init, polkit_exit)
