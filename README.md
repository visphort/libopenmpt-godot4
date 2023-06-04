libopenmpt-godot
================

This is a Godot 4 GDExtension library that allows you to use
[libopenmpt][libopenmpt] within the Godot Engine.

This library has been tested with Godot 4.0.3 Stable.

## Related Reading

*See [this article][article] for a detailed description of why this was made.*

## Compilation Instructions

You will have to first download the [Godot C++ Bindings][godot-cpp] for your
target Godot version and then place the `godot-cpp` folder in the project root
folder.

You will need to have libopenmpt (>= 0.6.5) installed in your system to compile
this library.

### For Windows

Download the official compiled binaries from [lib.openmpt.org][libopenmpt-dl]
and extract them into `deps/libopenmpt`.


### For Linux

First you need to compile libopenmpt for your system, if you don't have it
installed already and don't have the appropriate version.

First, obtain the source code from [lib.openmpt.org][libopenmpt-dl] and extract
it into `deps/libopenmpt`, open a terminal in the same directory and run the
following commands:

```
CPPFLAGS=-fPIC ./configure --prefix=$(realpath ./install-files) --disable-openmpt123
make
make install
```

This will put the required libraries into `deps/libopenmpt/install-files`.


## Building

In the root directory, open a terminal and run:

```
scons platform=<platform name> target=<debug/release>
```

to build binaries for your system.


## Advanced Configuration

You can edit the `SConstruct` file in case you are not able to perform the build
properly or want to change libopenmpt's library or header file directories.
These are the lines that you should look for and change:


```
# libopenmpt-specific variables
# The windows version is based on the official libopenmpt distribution
# The linux version is based on direct compilation of the library.
if platform == "windows":
	libopenmpt_path = "deps/libopenmpt"
	libopenmpt_headers_path = libopenmpt_path + "/inc"
	libopenmpt_lib_path = libopenmpt_path + "/lib"
else:
	libopenmpt_path = "deps/libopenmpt/install-files"
	libopenmpt_headers_path = libopenmpt_path + "/include"
	libopenmpt_lib_path = libopenmpt_path + "/lib"

```

## Post-Installation

You may want to set the platform-specific paths for the compiled library in
`project/gdextension/openmpt.gdextension`. `.gdextension` files cannot be edited
using the godot editor yet at the time of writing, so this must be edited using
a text editor.

## Usage

See the `project` directory for an example Godot Project. The GDExtension class
implements the same interface as `openmpt::module` and will be available as
a class called `OpenMPT` in the editor, which derives from `Node`.

Add an `OpenMPT` node to a desired scene. Then the `module_file_path` property
of `OpenMPT` must be set to a valid module file.

```
var module = $path/to/module/node
module.module_file_path = "song.it"
```

This will immediately load the module file into the program.

An `AudioStreamPlayer` node must be created with the `stream` field/variable
set to an `AudioStreamGenerator`. Then the `buffer_length` field of the
`AudioStreamGenerator` must be set to a small value (like `0.1`) so that the
song and our current song position information do not go out of sync.

Then the `AudioStreamPlayer` must be passed to `OpenMPT` like so:

```
audio_stream_player.play()
module.set_audio_generator_playback(audio_stream_player))
```

This will now allow the `audio_stream_player` to play the audio being rendered
by the library.

Such assignment can be done in Godot 4 only after the
`AudioStreamPlayer` has already started playing. The playback object
`AudioStreamGeneratorPlayback` which is required by `OpenMPT` is invalidated
after the `AudioStreamPlayer` is stopped, so this must be done everytime the
module needs to be played after pausing or stopping.

To actually start the rendering, call the start() function of the module:

```
module.start()
```

The module file will immediately start playing once this is run.

Note: If the buffer of the `AudioStreamGenerator` runs out, the
`AudioStreamPlayer` will stop playing and `AudioStreamGeneratorPlayback` will
be invalidated.

## Bugs and Issues

The current library registers a `Node` Class when the class could derive from
`Reference` instead. This is a holdover from the earlier Godot 3 GDNative
version of the library. The API could be improved more now from the features
from GDExtension.

The user cannot directly pass an `AudioStreamGeneratorPlayback` which may
be inconvenient. Another function for this may be made in the future.

Currently `set_audio_generator_playback` only supports an `AudioStreamPlayer`
as an argument. This could be easily resolved by declaring more functions to
accept the other `AudioStreamPlayer` types. The function body should remain
the same, except for the type.

If you find any more bugs and issues with the library, feel free to report them
in the [Github Issues][github-issues] section.


## Licensing

This project is licensed under the MIT/X11 license. See `LICENSE` for details.


[godot-cpp]: https://github.com/godotengine/godot-cpp
[article]: https://visphort.net/articles/3
[libopenmpt]: https://lib.openmpt.org/
[libopenmpt-dl]: https://lib.openmpt.org/libopenmpt/download/
[github-issues]: https://github.com/visphort/libopenmpt-godot/issues
