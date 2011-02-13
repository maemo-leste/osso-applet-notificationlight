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
#include <gconf/gconf-client.h>
#include <gdk/gdkx.h>

#define GCONF_KEY_LED_NOTIFICATION    "/system/osso/dsm/leds/"
#define GCONF_KEY_DEVICE_ON           GCONF_KEY_LED_NOTIFICATION "PatternDeviceOn"
#define GCONF_KEY_COMMUNICATION_CALL  GCONF_KEY_LED_NOTIFICATION "PatternCommunicationCall"
#define GCONF_KEY_COMMUNICATION_SMS   GCONF_KEY_LED_NOTIFICATION "PatternCommunicationSMS"
#define GCONF_KEY_COMMUNICATION_EMAIL GCONF_KEY_LED_NOTIFICATION "PatternCommunicationEmail"
#define GCONF_KEY_COMMUNICATION_IM    GCONF_KEY_LED_NOTIFICATION "PatternCommunicationIM"
#define GCONF_KEY_BATTERY_CHARGING    GCONF_KEY_LED_NOTIFICATION "PatternBatteryCharging"
#define GCONF_KEY_COMMON_NOTIFICATION GCONF_KEY_LED_NOTIFICATION "PatternCommonNotification"

static void
_size_changed(GdkScreen* screen,
	      gpointer user_data)
{
  GdkGeometry geometry;
  if(gdk_screen_get_width(gdk_display_get_default_screen(gdk_display_get_default())) < 800)
  {
    geometry.min_height = 680;
  }
  else
  {
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
  GConfClient* gconf_client = gconf_client_get_default();
  g_assert(GCONF_IS_CLIENT(gconf_client));
  
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
  gtk_button_set_label(GTK_BUTTON(missed_call_check_button),dgettext("osso-display", "disp_li_led_missed_calls"));
  gtk_button_set_label(GTK_BUTTON(text_msg_check_button),dgettext("osso-display", "disp_li_led_received_sms"));
  gtk_button_set_label(GTK_BUTTON(email_msg_check_button),dgettext("osso-display", "disp_li_led_received_email"));
  gtk_button_set_label(GTK_BUTTON(im_msg_check_button),dgettext("osso-display", "disp_li_led_received_chat"));
  gtk_button_set_label(GTK_BUTTON(charging_check_button),dgettext("osso-display", "disp_li_led_charging"));
  gtk_button_set_label(GTK_BUTTON(other_check_button),dgettext("osso-display", "disp_li_led_other_notifications"));
  
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(device_on_check_button), 
				 gconf_client_get_bool(gconf_client, GCONF_KEY_DEVICE_ON, NULL));
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(missed_call_check_button),
				 gconf_client_get_bool(gconf_client, GCONF_KEY_COMMUNICATION_CALL, NULL));
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(text_msg_check_button),
				 gconf_client_get_bool(gconf_client, GCONF_KEY_COMMUNICATION_SMS, NULL));
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(email_msg_check_button),
				 gconf_client_get_bool(gconf_client, GCONF_KEY_COMMUNICATION_EMAIL, NULL));
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(im_msg_check_button),
				 gconf_client_get_bool(gconf_client, GCONF_KEY_COMMUNICATION_IM, NULL));
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(charging_check_button),
				 gconf_client_get_bool(gconf_client, GCONF_KEY_BATTERY_CHARGING, NULL));
  hildon_check_button_set_active(HILDON_CHECK_BUTTON(other_check_button),
				 gconf_client_get_bool(gconf_client, GCONF_KEY_COMMON_NOTIFICATION, NULL));
  
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
  if(gdk_screen_get_width(gdk_display_get_default_screen(gdk_display_get_default())) < 800)
  {
    geometry.min_height = 650;
  }
  else
  {
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
  
  if(response == GTK_RESPONSE_ACCEPT)
  {
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_DEVICE_ON, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(device_on_check_button)), 
			  NULL);
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_COMMUNICATION_CALL, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(missed_call_check_button)), 
			  NULL);
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_COMMUNICATION_SMS, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(text_msg_check_button)), 
			  NULL);
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_COMMUNICATION_EMAIL, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(email_msg_check_button)), 
			  NULL);
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_COMMUNICATION_IM, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(im_msg_check_button)), 
			  NULL);
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_BATTERY_CHARGING, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(charging_check_button)), 
			  NULL);
    gconf_client_set_bool(gconf_client, 
			  GCONF_KEY_COMMON_NOTIFICATION, 
			  hildon_check_button_get_active(HILDON_CHECK_BUTTON(other_check_button)), 
			  NULL);
  }
  gtk_widget_destroy(dialog);
  g_object_unref(gconf_client);
  return OSSO_OK;
}

osso_return_t save_state(osso_context_t* osso, gpointer user_data)
{
  return OSSO_OK;
}
