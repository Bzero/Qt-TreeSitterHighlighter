# Qt-TreeSitterHighlighter

Qt-TreeSitterHighlighter is a syntax-highlighter for Qt that can be used on QTextEdit, QPlainTextEdit or any other QTextDocument and rehighlights it on every change of the document.
It makes use of [tree-sitter](https://tree-sitter.github.io/tree-sitter/), an incremental parsing library to build a syntax tree of the document source code for accurate and fast highlighting.

## Usage
To use Qt-TreeSitterHighlighter, three configuration elements have to be provided:

* A tree-sitter [language](https://tree-sitter.github.io/tree-sitter/using-parsers#the-basic-objects) to parse the source and construct a syntax tree. Most often one of the [readily available parsers](https://github.com/tree-sitter/tree-sitter/wiki/List-of-parsers) can be used.
* A tree-sitter highlighting [query string](https://tree-sitter.github.io/tree-sitter/using-parsers#query-syntax) to extract the relevant captures from the syntax tree. Most available parsers also come with a highlight query.
* A highlighting format map to map capture names to their desired [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) format.

An example on how to use Qt-TreeSitterHighlighter can be found in [./examples/main.cpp](./examples/main.cpp).

## Requirements
* C++11
* Qt6
* [tree-sitter](https://github.com/tree-sitter/tree-sitter)

To build the example the [tree-sitter-cpp](https://github.com/tree-sitter/tree-sitter-cpp) parser is needed as well.

## Building

```
cd Qt-TreeSitterHighlighter
cmake . -B build
cmake --build build
```

Run the example to see see Qt-TreeSitterHighlighter in action:
```
./build/tree-sitter-highlighter-example
```

## Contributing
Contributions are very welcome!
