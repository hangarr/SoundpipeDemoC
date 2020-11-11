SoundpipeDemoC
==============
This is a simple demonstration of how to use the static shared object [Soundpipe](https://github.com/AudioKit/AudioKit) library in C.

This demo is based on example code found in the documentation of old versions of Soundpipe found in Github before Soundpipe was absorbed into AudioKit, e.g. ["How to Create a Soundpipe Module"](https://github.com/narner/Soundpipe/blob/dev/util/module_howto.md).

Building
--------
The `Makefile` assumes that the following are found in a folder `soundpipe` at the same level of the file system as the demo project folder `SoundpipeDemoC`:

- `soundpipe.h`
- `dr_wav.h`
- `md5.h`
- `libsoundpipe.so`
- `libsoundpipe.a`

Obviously one can modify `Makefile` to locate these files elsewhere.

The demo is built as:
```
> cd ./SoundpipeDemoC
> make all
```

The result should be two executables:

- `build/singlepipe`
- `build/mulitipipe`

Running
-------
These are both run the same way:
```
> cd ./build
> singlepipe [-d] -o ./path/to/dest.wav ./path/to/src.wav
> multipipe [-d] -o ./path/to/dest.wav ./path/to/src.wav
```
The `straightpipe` executable should just copy `src.wav` to `dest.wav`.  The `multipipe` executable should create multiple copies `dest_0.wav ... `dest_3.wav` of `src.wav`.

The `-d` option provides a minimal amount of debug information.  At this point this is just the specified `src.wav`, the specified `dest.wav`, and the copies actually created.

