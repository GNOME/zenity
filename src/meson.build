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

zenity_deps = [
  gtk_dep,
  libnotify,
  webkitgtk,
  x11
]

zenity_c_args = [
  '-DG_LOG_DOMAIN="Zenity"'
]

zenity = executable(
  meson.project_name(),
  zenity_sources,
  include_directories: zenity_root_dir,
  dependencies: zenity_deps,
  c_args: zenity_c_args,
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

install_data('zenity.ui')
