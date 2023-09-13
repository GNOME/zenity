# .desktop file

desktop_file = i18n.merge_file(
  input: '@0@.desktop.in'.format(app_id),
  output: '@0@.desktop'.format(app_id),
  po_dir: zenity_po_dir,
  type: 'desktop',
  install_dir: join_paths(zenity_datadir, 'applications'),
  install: true
)

desktop_file_validate_prg = find_program(
  'desktop-file-validate',
  required: false
)

if desktop_file_validate_prg.found()
  test(
    'Validate desktop file',
    desktop_file_validate_prg,
    args: [desktop_file]
  )
endif

# manpage -- generated from help2man

help2man = find_program('help2man', required: get_option('manpage'))
# help2man has to run zenity to generate the man page, which it can't do if
# we're cross building. We also can't tell help2man what exe wrapper to use.
if help2man.found() and not meson.is_cross_build() and get_option('manpage')
  help2man_opts = [
    '--no-info',
    '--section=1',
    '--help-option=--help-all',
    '--include=@INPUT@',
  ]

  custom_target('zenity.1',
                output: 'zenity.1',
                input: 'zenity.roff',
                command: [
                  help2man, help2man_opts, '--output=@OUTPUT@', zenity
                ],
                install: true,
                install_dir: join_paths(zenity_mandir, 'man1'))
endif
