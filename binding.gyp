{
  'targets': [
    {
      'target_name': 'erlpack',
      'dependencies': [
        'vendor/zlib.gyp:zlib',
      ],
      'include_dirs': [
        '<!(node -e \"require(\'nan\')\")',
      ],
      'sources': [
        'js/encoder.h',
        'js/erlpack.cc',
        'js/decoder.h',
      ],
    },
  ],
}
