project('zenity', 'c',
  version: '4.4.beta',
  meson_version: '>=1.1',
  license: 'LGPL-2.1-or-later',
  default_options: [ 'warning_level=2', 'werror=false', 'c_std=gnu11', ],
)

app_id = 'org.gnome.Zenity'
resource_base_path = '/org/gnome/Zenity'

zenity_prefix = get_option('prefix')
zenity_bindir = join_paths(zenity_prefix, get_option('bindir'))
zenity_libdir = join_paths(zenity_prefix, get_option('libdir'))
zenity_datadir = join_paths(zenity_prefix, get_option('datadir'))
zenity_localedir = join_paths(zenity_prefix, get_option('localedir'))
zenity_mandir = join_paths(zenity_prefix, get_option('mandir'))
zenity_iconsdir = join_paths(zenity_datadir, 'icons')

zenity_root_dir = include_directories('.')
zenity_po_dir = join_paths(meson.project_source_root(), 'po')

gnome = import('gnome')
i18n = import('i18n')

warning_cflags = [
  '-Werror=address',
  '-Werror=array-bounds',
  '-Wcast-align',
  '-Werror=empty-body',
  '-Wformat=2',
  '-Wformat-security',
  '-Wignored-qualifiers',
  '-Werror=implicit-fallthrough=3',
  '-Werror=implicit-int',
  '-Werror=implicit-function-declaration',
  '-Werror=incompatible-pointer-types',
  '-Werror=init-self',
  '-Werror=int-conversion',
  '-Werror=int-to-pointer-cast',
  '-Wlogical-op',
  '-Werror=main',
  '-Werror=misleading-indentation',
  '-Werror=missing-braces',
  '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Werror=nonnull',
  '-Wold-style-definition',
  '-Werror=overflow',
  '-Wparentheses',
  '-Werror=pointer-arith',
  '-Werror=pointer-to-int-cast',
  '-Werror=redundant-decls',
  '-Werror=return-type',
  '-Werror=sequence-point',
  '-Wshadow',
  '-Werror=strict-prototypes',
  '-Werror=trigraphs',
  '-Werror=undef',
  '-Werror=write-strings',
  '-Wno-cast-function-type',
  '-Wno-dangling-pointer',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-unused-parameter',
  '-Wno-unused-variable',
]

cc = meson.get_compiler('c')
supported_warning_cflags = cc.get_supported_arguments(warning_cflags)
add_global_arguments(supported_warning_cflags, language : 'c')

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

gtk_dep = dependency('gtk4', version: '>= 4.10')
adw_dep = dependency('libadwaita-1', version: '>= 1.2')

# Optional dependencies

opt_missing_str = '''
Requested optional @0@ support but library not found.
Please ensure you have any required development libraries installed.'''

webkitgtk_dep = dependency('webkitgtk-6.0', required: false)
if get_option('webkitgtk')
  if webkitgtk_dep.found()
    zenity_conf.set('HAVE_WEBKITGTK', true)
  else
    error(opt_missing_str.format('webkitgtk'))
  endif
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
	 'mandir': zenity_mandir,
	 'iconsdir': zenity_iconsdir,
        }, section: 'Directories')

summary({'webkitgtk': get_option('webkitgtk'),
         'manpage': get_option('manpage'),
        }, section: 'Configuration')

subdir('src')
subdir('data')
subdir('icons')
subdir('po')
subdir('help')

gnome.post_install(
  gtk_update_icon_cache: true,
)
