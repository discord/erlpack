{
  'targets': [
    {
      'target_name': 'zlib',
      'type': 'static_library',
      'include_dirs': [
        'zlib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'zlib',
        ],
      },
      'sources': [
        'zlib/adler32.c',
        'zlib/compress.c',
        'zlib/crc32.c',
        'zlib/crc32.h',
        'zlib/deflate.c',
        'zlib/deflate.h',
        'zlib/gzclose.c',
        'zlib/gzguts.h',
        'zlib/gzlib.c',
        'zlib/gzread.c',
        'zlib/gzwrite.c',
        'zlib/infback.c',
        'zlib/inffast.c',
        'zlib/inffast.h',
        'zlib/inffixed.h',
        'zlib/inflate.c',
        'zlib/inflate.h',
        'zlib/inftrees.c',
        'zlib/inftrees.h',
        'zlib/trees.c',
        'zlib/trees.h',
        'zlib/uncompr.c',
        'zlib/zconf.h',
        'zlib/zlib.h',
        'zlib/zutil.c',
        'zlib/zutil.h',
      ],
    },
  ],
}
