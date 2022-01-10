project('zenity', 'c',
  version: '4.alpha.1',
  meson_version: '>=0.53.0',
  license: 'LGPL-2.1-or-later'
)

app_id = 'org.gnome.Zenity'
resource_base_path = '/org/gnome/Zenity'

version_arr = meson.project_version().split('.')
zenity_version_major = version_arr[0].to_int()
zenity_version_minor = version_arr[1]
zenity_version_micro = version_arr[2].to_int()

zenity_prefix = get_option('prefix')
zenity_bindir = join_paths(zenity_prefix, get_option('bindir'))
zenity_libdir = join_paths(zenity_prefix, get_option('libdir'))
zenity_datadir = join_paths(zenity_prefix, get_option('datadir'))
zenity_localedir = join_paths(zenity_prefix, get_option('localedir'))
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

zenity_conf.set_quoted('APP_ID', app_id)
zenity_conf.set_quoted('RESOURCE_BASE_PATH', resource_base_path)

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

# Optional dependencies

opt_missing_str = '''
Requested optional @0@ support but library not found.
Please ensure you have any required development libraries installed.'''

# TODO
#webkitgtk = dependency('webkit2gtk-4.0', version: '>= 2.8.1', required: false)
#if get_option('webkitgtk')
#  if webkitgtk.found()
#    zenity_conf.set('HAVE_WEBKITGTK', true)
#  else
#    error(opt_missing_str.format('webkitgtk'))
#  endif
#endif

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
	 'iconsdir': zenity_iconsdir,
        }, section: 'Directories')

summary({'gdialog script': perl.found(),
        }, section: 'Configuration')

subdir('src')
subdir('data')
subdir('icons')
subdir('po')
subdir('help')

meson.add_install_script('meson_post_install.py')
