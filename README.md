# AmigaGuide on Linux

A standalone native Linux reader for AmigaGuide (`.guide`) documents, using Qt 6 for the user interface.

## Goal

Open and navigate classic AmigaGuide files on Linux without requiring AROS, AmigaOS, or an emulator.

## Architecture

```text
AmigaGuide file
      |
      v
+--------------------+
| AmigaGuide parser  |
| nodes / commands   |
| attributes / links |
+---------+----------+
          |
          v
+--------------------+
| Native document    |
| model              |
+---------+----------+
          |
          v
+--------------------+
| Qt 6 renderer      |
| navigation/history |
| links / formatting |
+--------------------+
```

The Linux project uses the AROS `amigaguide.datatype` as a format/behavior reference and reimplements the presentation layer natively. We do not port the AROS Intuition/RastPort datatype framework into the Qt application.

## Current status

The first rich-rendering milestone is now implemented. The Qt reader renders node bodies and supports a useful initial set of AmigaGuide text attributes:

- bold / italic / underline
- foreground and background colors for common named colors
- left / centered / right alignment
- indentation
- paragraph, line and tab handling
- clickable `LINK` attributes between nodes
- node anchors and `NEXT` / `PREV` navigation links
- safe handling of unsupported action links (they are not executed)

A deliberately comprehensive test document is included at `tests/data/formatting-test.guide`. It is intended to be opened directly in the application while developing the renderer.

This is intentionally not yet a claim of complete AmigaGuide compatibility. The next work will expand the parser/renderer against real AmigaGuide documents and the AROS implementation.

## Initial targets

- Parse `.guide` files and identify global metadata and nodes.
- Resolve node links and cross-document links.
- Support AmigaGuide navigation targets such as `MAIN`, `CONTENTS`, `INDEX`, `HELP`, `NEXT`, and `PREV`.
- Support common text attributes: bold, italic, underline, foreground/background colors, justification, indentation, tabs, and wrapping.
- Provide a native Qt 6 reader with open-file support, scrolling, clickable links, back/forward history, and node navigation.
- Build compatibility tests from real AmigaGuide documents.

## AROS reference

The primary reference is the AROS `workbench/classes/datatypes/amigaguide/` implementation. Relevant source includes `file.c`, `util.c`, `classdata.h`, `nodeclass.c`, `amigaguideclass.c`, and `navigator.c`.

AROS is distributed under the AROS Public License. If AROS source is copied or adapted, its applicable copyright and license notices will be preserved. Where practical, this project will implement equivalent behavior independently rather than copying the AROS datatype framework.

## Build

The intended build system is CMake and Qt 6. On Debian/Ubuntu-style Linux systems, install `cmake`, `ninja-build`, `g++`, and `qt6-base-dev`, then:

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

The GitHub Actions workflow performs the same build and test sequence on Ubuntu.
