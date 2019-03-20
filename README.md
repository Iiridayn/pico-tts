Based on https://github.com/gmorenz/pico-tts and https://github.com/DougGore/picopi, code cribbed from both, and I've not worked with a Makefile in about 15 years. Gradually hope to rewrite out copyrighted code, or ensure is properly licensed. Needs help w/the build process.

This repository uses submodules - to checkout completely, `git submodule init`, `git submodule update`. So, should upstream delete their repo we're out of luck - but this is otherwise a fairly simple and clean method of tracking dependencies.

To use the binary (other than the LD_LIBRARY_PATH stuff), simply pass text in over stdin and receive uncompressed PCM over stdout. Pipe to `aplay -q -f S16_LE -r 16` to listen to it, or redirect to a file, whichever.

## TODO

* Output file flag w/`-` for stdout, so I don't throw garbage at the terminal without a warning
* Fix build system to know how to find library in binary
* Same for the language files - need to be able to find those from the binary as well
