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
  'util.c'
]

zenity_res = gnome.compile_resources(
  'zenity-resources',
  'zenity.gresource.xml',
  c_name: 'zenity'
)

zenity_c_args = [
  '-DG_LOG_DOMAIN="Zenity"'
]

zenity = executable(
  meson.project_name(),
  zenity_sources + zenity_res,
  include_directories: zenity_root_dir,
  c_args: zenity_c_args,
  dependencies: gtk_dep,
  install: true
)

if perl.found()
  configure_file(
    input: 'gdialog.in',
    output: 'gdialog',
    configuration: zenity_conf,
    install_dir: zenity_bindir,
    install: true
  )
endif
