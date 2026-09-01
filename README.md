# AmigaGuide on Linux

A native Linux reader for classic AmigaGuide (`.guide`) documents.

**Read the guides. Keep the Amiga spirit. Run it natively on Linux.**

Built with **C++**, **Qt 6**, and CMake, this project brings the AmigaGuide document format to a modern Linux desktop without requiring AmigaOS, AROS, UAE, or an Amiga emulator.

> **Status: v1.0 development milestone**
>
> Real-world AmigaGuide documents have now been successfully tested, including formatting such as centered/right-aligned text and indentation.

## What is AmigaGuide?

AmigaGuide was Amiga's hypertext documentation format: lightweight text files containing nodes, navigation, formatting commands, and links. It was widely used for software documentation, programming references, utilities, and the enormous collection of Amiga material distributed through Aminet.

This project aims to make that library of documentation useful again on Linux.

## Features

The current reader supports a growing set of the AmigaGuide format, including:

- 📖 Native Qt 6 document viewer
- 🧭 Node-based navigation
- 🔗 Clickable `LINK` attributes between nodes
- ↩️ `NEXT` / `PREV` navigation
- 🔖 Node anchors and navigation targets
- **Bold**, *italic*, and <u>underline</u> text attributes
- Foreground and background colors
- Left, centered, and right justification
- Indented text with `LINDENT`
- Paragraph, line-break, and tab handling
- Scrolling and normal desktop text viewing
- Safe handling of unsupported action links
- No execution of AmigaDOS or ARexx commands from documents

The renderer is designed around the actual behavior of AmigaGuide rather than treating `.guide` files as ordinary plain text.

## Why this exists

There is a huge amount of Amiga documentation sitting in old archives and collections. Much of it is still useful—especially programming documentation, technical references, software manuals, and historical material—but opening it on a modern Linux desktop should not require firing up an entire emulated Amiga just to read a document.

**AmigaGuide on Linux** is an attempt to solve that problem with a small, native application.

## Architecture

```text
                 AmigaGuide .guide file
                           |
                           v
                 +---------------------+
                 |  AmigaGuide Parser  |
                 | nodes / commands    |
                 | attributes / links  |
                 +----------+----------+
                            |
                            v
                 +---------------------+
                 |   Document Model    |
                 +----------+----------+
                            |
                            v
                 +---------------------+
                 |    Qt 6 Renderer    |
                 | formatting / links  |
                 | navigation / history|
                 +---------------------+
                            |
                            v
                     Native Linux GUI
```

The project uses the AROS `amigaguide.datatype` implementation as a format and behavior reference, while implementing the presentation layer natively for Qt. It does **not** attempt to port the AROS Intuition/RastPort datatype framework into the Linux application.

## Current status

The first rich-rendering milestone is complete.

The reader can parse and display real AmigaGuide documents and has been tested against an Aminet AmigaGuide document as well as the project's dedicated formatting test file.

The current implementation is deliberately conservative: unsupported or potentially dangerous action commands are displayed/ignored rather than executed.

This is **not yet a claim of complete AmigaGuide compatibility**. The goal is to expand compatibility through testing against real-world AmigaGuide files and by comparing behavior with the original/AROS implementations.

## Test document

A deliberately comprehensive test document is included at:

```text
tests/data/formatting-test.guide
```

It exercises formatting, colors, links, node navigation, justification, indentation, and other common AmigaGuide attributes.

Real AmigaGuide files are equally important to the test suite. Strange syntax and historical behavior are exactly the sort of things this project needs to uncover.

## Build

The intended build system is **CMake + Ninja + Qt 6**.

On Debian/Ubuntu-style systems:

```sh
sudo apt install cmake ninja-build g++ qt6-base-dev
```

Then:

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the reader with:

```sh
./build/amigaguide-reader
```

The GitHub Actions workflow performs the same configure, build, and test sequence on Ubuntu.

## AROS reference

The primary implementation reference is the AROS `workbench/classes/datatypes/amigaguide/` source tree. Relevant source includes:

- `file.c`
- `util.c`
- `classdata.h`
- `nodeclass.c`
- `amigaguideclass.c`
- `navigator.c`

Additional format references include the AmigaGuide datatype documentation and historical AmigaGuide references.

AROS is distributed under the AROS Public License. If AROS source is copied or adapted, its applicable copyright and license notices will be preserved. Where practical, this project implements equivalent behavior independently rather than copying the AROS datatype framework.

## Roadmap

The long-term goal is a capable, lightweight AmigaGuide reader that can handle the real-world files found in old Amiga software collections.

Planned work includes:

- More complete AmigaGuide command and attribute coverage
- Better compatibility with unusual historical documents
- Cross-document links
- More complete navigation targets such as `MAIN`, `CONTENTS`, `INDEX`, and `HELP`
- Additional real-world compatibility tests
- Improved document search
- Packaging for common Linux distributions
- Continued renderer and parser cleanup

## Philosophy

Keep it native. Keep it small. Preserve the format rather than requiring an emulator to preserve the experience.

AmigaGuide was designed to make technical information easy to navigate. There is no reason that information has to stay locked inside an emulated Amiga.

## License

See the repository for the project's license information and the applicable AROS licensing information for referenced material.
