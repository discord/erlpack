{
  "targets": [
    {
      "target_name": "erlpackjs",
      "sources": [ "js/erlpack.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}