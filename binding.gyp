{
  "targets": [
    {
      "target_name": "erlpack",
      "sources": [ "js/erlpack.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        '<(nodedir)/deps/zlib',
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11']
            },
        }],
      ],
    }
  ]
}