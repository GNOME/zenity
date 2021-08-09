project('zenity', 'c',
  version: '3.41.0',
  meson_version: '>=0.53.0',
  license: 'LGPL-2.1-or-later'
)

version_arr = meson.project_version().split('.')
zenity_version_major = version_arr[0].to_int()
zenity_version_minor = version_arr[1]
zenity_version_micro = version_arr[2].to_int()

zenity_prefix = get_option('prefix')
zenity_bindir = join_paths(zenity_prefix, get_option('bindir'))
zenity_libdir = join_paths(zenity_prefix, get_option('libdir'))
zenity_datadir = join_paths(zenity_prefix, get_option('datadir'), 'zenity')
zenity_localedir = join_paths(zenity_prefix, get_option('localedir'))

zenity_root_dir = include_directories('.')
zenity_po_dir = join_paths(meson.source_root(), 'po')

gnome = import('gnome')
i18n = import('i18n')

cc = meson.get_compiler('c')

zenity_conf = configuration_data()
zenity_conf.set_quoted('ZENITY_NAME', meson.project_name())
zenity_conf.set_quoted('ZENITY_VERSION', meson.project_version())
zenity_conf.set_quoted('ZENITY_STRING',
  '@0@ @1@'.format(meson.project_name(), meson.project_version()))
zenity_conf.set_quoted('ZENITY_DATADIR', zenity_datadir)
zenity_conf.set_quoted('ZENITY_LIBDIR', zenity_libdir)
zenity_conf.set_quoted('ZENITY_LOCALE_DIR', zenity_localedir)

zenity_conf.set('VERSION', 'ZENITY_VERSION')
zenity_conf.set('GETTEXT_PACKAGE', 'ZENITY_NAME')
zenity_conf.set('LOCALEDIR', 'ZENITY_LOCALE_DIR')

zenity_conf.set('DEBUG', get_option('debug'))

check_headers = [
  'sys/types.h',
  'unistd.h',
  'langinfo.h',
  'locale.h'
]

foreach h : check_headers
  cc.has_header(h, required: true)
endforeach

gtk_dep = dependency('gtk+-3.0', version: '>= 3.16.0')

# this is the minimum required glib according to the above-mentioned version of gtk
# it's really just being documented here for ease of reference.
glib_dep = dependency('glib-2.0', version: '>= 2.43.4')

# Optional dependencies

opt_missing_str = '''
Requested optional @0@ support but library not found.
Please ensure you have any required development libraries installed.'''

libnotify = dependency('libnotify', version: '>= 0.6.1', required: false)
if get_option('libnotify')
  if libnotify.found()
    zenity_conf.set('HAVE_LIBNOTIFY', true)
  else
    error(opt_missing_str.format('libnotify'))
  endif
endif

webkitgtk = dependency('webkit2gtk-4.0', version: '>= 2.8.1', required: false)
if get_option('webkitgtk')
  if webkitgtk.found()
    zenity_conf.set('HAVE_WEBKITGTK', true)
  else
    error(opt_missing_str.format('webkitgtk'))
  endif
endif

# link Xlib if we have it. This will likely be removed after gtk4 migration.
x11 = dependency('x11', required: false)

perl = find_program('perl', required: false)
if perl.found()
  zenity_conf.set('PERL', perl.path())
endif

configure_file(
  output: 'config.h',
  configuration: zenity_conf
)

# Print a summary of options at the end.

summary({'prefix': zenity_prefix,
         'libdir': zenity_libdir,
         'datadir': zenity_datadir,
	 'localedir': zenity_localedir,
        }, section: 'Directories')

summary({'libnotify': get_option('libnotify'),
         'webkitgtk': get_option('webkitgtk'),
         'gdialog script': perl.found(),
        }, section: 'Configuration')

subdir('src')
subdir('data')
subdir('po')
subdir('help')

meson.add_install_script('meson_post_install.py')
