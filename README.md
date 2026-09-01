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

The intended build system is CMake and Qt 6. The first milestone is a small native parser/library and a minimal Qt viewer.

## Status

Early development.
