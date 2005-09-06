/*
 * about.c
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 * Copyright (C) 2001 CodeFactory AB
 * Copyright (C) 2001, 2002 Anders Carlsson
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Authors: Glynn Foster <glynn.foster@sun.com>
 *          Anders Carlsson <andersca@gnu.org>
 */

#include "config.h"
#include "zenity.h"
#include "util.h"
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include <glade/glade.h>

#define GTK_RESPONSE_CREDITS 0
#define ZENITY_HELP_PATH ZENITY_DATADIR "/help/"
#define ZENITY_CLOTHES_PATH ZENITY_DATADIR "/clothes/"

#define ZENITY_CANVAS_X 400.0
#define ZENITY_CANVAS_Y 280.0

static GtkWidget *dialog;
static GtkWidget *cred_dialog;

static void zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data);

/* Sync with the people in the THANKS file */
static const gchar *const author_credits[] = {
  "Authors",
  "=======",
  "Glynn Foster <glynn foster sun com>",
  "Mike Newman <mikegtn gnome org>",
  "",
  "Patches from the following people",
  "=================================",
  "Darren Adams  <darrenadams dsl pipex com>"
  "Peter Astrand  <astrand lysator liu se>",
  "Jonathan Blandford  <jrb redhat com>",
  "Paul Bolle  <pebolle tiscali nl>",
  "Paolo Borelli  <pborelli katamail com>",
  "Leonardo Boshell  <p kapcoweb com>",
  "Ross Burton  <ross burtonini com>",
  "Damien Carbery  <damien carbery sun com>",
  "Anders Carlsson  <andersca codefactory se>",
  "Ed Catmur  <ed catmur co uk>",
  "Nicholas Curran  <quasar austar net au>",
  "John Fleck  <jfleck inkstain net>",
  "Sebastian Heinlein <glatzor pimpzkru de>",
  "James Henstridge  <james daa com au>",
  "Chris Lahey  <clahey ximian com>",
  "Mihai T Lazarescu  <mihai email it>",
  "Sebastian Kapfer <sebastian_kapfer web de>",
  "Tomasz Koczko  <kloczek pld org pl>",
  "Jordi Mallach  <jordi sindominio net>",
  "Kjartan Maraas  <kmaraas gnome org>",
  "Breda McColgan  <breda mccolgan sun com>",
  "Baptiste Mille-Mathias  <bmm80 free fr>",
  "Buhan Milne  <bgmilne linux-mandrake com>",
  "Christian Monneckes  <c-w-m gmx de>",
  "Ivan Noris  <vix vazka sk>",
  "Havoc Pennington  <hp redhat com>",
  "Jan Arne Petersen  <jpetersen uni-bonn de>",
  "Kevin C  Krinke  <kckrinke opendoorsoftware com>",
  "Kristian Rietveld  <kris gtk org>",
  "Lucas Rocha  <lucasr im ufba br>",
  "Christian Rose  <menthos menthos com>",
  "Jakub Steiner  <jimmac ximian com>",
  "Luke Suchocki  <gnome lukiepoo com>",
  "Daniel d'Surreal  <dagmar speakeasy net>",
  "Hidetoshi Tajima  <hidetoshi tajima sun com>",
  "Tom Tromey  <tromey redhat com>",
  "Yann  <bloch iie cnam fr>",
  "Norman Rasmussen <normanr gmail com>",
  "Benoît Dejean <tazforever dlfp org>",
  "",
  "And all the translators that rock my world",
  "==========================================",
  "Vincent van Adrighem  <v vanadrighem dirck mine nu>",
  "Taneem Ahmed  <taneem bengalinux org>",
  "Takeshi Aihana <aihana gnome gr jp>",
  "Amanpreet Singh Alam  <amanlinux netscape net>",
  "Metin Amiroff  <metin karegen com>",
  "Sanlig Badral  <badral openmn org>",
  "John C Barstow  <jbowtie amathaine com>",
  "Aygimantas Beruka  <uid0 tuxfamily org>",
  "Alberto Fernandez Benito <afernn euskalnet net>",
  "Stefano Canepa  <sc linux it>",
  "Young-Ho Cha <ganadist mizi com>",
  "Abel Cheung  <maddog linux org hk>",
  "Zbigniew Chyla  <cyba gnome pl>",
  "Mohammad Damt  <mdamt bisnisweb com>",
  "Fatih Demir  <kabalak gtranslator org>",
  "Laurent Dhima  <laurenti alblinux net>",
  "Paul Duffy <dubhthach frink nuigalway ie>",
  "Laszlo Dvornik  <dvornik gnome hu>",
  "Maxim Dziumanenko  <mvd mylinux com ua>",
  "Francisco Javier Fernandez  <serrador arrakis es>",
  "Artur Flinta  <aflinta cvs gnome org>",
  "Alessio Frusciante  <algol firenze linux it>",
  "Evandro Fernandes Giovanini  <evandrofg ig com br>",
  "Pablo Gonzalo del Campo  <pablodc bigfoot com>",
  "Dhurba Gnawali  <dhurbagnawali hotmail com>",
  "Sammi Gunnarsson  <sammi techattack nu>",
  "Martin Willemoes Hansen  <mwh sysrq dk>",
  "Dafydd Harries  <daf parnassus ath cx>",
  "Raphael Higino  <raphaelh cvs gnome org>",
  "Wang Jian  <lark linux net cn>",
  "Guntupalli Karunakar  <karunakar freedomink org>",
  "Tomas Kuliavas  <tokul users sourceforge net>",
  "Priit Laes  <amd tt ee>",
  "Iaki Larraaga  <dooteo euskalgnu org>",
  "Ole Laursen  <olau hardworking dk>",
  "Toivo Leedjrv  <toivo linux ee>",
  "David Lodge  <dave cirt net>",
  "Duarte Loreto  <happyguy_pt hotmail com>",
  "Johanna Makkonen  <johanna makkonen iki fi>",
  "Jordi Mallach  <jordi sindominio net>",
  "Kjartan Maraas  <kmaraas gnome org>",
  "Jordi Mas  <jmas softcatala org>",
  "Kamagasako Masatoshi  <emerald gnome gr jp>",
  "Dmitry G  Mastrukov  <dmitry taurussoft org>",
  "Arafat Medini  <lumina silverpen de>",
  "Christophe Merlet  <redfox redfoxcenter org>",
  "Mike Newman  <mikegtn gnome org>",
  "Ahmad Riza H Nst  <rizahnst eriagempita co id>",
  "Alexandre Folle de Menezes  <afmenez terra com br>",
  "Christian Neumair  <chris gnome-de org>",
  "Metin Omirov  <metin karegen com>",
  "Gareth Owen  <gowen72 yahoo com>",
  "Kostas Papadimas  <pkst gmx net>",
  "Ankit Patel  <ankit redhat com>",
  "Sami Pesonen  <sampeson iki fi>",
  "Roozbeh Pournader  <roozbeh sharif edu>",
  "Jarkko Ranta  <jjranta cc joensuu fi>",
  "Rostislav Raykov  <zbrox dir bg>",
  "Hendrik Richter  <hendrik gnome de org>",
  "Christian Rose  <menthos menthos com>",
  "Changwoo Ryu  <cwryu debian org>",
  "Pablo Saratxaga  <pablo mandrakesoft com>",
  "Robert Sedak  <robert sedak sk htnet hr>",
  "Paisa Seeluangsawat  <paisa users sf net>",
  "Danilo Segan  <dsegan gmx net>",
  "Alexander Shopov  <ash contact bg>",
  "Aasmund Skjaveland  <aasmunds fys uio no>",
  "Yuriy Syrota  <rasta renome rovno ua>",
  "Marcel Telka  <marcel telka sk>",
  "Andras Timar  <timar gnome hu>",
  "Miloslav Trmac  <mitr volny cz>",
  "Mugurel Tudor <mugurelu go ro>",
  "Daniel Yacob  <locales geez org>",
  "Funda Wang <fundawang linux net cn>",
  "Alexander Winston  <alexander winston comcast net>",
  "Theppitak Karoonboonyanan <thep linux thai net>",
  "Martin Willemoes Hansen <mwh sysrq dk>",
  "Ignacio Casal Quinteiro <nacho.resa gmail com>",
  "Pawan Chitrakar <pawan nplinux org>",
  "Rajesh Ranjan <rajeshkajha yahoo org>",
  "Vladimir Petkov <vpetkov i-space org>",
  "Abduxukur Abdurixit <abduxukur.abdurixit t-systems ch>",
  "Adi Attar <aattar cvs gnome org>",
  "Steve Murphy <murf e-tools com>",
  "Josep Puigdemont <josep imatge-sintetica com>",
  "Adam Weinberger <adamw gnome org>",
  "Ivar Smolin <okul linux ee>",
  "Jens Seidel <jensseidel users sf net>",
  "Benoît Dejean <tazforever dlfp org>",
  "Yair Hershkovitz <yairhr gmail com>",
  "Clytie Siddall <clytie riverland net au>",
  "Ilkka Tuohela <hile iki fi>",
  "Chao-Hsiung Liao <j_h_liau yahoo com tw>",
  "Žygimantas Beručka <zygis gnome org>",
  "Gabor Kelemen <kelemeng gnome hu>",
  "Telsa Gwynne <hobbit aloss ukuu org uk>",
  "Slobodan D. Sredojevic <ssl uns ns ac yu>",
  "Baris Cicek <baris teamforce name tr>",
  NULL
};

static gchar *translator_credits;

static gint
zenity_move_clothes_event (GnomeCanvasItem *item, 
                           GdkEvent *event,
                           gpointer data)
{
  static double x, y;
  double new_x, new_y;
  static int dragging;
  double item_x, item_y;

  /* set item_[xy] to the event x,y position in the parent's 
   * item-relative coordinates
   */
  
  item_x = event->button.x;
  item_y = event->button.y;
  gnome_canvas_item_w2i (item->parent, &item_x, &item_y);

  switch (event->type) {
    case GDK_BUTTON_PRESS:
      x = item_x;
      y = item_y;
      gnome_canvas_item_ungrab (item, event->button.time);
      gnome_canvas_item_raise_to_top (item);
      dragging = TRUE;
      break;

    case GDK_MOTION_NOTIFY:
      if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
        new_x = item_x;
        new_y = item_y;

        gnome_canvas_item_move (item, new_x - x, new_y - y);
        x = new_x;
        y = new_y;
      }
      break;

    case GDK_BUTTON_RELEASE:
      gnome_canvas_item_ungrab (item, event->button.time);
      dragging = FALSE;
      break;

    default:
      break;
  }

  return FALSE;
}

typedef struct 
{
  const gchar *filename;
  gdouble x, y;
} MonkClothes;

static const MonkClothes monk_clothes[] = {
  {"gnome-tshirt.png", 30.0, 20.0},
  {"sunglasses.png", ZENITY_CANVAS_X - 100.0 , ZENITY_CANVAS_Y - 150.0 },
  {"surfboard.png", 30.0, ZENITY_CANVAS_Y - 200.0},
  {"hawaii-shirt.png", ZENITY_CANVAS_X - 50.0, 20.0}
};

static void
zenity_create_clothes (GtkWidget *canvas_board)
{
  GdkPixbuf *pixbuf;
  GnomeCanvasItem *canvas_item;
  gchar *pixbuf_path;
  size_t i;

  for (i = 0; i < G_N_ELEMENTS (monk_clothes); i++) {
    pixbuf_path = g_strconcat (ZENITY_CLOTHES_PATH, monk_clothes[i].filename, NULL); 
    pixbuf = gdk_pixbuf_new_from_file (pixbuf_path, NULL);

    canvas_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (GNOME_CANVAS (canvas_board)->root),
                                         GNOME_TYPE_CANVAS_PIXBUF,
                                         "x", monk_clothes[i].x,
                                         "y", monk_clothes[i].y,
                                         "pixbuf", pixbuf,
                                         "anchor", GTK_ANCHOR_NW,
                                          NULL);
    g_signal_connect (G_OBJECT (canvas_item), "event",
                      G_CALLBACK (zenity_move_clothes_event), NULL);
  }
}

static GtkWidget *
zenity_create_monk (void)
{
  GtkWidget *canvas_board;
  GnomeCanvasItem *canvas_item;
  GdkPixbuf *pixbuf;
  GdkColor color = { 0, 0xffff, 0xffff, 0xffff };

  canvas_board = gnome_canvas_new ();

  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas_board), 0, 0,
                                  ZENITY_CANVAS_X, ZENITY_CANVAS_Y);

  gtk_widget_set_size_request (canvas_board, ZENITY_CANVAS_X, ZENITY_CANVAS_Y);

  gdk_colormap_alloc_color (gtk_widget_get_colormap (GTK_WIDGET (canvas_board)),
                            &color, FALSE, TRUE);

  gtk_widget_modify_bg (GTK_WIDGET (canvas_board), GTK_STATE_NORMAL, &color);

  pixbuf = gdk_pixbuf_new_from_file (ZENITY_CLOTHES_PATH "monk.png", NULL);

  canvas_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (GNOME_CANVAS (canvas_board)->root),
                                       GNOME_TYPE_CANVAS_PIXBUF,
                                       "x", (ZENITY_CANVAS_X / 2.0)/2.0 + 10.0,
                                       "y", (ZENITY_CANVAS_Y / 2.0)/2.0 - 50.0,
                                       "pixbuf", pixbuf,
                                       "anchor", GTK_ANCHOR_NW,
                                       NULL);

  zenity_create_clothes (canvas_board);

  return canvas_board;
}

static GtkWidget *
zenity_create_boutique (void) 
{
  GtkWidget *window;
  GtkWidget *canvas;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  zenity_util_set_window_icon (window, NULL, ZENITY_IMAGE_FULLPATH ("zenity.png"));
  canvas = zenity_create_monk ();
  gtk_container_add (GTK_CONTAINER (window), canvas);

  return window;
}

static gboolean
zenity_zen_wisdom (GtkDialog *dialog, GdkEventKey *event, gpointer user_data)
{
  static gint string_count;

  if (string_count >= 3)
    return FALSE;

  switch (event->keyval) {
    case GDK_N:
    case GDK_n:
      if (string_count == 2) {
        GtkWidget *window;
        window = zenity_create_boutique ();
        gtk_widget_show_all (window);
        string_count++;
      } else {
        string_count = 0;
      }
      break;
    case GDK_Z:
    case GDK_z:
      if (string_count == 0)
        string_count++;
      else
        string_count = 0;
      break;
    case GDK_E:
    case GDK_e:
      if (string_count == 1)
        string_count++;
      else
        string_count = 0;
      break;
    default:
      string_count = 0;
  }

  return FALSE;
}

void 
zenity_about (ZenityData *data)
{
  GladeXML *glade_dialog = NULL;
  GdkPixbuf *pixbuf;
  GtkWidget *label;
  GtkWidget *image;
  gchar *text;

  glade_dialog = zenity_util_load_glade_file ("zenity_about_dialog");

  if (glade_dialog == NULL) {
    data->exit_code = zenity_util_return_exit_code (ZENITY_ERROR);
    return;
  }

  /* Translators: This is a special message that shouldn't be translated
                  literally. It is used in the about box to give credits to
                  the translators.
                  Thus, you should translate it to your name and email address.
                  You can also include other translators who have contributed to
                  this translation; in that case, please write them on separate
                  lines seperated by newlines (\n). */

  translator_credits = _("translator-credits");

  glade_xml_signal_autoconnect (glade_dialog);

  dialog = glade_xml_get_widget (glade_dialog, "zenity_about_dialog");

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (zenity_about_dialog_response), data);
  g_signal_connect (G_OBJECT (dialog), "key_press_event",
                    G_CALLBACK (zenity_zen_wisdom), glade_dialog);

  zenity_util_set_window_icon (dialog, NULL, ZENITY_IMAGE_FULLPATH ("zenity.png"));

  image = glade_xml_get_widget (glade_dialog, "zenity_about_image");

  pixbuf = gdk_pixbuf_new_from_file (ZENITY_IMAGE_FULLPATH ("zenity.png"), NULL);
    
  if (pixbuf != NULL) {
    gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
    g_object_unref (pixbuf);
  }

  label = glade_xml_get_widget (glade_dialog, "zenity_about_version");
  gtk_label_set_selectable (GTK_LABEL (label), FALSE);
  text = g_strdup_printf ("<span size=\"xx-large\" weight=\"bold\">Zenity %s</span>", VERSION);
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);

  label = glade_xml_get_widget (glade_dialog, "zenity_about_description");
  gtk_label_set_selectable (GTK_LABEL (label), FALSE);
  gtk_label_set_text (GTK_LABEL (label), _("Display dialog boxes from shell scripts"));

  label = glade_xml_get_widget (glade_dialog, "zenity_about_copyright");
  gtk_label_set_selectable (GTK_LABEL (label), FALSE);
  text = g_strdup_printf ("<span size=\"small\">%s</span>", _("(C) 2003 Sun Microsystems"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);

  if (glade_dialog)
    g_object_unref (glade_dialog);

  zenity_util_show_dialog (dialog);
  gtk_main ();
}

static GtkWidget *
zenity_about_create_label (void)
{
  GtkWidget *label;

  label = gtk_label_new ("");
  gtk_label_set_selectable (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_misc_set_padding (GTK_MISC (label), 8, 8);

  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  return label;
}

static void
zenity_about_update_author_label (GtkWidget *label)
{
  GString *string;
  gchar *tmp;
  gint i = 0;

  gtk_widget_show (label);

  string = g_string_new ("");

  for (i = 0; author_credits[i] != NULL; i++) {
    tmp = g_markup_escape_text (author_credits[i], -1);
    g_string_append (string, tmp);

    if (author_credits[i+1] != NULL)
      g_string_append (string, "\n");

    g_free (tmp);
  }
  gtk_label_set_markup (GTK_LABEL (label), string->str);
  g_string_free (string, TRUE);
}

static void
zenity_about_update_translator_label (GtkWidget *label)
{
  GString *string;
  gchar *tmp;

  if (strcmp (translator_credits, "translator-credits") == 0) {
    gtk_widget_hide (label);
    return;
  } else {
    gtk_widget_show (label);
  }

  string = g_string_new ("");

  tmp = g_markup_escape_text (translator_credits, -1);
  g_string_append (string, tmp);
  g_free (tmp);

  gtk_label_set_markup (GTK_LABEL (label), string->str);
  g_string_free (string, TRUE);
}

static void
zenity_about_display_credits_dialog (void)
{
  GtkWidget *credits_dialog;
  GtkWidget *label, *notebook, *sw;

  if (cred_dialog != NULL) {
    gtk_window_present (GTK_WINDOW (cred_dialog));
    return;
  }

  credits_dialog = gtk_dialog_new_with_buttons (_("Credits"), 
                                                GTK_WINDOW (dialog), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

  g_signal_connect (G_OBJECT (credits_dialog), "response",
                    G_CALLBACK (gtk_widget_destroy), credits_dialog);
  g_signal_connect (G_OBJECT (credits_dialog), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &cred_dialog);

  cred_dialog = credits_dialog;

  gtk_window_set_default_size (GTK_WINDOW (credits_dialog), 360, 260);
  gtk_dialog_set_default_response (GTK_DIALOG (credits_dialog), GTK_RESPONSE_OK);

  notebook = gtk_notebook_new ();
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 8);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (credits_dialog)->vbox), notebook, TRUE, TRUE, 0);
       
  if (author_credits != NULL) {
    label = zenity_about_create_label ();
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
    gtk_label_new (_("Written by")));
    zenity_about_update_author_label (label);
  }

  if (translator_credits != NULL && strcmp (translator_credits, "translator-credits") != 0) {
    label = zenity_about_create_label ();
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), 
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), label);
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN (sw)->child), GTK_SHADOW_NONE);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), sw,
    gtk_label_new (_("Translated by")));
    zenity_about_update_translator_label (label);
  }

  gtk_widget_show_all (credits_dialog);
}

static void
zenity_about_dialog_response (GtkWidget *widget, int response, gpointer data)
{
  ZenityData *zen_data = data;

  switch (response) {
    case GTK_RESPONSE_OK:
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_OK);
      gtk_main_quit ();
      break;

    case GTK_RESPONSE_HELP:
      zenity_util_show_help (NULL);
      break;

    case GTK_RESPONSE_CREDITS:
      zenity_about_display_credits_dialog ();
      break;

    default:
      /* Esc dialog */
      zen_data->exit_code = zenity_util_return_exit_code (ZENITY_ESC);
      break;
  }
}
