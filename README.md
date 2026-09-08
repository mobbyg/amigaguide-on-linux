# AmigaGuide on Linux

A native reader and editor for classic AmigaGuide (`.guide`) documents.

**Read the guides. Keep the Amiga spirit. Run it natively on Linux and Windows.**

Built with **C++**, **Qt 6**, and CMake, this project brings the AmigaGuide document format to modern desktops without requiring AmigaOS, AROS, UAE, or an Amiga emulator.

> **Status: v0.9-rc1 — release candidate**
>
> The first release candidate is available for real-world testing on Linux and Windows. It represents a major step from the original reader prototype toward a practical AmigaGuide reader and editor.

## Download

Prebuilt portable packages are available from the [v0.9-rc1 release](https://github.com/mobbyg/amigaguide-on-linux/releases/tag/v0.9-rc1).

- [Linux x64](https://github.com/mobbyg/amigaguide-on-linux/releases/download/v0.9-rc1/amigaguide-reader-linux-x64.tar.gz) — `amigaguide-reader-linux-x64.tar.gz`
- [Windows x64](https://github.com/mobbyg/amigaguide-on-linux/releases/download/v0.9-rc1/amigaguide-reader-windows-x64.zip) — `amigaguide-reader-windows-x64.zip`

The release candidate is intended for testing and stabilization. It is **not** yet a claim of complete AmigaGuide compatibility.

## What is AmigaGuide?

AmigaGuide was Amiga's hypertext documentation format: lightweight text files containing nodes, navigation, formatting commands, and links. It was widely used for software documentation, programming references, utilities, and the enormous collection of Amiga material distributed through Aminet.

This project aims to make that library of documentation useful again on modern systems.

## Features

### Reader

The native Qt 6 reader currently supports a growing set of the AmigaGuide format, including:

- 📖 Native Qt 6 document viewer
- 🧭 Node-based navigation
- 🏠 Home/TOC navigation
- 🔎 Search with next-match and wrapping support
- ↩️ Back/Forward navigation history
- 🔗 Local cross-document navigation
- `LINK` attributes in canonical and legacy pipe-delimited syntax
- 🔖 Node anchors and navigation targets
- **Bold**, *italic*, and underline text attributes
- Foreground and background colors
- Left, centered, and right justification
- Indented text with `LINDENT`
- Paragraph, line-break, and tab handling
- UTF-8-safe document and node handling
- Safe handling of unsupported action links
- No execution of AmigaDOS or ARexx commands from documents

The renderer is designed around the actual behavior of AmigaGuide rather than treating `.guide` files as ordinary plain text.

### Editor

The project now includes a source-oriented AmigaGuide editor with:

- Open, Save, and Save As
- Create new AmigaGuide documents
- Raw source editing
- Undo/Redo
- Node list and node selection
- Create, rename, and delete nodes
- Node title editing
- Node properties and metadata editing
- Document properties and metadata editing
- Word wrap and Smart Wrap controls
- Link inspection
- Source-aware document validation

The editor deliberately remains **source-oriented rather than WYSIWYG**. The original AmigaGuide source remains authoritative, and edits are designed to preserve existing structure and unrelated content wherever practical.

## Compatibility and safety

Compatibility work includes a number of real-world AmigaGuide conventions, including:

- Legacy pipe-delimited `LINK` syntax
- Document and node metadata
- `@WORDWRAP`
- `@SMARTWRAP`
- `@PROPORTIONAL`
- Node-level fonts and tab settings
- UTF-8 document content
- Structural and malformed-document detection
- Unsupported or unknown commands reported without being executed

The application does not execute AmigaDOS, ARexx, or other external commands embedded in documents.

This is **not yet a claim of complete AmigaGuide compatibility**. Real-world `.guide` files are an important part of ongoing compatibility testing.

## Why this exists

There is a huge amount of Amiga documentation sitting in old archives and collections. Much of it is still useful—especially programming documentation, technical references, software manuals, and historical material—but opening it on a modern desktop should not require firing up an entire emulated Amiga just to read a document.

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
                 | editing / validation|
                 +----------+----------+
                            |
                            v
                 +---------------------+
                 | Navigation / Dest.  |
                 | history / links     |
                 +----------+----------+
                            |
                            v
                 +---------------------+
                 | Application Behavior |
                 +----------+----------+
                            |
                            v
                 +---------------------+
                 | Qt 6 Presentation   |
                 | reader / editor UI   |
                 +---------------------+
```

The core AmigaGuide parsing, document, navigation, destination, validation, and editing logic is separated from the Qt presentation layer where practical. The editor UI is further separated so that application/document behavior does not have to own the presentation details.

The project uses the AROS `amigaguide.datatype` implementation as a format and behavior reference, while implementing the presentation layer natively for Qt. It does **not** attempt to port the AROS Intuition/RastPort datatype framework into the Linux application.

## Current status

**v0.9-rc1 is the first release candidate.**

The project has progressed from a reader prototype to a practical reader/editor with automated regression coverage for the parser, document model, navigation, links, validation, and editor functionality. Linux and Windows release builds are produced automatically through GitHub Actions.

The current release is deliberately conservative. Unsupported or potentially dangerous action commands are displayed/ignored rather than executed, and areas outside the current compatibility scope remain explicit future work.

The goal of the release-candidate period is to find compatibility problems and usability issues before the v0.9 final release. Real AmigaGuide documents from Amiga, AROS, Aminet, and other collections are especially valuable for testing.

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

The GitHub Actions workflows perform configure, build, test, and release packaging on supported Linux and Windows environments.

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

The long-term goal is a capable, lightweight AmigaGuide reader and editor that can handle the real-world files found in old Amiga software collections.

Planned work includes:

- More complete AmigaGuide command and attribute coverage
- Better compatibility with unusual historical documents
- More complete support for navigation targets such as `MAIN`, `CONTENTS`, `INDEX`, and `HELP`
- Additional real-world compatibility tests
- HTTP/HTTPS document loading
- `ag://` library references
- AmigaGuide library/index/crawler functionality
- Advanced link rewriting
- WYSIWYG editing, if it can be added without compromising source fidelity
- Packaging/install polish for common Linux distributions
- Continued parser, renderer, and UI cleanup

## Philosophy

Keep it native. Keep it small. Preserve the format rather than requiring an emulator to preserve the experience.

AmigaGuide was designed to make technical information easy to navigate. There is no reason that information has to stay locked inside an emulated Amiga.

## License

See the repository for the project's license information and the applicable AROS licensing information for referenced material.
