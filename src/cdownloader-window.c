/* cdownloader-window.c
 *
 * Copyright 2024 sdf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include <glib/gprintf.h>

#include "cdownloader-window.h"

struct _CdownloaderWindow
{
	AdwApplicationWindow  parent_instance;

	/* Template widgets */
	AdwHeaderBar        *header_bar;
	GtkLabel            *folderLabel;

	GtkButton* saveLocationB;
	GtkButton* installB;
	GtkEntry* entry;
	AdwToastOverlay* tOverlay;
	GSettings* gsettings;
	GString* location;
};

G_DEFINE_FINAL_TYPE (CdownloaderWindow, cdownloader_window, ADW_TYPE_APPLICATION_WINDOW)

static void
cdownloader_window_class_init (CdownloaderWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/yt/vid/downloader/cdownloader-window.ui");
	gtk_widget_class_bind_template_child (widget_class, CdownloaderWindow, header_bar);
	gtk_widget_class_bind_template_child (widget_class, CdownloaderWindow, folderLabel);
	gtk_widget_class_bind_template_child (widget_class, CdownloaderWindow, saveLocationB);
	gtk_widget_class_bind_template_child (widget_class, CdownloaderWindow, installB);
	gtk_widget_class_bind_template_child (widget_class, CdownloaderWindow, entry);
	gtk_widget_class_bind_template_child (widget_class, CdownloaderWindow, tOverlay);
}

static void
show_toast(CdownloaderWindow* self, const gchar* msg)
{
	AdwToast* toast;
	toast = adw_toast_new(msg);

	adw_toast_overlay_add_toast(self->tOverlay, toast);
}

static gchar*
format_location(GString *loc)
{
	for (int i = loc->len - 1; i >= 0; i--) {
		if (G_IS_DIR_SEPARATOR(loc->str[i])) {
			return loc->str + i + 1;
		}
	}

	return loc->str;
}

static void
set_save_location(CdownloaderWindow *self, GString* curr)
{
	GString* loc = self->location;

	g_string_replace(curr, "~", g_get_home_dir(), 0);

	if (loc != NULL) {
		g_string_replace(loc, loc->str, curr->str, 1);
	} else {
		loc = g_string_new(curr->str);
		self->location = loc;
	}

	g_settings_set_string(self->gsettings, "save-location", loc->str);
}

static GString*
get_save_location(CdownloaderWindow *self)
{
	return g_string_new(self->location->str);
}

static void
install_start_cb(GObject* oProcYtDlp, GAsyncResult* res, gpointer data)
{
	GSubprocess* procYtDlp;
	gboolean success;
	CdownloaderWindow* parent = data;

	procYtDlp = (GSubprocess*)oProcYtDlp;

	success = g_subprocess_wait_check_finish(procYtDlp, res, NULL);

	if (!success) {
		show_toast(parent, "failure!");
		return;
	}

	show_toast(parent, "success!");
}

static void
installB_clicked_cb(GtkButton *self, CdownloaderWindow *parent)
{
	const char* url;
	GtkEntryBuffer* buffer;
	GSubprocess* procYtDlp;
	GString* format;
	GError* err;
	gchar separator_str[2];

	err = NULL;
	buffer = gtk_entry_get_buffer(parent->entry);
	url = gtk_entry_buffer_get_text(buffer);
	
	format = get_save_location(parent);

	separator_str[1] = 0;
	separator_str[0] = G_DIR_SEPARATOR; 

	g_string_append(format, separator_str);
	g_string_append(format, "/%(title)s-%(id)s.%(ext)s");

	procYtDlp = g_subprocess_new(G_SUBPROCESS_FLAGS_NONE, &err, "yt-dlp", url, "-o", format->str, NULL);

	g_free(format);

	if (procYtDlp == NULL) {
		g_printerr("%s\n", err->message);
		g_clear_error(&err);
		return;
	}

	g_subprocess_wait_check_async(procYtDlp, NULL, install_start_cb, parent);
}


static void
put_save_location_cb(GObject* oFdialog, GAsyncResult* res, gpointer data)
{
	CdownloaderWindow *self;
	GFile* new_loc_folder;
	GtkFileDialog* fdialog;
	GString* loc;
	gchar *loc_str;

	GError* err = NULL;

	self = data;
	fdialog = GTK_FILE_DIALOG(oFdialog);

	new_loc_folder = gtk_file_dialog_select_folder_finish(fdialog, res, &err);

	if (new_loc_folder == NULL) {
		g_printerr("Error: %s\n", err->message);
		g_clear_error(&err);
		g_object_unref(GTK_FILE_DIALOG(oFdialog));
		return;
	}

	loc_str = g_file_get_path(new_loc_folder);
	loc = g_string_new(loc_str);

	gtk_label_set_label(self->folderLabel, format_location(loc));
	set_save_location(self, loc);
	
	g_free(loc);
	g_free(loc_str);
	g_object_unref(GTK_FILE_DIALOG(oFdialog));
}

static void
saveLocationB_clicked_cb(GtkButton* self, CdownloaderWindow *parent)
{
	GString* location;
	GtkFileDialog* fdialog;
	GFile* folder;

	location = get_save_location(parent);

	folder = g_file_new_for_path(location->str);

	fdialog = gtk_file_dialog_new();
	gtk_file_dialog_set_initial_folder(fdialog, folder);

	gtk_file_dialog_select_folder(fdialog, GTK_WINDOW(parent), NULL, put_save_location_cb, parent);

	g_free(location);
	g_object_unref(folder);
}

static void
cdownloader_window_init (CdownloaderWindow *self)
{
	GtkButton* installB;
	GString* location;
	gchar* str;

	gtk_widget_init_template (GTK_WIDGET (self));

	installB = self->installB;

	self->location = NULL;
	self->gsettings = g_settings_new("yt.vid.downloader");

	str = g_settings_get_string(self->gsettings, "save-location");
	location = g_string_new(str);
	g_string_replace(location, "~", g_get_home_dir(), 0);

	set_save_location(self, location);

	if (location != NULL) {
		gtk_label_set_label(self->folderLabel, format_location(location));
		g_free(location);
	}

	g_signal_connect(installB, "clicked", G_CALLBACK(installB_clicked_cb), self);
	g_signal_connect(self->saveLocationB, "clicked", G_CALLBACK(saveLocationB_clicked_cb), self);
}

