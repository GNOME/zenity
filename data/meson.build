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

# manpage

install_man('zenity.1')
