# jg

General C++ utilities that make me happy

## CMake configuration

### Windows

For example

    jg> mkdir samples\build
    jg> cd samples\build
    jg\samples\build> cmake .. -G "Visual Studio 15 2017 Win64"

Output similar to

    -- Selecting Windows SDK version 10.0.18362.0 to target Windows 10.0.18363.
    -- The C compiler identification is MSVC 19.16.27035.0
    -- The CXX compiler identification is MSVC 19.16.27035.0
    -- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe
    -- Check for working C compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe -- works
    -- Detecting C compiler ABI info
    -- Detecting C compiler ABI info - done
    -- Detecting C compile features
    -- Detecting C compile features - done
    -- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe
    -- Check for working CXX compiler: C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/VC/Tools/MSVC/14.16.27023/bin/Hostx86/x64/cl.exe -- works
    -- Detecting CXX compiler ABI info
    -- Detecting CXX compiler ABI info - done
    -- Detecting CXX compile features
    -- Detecting CXX compile features - done
    -- Configuring done
    -- Generating done
    -- Build files have been written to: C:/source/jg/samples/build

Build debug (the default)

    jg\samples\build> cmake --build . --target jg_stacktrace -- /verbosity:minimal
    Microsoft (R) Build Engine version 15.9.21+g9802d43bc3 for .NET Framework
    Copyright (C) Microsoft Corporation. All rights reserved.
    
      jg_stacktrace.cpp
      jg_stacktrace.vcxproj -> C:\source\jg\samples\build\Debug\jg_stacktrace.exe

Arguments after `--` are passed to MSBuild (since the generator is Visual Studio)

Build release

    jg\samples\build> cmake --build . --target jg_stacktrace --config Release -- /verbosity:minimal
    Microsoft (R) Build Engine version 15.9.21+g9802d43bc3 for .NET Framework
    Copyright (C) Microsoft Corporation. All rights reserved.
    
      jg_stacktrace.cpp
      jg_stacktrace.vcxproj -> C:\source\jg\samples\build\Release\jg_stacktrace.exe

Build release with debug info

    jg\samples\build> cmake --build . --target jg_stacktrace --config RelWithDebInfo -- /verbosity:minimal
    Microsoft (R) Build Engine version 15.9.21+g9802d43bc3 for .NET Framework
    Copyright (C) Microsoft Corporation. All rights reserved.
    
      jg_stacktrace.cpp
      jg_stacktrace.vcxproj -> C:\source\jg\samples\build\RelWithDebInfo\jg_stacktrace.exe