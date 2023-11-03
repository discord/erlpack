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
        'js/erlpack.cc',
        'js/encoder.h',
        'js/decoder.h',
      ],
    },
  ],
}
