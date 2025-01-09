# Overview

Qt-TreeSitterHighlighter is a [tree-sitter](https://tree-sitter.github.io/tree-sitter/) based syntax highlighter library for Qt.
It can be used with any [QTextDocument](https://doc.qt.io/qt-6/qtextdocument.html), in particular with [QTextEdit](https://doc.qt.io/qt-6/qtextedit.html) and [QPlainTextEdit](https://doc.qt.io/qt-6/qplaintextedit.html), and rehighlights the source code immediately whenever it changes, e.g. when typing.

To use Qt-TreeSitterHighlighter, it must be provided with:

* A tree-sitter [language](https://tree-sitter.github.io/tree-sitter/using-parsers/1-getting-started.html#the-basic-objects) to parse the source and construct a syntax tree.
* A tree-sitter highlighting [query string](https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html) which extracts the relevant captures from the syntax tree.
* A highlighting format map which maps capture names to their desired [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) format.

The highlighter performs the following steps:

1. The QTextDocuments text is parsed to create a syntax tree.
2. The highlighting query is applied to the syntax tree to extract highlighting captures.
3. The text of each capture is highlighted according to its format specified in the format map.

When the QTextDocuments text is updated, e.g. by typing, the steps above are applied incrementally.
