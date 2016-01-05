{
  "targets": [
    {
      "target_name": "erlpackjs",
      "sources": [ "encoder.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}