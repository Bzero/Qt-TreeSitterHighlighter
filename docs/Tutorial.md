# Tutorial

> The completed example code for this tutorial can be found in the [git repository](https://github.com/Bzero/Qt-TreeSitterHighlighter/blob/main/examples/main.cpp).

To use TreeSitterHighlighter, you need to provide three pieces of information:

* The language you would like to parse in form of a tree-sitter language.
* A highlight query which extracts the nodes to highlight from the syntax tree of your source file.
* A format map that maps highlight query capture names to their highlighting format.

Let's start with the language. In most cases one will be able to make use of one of the [available tree-sitter parsers](https://github.com/tree-sitter/tree-sitter/wiki/List-of-parsers). For this tutorial, let's use [tree-sitter-cpp](https://github.com/tree-sitter/tree-sitter-cpp). To make use of it in our example we declare the `tree_sitter_cpp` function it provides and to which we will link later:

```c++
--8<-- "examples/main.cpp:8:11"
```

Next up is the highlighting query. Many parsers come with a query in their repository (usually under `queries/highlights.scm`) which provides a good starting point. For this tutorial we will use a custom (minimal) query:

```c++
--8<-- "examples/main.cpp:13:47"
```

Most statements in this query simply match a syntax tree node and assign it a capture name, e.g:  `(tree_node) @capture_name`.
For information on how to write a highlighting query, please refer to the [tree-sitter documentation](https://tree-sitter.github.io/tree-sitter/using-parsers/queries/1-syntax.html).

Finally, we need to specify how these captures are highlighted.
For this, we define a map from (some of) the `capture_names` above to a [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html) with which the capture should be highlighted.
In this example we again use a minimal map:
```c++
--8<-- "examples/main.cpp:49:67"
```


To use TreeSitterHighlighter we first create a QPlainTextEdit instance to which it will be connected:

```c++
--8<-- "examples/main.cpp:74:74"
```

Now we create a TreeSitterHighlighter instance using the three ingredients prepared above and connect it do the editor's document:

```c++
--8<-- "examples/main.cpp:77:77"
```

That's it, TreeSitterHighlighter will now automatically rehighlight every time the editor's text changes.

---

The complete [example](https://github.com/Bzero/Qt-TreeSitterHighlighter/blob/main/examples/main.cpp) can be obtained, built and run as follows:

```
git clone https://github.com/Bzero/Qt-TreeSitterHighlighter --recurse-submodules
cd Qt-TreeSitterHighlighter
cmake . -B build
cmake --build build
./build/tree-sitter-highlighter-example
```
