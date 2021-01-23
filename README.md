# jg

General C++ utilities that make me happy

## Building with CMake

### Using the Visual Studio generator

The **Visual Studio** CMake generator (which uses MSBuild under the hood) is called a multi-configuration generator. This means that the same CMake configuration can be used for both _Debug_, _Release_, and _RelWithDebInfo_ builds.

Create the configuration:

    > mkdir build\windows
    > cd build\windows
    > cmake ..\.. -G "Visual Studio 15 2017 Win64"

or

    > cmake ..\.. -G "Visual Studio 16 2019" -A Win64

Build all targets in Debug (the default):

    > cd build\windows
    > cmake --build . -- /verbosity:minimal

Build all targets in Release:

    > cd build\windows
    > cmake --build . --config Release -- /verbosity:minimal

Build one target in Debug:

    > cd build\windows
    > cmake --build . --target jg_stacktrace -- /verbosity:minimal

Build one target in Release:

    > cd build\windows
    > cmake --build . --target jg_stacktrace --config Release -- /verbosity:minimal

Arguments after `--` are passed to the generator (specifically to MSBuild when the generator is Visual Studio).

To build Release with debug info, just change _Release_ to _RelWithDebInfo_.

### Using the Ninja generator

The **Ninja** CMake generator is called a single-configuration generator. This means that separate CMake configurations are used for each of _Debug_, _Release_, and _RelWithDebInfo_ builds.

Create a Debug configuration:

    > mkdir build\windows\debug
    > cd build\windows\debug
    > cmake ..\..\.. -DCMAKE_BUILD_TYPE=Debug -GNinja

Build all targets in Debug:

    > cd build\windows\debug
    > cmake --build .

Build one target in Debug:

    > cd build\windows\debug
    > cmake --build . --target jg_stacktrace