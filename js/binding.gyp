{
  "targets": [
    {
      "target_name": "erlpackjs",
      "sources": [ "erlpack.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}