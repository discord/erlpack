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
      'cflags_cc': [
        '-std=c++11',
      ],
      'sources': [
        'js/erlpack.cc',
        'js/encoder.h',
        'js/decoder.h',
      ],
    },
  ],
}
