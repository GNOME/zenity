# Zenity

This is Zenity: the GNOME port of the venerable 'dialog' program,
which allows you to display dialog boxes from the command-line
and shell scripts.

This software is licensed under the GNU Lesser General Public
License, version 2.1, or, at your option, a later version.

Please see COPYING for a full copy of the license.

Zenity is part of the GNOME Extra Apps family and is not a core
GNOME application.

## Dependencies

* gtk >= 4.6
* libadwaita >= 1.2

## Optional Dependencies

* webkit2gtk-6.0 (for HTML support)

Please see the meson.build file for minimal versions required
for optional dependencies, and meson_options.txt for the build
options to enable these features if desired.
