{ 'includes': [
    'common.gypi',
    'config.gypi',
  ],
  'targets': [
    # your M2M/IoT application
    {
      'target_name': '<(package_name)',
      'sources': [
        'src/nh_controller_mapper.c',
        'src/nh_controller.c',
        'src/nh_request_builder.c',
        'src/nh_response_parser.c',
        'src/<(package_name).c',
       ],
      'product_prefix': '',
      'type': 'shared_library',
      'cflags': [ '-fPIC' ],
      'include_dirs' : [
      ],
      'libraries': [
        '-lmoatapp',
      ],
      'dependencies': [
      ],
    },

  ],
}
