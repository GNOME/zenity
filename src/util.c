/*
 * util.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 *           (C) 1999, 2000 Red Hat Inc.
 *           (C) 1998 James Henstridge
 *           (C) 1995-2002 Free Software Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 *          Havoc Pennington <hp@redhat.com>
 *          James Henstridge <james@daa.com.au>
 *          Tom Tromey <tromey@redhat.com>
 */

#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include "config.h"
#include "util.h"
#include "zenity.h"
#include <gconf/gconf-client.h>

#define URL_HANDLER_DIR      "/desktop/gnome/url-handlers/"
#define DEFAULT_HANDLER_PATH "/desktop/gnome/url-handlers/unknown/command"

GladeXML*
zenity_util_load_glade_file (const gchar *widget_root) 
{
  GladeXML *xml = NULL;

  if (g_file_test (ZENITY_GLADE_FILE_RELATIVEPATH, G_FILE_TEST_EXISTS)) {
    /* Try current dir, for debugging */
    xml = glade_xml_new (ZENITY_GLADE_FILE_RELATIVEPATH, widget_root, GETTEXT_PACKAGE);
  }
 
  if (xml == NULL)
    xml = glade_xml_new (ZENITY_GLADE_FILE_FULLPATH, widget_root, GETTEXT_PACKAGE);

  if (xml == NULL) {
    g_warning ("Could not load glade file : %s", ZENITY_GLADE_FILE_FULLPATH);
    return NULL;
  }

  return xml;
}

gchar*
zenity_util_strip_newline (gchar *string)
{
    gsize len;
    
    g_return_val_if_fail (string != NULL, NULL);
                                                                                                                                                                             
    len = strlen (string);
    while (len--) 
    {
      if (string[len] == '\n')
        string[len] = '\0';
      else
        break;
    }
            
    return string;
}

gboolean
zenity_util_fill_file_buffer (GtkTextBuffer *buffer, const gchar *filename) 
{
  GtkTextIter iter, end;
  FILE *f;
  gchar buf[2048];
  gint remaining = 0;

  if (filename == NULL)
    return FALSE;

  f = fopen (filename, "r");

  if (f == NULL) {
    g_warning ("Cannot open file '%s': %s", filename, g_strerror (errno));
    return FALSE;
  }

  gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

  while (!feof (f)) {
    gint count;
    const char *leftover;
    int to_read = 2047  - remaining;

    count = fread (buf + remaining, 1, to_read, f);
    buf[count + remaining] = '\0';

    g_utf8_validate (buf, count + remaining, &leftover);

    g_assert (g_utf8_validate (buf, leftover - buf, NULL));
    gtk_text_buffer_insert (buffer, &iter, buf, leftover - buf);

    remaining = (buf + remaining + count) - leftover;
    g_memmove (buf, leftover, remaining);

    if (remaining > 6 || count < to_read)
      break;
  }

  if (remaining) {
    g_warning ("Invalid UTF-8 data encountered reading file '%s'", filename);
    return FALSE;
  }

  /* We had a newline in the buffer to begin with. (The buffer always contains
   * a newline, so we delete to the end of the buffer to clean up.
   */
  
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_delete (buffer, &iter, &end);

  gtk_text_buffer_set_modified (buffer, FALSE);

  return TRUE;
}

static GList *
zenity_util_list_from_char_array (const char **s)
{
  GList *list = NULL;
  gint i;

  for (i = 0; s[i]; i++) {
    GdkPixbuf *pixbuf;

    pixbuf = gdk_pixbuf_new_from_file (s[i], NULL);
    if (pixbuf)
      list = g_list_prepend (list, pixbuf);
  }

  return list;
}

static void
zenity_util_free_list (GList *list)
{
  g_list_foreach (list, (GFunc) g_object_unref, NULL);
  g_list_free (list);
}

void
zenity_util_set_window_icon (GtkWidget *widget, const gchar *filename)
{
  const gchar *filenames[2] = { NULL};
  GList *list;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WINDOW (widget));

  if (filename == NULL)
    return;

  filenames[0] = filename;
  list = zenity_util_list_from_char_array (filenames);
  gtk_window_set_icon_list (GTK_WINDOW (widget), list);
  zenity_util_free_list (list);
}

void 
zenity_util_set_window_icon_from_stock (GtkWidget *widget, const gchar *stock_id)
{
  GdkPixbuf *pixbuf;
	
  pixbuf = gtk_widget_render_icon (widget, stock_id, (GtkIconSize) -1, NULL);
  gtk_window_set_icon (GTK_WINDOW (widget), pixbuf);
  g_object_unref (pixbuf);
}

/* This is copied from libgnome/gnome-i18n.c since we try and avoid using
 * the evils of gnome_program_init (), which messes up the commandline
 */

static GHashTable *alias_table = NULL;

/*read an alias file for the locales*/
static void
zenity_read_aliases (char *file)
{
  FILE *fp;
  char buf[256];

  if (!alias_table)
  alias_table = g_hash_table_new (g_str_hash, g_str_equal);

  fp = fopen (file,"r");

  if (!fp)
    return;

  while (fgets (buf,256,fp)) {
    gchar *p;
    g_strstrip (buf);

    if (buf[0]=='#' || buf[0]=='\0')
      continue;

    p = (gchar *) strtok (buf, "\t ");

    if (!p)
      continue;

    p = (gchar *) strtok (NULL, "\t ");

    if(!p)
      continue;

    if (!g_hash_table_lookup (alias_table, buf))
      g_hash_table_insert (alias_table, g_strdup (buf), g_strdup (p));
  }

  fclose (fp);
}

/*return the un-aliased language as a newly allocated string*/
static char *
zenity_unalias_lang (char *lang)
{
  char *p;
  int i;

  if (!alias_table) { 
    zenity_read_aliases ("/usr/share/locale/locale.alias");
    zenity_read_aliases ("/usr/local/share/locale/locale.alias");
    zenity_read_aliases ("/usr/lib/X11/locale/locale.alias");
    zenity_read_aliases ("/usr/openwin/lib/locale/locale.alias");
  }
  
  i = 0;
  while ((p = g_hash_table_lookup (alias_table,lang)) && strcmp (p, lang)) {
    lang = p;
       
    if (i++ == 30) {
      static gboolean said_before = FALSE;

      if (!said_before)
        g_warning (_("Too many alias levels for a locale may indicate a loop"));

      said_before = TRUE;
      return lang;
    }
  }
  return lang;
}

/* Mask for components of locale spec. The ordering here is from
 * least significant to most significant
 */
enum
{
  COMPONENT_CODESET =   1 << 0,
  COMPONENT_TERRITORY = 1 << 1,
  COMPONENT_MODIFIER =  1 << 2
};

/* Break an X/Open style locale specification into components
 */
static guint
zenity_explode_locale (const gchar *locale,
                       gchar      **language, 
                       gchar      **territory, 
                       gchar      **codeset, 
                       gchar      **modifier)
{
  const gchar *uscore_pos;
  const gchar *at_pos; 
  const gchar *dot_pos; 
  guint mask = 0;

  uscore_pos = (const gchar *) strchr (locale, '_'); 
  dot_pos = (const gchar *) strchr (uscore_pos ? uscore_pos : locale, '.'); 
  at_pos = (const gchar *) strchr (dot_pos ? dot_pos : (uscore_pos ? uscore_pos : locale), '@'); 

  if (at_pos) { 
    mask |= COMPONENT_MODIFIER; 
    *modifier = g_strdup (at_pos);
  } 
  else 
    at_pos = locale + strlen (locale);

  if (dot_pos) { 
    mask |= COMPONENT_CODESET; 
    *codeset = g_new (gchar, 1 + at_pos - dot_pos); 
    strncpy (*codeset, dot_pos, at_pos - dot_pos);
    (*codeset) [at_pos - dot_pos] = '\0';
  } 
  else 
    dot_pos = at_pos;

  if (uscore_pos) {
    mask |= COMPONENT_TERRITORY; 
    *territory = g_new (gchar, 1 + dot_pos - uscore_pos); 
    strncpy (*territory, uscore_pos, dot_pos - uscore_pos); 
    (*territory)[dot_pos - uscore_pos] = '\0';
  } 
  else
    uscore_pos = dot_pos;

  *language = g_new (gchar, 1 + uscore_pos - locale); 
  strncpy (*language, locale, uscore_pos - locale); 
  (*language) [uscore_pos - locale] = '\0';

  return mask;
}

static GList *
zenity_compute_locale_variants (const gchar *locale)
{
  GList *retval = NULL; 
  gchar *language; 
  gchar *territory; 
  gchar *codeset; 
  gchar *modifier; 
  guint mask;
  guint i;


  g_return_val_if_fail (locale != NULL, NULL); 
      
  mask = zenity_explode_locale (locale, &language, &territory, &codeset, &modifier); 
        
  /* Iterate through all possible combinations, from least attractive
   * to most attractive.
   */
  
  for (i = 0; i <= mask; i++) 
    if ((i & ~mask) == 0) { 
      gchar *val = g_strconcat (language, (i & COMPONENT_TERRITORY) ? territory : "", 
                                          (i & COMPONENT_CODESET) ? codeset : "", 
                                          (i & COMPONENT_MODIFIER) ? modifier : "", NULL);
      retval = g_list_prepend (retval, val);
    }

    g_free (language); 
        
    if (mask & COMPONENT_CODESET) 
      g_free (codeset); 
    if (mask & COMPONENT_TERRITORY) 
      g_free (territory); 
    if (mask & COMPONENT_MODIFIER) 
      g_free (modifier);

    return retval;
}

static const gchar *
zenity_guess_category_value (const gchar *categoryname)
{
  const gchar *retval;

  /* The highest priority value is the `LANGUAGE' environment 
   * variable.  This is a GNU extension.
   */

  retval = g_getenv ("LANGUAGE");

  if (retval != NULL && retval[0] != '\0') 
    return retval;

  /* `LANGUAGE' is not set.  So we have to proceed with the POSIX 
   * methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some 
   * systems this can be done by the `setlocale' function itself. 
   */

  /* Setting of LC_ALL overwrites all other.  */

  retval = g_getenv ("LC_ALL");

  if (retval != NULL && retval[0] != '\0') 
    return retval;

  /* Next comes the name of the desired category.  */ 
  retval = g_getenv (categoryname);

  if (retval != NULL && retval[0] != '\0') 
    return retval;

  /* Last possibility is the LANG environment variable.  */ 
  retval = g_getenv ("LANG"); 
  if (retval != NULL && retval[0] != '\0') 
    return retval;

  return NULL;
}


static GHashTable *category_table= NULL;

static const GList *
zenity_i18n_get_language_list (const gchar *category_name)
{
  GList *list; 
        
  if (!category_name) 
    category_name= "LC_ALL"; 
        
  if (category_table) {
    list= g_hash_table_lookup (category_table, (const gpointer) category_name); 
  } else { 
    category_table= g_hash_table_new (g_str_hash, g_str_equal); 
    list= NULL; 
  }

  if (!list) {
    gint c_locale_defined= FALSE; 
    const gchar *category_value; 
    gchar *category_memory, *orig_category_memory; 
                
    category_value = zenity_guess_category_value (category_name); 
               
    if (! category_value) 
      category_value = "C"; 

      orig_category_memory = category_memory = g_malloc (strlen (category_value) + 1);
      
      while (category_value[0] != '\0') {
	while (category_value[0] != '\0' && category_value[0] == ':') 
        ++category_value; 
                        
        if (category_value[0] != '\0') {
	  char *cp= category_memory; 
                                
          while (category_value[0] != '\0' && category_value[0] != ':') 
            *category_memory++= *category_value++; 
                                
          category_memory[0]= '\0'; 
          category_memory++; 
                                
          cp = zenity_unalias_lang (cp); 
                               
          if (strcmp (cp, "C") == 0)
            c_locale_defined= TRUE; 

          list= g_list_concat (list, zenity_compute_locale_variants (cp)); 
        } 
     } 
                
     g_free (orig_category_memory); 
                
     if (!c_locale_defined) 
       list= g_list_append (list, "C"); 
                
     g_hash_table_insert (category_table, (gpointer) category_name, list); 
  } 
       
  return list;
}

/* This is copied from libgnome/gnome-help.c since we try and avoid using
 * the evils of gnome_program_init (), which messes up the commandline
 */

static char *
zenity_locate_help_file (const char *path, const char *doc_name)
{
  int i;
  char *exts[] = { ".xml", ".docbook", ".sgml", ".html", "", NULL };
  const GList *lang_list = zenity_i18n_get_language_list ("LC_MESSAGES");

  for (;lang_list != NULL; lang_list = lang_list->next) {
    const char *lang = lang_list->data;

    /* This has to be a valid language AND a language with
     * no encoding postfix.  The language will come up without
     * encoding next
     */

    if (lang == NULL || (gchar *) strchr (lang, '.') != NULL)
      continue;

    for (i = 0; exts[i] != NULL; i++) {
       char *name;
       char *full;
      
       name = g_strconcat (doc_name, exts[i], NULL);
       full = g_build_filename (path, lang, name, NULL);

       if (g_file_test (full, G_FILE_TEST_EXISTS))
         return full;

       g_free (full);

    }
  }

  return NULL;
}

/* This is copied from libgnome/gnome-url.c since we try and avoid using
 * the evils of gnome_program_init (), which messes up the commandline
 */

gboolean
zenity_util_show_help (const gchar *path, const gchar *document, GError **error)
{
  GConfClient *client;
  gint i;
  gchar *pos, *template;
  int argc;
  char **argv;
  gboolean ret;
  char *url;
	
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (document != NULL, FALSE);
        
  url = g_strconcat ("ghelp:///", zenity_locate_help_file (path, document), NULL);
  pos = (gchar *) strchr (url, ':');

  client = gconf_client_get_default ();

  if (pos != NULL) {
    gchar *protocol, *path;

    g_return_val_if_fail (pos >= url, FALSE);

    protocol = g_new (gchar, pos - url + 1);
    strncpy (protocol, url, pos - url);
    protocol[pos - url] = '\0';
    g_ascii_strdown (protocol, -1);
    
    path = g_strconcat (URL_HANDLER_DIR, protocol, "/command", NULL);
    template = gconf_client_get_string (client, path, NULL);

    if (template == NULL) {
      gchar* template_temp;
			
      template_temp = gconf_client_get_string (client, DEFAULT_HANDLER_PATH, NULL);
						
      /* Retry to get the right url handler */
      template = gconf_client_get_string (client, path, NULL);

      if (template == NULL) 
        template = template_temp;
      else
        g_free (template_temp);

    }
		
    g_free (path);
    g_free (protocol);

  } else {
    /* no ':' ? this shouldn't happen. Use default handler */
    template = gconf_client_get_string (client, DEFAULT_HANDLER_PATH, NULL);
  }

  g_object_unref (G_OBJECT (client));

  if (!g_shell_parse_argv (template, &argc, &argv, error)) {
    g_free (template);
    return FALSE;
  }

  g_free (template);

  for (i = 0; i < argc; i++) {
    char *arg;

    if (strcmp (argv[i], "%s") != 0)
      continue;

    arg = argv[i];
    argv[i] = g_strdup (url);
    g_free (arg);
  }
	
  /* This can return some errors */
  ret = g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, error); 
  g_strfreev (argv);

  return ret;
}
