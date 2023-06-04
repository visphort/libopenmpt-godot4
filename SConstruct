#!/usr/bin/env python3

import os
import scons_compiledb

env = SConscript("godot-cpp/SConstruct")

opts = Variables([], ARGUMENTS)

# Define the relative path to the Godot headers.
godot_headers_path = "godot-cpp/godot-headers"
godot_bindings_path = "godot-cpp"

extension_api_builder = Builder(action = "godot --dump-extension-api $TARGET")

# Gets the standard flags CC, CCX, etc.
#env = DefaultEnvironment(BUILDERS = { "ExtensionApiBuilder": extension_api_builder })
scons_compiledb.enable_with_cmdline(env)

# Define our options. Use future-proofed names for platforms.
platform_array = ["", "linux", "macos", "windows", "android", "ios", "javascript"]
opts.Add(PathVariable("godot_path", "Command/Path to a Godot 4.0.x executable", "godot", PathVariable.PathAccept))
opts.Add(PathVariable("target_name", "The library name.", "libopenmptgodot", PathVariable.PathAccept))
opts.Add(EnumVariable("platform", "Compilation platform", "", platform_array))
opts.Add(EnumVariable("p", "Alias for 'platform'", "", platform_array))
opts.Add(PathVariable("target_path", "The path where the lib will be installed.", "project/gdextension/libopenmpt"))
opts.Add(PathVariable("godot_path", "Command/Path to a Godot 4.0.x executable", "godot", PathVariable.PathAccept))


# Updates the environment with the option variables.
opts.Update(env)

platform = env["platform"]

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

# Check our platform specifics.
if platform == "osx":
	if not env["use_llvm"]:
		env["use_llvm"] = "yes"
	if env["target"] in ("debug", "d"):
		env.Append(CCFLAGS=["-g", "-O2", "-arch", "x86_64", "-std=c++17", "-static"])
		env.Append(LINKFLAGS=["-arch", "x86_64", "-fPIC"])
	else:
		env.Append(CCFLAGS=["-g", "-O3", "-arch", "x86_64", "-std=c++17", "-static"])
		env.Append(LINKFLAGS=["-arch", "x86_64", "-fPIC"])
elif platform == "linux":
	if env["target"] in ("debug", "d"):
		env.Append(CCFLAGS=["-fPIC", "-g3", "-Og", "-std=c++17", "-static"])
		env.Append(LINKFLAGS=["-fPIC"])
	else:
		env.Append(CCFLAGS=["-fPIC", "-g", "-O3", "-std=c++17", "-static"])
		env.Append(LINKFLAGS=["-fPIC"])
elif platform == "windows":

	## Warning: This is old code. Not sure if this works now or not.

	# This makes sure to keep the session environment variables
	# on Windows, so that you can run scons in a VS 2017 prompt
	# and it will find all the required tools.
	env = Environment(ENV=os.environ)
	opts.Update(env)

	env.Append(CCFLAGS=["-DWIN32", "-D_WIN32", "-D_WINDOWS", "-W3", "-GR", "-D_CRT_SECURE_NO_WARNINGS"])
	if env["target"] in ("debug", "d"):
		env.Append(CCFLAGS=["-EHsc", "-D_DEBUG", "-MDd"])
	else:
		env.Append(CCFLAGS=["-O2", "-EHsc", "-DNDEBUG", "-MD"])

if env["use_llvm"] == "yes":
	env["CC"] = "clang"
	env["CXX"] = "clang++"

def add_sources(sources, dir):
	for f in os.listdir(dir):
		if f.endswith(".cpp"):
			sources.append(dir + "/" + f)

env.Append(
	CPPPATH=[
		libopenmpt_path + "/include",
		godot_bindings_path + "/gdextension",
		godot_bindings_path + "/include",
		godot_bindings_path + "/include/godot_cpp/classes",
		godot_bindings_path + "/include/godot_cpp/core",
		godot_bindings_path + "/include/godot_cpp/variant",
		godot_bindings_path + "/gen/include",
		godot_bindings_path + "/gen/include/godot_cpp/classes",
		godot_bindings_path + "/gen/include/godot_cpp/core",
		godot_bindings_path + "/gen/include/godot_cpp/variant",
	]
)

env.Append(
	LIBPATH=[
		godot_bindings_path + "/bin/",
		libopenmpt_path + "/lib"
	]
)

# TODO make the pathing dynamic here
env.Append(
	LIBS=[
		env.File(os.path.join("godot-cpp/bin", "libgodot-cpp.%s.%s.%s%s" %
				 (platform, env["target"], env["arch"], env["LIBSUFFIX"]))),
		"openmpt", "z", "mpg123", "vorbisfile", "vorbis", "m", "ogg"
	]
)

sources = []
add_sources(sources, "src")

if env["platform"] == "macos":
	library = env.SharedLibrary(
		os.path.join(env["target_path"], "%s.%s.%s.framework/%s.%s.%s" %
			(env["target_name"], env["platform"], env["target"], env["target_name"], env["platform"], env["target"])),
		source=sources,
	)
else:
	library = env.SharedLibrary(target=
		os.path.join(env["target_path"], "%s.%s.%s.%s%s" %
					 (env["target_name"], platform, env["target"], env["arch"], env["SHLIBSUFFIX"])), source=sources)


# if env["platform"] == "macos":
# 	library = env.SharedLibrary(
# 		os.path.join(env["target_path"], "%s.%s.%s.framework/libgdexample.%s.%s" %
# 			(env["platform"], env["target"], env["platform"], env["target"])),
# 		source=sources,
# 	)
# else:
# 	library = env.SharedLibrary(target=
# 		os.path.join(env["target_path"], "libgodot-cpp.%s.%s.%s%s" %
# 					 (platform, env["target"], env["arch"], env["SHLIBSUFFIX"])), source=sources)


Default(library)
