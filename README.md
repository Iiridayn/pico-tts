Based on https://github.com/gmorenz/pico-tts, [upstream changes are CC0](https://github.com/gmorenz/pico-tts/issues/1). The Makefile is based on https://github.com/DougGore/picopi with an implicit license, and is probably original enough now anyway.

This repository uses submodules - to checkout completely, `git submodule init`, `git submodule update`. So, should upstream delete their repo we're out of luck - but this is otherwise a fairly simple and clean method of tracking dependencies.

To use the binary, simply pass text in over stdin and receive uncompressed PCM over stdout. Pipe to `aplay -q -f S16_LE -r 16` to listen to it, or redirect to a file, whichever.

## TODO

* Output file flag w/`-` for stdout, so I don't throw garbage at the terminal without a warning
    * Might also guess if stdout is a TTY and whine - heuristics aren't perfect, but possibly adequate
* Possibly: include header files in `/usr/include`, _maybe_ rename library to `libttspico.so` (for drop-in upgrades), and mark a conflict w/`svox-pico-bin` for an upgrade path
