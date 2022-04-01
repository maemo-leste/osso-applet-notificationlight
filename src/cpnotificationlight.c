/*
 *  notificationlight control panel plugin
 *  Copyright (C) 2011 Nicolai Hess
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <hildon-cp-plugin/hildon-cp-plugin-interface.h>
#include <hildon/hildon.h>
#include <gtk/gtk.h>
#include <libintl.h>
#include <string.h>
#include <locale.h>
#include <gdk/gdkx.h>
#include <mce/dbus-names.h>

#define KEY_DEVICE_ON           "PatternDeviceOn"
#define KEY_COMMUNICATION_CALL  "PatternCommunicationCall"
#define KEY_COMMUNICATION_SMS   "PatternCommunicationSMS"
#define KEY_COMMUNICATION_EMAIL "PatternCommunicationEmail"
#define KEY_COMMUNICATION_IM    "PatternCommunicationIM"
#define KEY_BATTERY_CHARGING    "PatternBatteryCharging"
#define KEY_COMMON_NOTIFICATION "PatternCommonNotification"


static GDBusConnection *get_dbus_connection(void)
{
	GError *error = NULL;
	char *addr;
	
	GDBusConnection *s_bus_conn;

	addr = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	if (addr == NULL) {
		g_error("fail to get dbus addr: %s\n", error->message);
		g_free(error);
		return NULL;
	}

	s_bus_conn = g_dbus_connection_new_for_address_sync(addr,
			G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
			G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
			NULL, NULL, &error);

	if (s_bus_conn == NULL) {
		g_error("fail to create dbus connection: %s\n", error->message);
		g_free(error);
	}

	return s_bus_conn;
}

static gboolean mce_get_pattern_bool(GDBusConnection *bus, const gchar *key, gboolean defval)
{
	GVariant *result;

	GError *error = NULL;

	result = g_dbus_connection_call_sync(bus, MCE_SERVICE, MCE_REQUEST_PATH,
		MCE_REQUEST_IF, MCE_LED_PATTERN_DISABLED, g_variant_new("(s)", key), NULL,
		G_DBUS_CALL_FLAGS_NONE, 2000, NULL, &error);
	
	if (error || !result) {
		g_critical("%s: Can not get value for %s", __func__, MCE_LED_PATTERN_DISABLED);
		return defval;
	}

	GVariantType *int_variant = g_variant_type_new("(b)");

	if (!g_variant_is_of_type(result, int_variant)) {
		g_critical("%s: Can not get value for %s wrong type: %s instead of (b)",
				   __func__, MCE_LED_PATTERN_DISABLED, g_variant_get_type_string(result));
		g_variant_unref(result);
		g_variant_type_free(int_variant);
		return defval;
	}

	g_variant_type_free(int_variant);

	gboolean value;
	g_variant_get(result, "(b)", &value);
	g_variant_unref(result);
	return value;
}

static gboolean mce_set_pattern_bool(GDBusConnection *bus, const gchar *key, gboolean val)
{
	GVariant *result;
	GError *error = NULL;
	
	const gchar *request = val ? MCE_ENABLE_LED_PATTERN : MCE_DISABLE_LED_PATTERN;

	result = g_dbus_connection_call_sync(bus, MCE_SERVICE, MCE_REQUEST_PATH,
		MCE_REQUEST_IF, request, g_variant_new("(s)", key), NULL,
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);

	if (error) {
		g_critical("%s: Failed to send %s to mce", __func__, request);
		return FALSE;
	}

	g_variant_unref(result);

	return TRUE;
}

static void _size_changed(GdkScreen* screen, gpointer user_data)
{
	GdkGeometry geometry;
	if(gdk_screen_get_width(gdk_display_get_default_screen(gdk_display_get_default())) < 800) {
		geometry.min_height = 680;
	} else {
		geometry.min_height = 360;
	}
	gtk_window_set_geometry_hints(GTK_WINDOW(user_data),
	                              GTK_WIDGET(user_data),
	                              &geometry,
	                              GDK_HINT_MIN_SIZE);
}

osso_return_t execute(osso_context_t* osso, gpointer user_data, gboolean user_activated)
{
	GtkWidget* dialog = gtk_dialog_new_with_buttons(dgettext("osso-display", "led_ti_led"),
	                    GTK_WINDOW(user_data),
	                    GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
	                    dgettext("hildon-libs", "wdgt_bd_save"), GTK_RESPONSE_ACCEPT,
	                    NULL);

	GDBusConnection *s_bus = get_dbus_connection();
	g_assert(s_bus);

	GtkWidget* pan = hildon_pannable_area_new();
	GtkWidget* box = gtk_vbox_new(TRUE, 0);
	GtkWidget* device_on_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	GtkWidget* missed_call_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	GtkWidget* text_msg_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	GtkWidget* email_msg_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	GtkWidget* im_msg_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	GtkWidget* charging_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);
	GtkWidget* other_check_button = hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT);

	gtk_button_set_label(GTK_BUTTON(device_on_check_button),dgettext("osso-display","disp_li_led_device_on"));
	gtk_button_set_label(GTK_BUTTON(missed_call_check_button),dgettext("osso-display",
	                     "disp_li_led_missed_calls"));
	gtk_button_set_label(GTK_BUTTON(text_msg_check_button),dgettext("osso-display", "disp_li_led_received_sms"));
	gtk_button_set_label(GTK_BUTTON(email_msg_check_button),dgettext("osso-display",
	                     "disp_li_led_received_email"));
	gtk_button_set_label(GTK_BUTTON(im_msg_check_button),dgettext("osso-display", "disp_li_led_received_chat"));
	gtk_button_set_label(GTK_BUTTON(charging_check_button),dgettext("osso-display", "disp_li_led_charging"));
	gtk_button_set_label(GTK_BUTTON(other_check_button),dgettext("osso-display",
	                     "disp_li_led_other_notifications"));

	hildon_check_button_set_active(HILDON_CHECK_BUTTON(device_on_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_DEVICE_ON, FALSE));
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(missed_call_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_COMMUNICATION_CALL, FALSE));
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(text_msg_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_COMMUNICATION_SMS, FALSE));
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(email_msg_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_COMMUNICATION_EMAIL, FALSE));
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(im_msg_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_COMMUNICATION_IM, FALSE));
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(charging_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_BATTERY_CHARGING, FALSE));
	hildon_check_button_set_active(HILDON_CHECK_BUTTON(other_check_button),
	                               mce_get_pattern_bool(s_bus, KEY_COMMON_NOTIFICATION, FALSE));

	gtk_box_pack_start(GTK_BOX(box), device_on_check_button, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), missed_call_check_button, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), text_msg_check_button, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), email_msg_check_button, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), im_msg_check_button, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), charging_check_button, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), other_check_button, TRUE, FALSE, 0);
	hildon_pannable_area_add_with_viewport(HILDON_PANNABLE_AREA(pan), box);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), pan, TRUE, TRUE, 0);

	GdkGeometry geometry;
	if(gdk_screen_get_width(gdk_display_get_default_screen(gdk_display_get_default())) < 800) {
		geometry.min_height = 650;
	} else {
		geometry.min_height = 360;
	}
	gtk_window_set_geometry_hints(GTK_WINDOW(dialog),
	                              dialog,
	                              &geometry,
	                              GDK_HINT_MIN_SIZE);
	g_signal_connect(gdk_display_get_default_screen(gdk_display_get_default()),
	                 "size-changed",
	                 G_CALLBACK(_size_changed), dialog);
	gtk_widget_show_all(dialog);
	guint response = gtk_dialog_run(GTK_DIALOG(dialog));

	if(response == GTK_RESPONSE_ACCEPT) {
		mce_set_pattern_bool(s_bus,
		                      KEY_DEVICE_ON,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(device_on_check_button)));
		mce_set_pattern_bool(s_bus,
		                      KEY_COMMUNICATION_CALL,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(missed_call_check_button)));
		mce_set_pattern_bool(s_bus,
		                      KEY_COMMUNICATION_SMS,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(text_msg_check_button)));
		mce_set_pattern_bool(s_bus,
		                      KEY_COMMUNICATION_EMAIL,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(email_msg_check_button)));
		mce_set_pattern_bool(s_bus,
		                      KEY_COMMUNICATION_IM,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(im_msg_check_button)));
		mce_set_pattern_bool(s_bus,
		                      KEY_BATTERY_CHARGING,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(charging_check_button)));
		mce_set_pattern_bool(s_bus,
		                      KEY_COMMON_NOTIFICATION,
		                      hildon_check_button_get_active(HILDON_CHECK_BUTTON(other_check_button)));
	}
	gtk_widget_destroy(dialog);
	g_object_unref(s_bus);
	return OSSO_OK;
}

osso_return_t save_state(osso_context_t* osso, gpointer user_data)
{
	return OSSO_OK;
}
