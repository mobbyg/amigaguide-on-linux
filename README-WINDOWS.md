# Windows build

The Windows target uses **Qt 6 + MinGW-w64** and the same CMake source tree as the Linux build.

Qt officially supports 64-bit Windows with MinGW-w64. The GitHub Actions workflow builds and tests the reader on Windows and uses Qt's `windeployqt` tool to assemble the runtime dependencies into a portable application directory.

The intended package is simply:

```text
amigaguide-reader.exe
        +
   Qt/runtime DLLs
        +
  Windows platform plugin
        =
  portable reader
```

The Windows build is currently a CI/development target. Release packaging will follow after the build has been exercised on real Windows hardware.
