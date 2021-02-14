project('zenity', 'c',
  version: '4.alpha.1',
  meson_version: '>=0.50.0',
  license: 'LGPL-2.0-or-later'
)

version_arr = meson.project_version().split('.')
zenity_version_major = version_arr[0].to_int()
zenity_version_minor = version_arr[1]
zenity_version_micro = version_arr[2].to_int()

zenity_prefix = get_option('prefix')
zenity_libdir = join_paths(zenity_prefix, get_option('libdir'))
zenity_includedir = join_paths(zenity_prefix, get_option('includedir'))
zenity_datadir = join_paths(zenity_prefix, get_option('datadir'))
zenity_localedir = join_paths(zenity_prefix, get_option('localedir'))
zenity_appdatadir = join_paths(zenity_datadir, 'metainfo')
zenity_iconsdir = join_paths(zenity_datadir, 'icons')

zenity_root_dir = include_directories('.')
zenity_po_dir = join_paths(meson.source_root(), 'po')

gnome = import('gnome')
i18n = import('i18n')

cc = meson.get_compiler('c')

zenity_conf = configuration_data()
zenity_conf.set_quoted('PACKAGE_NAME', meson.project_name())
zenity_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
zenity_conf.set_quoted('PACKAGE_STRING',
  '@0@ @1@'.format(meson.project_name(), meson.project_version()))
zenity_conf.set_quoted('PACKAGE_DATADIR', zenity_datadir)
zenity_conf.set_quoted('PACKAGE_LIBDIR', zenity_libdir)
zenity_conf.set_quoted('PACKAGE_LOCALE_DIR', zenity_localedir)

zenity_conf.set('VERSION', 'PACKAGE_VERSION')
zenity_conf.set('GETTEXT_PACKAGE', 'PACKAGE_NAME')
zenity_conf.set('LOCALEDIR', 'PACKAGE_LOCALE_DIR')

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

gtk_dep = dependency('gtk4', version: '>= 4.0.0')

if get_option('libnotify')
  libnotify = dependency('libnotify', version: '>= 0.6.1')
  zenity_conf.set('HAVE_LIBNOTIFY', true)
endif

# TODO
#if get_option('webkitgtk')
#  webkitgtk = dependency('webkit2gtk-4.0', version: '>= 2.8.1')
#  zenity_conf.set('HAVE_WEBKITGTK', true)
#endif

configure_file(
  output: 'config.h',
  configuration: zenity_conf
)

update_icon_cache_prg = find_program('gtk-update-icon-cache',
  required : false)

perl = find_program('perl', required: true)

subdir('src')
#subdir('data')
#subdir('po')
#subdir('help')
