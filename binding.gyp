{
  "targets": [
    {
      "target_name": "erlpackjs",
      "sources": [ "js/erlpack.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "electron/vendor/brightray/vendor/download/libchromiumcontent/src"
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