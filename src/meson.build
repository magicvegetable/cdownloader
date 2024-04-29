cdownloader_sources = [
  'main.c',
  'cdownloader-application.c',
  'cdownloader-window.c',
]

cdownloader_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

cdownloader_sources += gnome.compile_resources('cdownloader-resources',
  'cdownloader.gresource.xml',
  c_name: 'cdownloader'
)

executable('cdownloader', cdownloader_sources,
  dependencies: cdownloader_deps,
       install: true,
)
