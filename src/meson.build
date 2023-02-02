# for i in `ls *.c`; do echo "  '${i}',"; done
zenity_sources = [
  'about.c',
  'calendar.c',
  'color.c',
  'entry.c',
  'fileselection.c',
  'forms.c',
  'main.c',
  'msg.c',
  'notification.c',
  'option.c',
  'password.c',
  'progress.c',
  'scale.c',
  'text.c',
  'tree.c',
  'util.c',
  'zenity-tree-column-view.c',
]

zenity_cv_enums = gnome.mkenums_simple('zenity-tree-column-view-enums',
         sources: 'zenity-tree-column-view.h',
)

zenity_res_conf = configuration_data()
zenity_res_conf.set('resource_base_path', resource_base_path)

zenity_res_in = configure_file(
  input: 'zenity.gresource.xml.in',
  output: 'zenity.gresource.xml',
  configuration: zenity_res_conf
)

zenity_res = gnome.compile_resources(
  'zenity-resources',
  zenity_res_in,
  c_name: 'zenity'
)

zenity_c_args = [
  '-DG_LOG_DOMAIN="Zenity"'
]

zenity = executable(
  meson.project_name(),
  zenity_sources + zenity_cv_enums + zenity_res,
  include_directories: zenity_root_dir,
  c_args: zenity_c_args,
  dependencies: [adw_dep, webkitgtk_dep],
  install: true
)
