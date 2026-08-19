IMGENGINE Linux Runtime Package

Contents:
- bin/imgengine_cli: command-line renderer
- lib/libimgengine.so: native shared library
- lib/imgengine/plugins/: runtime plugins
- include/imgengine/api/v1/: public C API headers
- manifest.json: hashes and provenance for every packaged file

Runtime dependencies: libnuma, libturbojpeg, and liburing.
For a non-system installation, add the package lib directory to LD_LIBRARY_PATH.
